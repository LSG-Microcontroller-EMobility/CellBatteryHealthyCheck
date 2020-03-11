
/*
 Name:		Receiver.ino
 Created:	3/8/2020 1:53:08 AM
 Author:	luigi.santagada
*/
#include <SoftwareSerial.h>
SoftwareSerial* softwareSerial = new SoftwareSerial(3, 10);
// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(9600);
	softwareSerial->begin(600);
}

// the loop function runs over and over again until power down or reset
void loop() {
	if (softwareSerial->available() > 0)
	{
		char response[6] = {};
		softwareSerial->readBytesUntil('*', response, 6);
		String e = response;
		e.trim();
		if (e != "")
		{
			Serial.println(e);
		}
	}

}
