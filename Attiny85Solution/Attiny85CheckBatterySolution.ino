/*
 Name:		AtTiny85Test.ino
 Created:	3/3/2020 1:09:44 AM
 Author:	luigi.santagada
*/

//#define CLKOUT      1  
#include <SoftwareSerial.h>
//unsigned long pulse = 0;
double measure = 0.00; 
uint8_t idMessageCounter;

SoftwareSerial softwareSerial(99, 3, true);

void setup() {
	analogReference(EXTERNAL);
	softwareSerial.begin(600);
	//softwareSerial.println("Begin");
	//setPWM();
	//pinMode(CLKOUT, OUTPUT);  // Set pin as output
	delay(1000);
	idMessageCounter  = millis() / 100;
	measure = ((4.3 / 1024)*analogRead(A2));
	
}

// the loop function runs over and over again until power down or reset
void loop() {
	//measure =((4.3 / 1024)*analogRead(A2));// -0.4;     
	//delay(1000);
	if (idMessageCounter == 100) idMessageCounter = 1;
	if (idMessageCounter < 10) softwareSerial.print('0');
	softwareSerial.print(measure);
	softwareSerial.print(idMessageCounter);
	softwareSerial.print('*');
}


long getVoltage() {//this function was taken from: http://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/

	ADMUX = _BV(MUX3) | _BV(MUX2);

	delay(2); // Wait for Vref to settle
	ADCSRA |= _BV(ADSC); // Start conversion
	while (bit_is_set(ADCSRA, ADSC)); // measuring

	uint8_t low = ADCL; // must read ADCL first - it then locks ADCH  
	uint8_t high = ADCH; // unlocks both

	long result = (high << 8) | low;

	result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000

	ADMUX &= ~(REFS0);
	ADMUX &= ~(REFS1);

	return result; // Vcc in millivolts
}



//void setPWM()
//{
//	OCR1C = 255; // Yields approx. 16kHz to 48kHz freq range
//	OCR1A = 127;  // 50% duty cycle
//	TCCR1 = _BV(PWM1A) | _BV(COM1A0) | _BV(COM1A1) | _BV(CS10);
//	TIMSK |= (1 << TOIE1);
//}
//
//ISR(TIMER1_OVF_vect) {              // Interrupt vector for TIMER-1 OVR which set 50% period
//	pulse++;
//	if (pulse >= 600000)
//	{
//		idMessageCounter++;
//		measure = ((2.56 / 1024)*analogRead(A2)) + 0.04;
//		pulse = 0;
//	}
//
//}