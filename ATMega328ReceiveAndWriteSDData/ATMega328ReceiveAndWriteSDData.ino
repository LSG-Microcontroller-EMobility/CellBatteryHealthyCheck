/*
 Name:		Receiver.ino
 Created:	3/8/2020 1:53:08 AM
 Author:	luigi.santagada
*/
#include "localLibraries/SoftwareSerial.h"
#include <SD.h>
#include <SPI.h>
#include <string.h>
#include <DFRobotDFPlayerMini.h>

const uint8_t selectorMultiPlex0 = 4;

const uint8_t selectorMultiPlex1 = 5;

const uint8_t selectorMultiPlex2 = 6;

const uint8_t selectorMultiPlex3 = 7;

const uint8_t numberOfBattery = 6;

const uint8_t _pin_rx = 3;

const uint8_t _pin_reset_attiny85 = 9;

uint8_t _pin_buzzer = 8;

//-----------------------    ATTENZIONE PIN ASSEGNATI !!!!!!!   -------------------------------
// Pin 11 MOSI	Pin 12 MISO		Pin 13 SCK

const char* idBattery[numberOfBattery] = { "B0", "B1", "B2", "B3", "B4", "B5" }; //, "B1", "B2" };

const float deltaVoltage[numberOfBattery] = { 0.00, 0.00, 0.00, 0.00, 0.00, 0.00 }; //, 0.00, 0.00 };

float storedBatteryValues[numberOfBattery] = { 0.00, 0.00, 0.00, 0.00, 0.00, 0.00 }; //,0.00,0.00 };

uint8_t _demultiplexerPosition = 0;

uint8_t fileNumber = 0;

bool _is_buzzer_disabled = true;

bool _is_card_writing_disable = false;

uint8_t numeroDisallineamenti = 0;

char _idMessage[1] = {};

float batteryMaxLevel = 0.00f;

float batteryMinLevel = 0.00f;

uint8_t ii = 0;

void setup()
{
	delay(2000);

	pinMode(_pin_reset_attiny85, OUTPUT);

	pinMode(selectorMultiPlex0, OUTPUT);

	pinMode(selectorMultiPlex1, OUTPUT);

	pinMode(selectorMultiPlex2, OUTPUT);

	pinMode(selectorMultiPlex3, OUTPUT);

	digitalWrite(selectorMultiPlex0, LOW);

	digitalWrite(selectorMultiPlex1, LOW);

	digitalWrite(selectorMultiPlex2, LOW);

	digitalWrite(selectorMultiPlex3, LOW);

	digitalWrite(_pin_reset_attiny85, LOW);

	Serial.begin(9600);

	pinMode(_pin_buzzer, OUTPUT);

	initFileCard();


	Serial.println(F("restart"));

	playMessageOnDPlayer(6);
}

char fileName[15] = {};

void initFileCard()
{
	if (_is_card_writing_disable) return;

	if (SD.begin())
	{

		//Serial.println(F("card ready"));

		for (uint8_t i = 0; i < 1; i++)
		{
			strcpy(fileName, "batt");
			fileName[4] = (char)(i + 48);
			strcat(fileName, ".csv");
			strcat(fileName, "\0");
#ifdef _DEBUG
			Serial.println(fileName);
#endif // _DEBUG
			if (SD.exists(fileName))
			{
#ifdef _DEBUG
				Serial.println(F("File esiste"));
#endif // _DEBUG

				SD.remove(fileName);
			}
		}
		char headersText[37] = "IDMessage;Battery;Value;Delta;Origin";

		writeOnSDCard(headersText);
	}
	else
	{
		//buzzerSensorActivity(5, 400, 1000, 500);

#ifdef _DEBUG
		Serial.println(F("SD failed"));
#endif // _DEBUG
		playMessageOnDPlayer(4);
		return;
	}
}

void loop()
{
	Serial.println(F("giro"));

	resetAttiny85();

	setMultiplexer(_demultiplexerPosition);

	//if there is an attiny85 reset delay put here same delay 
	//delay(1000);

	char response[6] = {};

	getDataFromSerialBuffer(response);

#ifdef _DEBUG
	Serial.print(F("#"));
	Serial.print(response);
	Serial.println(F("#"));
#endif

	if (!is_number(response))
	{
		playMessageOnDPlayer(3);
#ifdef _DEBUG
		Serial.println(F("resp.not.num"));
#endif
		_demultiplexerPosition = 0;
		return;
	}

	_idMessage[0] = response[4];

	char csvTextLayOut[21] = {};

	prepareStringForSDCard(csvTextLayOut, response);

	if (csvTextLayOut[19] == '\0' && csvTextLayOut[20] != '\0')
	{
		Serial.println(F("text.problem"));
		_demultiplexerPosition = 0;
		playMessageOnDPlayer(2);
		return;
	}

	writeOnSDCard(csvTextLayOut);

	if (_demultiplexerPosition == 5)
	{

#ifdef _DEBUG
		printStoredBatteryValuesArray();
#endif // _DEBUG

		checkActivities();
		_demultiplexerPosition = 0;
		for (uint8_t i = 0; i < 6; i++) {
			storedBatteryValues[i] = 0.00;
		}
	}
	else
	{
		_demultiplexerPosition++;
	}

	/* 	if (responseString != "")
		{
			// Serial.println(F("-----------------START------------------------"));

			_idMessage = responseString.substring(5, 6);

			csvString = "";

			csvString = prepareStringForSDCard(responseString, demultiplexerPosition);

			if (csvString.length() != 21)
			{
				Serial.print(F("dis-"));
				Serial.println(responseString);
				return;
			}

			writeOnSDCard(csvString);

			++demultiplexerPosition;

			bool condition = true;

			numeroDisallineamenti = 0;

			unsigned long timeForSerialData = millis();

			// clearSerialBuffer();

			while ((millis() - timeForSerialData < 10000) && (demultiplexerPosition < numberOfBattery) && condition)
			{
				setMultiplexer(demultiplexerPosition);

				responseString = "";

				responseString = getDataFromSerialBuffer();

				if (responseString != "")
				{
					numeroDisallineamenti = 0;

					String csvString = "";

					csvString = prepareStringForSDCard(responseString, demultiplexerPosition);

					if (csvString.length() == 21 && responseString.substring(5, 6) == _idMessage)
					{
						writeOnSDCard(csvString);

						timeForSerialData = millis();

						++demultiplexerPosition;
					}
					else
					{
						Serial.print(F("disallineamento - "));
						Serial.println(responseString);
					}
				}
			}
			buzzerSensorActivity(15, 1000, 100, 100);

			checkBatteriesMaxLevel(storedBatteryValues[0]);

			checkBatteriesMinLevel(storedBatteryValues[0]);

			Serial.print(F("Mx.v:"));
			Serial.println(batteryMaxLevel);

			Serial.print(F("M.v:"));
			Serial.println(batteryMinLevel);

			if (thereAreUnbalancedBatteries(90))
			{
				Serial.println(F("Unbalanced batteries"));
				buzzerSensorActivity(5, 2500, 80, 200);
			}

			resetAttiny85();
			clearSerialBuffer();
			//idCurrentMessage = "";
			// Serial.println(F("-----------------END------------------------"));
		} */
}

void printStoredBatteryValuesArray()
{
	for (uint8_t i = 0; i < 6; i++)
	{
		Serial.println(storedBatteryValues[i]);
	}
}

void checkActivities()
{
	checkBatteriesMaxLevel(storedBatteryValues[0]);

	checkBatteriesMinLevel(storedBatteryValues[0]);


#ifdef _DEBUG
	Serial.print(F("Mx.v:"));
	Serial.println(batteryMaxLevel);
#endif // _DEBUG

#ifdef _DEBUG
	Serial.print(F("M.v:"));
	Serial.println(batteryMinLevel);
#endif // _DEBUG

	if (thereAreUnbalancedBatteries(5))
	{
#ifdef _DEBUG
		Serial.println(F("Unbalanced batteries"));
#endif // _DEBUG
		playMessageOnDPlayer(1);
		//buzzerSensorActivity(5, 2500, 80, 200);
	}
}

bool is_number(const String& s)
{
	char* end = nullptr;
	float val = strtod(s.c_str(), &end);
	return end != s.c_str() && *end == '\0' /* && val != 0.00 */;
}

bool thereAreUnbalancedBatteries(uint8_t maxPercentageForAlarm)
{
	float percentageValue = 100 - ((batteryMinLevel / batteryMaxLevel) * 100);

#ifdef _DEBUG
	Serial.print(F("Percentage value : "));
	Serial.print(percentageValue);
	Serial.println(F("%"));
#endif // _DEBUG

	if (percentageValue > maxPercentageForAlarm)
	{
		return true;
	}
	return false;
}

float getNumber(char* response)
{
	return atof(response);
}

void getDataFromSerialBuffer(char* response)
{
	SoftwareSerial softwareSerial(3, 99);

	softwareSerial.begin(600);

	//response[0] = {};

	delay(500);

	if (softwareSerial.available() > 0)
	{
		softwareSerial.readStringUntil('*');
	}
	if (softwareSerial.available() > 0)
	{
		softwareSerial.readBytesUntil('*', response, 6);

#ifdef _DEBUG
		response[6] = '\0';
		Serial.print(F("data from attiny85 : "));
		Serial.println(response);
#endif // _DEBUG

	}
}

void setMultiplexer(int channel)
{
	// Serial.print("setMultiplexer channel : "); Serial.println(channel);
	int controlPin[] = { selectorMultiPlex0, selectorMultiPlex1, selectorMultiPlex2, selectorMultiPlex3 };

	int muxChannel[16][4] = {
		{0, 0, 0, 0}, // channel 0
		{1, 0, 0, 0}, // channel 1
		{0, 1, 0, 0}, // channel 2
		{1, 1, 0, 0}, // channel 3
		{0, 0, 1, 0}, // channel 4
		{1, 0, 1, 0}, // channel 5
		{0, 1, 1, 0}, // channel 6
		{1, 1, 1, 0}, // channel 7
		{0, 0, 0, 1}, // channel 8
		{1, 0, 0, 1}, // channel 9
		{0, 1, 0, 1}, // channel 10
		{1, 1, 0, 1}, // channel 11
		{0, 0, 1, 1}, // channel 12
		{1, 0, 1, 1}, // channel 13
		{0, 1, 1, 1}, // channel 14
		{1, 1, 1, 1}  // channel 15
	};

	// loop through the 4 sig
	for (int i = 0; i < 4; i++)
	{
		digitalWrite(controlPin[i], muxChannel[channel][i]);
	}
}

void prepareStringForSDCard(char* csvTextLayOut, char* response)
{
	char num_to_string[4] = {};

	char deltaVoltage_to_string[4] = {};

	response[4] = '\0';

	float number = getNumber(response);

	//Serial.println(number);

	number = number + deltaVoltage[_demultiplexerPosition];

	dtostrf(number, 4, 2, num_to_string);

	dtostrf(deltaVoltage[_demultiplexerPosition], 4, 2, deltaVoltage_to_string);

	csvTextLayOut[0] = _idMessage[0];
	strcat(csvTextLayOut, ";");
	strcat(csvTextLayOut, idBattery[_demultiplexerPosition]);
	strcat(csvTextLayOut, ";");
	strcat(csvTextLayOut, num_to_string);
	strcat(csvTextLayOut, ";");
	strcat(csvTextLayOut, deltaVoltage_to_string);
	strcat(csvTextLayOut, ";");
	strcat(csvTextLayOut, response);
	strcat(csvTextLayOut, _idMessage);
	csvTextLayOut[20] = '\0';
#ifdef _DEBUG
	Serial.println(csvTextLayOut);
#endif // _DEBUG
	storedBatteryValues[_demultiplexerPosition] = number;
}
/*
String prepareStringForSDCardOld(String message, uint8_t demultiplexerPosition)
{
	String csvString = "";
	double number = getNumber(message);
	number = number + deltaVoltage[demultiplexerPosition];
	csvString = message.substring(5, 6) + ";" + String(idBattery[demultiplexerPosition]) + ";" + String(number) + ";" + String(deltaVoltage[demultiplexerPosition]) + ";" + message;
	Serial.println(csvString);
	storedBatteryValues[demultiplexerPosition] = number;
	// writeOnSDCard(csvString);
	return csvString;
} */

void writeOnSDCard(char* message)
{
	File myFile;

	if (_is_card_writing_disable)return;
	// Create/Open file
	// String fileName = "batt" + String(fileNumber) + ".csv";
	// Serial.print(F("Apro file:"));
	// Serial.println(fileName);
	myFile = SD.open(fileName, FILE_WRITE);
	delay(500);
	if (myFile)
	{
		myFile.println(message);
#ifdef _DEBUG
		Serial.println(F("Scrivo"));
#endif // _DEBUG
		myFile.close();
	}
	else
	{
		//buzzerSensorActivity(5, 400, 1000, 500);
#ifdef _DEBUG
		Serial.println(F("error opening battery.cvs"));
#endif // _DEBUG
		playMessageOnDPlayer(4);
		myFile.close();
	}

	//// Reading the file
	// myFile = SD.open("batteryValues.csv");
	// if (myFile) {
	//	//Serial.println("Read:");
	//	// Reading the whole file
	//	while (myFile.available()) {
	//		Serial.write(myFile.read());
	//	}
	//	myFile.close();
	// }
	// else {
	//	Serial.println("error opening file batteryValues.csv");
	// }
}

void resetAttiny85()
{
	digitalWrite(_pin_reset_attiny85, HIGH);
	delay(500);
	digitalWrite(_pin_reset_attiny85, LOW);
}

void buzzerSensorActivity(uint8_t numberOfCicle, unsigned int frequency, unsigned long duration, uint16_t pause)
{
	if (_is_buzzer_disabled)return;

	for (uint8_t i = 0; i < numberOfCicle; i++)
	{
		tone(_pin_buzzer, frequency, duration);
		delay(duration);
		delay(pause);
		noTone(_pin_buzzer);
	}
	delay(pause);
}

void checkBatteriesMaxLevel(float value)
{
	ii = 0;
	while (ii < numberOfBattery)
	{
		// Serial.println(ii);

		if (value >= storedBatteryValues[ii])
		{

#ifdef _DEBUG
			/*Serial.print(F("if ")); Serial.print(value); Serial.print(F(" maggiore o uguale a ")); Serial.println(storedBatteryValues[ii]);
			Serial.print(F("metto ")); Serial.print(value); Serial.println(F(" in batteryMaxLevel"));
			 delay(1000);*/
#endif // _DEBUG


			batteryMaxLevel = value;
			ii++;
		}
		else
		{
#ifdef _DEBUG
			// Serial.print(F("mando ")); Serial.print(storedBatteryValues[ii]); Serial.println(F(" in funzione"));
#endif // _DEBUG
			checkBatteriesMaxLevel(storedBatteryValues[ii]);
		}
	}
}

void checkBatteriesMinLevel(float value)
{
	ii = 0;
	while (ii < numberOfBattery)
	{
		if (value <= storedBatteryValues[ii])
		{

#ifdef _DEBUG
			/*Serial.print(F("if ")); Serial.print(value); Serial.print(F(" maggiore o uguale a ")); Serial.println(storedBatteryValues[ii]);
			Serial.print(F("metto ")); Serial.print(value); Serial.println(F(" in batteryMaxLevel"));
			delay(1000);*/
#endif // _DEBUG
			batteryMinLevel = value;
			ii++;
		}
		else
		{
#ifdef _DEBUG
			// Serial.print(F("mando ")); Serial.print(storedBatteryValues[ii]); Serial.println(F(" in funzione"));
#endif // _DEBUG
			checkBatteriesMinLevel(storedBatteryValues[ii]);
		}
	}
}

void playMessageOnDPlayer(uint8_t messageCode)
{
	DFRobotDFPlayerMini myDFPlayer;
	SoftwareSerial mySoftwareSerial(A1, A2); // rx, tx
	delay(500);
	mySoftwareSerial.begin(9600);
	delay(500);

	if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.

#ifdef _DEBUG
		Serial.println(F("Unable to begin:"));
		Serial.println(F("1.Please recheck the connection!"));
		Serial.println(F("2.Please insert the SD card!"));
#endif // _DEBUG
		buzzerSensorActivity(5, 2500, 80, 200);
		while (true);
	}

#ifdef _DEBUG
	Serial.println(F("DFPlayer Mini online."));
#endif // _DEBUG
	uint16_t volume = (30.00 / 1024.00) * analogRead(A3);
	
	//Serial.println(volume);

	myDFPlayer.volume(volume);  //Set volume value. From 0 to 30

	myDFPlayer.play(messageCode);  //Play next mp3 every 3 second.
}