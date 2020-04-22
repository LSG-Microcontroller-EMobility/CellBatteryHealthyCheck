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

const uint8_t numberOfBattery = 2;

const uint8_t rxPin = 3;

const uint8_t resetAttiny85TransistorPin = 9;

//Pin 11 MOSI	Pin 12 MISO		Pin 13 SCK



//const char* idBattery[numberOfBattery] = { "B1","B2","B3","B4","B5","B6","B7","B8","B9","B10","B11","B12","B13", "B14","B15","B16" };
const char* idBattery[numberOfBattery] = { "B0","B1" };// , "B2", "B3", "B4", "B5", "B6", "B7" }; //"B2", "B3", "B4", "B5", "B6", "B7" };// , "B3", "B4", "B5", "B6", "B7", "B8", "B9", "B10", "B11", "B12", "B13", "B14", "B15", "B16" };

SoftwareSerial* softwareSerial = new SoftwareSerial(rxPin, 66);

File myFile;

uint8_t demultiplexerPosition;

uint8_t fileNumber = 0;

void setup() {

	demultiplexerPosition = 0;

	pinMode(resetAttiny85TransistorPin, OUTPUT);

	pinMode(selectorMultiPlex0, OUTPUT);

	pinMode(selectorMultiPlex1, OUTPUT);

	pinMode(selectorMultiPlex2, OUTPUT);

	pinMode(selectorMultiPlex3, OUTPUT);

	digitalWrite(selectorMultiPlex0, LOW);

	digitalWrite(selectorMultiPlex1, LOW);

	digitalWrite(selectorMultiPlex2, LOW);

	digitalWrite(selectorMultiPlex3, LOW);

	digitalWrite(resetAttiny85TransistorPin, LOW);

	Serial.begin(9600);

	softwareSerial->begin(600);

	if (SD.begin())
	{
		Serial.println("SD card is ready to use.");
		for (uint8_t i = 0; i < 100; i++)
		{
			String fileName = "battery" + String(i) + ".csv";
			if (SD.exists(fileName))
			{
				Serial.println("File esiste");
				fileNumber = i + 1;
			}
		}
		//SD.remove("battery.csv");
	}
	else
	{
		Serial.println("SD card initialization failed");
		return;
	}
}

String idCurrentMessage = "";

String responseString = "";

String csvString = "";

uint8_t numeroDisallineamenti = 0;

void loop() {

	demultiplexerPosition = 0;

	setMultiplexer(demultiplexerPosition);

	responseString = "";

	responseString = getDataFromSerialBuffer();

	responseString.trim();

	/*if (responseString != "" && (!idCurrentMessage.equals(responseString.substring(0, 2))))*/
	if (responseString != "" && 
		((idCurrentMessage.compareTo(responseString.substring(0, 2)) != 0)))
	{
		if (!checkidCurrentMessage(responseString.substring(0, 2))) { return; };

		//if (responseString.substring(2, 6) == "0.00") { return; };

		idCurrentMessage = responseString.substring(0, 2);

		csvString = "";

		csvString = prepareStringForSDCard(responseString, demultiplexerPosition);

		if (csvString.length() != 10) 
		{ 
			//Serial.print("disallineamento su lettura iniziale :  "); Serial.println(responseString);
			return; }

		writeOnSDCard(csvString);

		++demultiplexerPosition;

		bool condition = true;

		numeroDisallineamenti = 0;

		unsigned long timeForSerialData = millis();


		while ((millis() - timeForSerialData < 1500) && (demultiplexerPosition < numberOfBattery) && condition)
		{
			//Serial.println(demultiplexerPosition);

			setMultiplexer(demultiplexerPosition);

			responseString = "";

			responseString = getDataFromSerialBuffer();

			responseString.trim();

			if (responseString != "")
			{
				//Serial.println(responseString);
				if (idCurrentMessage.compareTo(responseString.substring(0, 2)) != 0)
				{
					numeroDisallineamenti++;
					//Serial.print("disallineamento su loop :  "); Serial.println(responseString);
					timeForSerialData = millis();
					if (numeroDisallineamenti > 3)
					{
						condition = false;
						digitalWrite(resetAttiny85TransistorPin, HIGH);
						delay(500);
						digitalWrite(resetAttiny85TransistorPin, LOW);
						idCurrentMessage = "";
					}
				}
				else
				{
					numeroDisallineamenti = 0;

					String csvString = "";

					csvString = prepareStringForSDCard(responseString, demultiplexerPosition);

					writeOnSDCard(csvString);

					timeForSerialData = millis();

					++demultiplexerPosition;
				}

			}
		}
		digitalWrite(resetAttiny85TransistorPin, HIGH);
		delay(500);
		digitalWrite(resetAttiny85TransistorPin, LOW);
		idCurrentMessage = "";
		//Serial.println("reset all attiny85"); 
	}

}

bool checkidCurrentMessage(String idCurrentMessage)
{
	char id[2];

	idCurrentMessage.toCharArray(id, 3);

	uint8_t number;

	number = atoi(id);

	//Serial.println(idCurrentMessage);

	//Serial.println(number);

	if (number > 0 && number < 100) return true;

	//Serial.println("numero non valido");

	return false;
}

String getDataFromSerialBuffer()
{
	char response[6];
	response[0] = '\0';
	String responseString = "";
	if (softwareSerial->available() > 0)
	{
		softwareSerial->readBytesUntil('*', response, 6);
		responseString = response;
		responseString.trim();
		//Serial.println(responseString);
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

	////read the value at the SIG pin
	//int val = analogRead(A0);

	//return the value
	//return val;
}

String prepareStringForSDCard(String message, uint8_t demultiplexerPosition)
{
	String csvString = "";
	csvString = message.substring(0, 2) + ";" + String(idBattery[demultiplexerPosition]) + ";" + message.substring(2, 6);
	Serial.println(csvString);
	//writeOnSDCard(csvString);
	return csvString;
}

void writeOnSDCard(String message)
{
	// Create/Open file 
	String fileName = "battery" + String(fileNumber) + ".csv";
	myFile = SD.open(fileName, FILE_WRITE);
	if (myFile) {
		myFile.println(message);
		Serial.println("Scrivo su card");
		myFile.close();
	}

	else {
		Serial.println("error opening battery.cvs");
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
