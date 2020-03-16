/*
 Name:		Receiver.ino
 Created:	3/8/2020 1:53:08 AM
 Author:	luigi.santagada
*/
#include <SoftwareSerial.h>
#include <SD.h>
#include <SPI.h>

int s0 = 4;
int s1 = 5;
int s2 = 6;
int s3 = 7;

const uint8_t numberOfBattery = 16;
const char* idBattery[numberOfBattery] = { "B1","B2","B3","B4","B5","B6","B7","B8","B9","B10","B11","B12","B13", "B14","B15","B16" };
SoftwareSerial* softwareSerial = new SoftwareSerial(3, 10);
File myFile;

void setup() {

	pinMode(s0, OUTPUT);
	pinMode(s1, OUTPUT);
	pinMode(s2, OUTPUT);
	pinMode(s3, OUTPUT);

	digitalWrite(s0, LOW);
	digitalWrite(s1, LOW);
	digitalWrite(s2, LOW);
	digitalWrite(s3, LOW);

	Serial.begin(9600);

	softwareSerial->begin(600);

	if (SD.begin())
	{
		Serial.println("SD card is ready to use.");
		SD.remove("battery.csv");
	}
	else
	{
		Serial.println("SD card initialization failed");
		return;
	}
}

uint8_t demultiplexerPosition;

void loop() {
	demultiplexerPosition = 0;

	setMultiplexer(demultiplexerPosition);

	String responseString = "";

	String idCurrentMessage = "";

	responseString = getDataFromSerialBuffer();

	if (responseString != "")
	{
		String csvString = "";

		csvString = prepareStringForSDCard(responseString, demultiplexerPosition);

		writeOnSDCard(csvString);

		idCurrentMessage = responseString.substring(0, 2);

		while (responseString.substring(0, 2).equals(idCurrentMessage))
		{
			++demultiplexerPosition;

			setMultiplexer(demultiplexerPosition);

			responseString = getDataFromSerialBuffer();

			if (idCurrentMessage.equals(responseString.substring(0, 2)))
			{
				csvString = prepareStringForSDCard(responseString, demultiplexerPosition);

				writeOnSDCard(csvString);
			}
		}
		//for (uint8_t i = 0; i < numberOfBattery; i++)
		//{
		//	readMux(i);
		//	//delay(500);
				//
		/*Serial.print("Store value ID record :"); Serial.print(idMessageCounter);Serial.print(" on SD card ");
		Serial.print("id battery : "); Serial.print(idBattery[0]);
		Serial.print(" data aquired : "); Serial.println(e.substring(2, 6));*/
		//	/*csvString = responseString.substring(0, 2) + ";" + String(idBattery[i]) + ";" + responseString.substring(2, 6);
		//	Serial.println(csvString);
		//	writeOnSDCard(csvString);*/
		//	//}
		////}
		////if(e.substring(0, 2) != idMessageCounter)
		////{
		////	idMessageCounter = e.substring(0, 2);
		////	//Serial.println("cambio valore");
		////}
		//}
		//if (i == (numberOfBattery - 1))
		//{
		//	idCurrentMessage = responseString.substring(0, 2);
		//}
	}
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
		return responseString;
	}
	return "";
}

void setMultiplexer(int channel) {
	int controlPin[] = { s0, s1, s2, s3 };

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
	myFile = SD.open("battery.csv", FILE_WRITE);
	if (myFile) {
		myFile.println(message);
		myFile.close();
	}

	else {
		Serial.println("error opening battery.csv");
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
