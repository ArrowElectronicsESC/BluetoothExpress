//ArrowBGXShield.cpp

#include "Arduino.h"
#include "ArrowBGXShield.h"
#include <Wire.h>


#if defined(ARDUINO_AVR_UNO)
	#include "SoftwareSerial.h"
	//Nothing yet
#else
  #define TWCR 0
#endif

PCA9633::PCA9633(uint8_t adr){
	_adr = adr; //Set the address ofr the device 0x60 is the default 7bit address
/*	if (TWCR == 0){
		Wire.begin();
		Wire.setClock(100000);
	}*/
}

void PCA9633::_I2CWrite(uint8_t commmand, uint8_t data){
	Wire.beginTransmission(_adr);
	Wire.write(commmand); // control register
	Wire.write(data);  //  send byte data
	Wire.endTransmission();
}

uint8_t PCA9633::_I2CRead(uint8_t commmand){
	uint8_t readData;
	Wire.beginTransmission(_adr);
	Wire.write(commmand); // control register
	Wire.endTransmission();

	Wire.requestFrom(_adr, SINGLE_READ, SEND_STOP); // request one byte

	readData = Wire.read();
	return readData;
}

void PCA9633::init(void){
	uint8_t settings = NO_AUTO_INC | SLEEP_DISABLE | ALT_ADDR_DISABLE;
	_I2CWrite(MODE1, settings);
	settings = GROUP_DIMMING | INVRT | CHANGE_ON_ACK | TOTEM_CONFIG;
	_I2CWrite(MODE2, settings);
	settings = LED_INDV_GRP(LED0) | LED_INDV_GRP(LED1) | LED_INDV_GRP(LED2) | LED_INDV_GRP(LED3); //TODO CHANGE TO MACRO FUNCTIONS
	_I2CWrite(LEDOUT, settings);
}

void PCA9633::onBoardMode(void){
	uint8_t settings = GROUP_DIMMING | NON_INVRT | CHANGE_ON_ACK | TOTEM_CONFIG;
	_I2CWrite(MODE2, settings);
}

void PCA9633::outputMode(void){
	uint8_t settings = GROUP_DIMMING | INVRT | CHANGE_ON_ACK | TOTEM_CONFIG;
	_I2CWrite(MODE2, settings);
}
void PCA9633::rgbwControl(uint8_t red, uint8_t green, uint8_t blue, uint8_t white ){
	_I2CWrite(PWM0, white);
	_I2CWrite(PWM1, blue);
	_I2CWrite(PWM2, red);
	_I2CWrite(PWM3, green);
	uint8_t	settings = LED_INDV_GRP(LED0) | LED_INDV_GRP(LED1) | LED_INDV_GRP(LED2) | LED_INDV_GRP(LED3); //TODO CHANGE TO MACRO FUNCTIONS
	_I2CWrite(LEDOUT, settings);
	return;
}

SI7050::SI7050(uint8_t adr){
	_adr = adr; //Set the address ofr the device 0x60 is the default 7bit address
/*	if (TWCR == 0){
		Wire.begin();
		Wire.setClock(100000);
	}*/
}

void SI7050::startTempMeasure(void){
    Wire.beginTransmission(_adr);
    // Send temperature measurement command, NO HOLD MASTER
    Wire.write(START_TEMP);
    // Stop I2C transmission
    Wire.endTransmission();
}

float SI7050::getTempMeasureC(void){
	unsigned int data[2];
    Wire.requestFrom(_adr, 2);
    // Read 2 bytes of data
    // temp msb, temp lsb
    if(Wire.available() == 2)
    {
      data[0] = Wire.read();
      data[1] = Wire.read();
    }
    float temp  = ((data[0] * 256.0) + data[1]);
    float ctemp = ((175.72 * temp) / 65536.0) - 46.85;
    return ctemp;
}

float SI7050::getTempMeasureF(void){
	unsigned int data[2];
    Wire.requestFrom(_adr, 2);
    // Read 2 bytes of data
    // temp msb, temp lsb
    if(Wire.available() == 2)
    {
      data[0] = Wire.read();
      data[1] = Wire.read();
    }
    float temp  = ((data[0] * 256.0) + data[1]);
    float ctemp = ((175.72 * temp) / 65536.0) - 46.85;
    float ftemp = ctemp * 1.8 + 32;
    return ftemp;
}

VEML6030::VEML6030(uint8_t adr){
	_adr = adr; //Set the address ofr the device 0x10 is the default 7bit address
/*	if (TWCR == 0){
		Wire.begin();
		Wire.setClock(100000);
	}*/
}

void VEML6030::defaultConfig(void){
	writeCommand(0, 0x0000);
	writeCommand(1, 0x0000);
	writeCommand(2, 0x0000);

}


uint16_t VEML6030::getALS(void){
	float luxCorrectionFactor [4] = {0.0576,0.0288,0.4608,0.2304};
	float integrationFactor [6] = {1.0,0.5,0.25,0.125,2.0,4.0};
	uint16_t conversion =  readData(CONF_REF);
	uint16_t gain = ((conversion && 0x1800) >> 11);
	uint16_t integration = ((conversion && 0x03C0) >> 6);
	integration = (integration > 0x4) ? (integration >> 1) : integration;
	return readData(ALS_REG) * luxCorrectionFactor[gain] * integrationFactor[integration];
}

uint16_t VEML6030::getWhite(void){
	return readData(WHITE_REG);
}

void VEML6030::writeCommand(uint8_t reg, uint16_t data){
	Wire.beginTransmission(_adr);
    Wire.write(reg);
    Wire.write( (uint8_t) (data & 0xFF));
    Wire.write( (uint8_t) ((data >> 8) & 0xFF) );
    Wire.endTransmission(); 
}

uint16_t VEML6030::readData(uint8_t reg){
	Wire.beginTransmission(_adr);
    Wire.write(reg);
    Wire.endTransmission(false); 
    Wire.requestFrom(_adr, 2, true);
	uint8_t byteLow = Wire.read();  //Read in high and low bytes (big endian)
	uint8_t byteHigh = Wire.read();
	return ((byteHigh << 8) | byteLow);
}