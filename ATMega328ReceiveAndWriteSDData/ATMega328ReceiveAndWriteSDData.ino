/*
 Name:		Receiver.ino
 Created:	3/8/2020 1:53:08 AM
 Author:	luigi.santagada
*/
#include <SoftwareSerial.h>
#include <SD.h>
#include <SPI.h>
#include <string.h>

const uint8_t selectorMultiPlex0 = 4;

const uint8_t selectorMultiPlex1 = 5;

const uint8_t selectorMultiPlex2 = 6;

const uint8_t selectorMultiPlex3 = 7;

const uint8_t numberOfBattery = 6;

const uint8_t _pin_rx = 3;

const uint8_t _pin_reset_attiny85 = 9;

uint8_t _pin_buzzer = 8;

// Pin 11 MOSI	Pin 12 MISO		Pin 13 SCK

const char *idBattery[numberOfBattery] = {"B0", "B1", "B2", "B3", "B4", "B5"}; //, "B1", "B2" };

const double deltaVoltage[numberOfBattery] = {0.00, 0.00, 0.00, 0.00, 0.00, 0.00}; //, 0.00, 0.00 };

double storedBatteryValues[numberOfBattery] = {0.00, 0.00, 0.00, 0.00, 0.00, 0.00}; //,0.00,0.00 };

File myFile;

uint8_t demultiplexerPosition = 0;

uint8_t fileNumber = 0;

bool _isBuzzerDisabled = true;

bool _isFileCardWritingDisable = true;

uint8_t numeroDisallineamenti = 0;

char _idMessage[1] = {};

double batteryMaxLevel = 0.00f;

double batteryMinLevel = 0.00f;

uint8_t ii = 0;

void setup()
{

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
}

char fileName[15] = {};

void initFileCard()
{
	if (_isFileCardWritingDisable)
		return;

	if (SD.begin())
	{
		Serial.println(F("card ready"));

		for (uint8_t i = 0; i < 1; i++)
		{
			strcpy(fileName, "batt");
			fileName[4] = (char)(i + 48);
			strcat(fileName, ".csv");
			strcat(fileName, "\0");
			Serial.println(fileName);
			if (SD.exists(fileName))
			{
				Serial.println(F("File esiste"));
				SD.remove(fileName);
			}
		}
		char headersText[37] = "IDMessage;Battery;Value;Delta;Origin";

		writeOnSDCard(headersText);
	}
	else
	{
		buzzerSensorActivity(5, 400, 1000, 500);
		Serial.println(F("SD card initialization failed"));
		return;
	}
}

void loop()
{
	Serial.println(F("giro"));

	resetAttiny85();

	storedBatteryValues[0] = {};

	setMultiplexer(demultiplexerPosition);

	delay(1000);

	char response[7] = {};

	getDataFromSerialBuffer(response);

	/* Serial.print('#'); */
	/* Serial.print(response); */
	/* Serial.println('#'); */

	if (!is_number(response))
	{
		Serial.println(F("resp.not.num"));
		// return;
	}

	_idMessage[0] = response[4];

	Serial.println(_idMessage[0]);

	char csvTextLayOut[21] = {};

	prepareStringForSDCard(csvTextLayOut, response, demultiplexerPosition);

	if (csvTextLayOut[20] == '\0')
	{
		Serial.println(F("text.problem"));
		return;
	}

	if (csvTextLayOut[19] == '\0' && csvTextLayOut[20] != '\0')
	{
		Serial.println(F("text.problem"));
		// return;
	}

	writeOnSDCard(csvTextLayOut);

	if (demultiplexerPosition == 5)
	{
		demultiplexerPosition = 0;
	}
	else
	{
		demultiplexerPosition++;
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

bool is_number(const String &s)
{
	char *end = nullptr;
	double val = strtod(s.c_str(), &end);
	return end != s.c_str() && *end == '\0' /* && val != 0.00 */;
}

/// <summary>
/// maxPercentageForAlarm is the percentage of minValue in maxValue
/// </summary>
/// <param name="maxPercentageForAlarm"></param>
/// <returns></returns>
bool thereAreUnbalancedBatteries(uint8_t maxPercentageForAlarm)
{
	double percentageValue = (batteryMinLevel / batteryMaxLevel) * 100;
	Serial.print(F("Percentage value : "));
	Serial.print(percentageValue);
	Serial.println(F("%"));
	if (percentageValue < maxPercentageForAlarm)
	{
		return true;
	}
	return false;
}

double getNumber(char *response)
{
	return atof(response);
}

void getDataFromSerialBuffer(char *response)
{
	SoftwareSerial softwareSerial(3, 99);

	softwareSerial.begin(600);

	response[0] = {};

	delay(500);

	if (softwareSerial.available() > 0)
	{
		softwareSerial.readStringUntil('*');
	}
	if (softwareSerial.available() > 0)
	{
		softwareSerial.readBytesUntil('*', response, 7);

		Serial.print(F("data from attiny85 : "));
		Serial.println(response);
	}
}

void setMultiplexer(int channel)
{
	// Serial.print("setMultiplexer channel : "); Serial.println(channel);
	int controlPin[] = {selectorMultiPlex0, selectorMultiPlex1, selectorMultiPlex2, selectorMultiPlex3};

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

void prepareStringForSDCard(char *csvTextLayOut, char *response, uint8_t demultiplexerPosition)
{
	char num_to_string[4] = {};

	char deltaVoltage_to_string[4] = {};

	response[4] = '\0';

	double number = getNumber(response);

	Serial.println(number);

	number = number + deltaVoltage[demultiplexerPosition];

	dtostrf(number, 4, 2, num_to_string);

	dtostrf(deltaVoltage[demultiplexerPosition], 4, 2, deltaVoltage_to_string);

	csvTextLayOut[0] = _idMessage[0];
	strcat(csvTextLayOut, ";");
	strcat(csvTextLayOut, idBattery[demultiplexerPosition]);
	strcat(csvTextLayOut, ";");
	strcat(csvTextLayOut, num_to_string);
	strcat(csvTextLayOut, ";");
	strcat(csvTextLayOut, deltaVoltage_to_string);
	strcat(csvTextLayOut, ";");
	strcat(csvTextLayOut, response);
	strcat(csvTextLayOut, _idMessage);
	csvTextLayOut[20] = '\0';
	Serial.println(csvTextLayOut);
	storedBatteryValues[demultiplexerPosition] = number;
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

void writeOnSDCard(char *message)
{
	if (_isFileCardWritingDisable)
		return;
	// Create/Open file
	// String fileName = "batt" + String(fileNumber) + ".csv";
	// Serial.print(F("Apro file:"));
	// Serial.println(fileName);
	myFile = SD.open(fileName, FILE_WRITE);
	if (myFile)
	{
		myFile.println(message);
		Serial.println(F("Scrivo"));
		myFile.close();
	}

	else
	{
		buzzerSensorActivity(5, 400, 1000, 500);
		Serial.println(F("error opening battery.cvs"));
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

/* void clearSerialBuffer() */
/* { */
/* 	for (int i = 0; i < 100; i++) */
/* 	{ */
/* 		softwareSerial->read(); */
/* 	} */
/* }  */

void buzzerSensorActivity(uint8_t numberOfCicle, unsigned int frequency, unsigned long duration, uint16_t pause)
{
	if (_isBuzzerDisabled)
		return;

	for (uint8_t i = 0; i < numberOfCicle; i++)
	{
		tone(_pin_buzzer, frequency, duration);
		delay(duration);
		delay(pause);
		noTone(_pin_buzzer);
	}
	delay(pause);
}

void checkBatteriesMaxLevel(double value)
{
	// Serial.println(F(" entro"));
	ii = 0;
	while (ii < numberOfBattery)
	{
		// Serial.println(ii);

		if (value >= storedBatteryValues[ii])
		{
			/*	Serial.print(F("if ")); Serial.print(value); Serial.print(F(" maggiore o uguale a ")); Serial.println(storedBatteryValues[ii]);
				Serial.print(F("metto ")); Serial.print(value); Serial.println(F(" in batteryMaxLevel"));*/
			// delay(1000);
			batteryMaxLevel = value;
			ii++;
		}
		else
		{
			// Serial.print(F("mando ")); Serial.print(storedBatteryValues[ii]); Serial.println(F(" in funzione"));
			checkBatteriesMaxLevel(storedBatteryValues[ii]);
		}
	}
}

void checkBatteriesMinLevel(double value)
{
	// Serial.println(F(" entro"));
	ii = 0;
	while (ii < numberOfBattery)
	{
		// Serial.println(ii);
		if (value <= storedBatteryValues[ii])
		{
			/*Serial.print(F("if ")); Serial.print(value); Serial.print(F(" maggiore o uguale a ")); Serial.println(storedBatteryValues[ii]);
			Serial.print(F("metto ")); Serial.print(value); Serial.println(F(" in batteryMaxLevel"));
			delay(1000);*/
			batteryMinLevel = value;
			ii++;
		}
		else
		{
			// Serial.print(F("mando ")); Serial.print(storedBatteryValues[ii]); Serial.println(F(" in funzione"));
			checkBatteriesMinLevel(storedBatteryValues[ii]);
		}
	}
}