
/*
 Name:		Receiver.ino
 Created:	3/8/2020 1:53:08 AM
 Author:	luigi.santagada
*/
#include <SoftwareSerial.h>
#include <SD.h>
#include <SPI.h>

String idMessageCounter = "";
const char* idBattery[16] = {"B1","B2","B3","B4","B5","B6","B7","B8","B9","B10","B11","B12","B13", "B14","B15","B16"};
SoftwareSerial* softwareSerial = new SoftwareSerial(3, 10);
File myFile;

void setup() {
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
void loop() {
	if (softwareSerial->available() > 0)
	{
		char response[6] = {};
		softwareSerial->readBytesUntil('*', response, 6);
		String e = response;
		e.trim();
		if (e != "")
		{
			String a;
			if (!e.substring(0, 2).equals(idMessageCounter))
			{
				idMessageCounter = e.substring(0, 2);
				/*Serial.print("Store value ID record :"); Serial.print(idMessageCounter);Serial.print(" on SD card ");
				Serial.print("id battery : "); Serial.print(idBattery[0]);
				Serial.print(" data aquired : "); Serial.println(e.substring(2, 6));*/
				a = e.substring(0, 2) + ";" + String(idBattery[0]) + ";" + e.substring(2, 6);
				Serial.println(a);
				writeOnSDCard(a);
			}
			//if(e.substring(0, 2) != idMessageCounter)
			//{
			//	idMessageCounter = e.substring(0, 2);
			//	//Serial.println("cambio valore");
			//}
		}
	}

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
