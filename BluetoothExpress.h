/*
Add header details and liscence here
*/


#ifndef BGX_h
#define BGX_h


#include "Arduino.h"

#include "BluetoothExpressDefines.h"

#if defined(ARDUINO_AVR_UNO)
  #include <SoftwareSerial.h>
#else
  #define TWCR 0
#endif

class BGX13
{
  public:
#if defined(ARDUINO_AVR_UNO)
  	BGX13(SoftwareSerial * arduinoSoftwareSerial); //Initialize the BGX connection
#endif
    BGX13(HardwareSerial  *arduinoHardwareSerial);
    void serialConnect(long baud);
    int BGXRead(void);
    int printBGXBuffer(void);
    void getBGXBuffer(char* data);
    void sendCommand(int readTime);
    void sendString(char* toSend);
    //adv - functions for advertising as a peripherial
  	void advertiseHigh(void);
  	void advertiseLow(void);
  	void advertiseOff(void);
  	//clrb - Deletes all bonding information
  	void clearBondingInfo(void); 
  	//dct - Disconnect from Peripherial or Central
  	void disconnect(void);
  	//fac - Factory Rest
  	void factoryReset(void);
  	//gdi - Set GPIO direction and pin mode
  	void gpioSetIn(int number, BGX_input_direction direction, int debounce=-1);
  	void gpioSetOut(int number, BGX_output_direction direction, int mode=-1, bool enablePullResistor=true, int driveStrength=0);
  	//get - Get the value of a variable
  	void getVariable(char *variable, char *buffer, int length);
  	//gfu - Select GPIO Function
  	void selectGPIOFunction(int gpioNumber, char *function);
  	//gge - Get GPIO value
  	int getGPIOValue(int number);
  	//gse - Set Gpio Value
  	void setGPIOValue(int number, int value);
  	//reboot - Restart the device
  	void reboot(void);
  	//save
    void saveConfiguration(void);
  	//scan
  	void scan(int timeScan);
  	//set
    void setVariable(char *variable, char *value);
  	//sleep
    void sleepMode(void);
  	//str
    void streamMode(void);
  	//uartu
    void updateUartSettings(void);
  	//uevt
    void userEventTrigger(int functionNumber, bool assertHigh, int gpioNumber);
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
    Stream *_bgxSerial;
    char _uart_tx_buffer[UART_BUFFER_SIZE];
    char _uart_rx_buffer[UART_BUFFER_SIZE];
    int _uart_rx_write_ptr = 0;
  	void waitForStreamMode(void);
    void setCommandMode(void);
    void serialBegin(long baud);
    bool _swSerial = false;
};



#endif //ifndef BGX_h 