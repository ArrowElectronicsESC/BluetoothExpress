/*
Add header details and liscence here
*/


#include "Arduino.h"
#include "BluetoothExpress.h"
#include <SoftwareSerial.h>
//#include "BluetoothExpressDefines.h"


BGX13::BGX13(SoftwareSerial *arduinoSoftwareSerial){
	_state = 0;
  _bgxSerial = arduinoSoftwareSerial;
  pinMode(BUS_MODE_PIN, OUTPUT);
  COMMAND_MODE; //Sets command mode
}

void BGX13::serialConnect(long baud){
  _bgxSerial->begin(baud);
	while (!_bgxSerial) {
  	; // wait for serial port to connect.
	}
  _bgxSerial->listen();
  delay(5);
  _bgxSerial->println("fac D0CF5ED8FCE0"); //Set the indicator high when in stream mode
  delay(500);
  //ecb remoVE
  _bgxSerial->println("gfu 4 str_active"); //Set the indicator high when in stream mode
  delay(5);
  _bgxSerial->println("set bu s c level"); //Set to Logic Level Bus Mode Control
  delay(5);
  _bgxSerial->println("gfu 0 str_select"); //set gpio as bus control
  delay(5);
  _bgxSerial->println("gdi 0 olo"); //SET GPIO to out low-init
  delay(5);
  _bgxSerial->println("gfu 2 stdio"); 
  delay(5);
  _bgxSerial->println("gdi 2 ipu");   
  delay(5);
  _bgxSerial->println("gfu 3 stdio"); 
  delay(5);
  _bgxSerial->println("gdi 3 ipu");  
  delay(5);
  

  BGXRead();//ECB replace above with command calls
  RESET_UART_RX; 
  delay(10);
  STREAM_MODE; //Sets stream mode
  delay(5);
}

void BGX13::waitForStreamMode(void) {
  // 
  int char_iter;
  int matchCount = 0;
  for (char_iter = 0; (char_iter < _uart_rx_write_ptr) && (char_iter < strlen(stream)); char_iter++) {
    if (_uart_rx_buffer[char_iter] == stream[char_iter]) {
      matchCount++;
    }
  }
  if (matchCount > 4) {
    _state = RUNNING;
    _bgxSerial->println("Streaming");
  }
  if (_uart_rx_write_ptr > 0) {
    _bgxSerial->println(_uart_rx_buffer);
  }
  _uart_rx_write_ptr = 0;
}

int BGX13::BGXRead(void){
  int byte_counter;
  _bgxSerial->listen();
  delay(READ_DELAY);
  int bytes_available = _bgxSerial->available();
  
  for (byte_counter = 0; byte_counter < bytes_available; byte_counter++) {
    _uart_rx_buffer[_uart_rx_write_ptr++]=_bgxSerial->read();
  }
  _uart_rx_buffer[_uart_rx_write_ptr] = NULL;

  return 0;
}

int BGX13::printBGXBuffer(void){
  Serial.println(_uart_rx_buffer);
  //gets and pushes _uart_rx_buffer to the UART
}

void BGX13::getBGXBuffer(char* data){
  BGXRead();
  strcpy(data, _uart_rx_buffer);
  //gets and pushes _uart_rx_buffer
}

void BGX13::sendCommand(void) {
  COMMAND_MODE; //Sets COMMAND mode
  _bgxSerial->println(_uart_tx_buffer);
  RESET_UART_RX;
  BGXRead(); //Eat the output
  STREAM_MODE; //Sets stream mode
}

void BGX13::advertiseHigh(void){
  snprintf(_uart_tx_buffer, UART_BUFFER_SIZE, ADVERTISE_HIGH);
  sendCommand();
}

void BGX13::advertiseLow(void){
  snprintf(_uart_tx_buffer, UART_BUFFER_SIZE, ADVERTISE_LOW);
  sendCommand();
}

void BGX13::advertiseOff(void){
  snprintf(_uart_tx_buffer, UART_BUFFER_SIZE, ADVERTISE_OFF);
  sendCommand();
}


void BGX13::clearBondingInfo(void) {
  snprintf(_uart_tx_buffer, UART_BUFFER_SIZE, CMD_CLEAR_BONDING_INFO);
  sendCommand();
}

void BGX13::disconnect(void){
  snprintf(_uart_tx_buffer, UART_BUFFER_SIZE, DISCONNECT);
  sendCommand();
}

void BGX13::factoryReset(void){
  String command = FACTORY_RESET;
  COMMAND_MODE; //Sets COMMAND mode
  BGXRead();
  _uart_rx_write_ptr = 0;
  _bgxSerial->println(GET_BT_ADDR);
  //_bgxSerial->println("fac D0CF5ED8FCE0");  BGXRead();
  _uart_rx_buffer[ADDRESS_INDEX] = NULL;
  command += &_uart_rx_buffer[ADDRESS_INDEX];
  _bgxSerial->println(command);
  BGXRead();
  STREAM_MODE; //Sets stream mode
}

/* Directions are as follows:
   "in", //0
    "ipd", //1
    "ipu", //2
    "inw", //3
    "ipuw", //4
    "ipdw" //5
*/
void BGX13::gpioSetIn(int number, int direction, int debounce) {
  String command = "gdi ";
  if (direction > 5) {
    return;
  }
  command += number;
  command += SPACE;
  command += inDirections[direction];
  command += SPACE;

  if (debounce >= 0 && debounce < 10) {
    command += "db";
    command += debounce;
  }

  snprintf(_uart_tx_buffer, UART_BUFFER_SIZE, command.c_str());
  sendCommand();
}

/* Directions are as follows:
    "olo", //0
    "ohi", //1
    "hiz"  //2

    Push Pull Mode is as follows:
    "pp", //0
    "od", //1
    "os"  //2

    Drive Strenght is as follows:
    "drvst", //0
    "drvwk" //1
*/
void BGX13::gpioSetOut(int number, int direction, int mode, int pullResistor, int driveStrength){
  String command = "gdi ";
  if(direction > 2) return;
  if(mode > 2) return;
  if(driveStrength > 1) return;
  if(mode > 0 && direction == 2) return;
  command += number;
  command += SPACE;
  command += outDirections[direction];
  //command += SPACE;
  //command += pushPullModes[mode];
  if (mode == 1 || mode == 2) {
    command += SPACE;
    if (pullResistor) {
      command += "pren";
    } else {
      command += "prdi";
    }
    command += SPACE;
    command += driveStrengths[driveStrength];
  }

  snprintf(_uart_tx_buffer, UART_BUFFER_SIZE, command.c_str());
  sendCommand();
}

void BGX13::reboot(void){
  snprintf(_uart_tx_buffer, UART_BUFFER_SIZE, REBOOT);
  sendCommand();
}

void BGX13::saveConfiguration(void){
  snprintf(_uart_tx_buffer, UART_BUFFER_SIZE, SAVE);
  sendCommand();
}

void BGX13::sleepMode(void){
  snprintf(_uart_tx_buffer, UART_BUFFER_SIZE, SLEEP);
  sendCommand();
}

void BGX13::streamMode(void){
  snprintf(_uart_tx_buffer, UART_BUFFER_SIZE, SET_STREAM_MODE);
  sendCommand();
}

void BGX13::updateUartSettings(void){
  snprintf(_uart_tx_buffer, UART_BUFFER_SIZE, UPDATE_UART_SETTINGS);
  sendCommand();
}

void BGX13::userFunctionAssign(int number, char* function){
  String command = "ufu ";
  command += number;
  command += SPACE;
  command += function;
  snprintf(_uart_tx_buffer, UART_BUFFER_SIZE, command.c_str());
  sendCommand();
}

void BGX13::getLastUserFunctionResult(){
  snprintf(_uart_tx_buffer, UART_BUFFER_SIZE, GET_LAST_USER_FUNC_STATUS);
  sendCommand();
}

void BGX13::userFunctionRun(int number){
  String command = "urun ";
  command += number;
  snprintf(_uart_tx_buffer, UART_BUFFER_SIZE, command.c_str());
  sendCommand();
}

void BGX13::getVersion(char *ver){
  COMMAND_MODE; //Sets COMMAND mode
  BGXRead(); //needed to ensure the buffer is clear
  _uart_rx_write_ptr = 0;
  _bgxSerial->println("ver");
  BGXRead();
  strcpy(ver, &_uart_rx_buffer[VERSION_BEGIN]);
  STREAM_MODE; //Sets stream mode
}

void BGX13::wake(void){
  snprintf(_uart_tx_buffer, UART_BUFFER_SIZE, WAKE);
  sendCommand(); 
}





BGX13::~BGX13(void){
  __asm__("nop");
}