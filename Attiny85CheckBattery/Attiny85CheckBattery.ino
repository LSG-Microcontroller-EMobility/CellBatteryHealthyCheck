/*
 Name:		AtTiny85Test.ino
 Created:	3/3/2020 1:09:44 AM
 Author:	luigi.santagada
*/
  
#include <SoftwareSerial.h>
double measure = 0.00;
unsigned long idMessageCounter;

SoftwareSerial softwareSerial(99, 3, true);

void setup() {
	//delay(900);  // attiny85 reset delay 
	analogReference(EXTERNAL);
	softwareSerial.begin(600);
	for (int i = 0; i < 500; i++)
	{
		measure = measure + ((4.3 / 1024) * analogRead(A2));
	}
	measure = measure / 500;
	idMessageCounter = millis() / 10UL;
}

void loop() {
	softwareSerial.print(measure);
	softwareSerial.print(idMessageCounter);
	softwareSerial.print('*');
	delay(100);
}
