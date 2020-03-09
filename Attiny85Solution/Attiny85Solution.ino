/*
 Name:		AtTiny85Test.ino
 Created:	3/3/2020 1:09:44 AM
 Author:	luigi.santagada
*/
#include <eRCaGuy_analogReadXXbit.h>
#include <SoftwareSerial.h>

SoftwareSerial softwareSerial(10, 3, false);
//eRCaGuy_analogReadXXbit* a = new eRCaGuy_analogReadXXbit();
// the setup function runs once when you press reset or power the board
void setup() {
	softwareSerial.begin(38400);
	analogReference(INTERNAL1V1);
}

// the loop function runs over and over again until power down or reset
void loop() {
	//float b = a->analogReadXXbit(A2, 10, 5);
	//softwareSerial.println((2.56 /1024)*b);
	///*softwareSerial.println(b);*/
	/*softwareSerial.print("------");*/
	softwareSerial.println((1.1 / 1024)*analogRead(A2) + 0.04);
	//delay(1000);
}