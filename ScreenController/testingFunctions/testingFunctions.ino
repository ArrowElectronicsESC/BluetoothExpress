
#define ARDUINO_UNO
#include "Arduino.h"
#include <ArrowBGXShield.h>

#include <BluetoothExpress.h>
#include <Wire.h>


#if defined(ARDUINO_AVR_UNO)
  #include "SoftwareSerial.h"
  SoftwareSerial bgxSerial(4,5);
  BGX13 bgx(&bgxSerial);
#else
  //HardwareSerial * bgxSerial = (HardwareSerial*)&Serial1;
  BGX13 bgx( &Serial1 );
#endif
PCA9633 dimmer(0x60);
SI7050 temp(0x40);
VEML6030 light(0x10);

char print_buffer[500];

uint8_t swell[] = {0,1,5,20,255,20,5,1,0};
uint8_t ind = 0;
int LED_Indicator = 0;



void setup() {
  // put your setup code here, to run once:  
  pinMode(BUS_MODE_PIN,OUTPUT);
  Serial.println("Hello World!");
  Wire.begin();
  Serial.begin(115200);
  while (!Serial) {
      ;  //wait for serial port to connect. Needed for native USB port only
  }  
  Serial.println("Serial CONNECTED");
  bgx.serialConnect(115200);
  Serial.println("BGX CONNECTED");
  dimmer.init();  
  dimmer.outputMode();
  Serial.println("0-10V Controller CONNECTED");
  light.defaultConfig();
  Serial.println("LUX Sensor CONNECTED");

  delay(1000); 


}

void loop() {
  // put your main code here, to run repeatedly:
  //Command to test:

    temp.startTempMeasure();
    delay(400);
    float ambient = light.getWhite();
    ambient = light.getALS();
    float ctemp = temp.getTempMeasureC();
    snprintf(print_buffer, 100, "Temperature in Celsius: %d.%02dC", int(ctemp),int(ctemp *100)%100);
    //bgx.sendString(print_buffer);
    snprintf(print_buffer, 100, "Light in Lumens: %d.%02dC", int(ambient),int(ambient *100)%100);
    //bgx.sendString(print_buffer);
    //dimmer.rgbwControl( swell[(ind + 1) % 13]/2,swell[(ind + 0) % 13],swell[(ind + 2) % 13],swell[(ind + 3) % 13]/4 );
    //ind = ind < 12 ? ind + 1 : 0;
    dimmer.rgbwControl(ind,255-ind,ind,255-ind);
    ind+=25;
    Serial.print(ind);
    Serial.print(" ");
    Serial.println("Light Val");
    //bgx.scan(1500);
    delay(200);
  
    

    //dimmer.rgbwControl(0,0,0,0 );
  
}
