/*
 Name:		Receiver.ino
 Created:	3/8/2020 1:53:08 AM
 Author:	luigi.santagada
*/
#include "SoftwareSerial.h"
#include <SD.h>
#include <SPI.h>
#include <string.h>
#include <DFRobotDFPlayerMini.h>

#define AUDIO_DISLIVELLO_BATTERIE 1

#define AUDIO_TRACCIA_ERRATA 2

#define AUDIO_NUMERO_ERRATO 3

#define AUDIO_PROBLEMA_SCHEDA_MEMORIA 4

#define AUDIO_DATI_SUFFICIENTI 5

#define AUDIO_SISTEMA_INIZIALIZZATO 6

#define AUDIO_ID_MESSAGE_WRONG 7

#define AUDIO_AQUISIZIONE_DATI 9

#define AUDIO_SCHEDA_MEM_PIENA 10

#define AUDIO_DELETE_FILES_30_SEC 11

#define AUDIO_DELETE_FILES_10_SEC 12

#define AUDIO_ALL_FILES_DELETED 13	

const uint8_t numberOfBattery = 6;

const uint8_t _pin_selectorMultiPlex0 = 4;

const uint8_t _pin_selectorMultiPlex1 = 5;

const uint8_t _pin_selectorMultiPlex2 = 6;

const uint8_t _pin_selectorMultiPlex3 = 7;

const uint8_t _pin_rx = 3;

const uint8_t _pin_interrupt_to_attiny85 = 9;

//const uint8_t _pin_buzzer = 8;

const uint8_t _pin_dfMiniPlayer_rx = A1;

const uint8_t _pin_dfMiniPlayer_tx = A2;

const uint8_t _pin_dfMiniPlayer_volume = A3;

const uint8_t _pin_maxBatteryVoltageDifference = A4;

uint8_t controlPin[4] = { _pin_selectorMultiPlex0, _pin_selectorMultiPlex1, _pin_selectorMultiPlex2, _pin_selectorMultiPlex3 };

const uint8_t muxChannel[6][4] = {
	{0, 0, 0, 0}, // channel 0
	{1, 0, 0, 0}, // channel 1
	{0, 1, 0, 0}, // channel 2
	{1, 1, 0, 0}, // channel 3
	{0, 0, 1, 0}, // channel 4
	{1, 0, 1, 0}, // channel 5
	//{0, 1, 1, 0}, // channel 6
	//{1, 1, 1, 0}, // channel 7
	//{0, 0, 0, 1}, // channel 8
	//{1, 0, 0, 1}, // channel 9
	//{0, 1, 0, 1}, // channel 10
	//{1, 1, 0, 1}, // channel 11
	//{0, 0, 1, 1}, // channel 12
	//{1, 0, 1, 1}, // channel 13
	//{0, 1, 1, 1}, // channel 14
	//{1, 1, 1, 1}  // channel 15
};

uint8_t total_takeovers = 0;

const uint8_t max_total_takeovers = 30;


//-----------------------    ATTENZIONE PIN ASSEGNATI a scheda SD file excel !!!!!!!   -------------------------------
// Pin 11 MOSI	Pin 12 MISO		Pin 13 SCK

const char* idBattery[numberOfBattery] = { "B0", "B1", "B2", "B3", "B4", "B5" }; //, "B1", "B2" };

const float deltaVoltage[numberOfBattery] = { 0.00, 0.00, 0.00, 0.00, 0.00, 0.00 }; //, 0.00, 0.00 };

float storedBatteryValues[numberOfBattery] = { 0.00, 0.00, 0.00, 0.00, 0.00, 0.00 }; //,0.00,0.00 };

uint8_t _demultiplexerPosition = 0;

uint8_t fileNumber = 0;

bool _is_buzzer_disabled = true;

bool _is_card_writing_disable = false;

uint8_t numeroDisallineamenti = 0;

char _idMessage[1] = { 'x' };

float batteryMaxLevel = 0.00f;

float batteryMinLevel = 0.00f;

const uint8_t max_files_numbers = 9;  //MAX 9

uint8_t ii = 0;

void setup()
{
	Send_Interrupt_To_All_Attiny85();

	delay(2000);

	pinMode(_pin_interrupt_to_attiny85, OUTPUT);

	pinMode(_pin_selectorMultiPlex0, OUTPUT);

	pinMode(_pin_selectorMultiPlex1, OUTPUT);

	pinMode(_pin_selectorMultiPlex2, OUTPUT);

	pinMode(_pin_selectorMultiPlex3, OUTPUT);

	digitalWrite(_pin_selectorMultiPlex0, LOW);

	digitalWrite(_pin_selectorMultiPlex1, LOW);

	digitalWrite(_pin_selectorMultiPlex2, LOW);

	digitalWrite(_pin_selectorMultiPlex3, LOW);

	digitalWrite(_pin_interrupt_to_attiny85, LOW);

	Serial.begin(9600);

	/*pinMode(_pin_buzzer, OUTPUT);*/

	initFileCard();

	//Serial.println(F("restart"));

	playMessageOnDPlayer(AUDIO_SISTEMA_INIZIALIZZATO);


}

char fileName[15] = {};

void initFileCard()
{
	if (_is_card_writing_disable) return;

	if (SD.begin())
	{
		bool exit = false;
		uint8_t cicle = 0;
		//Serial.println(F("card ready"));
		while (cicle < max_files_numbers && !exit)
		{
			strcpy(fileName, "batt");
			fileName[4] = (char)(cicle + 48);
			strcat(fileName, ".csv");
			fileName[9] = '\0';
			
#ifdef _DEBUG
			Serial.println(fileName);
#endif // _DEBUG
			if (SD.exists(fileName))
			{
#ifdef _DEBUG
				Serial.println(F("File esiste"));
#endif // _DEBUG
				cicle++;
				if (cicle == max_files_numbers)
				{
					playMessageOnDPlayer(AUDIO_SCHEDA_MEM_PIENA);
					playMessageOnDPlayer(AUDIO_DELETE_FILES_30_SEC);
					delay(20);
					playMessageOnDPlayer(AUDIO_DELETE_FILES_10_SEC);
					delay(10);
					for (uint8_t i = 0; i < max_files_numbers; i++)
					{
						strcpy(fileName, "batt");
						fileName[4] = (char)(i + 48);
						strcat(fileName, ".csv");
						fileName[9] = '\0';
						SD.remove(fileName);
					}
					playMessageOnDPlayer(AUDIO_ALL_FILES_DELETED);
					cicle = 0;
				}
			}
			else {
				exit = true;
			}
		}
		char headersText[37] = "IDMessage;Battery;Value;Delta;Origin";

		writeOnSDCard(headersText);
	}
	else
	{
		//buzzerSensorActivity(5, 400, 1000, 500);

#ifdef _DEBUG
		Serial.println(F("SD failed"));
#endif // _DEBUG
			playMessageOnDPlayer(AUDIO_PROBLEMA_SCHEDA_MEMORIA);
			while (true) {};
	}
}

void loop()
{
	setMultiplexer(_demultiplexerPosition);

	//if there is an attiny85 interrupt delay put here same delay (maybe not important) 
	//delay(800);

	char response[6] = {};

	getDataFromSerialBuffer(&response[0]);

#ifdef _DEBUG
	Serial.print(F("#"));
	Serial.print(response);
	Serial.println(F("#"));
#endif

	uint8_t max_attempts = 0;

	//Attempts if number transformation fails.
	while ((!is_number(response) || response[0] == '.') && max_attempts < 5)
	{
		getDataFromSerialBuffer(&response[0]);	//delay(500);

#ifdef _DEBUG
		Serial.println(F("num.wrong"));
#endif
		max_attempts++;
	}
	//if number transformation was failed
	if (!is_number(response))
	{
		playMessageOnDPlayer(AUDIO_NUMERO_ERRATO);

#ifdef _DEBUG
		Serial.println(F("resp.not.num"));
#endif
		_demultiplexerPosition = 0;
		return;
	}
	if (_idMessage[0] != 'x')
	{
		if (_idMessage[0] != response[4]) {
			playMessageOnDPlayer(AUDIO_ID_MESSAGE_WRONG);

#ifdef _DEBUG
			Serial.println(F("idMessage problems"));
#endif // _DEBUG

			_demultiplexerPosition = 0;
			_idMessage[0] = 'x';
			return;
		}
	}
	else {
		_idMessage[0] = response[4];
	}

	char csvTextLayOut[21] = {};

	prepareStringForSDCard(csvTextLayOut, response);

	for (uint8_t i = 0; i < 20; i++)
	{
		//Serial.println((char)csvTextLayOut[i]);
		if (((char)csvTextLayOut[i] < 46 || (char)csvTextLayOut[i] > 59) && (char)csvTextLayOut[2] != 'B')
		{
#ifdef _DEBUG
			Serial.println(F("text.problem"));
#endif // _DEBUG
			_demultiplexerPosition = 0;
			playMessageOnDPlayer(AUDIO_TRACCIA_ERRATA);
			return;
		}
	}

	//if (csvTextLayOut[19] == '\0' && csvTextLayOut[20] != '\0')
	//{
	//	Serial.println(F("text.problem"));
	//	_demultiplexerPosition = 0;
	//	playMessageOnDPlayer(2);
	//	return;
	//}

	writeOnSDCard(csvTextLayOut);

	if (_demultiplexerPosition == 5)
	{

#ifdef _DEBUG
		printStoredBatteryValuesArray();
#endif 
		checkActivities();

		_demultiplexerPosition = 0;

		_idMessage[0] = 'x';

		for (uint8_t i = 0; i < 6; i++) {
			storedBatteryValues[i] = 0.00;
		}

		total_takeovers++;

		if (total_takeovers == max_total_takeovers)
		{
			total_takeovers = 0;
			playMessageOnDPlayer(AUDIO_AQUISIZIONE_DATI);
		}

		Send_Interrupt_To_All_Attiny85();
	}
	else
	{
		_demultiplexerPosition++;
	}
	/* 	if (responseString != "")
		{
			// Serial.println(F("-----------------START------------------------"));

			_idMessage = responseString.substring(5, 6);

			csvString = "";

			csvString = prepareStringForSDCard(responseString, demultiplexerPosition);

			if (csvString.length() != 21)
			{
				Serial.print(F("dis-"));
				Serial.println(responseString);
				return;
			}

			writeOnSDCard(csvString);

			++demultiplexerPosition;

			bool condition = true;

			numeroDisallineamenti = 0;

			unsigned long timeForSerialData = millis();

			// clearSerialBuffer();

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
						Serial.print(F("disallineamento - "));
						Serial.println(responseString);
					}
				}
			}
			buzzerSensorActivity(15, 1000, 100, 100);

			checkBatteriesMaxLevel(storedBatteryValues[0]);

			checkBatteriesMinLevel(storedBatteryValues[0]);

			Serial.print(F("Mx.v:"));
			Serial.println(batteryMaxLevel);

			Serial.print(F("M.v:"));
			Serial.println(batteryMinLevel);

			if (thereAreUnbalancedBatteries(90))
			{
				Serial.println(F("Unbalanced batteries"));
				buzzerSensorActivity(5, 2500, 80, 200);
			}

			Send_Interrupt_To_All_Attiny85();
			clearSerialBuffer();
			//idCurrentMessage = "";
			// Serial.println(F("-----------------END------------------------"));
		} */
}

void printStoredBatteryValuesArray()
{
	for (uint8_t i = 0; i < 6; i++)
	{
		Serial.println(storedBatteryValues[i]);
	}
}

void checkActivities()
{
	checkBatteriesMaxLevel(storedBatteryValues[0]);

	checkBatteriesMinLevel(storedBatteryValues[0]);


#ifdef _DEBUG
	Serial.print(F("Mx.v:"));
	Serial.println(batteryMaxLevel);
#endif // _DEBUG

#ifdef _DEBUG
	Serial.print(F("M.v:"));
	Serial.println(batteryMinLevel);
#endif // _DEBUG

	if (thereAreUnbalancedBatteries())
	{
#ifdef _DEBUG
		Serial.println(F("Unbalanced batteries"));
#endif // _DEBUG
		playMessageOnDPlayer(AUDIO_DISLIVELLO_BATTERIE);
		//buzzerSensorActivity(5, 2500, 80, 200);
	}
}

bool is_number(const String& s)
{
	char* end = nullptr;
	float val = strtod(s.c_str(), &end);
	return end != s.c_str() && *end == '\0' && val < 4.5 && val > 0.00;
}

bool thereAreUnbalancedBatteries()
{
	
	float maxPercentageForAlarm = analogRead(_pin_maxBatteryVoltageDifference) / (1024.00 / 10.00);
    
	float percentageValue = 100 - ((batteryMinLevel / batteryMaxLevel) * 100);

#ifdef _DEBUG
	Serial.print(F("Percentage value : "));
	Serial.print(percentageValue);
	Serial.println(F("%"));
#endif // _DEBUG

	if (percentageValue > maxPercentageForAlarm)
	{
		return true;
	}
	return false;
}

float getNumber(char* response)
{
	return atof(response);
}

void getDataFromSerialBuffer(char* response)
{
	SoftwareSerial softwareSerial(3, 99);

	softwareSerial.begin(600);

	while (!softwareSerial);

	//delay(500);

	if (softwareSerial.available() > 0)
	{
		for (int i = 0; i < 19; i++)
		{
			softwareSerial.read();
		}
	}

	delay(500);

	if (softwareSerial.available() > 0)
	{
		softwareSerial.readBytesUntil('*', response, 6);
	}
	response[5] = '\0';

}

//Serial.print("setMultiplexer channel : "); Serial.println(channel);


void setMultiplexer(int channel)
{

	// loop through the 4 sig
	for (int i = 0; i < 4; i++)
	{
		digitalWrite(controlPin[i], muxChannel[channel][i]);
	}
}

void prepareStringForSDCard(char* csvTextLayOut, char response[6])
{
	char deltaVoltage_to_string[4] = {};

	response[4] = '\0';

	float number = getNumber(response);

	//Serial.println(number);

	number = number + deltaVoltage[_demultiplexerPosition];

	dtostrf(deltaVoltage[_demultiplexerPosition], 4, 2, deltaVoltage_to_string);

	csvTextLayOut[0] = _idMessage[0];
	strcat(csvTextLayOut, ";");
	strcat(csvTextLayOut, idBattery[_demultiplexerPosition]);
	strcat(csvTextLayOut, ";");
	strcat(csvTextLayOut, response);
	strcat(csvTextLayOut, ";");
	strcat(csvTextLayOut, deltaVoltage_to_string);
	strcat(csvTextLayOut, ";");
	strcat(csvTextLayOut, response);
	strcat(csvTextLayOut, _idMessage);
	csvTextLayOut[20] = '\0';
#ifdef _DEBUG
	Serial.println(csvTextLayOut);
#endif // _DEBUG
	storedBatteryValues[_demultiplexerPosition] = number;
}

/*
String prepareStringForSDCardOld(String message, uint8_t demultiplexerPosition)
{
	String csvString = "";
	double number = getNumber(message);
	number = number + deltaVoltage[demultiplexerPosition];
	csvString = message.substring(5, 6) + ";" + String(idBattery[demultiplexerPosition]) + ";" + String(number) + ";" + String(deltaVoltage[demultiplexerPosition]) + ";" + message;
	Serial.println(csvString);
	storedBatteryValues[demultiplexerPosition] = number;
	// writeOnSDCard(csvString);
	return csvString;
} */

void writeOnSDCard(char* message)
{
	File myFile;

	if (_is_card_writing_disable)return;
	// Create/Open file
	// String fileName = "batt" + String(fileNumber) + ".csv";
	// Serial.print(F("Apro file:"));
	// Serial.println(fileName);
	myFile = SD.open(fileName, FILE_WRITE);
	delay(500);
	if (myFile)
	{
		myFile.println(message);
#ifdef _DEBUG
		Serial.println(F("write"));
#endif // _DEBUG
		myFile.close();
	}
	else
	{
		//buzzerSensorActivity(5, 400, 1000, 500);
#ifdef _DEBUG
		Serial.println(F("err.open.file"));
#endif // _DEBUG
		playMessageOnDPlayer(AUDIO_PROBLEMA_SCHEDA_MEMORIA);
		myFile.close();
	}
	//// Reading the file
	// myFile = SD.open("batteryValues.csv");
	// if (myFile) {
	//	//Serial.println("Read:");
	//	// Reading the whole file
	//	while (myFile.available()) {
	//		Serial.write(myFile.read());
	//	}
	//	myFile.close();
	// }
	// else {
	//	Serial.println("error opening file batteryValues.csv");
	// }
}

void Send_Interrupt_To_All_Attiny85()
{
	digitalWrite(_pin_interrupt_to_attiny85, HIGH);
	//delay(500);
	digitalWrite(_pin_interrupt_to_attiny85, LOW);
}

void buzzerSensorActivity(uint8_t numberOfCicle, unsigned int frequency, unsigned long duration, uint16_t pause)
{
	/*if (_is_buzzer_disabled)return;

	for (uint8_t i = 0; i < numberOfCicle; i++)
	{
		tone(_pin_buzzer, frequency, duration);
		delay(duration);
		delay(pause);
		noTone(_pin_buzzer);
	}
	delay(pause);*/
}

void checkBatteriesMaxLevel(float value)
{
	ii = 0;
	while (ii < numberOfBattery)
	{
		// Serial.println(ii);

		if (value >= storedBatteryValues[ii])
		{

#ifdef _DEBUG
			/*Serial.print(F("if ")); Serial.print(value); Serial.print(F(" maggiore o uguale a ")); Serial.println(storedBatteryValues[ii]);
			Serial.print(F("metto ")); Serial.print(value); Serial.println(F(" in batteryMaxLevel"));
			 delay(1000);*/
#endif // _DEBUG


			batteryMaxLevel = value;
			ii++;
		}
		else
		{
#ifdef _DEBUG
			// Serial.print(F("mando ")); Serial.print(storedBatteryValues[ii]); Serial.println(F(" in funzione"));
#endif // _DEBUG
			checkBatteriesMaxLevel(storedBatteryValues[ii]);
		}
	}
}

void checkBatteriesMinLevel(float value)
{
	ii = 0;
	while (ii < numberOfBattery)
	{
		if (value <= storedBatteryValues[ii])
		{

#ifdef _DEBUG
			/*Serial.print(F("if ")); Serial.print(value); Serial.print(F(" maggiore o uguale a ")); Serial.println(storedBatteryValues[ii]);
			Serial.print(F("metto ")); Serial.print(value); Serial.println(F(" in batteryMaxLevel"));
			delay(1000);*/
#endif // _DEBUG
			batteryMinLevel = value;
			ii++;
		}
		else
		{
#ifdef _DEBUG
			// Serial.print(F("mando ")); Serial.print(storedBatteryValues[ii]); Serial.println(F(" in funzione"));
#endif // _DEBUG
			checkBatteriesMinLevel(storedBatteryValues[ii]);
		}
	}
}

void playMessageOnDPlayer(uint8_t messageCode)
{
	DFRobotDFPlayerMini myDFPlayer;
	SoftwareSerial mySoftwareSerial(_pin_dfMiniPlayer_rx, _pin_dfMiniPlayer_tx); // rx, tx
	delay(500);
	mySoftwareSerial.begin(9600);
	delay(500);

	if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.

#ifdef _DEBUG
		Serial.println(F("Unable to begin:"));
		Serial.println(F("1.Please recheck the connection!"));
		Serial.println(F("2.Please insert the SD card!"));
#endif // _DEBUG
		//buzzerSensorActivity(5, 2500, 80, 200);
		while (true);
	}

#ifdef _DEBUG
	Serial.println(F("DFPlayer Mini online."));
#endif // _DEBUG

	uint16_t volume = (30.00 / 1024.00) * analogRead(A3);

	//Serial.println(volume);

	myDFPlayer.volume(volume);  //Set volume value. From 0 to 30

	myDFPlayer.play(messageCode);  //Play next mp3 every 3 second.

	delay(5000);
}