/*
 Name:		AtTiny85Test.ino
 Created:	3/3/2020 1:09:44 AM
 Author:	luigi.santagada
*/
#include <SoftwareSerial.h>
float volatile measure = 0.00f;
//Used for compatibility
const uint8_t idMessageCounter = 0;
const float _formula = (3.90f / 1024.00f);
SoftwareSerial softwareSerial(99, 3, true);
const uint8_t _reading_divider = 20;
//#define IS_ON_TEST
void setup() {
	analogReference(EXTERNAL);
	softwareSerial.begin(600);
	setupADC();
	pinMode(2, INPUT_PULLUP);
	attachInterrupt(0, activateSystemOnAlarmInterrupt, CHANGE);
	for (int i = 0; i < _reading_divider; i++) {
		measure += (_formula * analogRead(A2));
	}
	measure /= _reading_divider;
}
void loop() {
#ifdef IS_ON_TEST
	measure = 0.00f;
	for (int i = 0; i < _reading_divider; i++) {
		measure += (_formula * analogRead(A2));
	}
	measure /= _reading_divider;
#endif // IS_ON_TEST
	softwareSerial.print(measure);
	softwareSerial.print(idMessageCounter);
	softwareSerial.print('*');
	//importante perchè altrimenti non intercetta interrupt
	delay(100);
}
void setupADC() {
	ADMUX = (1 << REFS0);              // Usa AVcc come riferimento
	ADCSRA = (1 << ADEN)               // Abilita ADC
		| (1 << ADPS2) | (1 << ADPS1); // Imposta prescaler a 64
}
void activateSystemOnAlarmInterrupt(){
	measure = 00.00f;
	for (int i = 0; i < _reading_divider; i++) {
		measure += (_formula * analogRead(A2));
	}
	measure /= _reading_divider;
}
