//BluetoothExpressDefines.h

#ifndef BGXDefines_h
#define BGXDefines_h

#define RX_PIN 4
#define TX_PIN 5

#define UART_BUFFER_SIZE 100

#define CMD_CLEAR_BONDING_INFO "clrb"

char uart_send_buffer[UART_BUFFER_SIZE];
char uart_buffer[UART_BUFFER_SIZE];
// TODO: Rename uart_buffer_data to something more descriptive like
// uart_buffer_write_pointer, and maybe make it a real char*
int uart_buffer_data = 0;

// TODO: Doc Strings!
// TODO: Standardize Delay times with defines
// TODO: Standardize functions that do nearly the same thing like advertiseHigh,
//       advertiseLow, advertiseOff, etc...
static const char stream[] = "STREAM_MODE";
static const char command[] = "COMMAND_MODE";
static const char success[] = "Success";

enum BGX_states{
	RESTART,
	INIT_BGX,
	INIT_SENSORS,	
	INIT_LIGHTING,
	WAIT_FOR_CONN,
	RUNNING
};

const char * inDirections[] = {
    "in", //0
    "ipd", //1
    "ipu", //2
    "inw", //3
    "ipuw", //4
    "ipdw" //5
};

const char * outDirections[] = {
    "olo", //0
    "ohi", //1
    "hiz"  //2
};

const char * pushPullModes[] = {
    "pp", //0
    "od", //1
    "os"  //2
};

const char * driveStrengths[] = {
    "drvst", //0
    "drvwk" //1
};



#endif