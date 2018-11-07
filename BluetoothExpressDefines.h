//BluetoothExpressDefines.h

#ifndef BGXDefines_h
#define BGXDefines_h


#define BUS_MODE_PIN 6 //Default Pin for BGX13 Arduino Shield 
//This will change the pin configuration for arduino only
#define COMMAND_MODE digitalWrite(BUS_MODE_PIN, LOW)
#define STREAM_MODE digitalWrite(BUS_MODE_PIN, HIGH)

#define RX_PIN 4
#define TX_PIN 5

#define UART_BUFFER_SIZE 100
#define RESET_UART_RX _uart_rx_write_ptr = 0

#define READ_DELAY 5
#define ADDRESS_END_INDEX 22
#define ADDRESS_INDEX 10
#define VERSION_BEGIN 5
#define SPACE ' '

#define CMD_CLEAR_BONDING_INFO 		"clrb"
#define ADVERTISE_HIGH 				"adv high"
#define ADVERTISE_LOW 				"adv low"
#define ADVERTISE_OFF				"adv off"
#define DISCONNECT					"dct"
#define FACTORY_RESET				"fac "
#define GET_BT_ADDR					"get bl a"
#define REBOOT						"REBOOT"
#define SAVE						"SAVE"
#define SLEEP						"SLEEP"
#define SET_STREAM_MODE				"STR"
#define UPDATE_UART_SETTINGS		"uartu"
#define GET_LAST_USER_FUNC_STATUS	"ulast"
#define WAKE						"wake"

// TODO: Doc Strings!
/**********************************************
* Function: getVersion
* Params:
* Description:
* Returns: 
***********************************************/ 
// TODO: A few more strings should be macros
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

static const char * inDirections[] = { 
    "in", //0
    "ipd", //1
    "ipu", //2
    "inw", //3
    "ipuw", //4
    "ipdw" //5
};

static const char * outDirections[] = {
    "olo", //0
    "ohi", //1
    "hiz"  //2
};

static const char * pushPullModes[] = {
    "pp", //0
    "od", //1
    "os"  //2
};

static const char * driveStrengths[] = {
    "drvst", //0
    "drvwk" //1
};



#endif