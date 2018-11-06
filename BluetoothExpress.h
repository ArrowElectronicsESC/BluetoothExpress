/*
Add header details and liscence here
*/


#ifndef BGX_h
#define BGX_h


#include "Arduino.h"
#include <SoftwareSerial.h>
#include "BluetoothExpressDefines.h"

class BGX13
{
  public:
  	BGX13(SoftwareSerial * arduinoSoftwareSerial); //Initialize the BGX connection
    void serialConnect(long baud);
    int BGXRead(void);
    int printBGXBuffer(void);
    void sendCommand(void);
    //adv - functions for advertising as a peripherial
  	void advertiseHigh(void);
  	void advertiseLow(void);
  	void advertiseOff(void);
  	//clrb - Deletes all bonding information
  	void clearBondingInfo(void); 
  	//con - Connect to a peripherial; see functions for returns
  	//int connectToPeripheral(char* indexOrAddress); //ECB TODO
  	//int connectToPeripheralWTimeout(char* indexOrAddress, int timeout); //ECB TODO
  	//dct - Disconnect from Peripherial or Central
  	void disconnect(void);
  	//dtm - device test mode
  	//void deviceTestModeStartRX(int channel, int phy); //ECB TODO
  	//void deviceTestModeStartTX(int channel, int phy, int packetType, int length); //ECB TODO 
  	//void deviceTestModeStop(void); //ECB TODO
  	//fac - Factory Rest
  	void factoryReset(void);
  	//gdi - Set GPIO direction and pin mode
  	void gpioSetIn(int number, int direction, int debounce);
  	void gpioSetOut(int number, int direction, int mode, int pullResistor, int driveStrength);
  	//gdis
  	//void gpioSetAll(long direction); 
  	//get - Get the value of a variable
  	//char* getVariable(char* variable);
  	//gfu - Select GPIO Function
  	//void selectGPIOFunction(int number, int function);
  	//gge - Get GPIO value
  	//int readGPIOValue(int number);
  	//gges - read all GPIO values
  	//char* readAllGPIOValues(void);
  	//gse - Set Gpio Value
  	//void setGPIOValue(int number, int value);
  	//gses - Set multiple GPIO values
  	//Not implimented currently
  	//rbmode - Change Remote Peripherial Bus Mode
  	//Not implimented currently
  	//reboot - Restart the device
  	void reboot(void);
  	//save
    void saveConfiguration(void);
  	//void saveConfiguration(void);
  	//scan - oh god
  	//void scan(void);
  	//set
  	//sleep
    void sleepMode(void);
  	//str
    void streamMode(void);
  	//uartu
    void updateUartSettings(void);
  	//uevt
  	//ufu
    void userFunctionAssign(int number, char* function);
  	//ulast
    void getLastUserFunctionResult(void);
  	//urun
    void userFunctionRun(int number);
  	//ver - Returns the version number of the BGX13
  	void getVersion(char* ver);
  	//wake
    void wake(void);
    //Destructor
    ~BGX13(void);

  private:
  	int _state;
    SoftwareSerial * _bgxSerial;
    char _uart_tx_buffer[UART_BUFFER_SIZE];
    char _uart_rx_buffer[UART_BUFFER_SIZE];
    int _uart_rx_write_ptr = 0;
  	void waitForStreamMode(void);
    void setCommandMode(void);
};



#endif //ifndef BGX_h 