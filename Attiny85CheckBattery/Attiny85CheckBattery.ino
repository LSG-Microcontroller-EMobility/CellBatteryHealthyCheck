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
 
  analogReference(EXTERNAL);

  softwareSerial.begin(600);

  cli();

  PCMSK |= bit(PCINT2);  // want pin D3 / pin 2
  GIFR |= bit(PCIF);    // clear any outstanding interrupts
  GIMSK |= bit(PCIE);    // enable pin change interrupts

  //printData("ID", false); printData(identifyNumber, true);

  pinMode(2, INPUT_PULLUP);

  attachInterrupt(0, activateSystemOnAlarmInterrupt, CHANGE);

  sei();

  

  for (int i = 0; i < 1000; i++) {
      measure = measure + ((3.9 / 1024) * analogRead(A2));
  }
  measure = measure / 1000;
  idMessageCounter = 0;
}

void loop() {
  softwareSerial.print(measure);
  softwareSerial.print(idMessageCounter);
  softwareSerial.print('*');
  //importante perchè altrimenti non intercetta interrupt
  delay(100);
}

void activateSystemOnAlarmInterrupt()
{
    softwareSerial.println("interrupt");
    for (int i = 0; i < 1000; i++) {
        measure = measure + ((3.9 / 1024) * analogRead(A2));
    }
    measure = measure / 1000;
    idMessageCounter = 0;
}
