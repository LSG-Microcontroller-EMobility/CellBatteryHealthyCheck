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

const uint8_t numberOfBattery = 3;

const uint8_t rxPin = 3;

const uint8_t resetAttiny85TransistorPin = 9;

uint8_t _pin_buzzer = 8;

//Pin 11 MOSI	Pin 12 MISO		Pin 13 SCK

const char* idBattery[numberOfBattery] = { "B0" , "B1", "B2" };

const double deltaVoltage[numberOfBattery] = { 0.00, 0.00, 0.00 };

double storedBatteryValues[numberOfBattery] = { 0.00,0.00,0.00 };

SoftwareSerial* softwareSerial = new SoftwareSerial(rxPin, 66);

File myFile;

uint8_t demultiplexerPosition;

uint8_t fileNumber = 0;

bool _isBuzzerDisabled = false;

String idCurrentMessage = "";

String responseString = "";

String csvString = "";

uint8_t numeroDisallineamenti = 0;

String _idMessage = "";

double batteryMaxLevel = 0.00f;

double batteryMinLevel = 0.00f;

uint8_t ii = 0;


void setup() {

	pinMode(resetAttiny85TransistorPin, OUTPUT);

	pinMode(selectorMultiPlex0, OUTPUT);

	pinMode(selectorMultiPlex1, OUTPUT);

	pinMode(selectorMultiPlex2, OUTPUT);

	pinMode(selectorMultiPlex3, OUTPUT);

	digitalWrite(selectorMultiPlex0, LOW);

	digitalWrite(selectorMultiPlex1, LOW);

	digitalWrite(selectorMultiPlex2, LOW);

	digitalWrite(selectorMultiPlex3, LOW);

	digitalWrite(resetAttiny85TransistorPin, HIGH);

	Serial.begin(9600);

	softwareSerial->begin(600);

	pinMode(8, OUTPUT);

	if (SD.begin())
	{
		Serial.println(F("SD card is ready to use."));
		for (uint8_t i = 0; i < 100; i++)
		{
			String fileName = "batt" + String(i) + ".csv";
			if (SD.exists(fileName))
			{
				Serial.println(F("File esiste"));
				fileNumber = i + 1;
			}
		}
		writeOnSDCard(F("IDMessage;Battery;Value;Delta;Origin"));
	}
	else
	{
		buzzerSensorActivity(5, 400, 1000, 500);
		Serial.println(F("SD card initialization failed"));
		return;
	}

	resetAttiny85();

}

void loop() {

	/*checkBatteriesMaxLevel(storedBatteryValues[0]);

	checkBatteriesMinLevel(storedBatteryValues[0]);*/

	/*Serial.print(F("--------------------------------Il valore massimo e' : ")); Serial.println(batteryMaxLevel);

	Serial.print(F("--------------------------------Il valore minimo e' : ")); Serial.println(batteryMinLevel);*/

	//delay(1000);

	//return;

	storedBatteryValues[0] = '\0';

	demultiplexerPosition = 0;

	setMultiplexer(demultiplexerPosition);

	responseString = "";

	responseString = getDataFromSerialBuffer();

	if (responseString != "")
	{
		Serial.println(F("-----------------START------------------------"));

		_idMessage = responseString.substring(5, 6);

		csvString = "";

		csvString = prepareStringForSDCard(responseString, demultiplexerPosition);

		if (csvString.length() != 21)
		{
			Serial.print(F("disallineamento - ")); Serial.println(responseString);
			return;
		}

		writeOnSDCard(csvString);

		++demultiplexerPosition;

		bool condition = true;

		numeroDisallineamenti = 0;

		unsigned long timeForSerialData = millis();

		clearSerialBuffer();

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
					Serial.print(F("disallineamento - ")); Serial.println(responseString);
				}


			}
		}
		buzzerSensorActivity(15, 1000, 100, 100);

		checkBatteriesMaxLevel(storedBatteryValues[0]);

		checkBatteriesMinLevel(storedBatteryValues[0]);

		resetAttiny85();
		clearSerialBuffer();
		idCurrentMessage = "";
		Serial.println(F("-----------------END------------------------"));
	}
	else
	{
		buzzerSensorActivity(1, 400, 100, 100);
		buzzerSensorActivity(1, 200, 100, 100);
		buzzerSensorActivity(1, 500, 100, 100);
	}

}

double getNumber(String responseString)
{
	String stringNumber = responseString.substring(1, 5);

	char id[5];

	stringNumber.toCharArray(id, 5);

	double number;

	number = atof(id);

	//Serial.print(F("checkidCurrentMessage ->stringNumber : ")); Serial.println(stringNumber);

	//Serial.print(F("checkidCurrentMessage ->number : ")); Serial.println(number);

	if (number > 0.00f && number < 100.00f) {
		return number;
	}

	//Serial.println(F("numero non valido"));

	buzzerSensorActivity(1, 400, 100, 100);
	buzzerSensorActivity(1, 200, 100, 100);
	buzzerSensorActivity(1, 500, 100, 100);

	return -1;
}

String getDataFromSerialBuffer()
{
	char response[7];
	response[0] = '\0';
	String responseString = "";
	if (softwareSerial->available() > 0)
	{
		softwareSerial->readBytesUntil('*', response, 7);
		responseString = response;
		responseString.trim();
		Serial.print(F("data from attiny85 : ")); Serial.println(responseString);
		return responseString;
	}
	return "";
}

void setMultiplexer(int channel) {
	//Serial.print("setMultiplexer channel : "); Serial.println(channel);
	int controlPin[] = { selectorMultiPlex0, selectorMultiPlex1, selectorMultiPlex2, selectorMultiPlex3 };

	int muxChannel[16][4] = {
	  {0,0,0,0}, //channel 0
	  {1,0,0,0}, //channel 1
	  {0,1,0,0}, //channel 2
	  {1,1,0,0}, //channel 3
	  {0,0,1,0}, //channel 4
	  {1,0,1,0}, //channel 5
	  {0,1,1,0}, //channel 6
	  {1,1,1,0}, //channel 7
	  {0,0,0,1}, //channel 8
	  {1,0,0,1}, //channel 9
	  {0,1,0,1}, //channel 10
	  {1,1,0,1}, //channel 11
	  {0,0,1,1}, //channel 12
	  {1,0,1,1}, //channel 13
	  {0,1,1,1}, //channel 14
	  {1,1,1,1}  //channel 15
	};

	//loop through the 4 sig
	for (int i = 0; i < 4; i++) {
		digitalWrite(controlPin[i], muxChannel[channel][i]);
	}
}

String prepareStringForSDCard(String message, uint8_t demultiplexerPosition)
{
	String csvString = "";
	double number = getNumber(message);
	number = number + deltaVoltage[demultiplexerPosition];
	csvString = message.substring(5, 6) + ";" + String(idBattery[demultiplexerPosition]) + ";" + String(number) + ";" + String(deltaVoltage[demultiplexerPosition]) + ";" + message;
	Serial.println(csvString);
	storedBatteryValues[demultiplexerPosition] = number;
	//writeOnSDCard(csvString);
	return csvString;
}

void writeOnSDCard(String message)
{
	// Create/Open file 
	String fileName = "batt" + String(fileNumber) + ".csv";
	Serial.print(F("Apro file:")); Serial.print(fileName);
	myFile = SD.open(fileName, FILE_WRITE);
	if (myFile) {
		myFile.println(message);
		Serial.println(F("Scrivo su card"));
		myFile.close();
	}

	else {
		buzzerSensorActivity(5, 400, 1000, 500);
		Serial.println(F("error opening battery.cvs"));
		myFile.close();
	}

	//// Reading the file
	//myFile = SD.open("batteryValues.csv");
	//if (myFile) {
	//	//Serial.println("Read:");
	//	// Reading the whole file
	//	while (myFile.available()) {
	//		Serial.write(myFile.read());
	//	}
	//	myFile.close();
	//}
	//else {
	//	Serial.println("error opening file batteryValues.csv");
	//}

}

void resetAttiny85()
{
	digitalWrite(resetAttiny85TransistorPin, LOW);
	delay(500);
	digitalWrite(resetAttiny85TransistorPin, HIGH);
}

void clearSerialBuffer()
{
	for (int i = 0; i < 10000; i++)
	{
		softwareSerial->read();
	}
}

void buzzerSensorActivity(uint8_t numberOfCicle, unsigned int frequency, unsigned long duration, uint16_t pause)
{
	if (_isBuzzerDisabled == true) { return; }
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
	//Serial.println(F(" entro"));
	ii = 0;
	while (ii < numberOfBattery)
	{
		//Serial.println(ii);

		if (value >= storedBatteryValues[ii])
		{
			/*	Serial.print(F("if ")); Serial.print(value); Serial.print(F(" maggiore o uguale a ")); Serial.println(storedBatteryValues[ii]);
				Serial.print(F("metto ")); Serial.print(value); Serial.println(F(" in batteryMaxLevel"));*/
				//delay(1000);
			batteryMaxLevel = value;
			ii++;
		}
		else
		{
			//Serial.print(F("mando ")); Serial.print(storedBatteryValues[ii]); Serial.println(F(" in funzione"));
			checkBatteriesMaxLevel(storedBatteryValues[ii]);
		}

	}

}

void checkBatteriesMinLevel(double value)
{
	//Serial.println(F(" entro"));
	ii = 0;
	while (ii < numberOfBattery)
	{
		//Serial.println(ii);
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
			//Serial.print(F("mando ")); Serial.print(storedBatteryValues[ii]); Serial.println(F(" in funzione"));
			checkBatteriesMinLevel(storedBatteryValues[ii]);
		}
	}
}