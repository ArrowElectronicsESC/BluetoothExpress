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
  _bgxSerial->println("gfu 4 str_active"); //Set the indicator high when in stream mode
  _bgxSerial->println("set bu s c level"); //Set to Logic Level Bus Mode Control
  _bgxSerial->println("gdi 0 olo"); //SET GPIO to out low-init
  _bgxSerial->println("gfu 0 str_select"); //set gpio as bus control
  BGXRead();//ECB Remove This and replace above with command calls
  RESET_UART_RX; //ECB TODO Macro for reset and privite
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
}

void BGX13::advertiseHigh(void){
  digitalWrite(6, LOW); //Sets COMMAND mode
  _bgxSerial->println("adv high");
  _uart_rx_write_ptr = 0;
  BGXRead(); //Eat the output
  digitalWrite(6, HIGH); //Sets stream mode
}

void BGX13::advertiseLow(void){
  digitalWrite(6, LOW); //Sets COMMAND mode
  _bgxSerial->println("adv low");
  _uart_rx_write_ptr = 0;
  BGXRead(); //Eat the output
  digitalWrite(6, HIGH); //Sets stream mode
}

void BGX13::advertiseOff(void){
  digitalWrite(6, LOW); //Sets COMMAND mode
  _bgxSerial->println("adv off");
  _uart_rx_write_ptr = 0;
  BGXRead(); //Eat the output
  digitalWrite(6, HIGH); //Sets stream mode
}


void BGX13::clearBondingInfo(void) {
  snprintf(_uart_tx_buffer, UART_BUFFER_SIZE, "clrb");
  sendCommand();
}

void BGX13::sendCommand(void) {
  digitalWrite(6, LOW); //Sets COMMAND mode
  _bgxSerial->println(_uart_tx_buffer);
  _uart_rx_write_ptr = 0;
  BGXRead(); //Eat the output
  digitalWrite(6, HIGH); //Sets stream mode
}
//ecb tODO
//int BGX13::connectToPeripheral(char* indexOrAddress){
  // String command = "conn ";
  // digitalWrite(6, LOW); //Sets COMMAND mode
  // _bgxSerial->println(command.concat(indexOrAddress));
  // _uart_rx_write_ptr = 0;
  // BGXRead();
  // for (i = 0; i < _uart_rx_write_ptr && i < 7; i++)
  // {
  //   if (_uart_rx_buffer[i] == success[i])
  //   {
  //     matchCount++;
  //   }
  // }
  // digitalWrite(6, HIGH);
  // if (matchCount > 4)
  // {
  //   return 0;
  // }
  // return -1;
//}

void BGX13::disconnect(void){
  digitalWrite(6, LOW); //Sets COMMAND mode
  _bgxSerial->println("dct ");
  _uart_rx_write_ptr = 0;
  BGXRead(); //Eat the output
  digitalWrite(6, HIGH); //Sets stream mode
}

void BGX13::factoryReset(void){
  String command = "fac ";
  digitalWrite(6, LOW); //Sets COMMAND mode
  BGXRead();
  _uart_rx_write_ptr = 0;
  _bgxSerial->println("get bl a");
  //_bgxSerial->println("fac D0CF5ED8FCE0");
  BGXRead();
  // TODO: The 22 here is a magic number
  // Also, be consistent with using 0x00 vs NULL
  _uart_rx_buffer[22] = 0x00;
  command += &_uart_rx_buffer[10];
  _bgxSerial->println(command);
  BGXRead();
  digitalWrite(6, HIGH); //Sets stream mode
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
  // TODO consider using a sprintf here, if performance/memory become and issue
  String command = "gdi ";
  if (direction > 5) {
    return;
  }
  command += number;
  command += ' ';
  command += inDirections[direction];
  command += ' ';

  if (debounce >= 0 && debounce < 10) {
    command += "db";
    command += debounce;
  }
  digitalWrite(6, LOW); //Sets COMMAND mode
  BGXRead();
  _uart_rx_write_ptr = 0;
  _bgxSerial->println(command);
  _uart_rx_write_ptr = 0;
  BGXRead(); //Eat the output
  digitalWrite(6, HIGH); //Sets stream mode
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
  command += ' ';
  command += outDirections[direction];
  //command += ' ';
  //command += pushPullModes[mode];
  if (mode == 1 || mode == 2) {
    command += ' ';
    if (pullResistor) {
      command += "pren";
    } else {
      command += "prdi";
    }
    command += ' ';
    command += driveStrengths[driveStrength];
  }

  // TODO: There is a similar block of code in gpioSetIn, maybe it could be commonized
  digitalWrite(6, LOW); //Sets COMMAND mode
  BGXRead();
  _uart_rx_write_ptr = 0;
  _bgxSerial->println(command);
  _uart_rx_write_ptr = 0;
  BGXRead(); //Eat the output
  digitalWrite(6, HIGH); //Sets stream mode
}

void BGX13::reboot(void){
  digitalWrite(6, LOW); //Sets COMMAND mode
  BGXRead(); //clears UART
  _uart_rx_write_ptr = 0; //Reset buffer
  _bgxSerial->println("reboot"); 
  BGXRead();
}

void BGX13::saveConfiguration(void){
  digitalWrite(6, LOW); //Sets COMMAND mode
  BGXRead(); //clears UART
  _uart_rx_write_ptr = 0; //Reset buffer
  _bgxSerial->println("save"); 
  BGXRead();
}

void BGX13::sleepMode(void){
  digitalWrite(6, LOW); //Sets COMMAND mode
  BGXRead(); //clears UART
  _uart_rx_write_ptr = 0; //Reset buffer
  _bgxSerial->println("sleep"); 
  BGXRead();
}

void BGX13::streamMode(void){
  digitalWrite(6, LOW); //Sets COMMAND mode
  BGXRead(); //clears UART
  _uart_rx_write_ptr = 0; //Reset buffer
  _bgxSerial->println("str"); 
  BGXRead();
}

void BGX13::updateUartSettings(void){
  digitalWrite(6, LOW); //Sets COMMAND mode
  BGXRead(); //clears UART
  _uart_rx_write_ptr = 0; //Reset buffer
  _bgxSerial->println("uartu"); 
  BGXRead();
}

void BGX13::userFunctionAssign(int number, char* function){
  String command = "ufu ";
  command += number;
  command += " ";
  command += function;
  digitalWrite(6, LOW); //Sets COMMAND mode
  BGXRead(); //clears UART
  _uart_rx_write_ptr = 0; //Reset buffer
  _bgxSerial->println(command); 
  BGXRead();
}

void BGX13::getLastUserFunctionResult(){
  digitalWrite(6, LOW); //Sets COMMAND mode
  BGXRead(); //clears UART
  _uart_rx_write_ptr = 0; //Reset buffer
  _bgxSerial->println("ulast"); 
  BGXRead();
}

void BGX13::userFunctionRun(int number){
  String command = "urun ";
  command += number;
  digitalWrite(6, LOW); //Sets COMMAND mode
  BGXRead(); //clears UART
  _uart_rx_write_ptr = 0; //Reset buffer
  _bgxSerial->println(command); 
  BGXRead();
}

/**********************************************
* Function: getVersion
* Params:
* Description:
* Returns: 
***********************************************/ 
void BGX13::getVersion(char *ver){
  digitalWrite(6, LOW); //Sets COMMAND mode
  BGXRead();
  _uart_rx_write_ptr = 0;
  _bgxSerial->println("ver");
  BGXRead();
  strcpy(ver, &_uart_rx_buffer[5]);
  digitalWrite(6, HIGH); //Sets stream mode
}

void BGX13::wake(void){
  digitalWrite(6, LOW); //Sets COMMAND mode
  BGXRead();
  _uart_rx_write_ptr = 0;
  _bgxSerial->println("wake");
  BGXRead();  
}





BGX13::~BGX13(void){
  __asm__("nop");
}