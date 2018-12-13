
#define ARDUINO_UNO
#include "Arduino.h"
#include <ArrowBGXShield.h>

#include <BluetoothExpress.h>
#include <Wire.h>

#define OUTPUT_MODE 1  //Set to 1 to enaboe 0-10V output, 0 to use onboard lights
#define SHIELD_VERSION 0 //Update for version 1 shields to enable sensing
#define ARD_ID 0 //ID per arduino
#define SWELL 1

//Conditional for different arduino versions
#if defined(ARDUINO_AVR_UNO)
  #include "SoftwareSerial.h"
  SoftwareSerial bgxSerial(4,5);
  BGX13 bgx(&bgxSerial);
#else
  //HardwareSerial * bgxSerial = (HardwareSerial*)&Serial1;
  BGX13 bgx( &Serial1 );
#endif

//Object declaratyion
PCA9633 dimmer(0x60);
SI7050 temp(0x40);
VEML6030 lux(0x10);

uint8_t swell[] = {0,0,0,0,0,1,5,10,20,50,90,150,255,150,90,50,20,10,5,1,0,0,0,0}; //Used to test lights with a "chase effect" - Also used for the init section
uint8_t ind = 0; //Index to use for chase math

//This table adjusts the values recieved to create a linear light output - May need some fine tuning
byte _ledTable[256] = {
  0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
  1,   2,   2,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   4,   4,
  4,   4,   4,   5,   5,   5,   5,   6,   6,   6,   6,   7,   7,   7,   7,   8,
  8,   8,   9,   9,   9,  10,  10,  10,  11,  11,  12,  12,  12,  13,  13,  14,
14,  15,  15,  15,  16,  16,  17,  17,  18,  18,  19,  19,  20,  20,  21,  22,
22,  23,  23,  24,  25,  25,  26,  26,  27,  28,  28,  29,  30,  30,  31,  32,
33,  33,  34,  35,  36,  36,  37,  38,  39,  40,  40,  41,  42,  43,  44,  45,
46,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,
61,  62,  63,  64,  65,  67,  68,  69,  70,  71,  72,  73,  75,  76,  77,  78,
80,  81,  82,  83,  85,  86,  87,  89,  90,  91,  93,  94,  95,  97,  98,  99,
101, 102, 104, 105, 107, 108, 110, 111, 113, 114, 116, 117, 119, 121, 122, 124,
125, 127, 129, 130, 132, 134, 135, 137, 139, 141, 142, 144, 146, 148, 150, 151,
153, 155, 157, 159, 161, 163, 165, 166, 168, 170, 172, 174, 176, 178, 180, 182,
184, 186, 189, 191, 193, 195, 197, 199, 201, 204, 206, 208, 210, 212, 215, 217,
219, 221, 224, 226, 228, 231, 233, 235, 238, 240, 243, 245, 248, 250, 253, 255 };

void setup() {
  //Initialize Arduino Fucntions
  pinMode(BUS_MODE_PIN,OUTPUT);
  Wire.begin();
  Serial.begin(115200);
  while (!Serial) {
      ;  //wait for serial port to connect. Needed for native USB port only
  }  
  //Serial.println("Serial CONNECTED");

  //Initialize BGX
  bgx.serialConnect(115200);
  Serial.println("BGX CONNECTED");

  //Initialize Lighting Controls
  dimmer.init();  
  dimmer.rgbwControl(0,0,0,0); //Init to off or your eyes r ded.
  //only enable one lighting mode - Use output to drive the 0-10V controllers
  if(OUTPUT_MODE){
      dimmer.outputMode();
      Serial.println("0-10V Controller Ready");
  } else {
      dimmer.onBoardMode();
      Serial.println("On Board LEDs Ready");
  }
  if(SHIELD_VERSION > 0 ){
    lux.defaultConfig(); //Initialize Lux Sensor - Not currently available
    Serial.println("LUX Sensor Ready");
    Serial.println("Temp Sensor Ready"); //Temp is auto -Initialized
  }
  delay(100); 
 
  Serial.println("System Ready");

}

char incomingBytes[255] = {0};
char incomingBytesPtr = 0;
uint8_t colorVals[4] = {0,0,0,0};
char colorPtr = 0;

bool host_connected = false;
float ambient, ctemp;
char temp_buffer[7] = {'T','E','M','P',NULL,NULL,NULL};
char lux_buffer[10] = {'L','U','X',NULL,NULL,NULL,NULL,NULL,NULL,NULL};
void loop() {
    if( SHIELD_VERSION > 0) {
      temp.startTempMeasure();
      delay(100);
      ambient  = lux.getALS();
      ctemp = temp.getTempMeasureC();
      snprintf(temp_buffer, 7, "%d.%02d", int(ctemp),int(ctemp *100)%100);
      snprintf(lux_buffer, 10, "%d.%02d", int(ambient),int(ambient *100)%100);
    }
    while (Serial.available() > 0) {
      // read the incoming byte:
      incomingBytes[incomingBytesPtr++] = Serial.read();
    }
    if(incomingBytesPtr > 0 && !host_connected ) { //Host is now connected
      host_connected = true;
    } else if (!host_connected){ // No host no data
      //Swell
      if(SWELL){
        dimmer.rgbwControl( swell[(ind + 6) % 13]/2,swell[(ind + 0) % 13],swell[(ind + 12) % 13],swell[(ind + 18) % 13]/4 );
        //ind = ind < 24 ? ind + 1 : 0;
        ind++;
        delay(400);
      }
    } 
    if(host_connected &&incomingBytesPtr > 0) { //Host connected and data ready
      switch(incomingBytes[0]){
        case 0:
          break;
        case 'L'://Command to change lights
          if(incomingBytesPtr >= 3){
            lightChange(&incomingBytes[1]);
          }
          break;
        case 'T': //Command to get temp
          Serial.println(temp_buffer);//print temp
          break;
        case 'X': //Command to get lux
          Serial.println(lux_buffer); //getlux
          break;
        case 'A': //Command for all data to be read and sent
          Serial.println(temp_buffer);//print temp
          Serial.println(lux_buffer); //getlux
          break;
        case 'I':
          Serial.println(ARD_ID);
          break;
        case 'E':
          host_connected = false;
          Serial.println("ERR: DISCONNECTED");
          break;
        default: 
          Serial.println("ERR: COMMAND NOT UNDERSTOOD");
          break;
      }
    } else if (host_connected){
        dimmer.rgbwControl(colorVals[0],colorVals[1],colorVals[2],colorVals[3]);
    }
    incomingBytesPtr = 0;
    delay(100); /// Give the UART a little time  
}

int lightChange(char * command){
      for(int color = 0; color < 4; color++){
        colorVals[color] = _ledTable[command[color]];
        Serial.print(colorVals[0]);
        Serial.print(" ");
      }
      Serial.println();
      dimmer.rgbwControl(colorVals[0],colorVals[1],colorVals[2],colorVals[3]);
} 
