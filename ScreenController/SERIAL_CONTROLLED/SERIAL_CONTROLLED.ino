
#define ARDUINO_UNO
#include "Arduino.h"
#include <ArrowBGXShield.h>
#include <BluetoothExpress.h>

#include <Wire.h>

#define OUTPUT_MODE 0  //Set to 1 to enaboe 0-10V output, 0 to use onboard lights
#define SHIELD_VERSION 0 //Update for version 1 shields to enable sensing
#define ARD_ID 0 //ID per arduino
#define SWELL 1

#define SERIAL_INPUT_BUFFER_SIZE       255
#define GET_ARDUINO_ID_COMMAND_LENGTH    1
#define LIGHT_CHANGE_COMMAND_LENGTH      7
#define GET_ALL_SENSORS_COMMAND_LENGTH   1
#define GET_TEMPERATURE_COMMAND_LENGTH   1
#define GET_LUX_COMMAND_LENGTH           1

enum ErrorCode {
  SUCCESS = 0,
  BAD_COMMAND_LENGTH,
  WRONG_ARDUINO_ID,
  BAD_CHECKSUM
};

union LedColor {
  struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t white;
  };

  uint8_t all[4];
};

struct RgbaiColorCommand {
  unsigned char command_id;
  unsigned char arduino_id;
  LedColor color;
  unsigned char checksum;
};

int getSerialInput(unsigned char *buffer, int size, int timeout);
ErrorCode handleSerialCommand(unsigned char *buffer, int size);
void readSensors(float *temperature, float *ambient_light);
void updateSwell(uint8_t swell_index);
LedColor normalizeColor(LedColor color);
void setLightColor(LedColor color);

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
PCA9633 *dimmer = NULL;
SI7050 *temp = NULL;
VEML6030 *lux = NULL;

unsigned char serial_input_buffer[SERIAL_INPUT_BUFFER_SIZE] = {0};
bool host_connected = false;

uint8_t swell[] = {0,0,0,0,0,1,5,10,20,50,90,150,255,150,90,50,20,10,5,1,0,0,0,0}; //Used to test lights with a "chase effect" - Also used for the init section
uint8_t swell_index = 0; //Index to use for chase math

//This table adjusts the values recieved to create a linear light output - May need some fine tuning
byte _ledTable[256] = {
   0,    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
   1,    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
   1,    2,   2,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   4,   4,
   4,    4,   4,   5,   5,   5,   5,   6,   6,   6,   6,   7,   7,   7,   7,   8,
   8,    8,   9,   9,   9,  10,  10,  10,  11,  11,  12,  12,  12,  13,  13,  14,
  14,   15,  15,  15,  16,  16,  17,  17,  18,  18,  19,  19,  20,  20,  21,  22,
  22,   23,  23,  24,  25,  25,  26,  26,  27,  28,  28,  29,  30,  30,  31,  32,
  33,   33,  34,  35,  36,  36,  37,  38,  39,  40,  40,  41,  42,  43,  44,  45,
  46,   46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,
  61,   62,  63,  64,  65,  67,  68,  69,  70,  71,  72,  73,  75,  76,  77,  78,
  80,   81,  82,  83,  85,  86,  87,  89,  90,  91,  93,  94,  95,  97,  98,  99,
 101,  102, 104, 105, 107, 108, 110, 111, 113, 114, 116, 117, 119, 121, 122, 124,
 125,  127, 129, 130, 132, 134, 135, 137, 139, 141, 142, 144, 146, 148, 150, 151,
 153,  155, 157, 159, 161, 163, 165, 166, 168, 170, 172, 174, 176, 178, 180, 182,
 184,  186, 189, 191, 193, 195, 197, 199, 201, 204, 206, 208, 210, 212, 215, 217,
 219,  221, 224, 226, 228, 231, 233, 235, 238, 240, 243, 245, 248, 250, 253, 255 };

void setup() {
  //Initialize Arduino Fucntions
  Wire.begin();
  Serial.begin(115200);
  while (!Serial) {
      ;  //wait for serial port to connect. Needed for native USB port only
  }
  
  bgx.serialConnect(115200);


  //Initialize Lighting Controls
  dimmer = new PCA9633(0x60);
  dimmer->init();
  //dimmer->rgbwControl(255,255,255,255); //Init to off or your eyes r ded.
  //only enable one lighting mode - Use output to drive the 0-10V controllers
  if (OUTPUT_MODE) {
      dimmer->outputMode();
      //Serial.println("0-10V Controller Ready");
  } else {
      dimmer->onBoardMode();
      dimmer->rgbwControl(64,0,0,64);
      //Serial.println("On Board LEDs Ready");
  }

  temp = new SI7050(0x40);

  lux = new VEML6030(0x10);
  if (SHIELD_VERSION > 0 ) {
    lux->defaultConfig(); //Initialize Lux Sensor - Not currently available
    //Serial.println("LUX Sensor Ready");
    //Serial.println("Temp Sensor Ready"); //Temp is auto -Initialized
  }
  delay(100); 
 
  //Serial.println("System Ready");
}

void loop() {
  ErrorCode status = SUCCESS;
  int num_bytes_received = getSerialInput(serial_input_buffer,
    SERIAL_INPUT_BUFFER_SIZE, 100);

  if (num_bytes_received > 0 && !host_connected ) { // Host is now connected
    host_connected = true;
  }

  if (host_connected) {
    if (num_bytes_received) {
      status = handleSerialCommand(serial_input_buffer, num_bytes_received);
      //Serial.write(status);
    } else {
      //dimmer->rgbwControl(0, 0, 0, 0);
    }
  } else {
#if SWELL
    updateSwell(swell_index++);
    delay(400);
#endif
  }
}

int getSerialInput(unsigned char *buffer, int size, int timeout) {
  int num_bytes_received = 0;
  unsigned long start_time = millis();

  while(!Serial.available()) {
    // Wait for bytes to come in
    if (timeout && ((millis() - start_time) > timeout)) {
      return 0;
    }
  }

  while (Serial.available() && (num_bytes_received < size) ) {
    buffer[num_bytes_received] = Serial.read();
    num_bytes_received++;
    delay(1);
  }

  return num_bytes_received;
}

unsigned char getChecksum(unsigned char *buffer, int length) {
  unsigned char sum = 0;

  for (int byte_iter = 0; byte_iter < length; byte_iter++) {
    sum += buffer[byte_iter];
  }

  return sum;
}

ErrorCode handleSerialCommand(unsigned char *buffer, int size) {
  ErrorCode status = SUCCESS;

  unsigned char command_id = buffer[0];
  
  switch (command_id) {
    case '\0':
    break;
    
    // Change Light Color
    case 'L':
      if (size == LIGHT_CHANGE_COMMAND_LENGTH) {
        RgbaiColorCommand color_command = *((RgbaiColorCommand *)buffer);
        if (color_command.arduino_id == ARD_ID) {
          if (color_command.checksum == getChecksum(buffer, size - 1) || true) {
            setLightColor(color_command.color);
          } else {
            status = BAD_CHECKSUM;
          }
        } else {
          status = WRONG_ARDUINO_ID;
        }
      } else {
        status = BAD_COMMAND_LENGTH;
      }
      if(status) {
        //Serial.write(status);
      }
    break;
    
    // Get Temperature
    case 'T':
      if (size == GET_TEMPERATURE_COMMAND_LENGTH) {
        float temperature;
        readSensors(&temperature, NULL);
        Serial.write((byte*)&temperature, sizeof(temperature));
      } else {
        status = BAD_COMMAND_LENGTH;
      }
    break;
    
    // Get Lux
    case 'X':
      if (size == GET_LUX_COMMAND_LENGTH) {
        float lux;
        readSensors(NULL, &lux);
        Serial.write((byte*)&lux, sizeof(lux));
      } else {
        status = BAD_COMMAND_LENGTH;
      }
    break;
    
    // Get All Sensor Data
    case 'A':
      if (size == GET_ALL_SENSORS_COMMAND_LENGTH) {
        float temperature;
        float lux;
        readSensors(&temperature, &lux);
        Serial.write((byte*)&temperature, sizeof(temperature));
        Serial.write((byte*)&lux, sizeof(lux));
      } else {
        status = BAD_COMMAND_LENGTH;
      }
    break;
    
    // Get Arduino Id
    case 'I':
      if (size == GET_ARDUINO_ID_COMMAND_LENGTH) {
        Serial.write(ARD_ID);
      } else {
        Serial.write(0xFF);
        status = BAD_COMMAND_LENGTH;
      }
    break;
    
    case 'E':
      host_connected = false;
      Serial.write(0xFE);
      //Serial.println("ERR: DISCONNECTED");
    break;
    
    default:
      Serial.write(command_id);//0xFF);
      //Serial.println("ERR: COMMAND NOT UNDERSTOOD");
    break;
  }

  return status;
}

void readSensors(float *temperature, float *ambient_light) {
  if (temperature && temp) {
    temp->startTempMeasure();
    delay(100);
    *temperature = temp->getTempMeasureC();
  }

  if (ambient_light && lux) {
    *ambient_light = lux->getALS();
  }
}

void updateSwell(uint8_t swell_index) {
  dimmer->rgbwControl(swell[(swell_index + 6)  % 13] / 2,
                      swell[(swell_index + 0)  % 13],
                      swell[(swell_index + 12) % 13],
                      swell[(swell_index + 18) % 13] / 4);
}

LedColor normalizeColor(LedColor color) {
  LedColor normalized_color;

  for (int color_iter = 0; color_iter < sizeof(color.all); color_iter++) {
    normalized_color.all[color_iter] = _ledTable[color.all[color_iter]];
  }

  return normalized_color;
}

void setLightColor(LedColor color) {
  LedColor normalized_color = normalizeColor(color);
  dimmer->rgbwControl(normalized_color.red, normalized_color.green,
    normalized_color.blue, normalized_color.white);
}
