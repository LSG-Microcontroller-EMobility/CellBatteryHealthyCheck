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
//#define _DEBUG
//#define _IS_ON_VOLTAGE_TEST
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
#define AUDIO_INIZIO_STRESS_TEST 14	
const uint8_t _numberOfBattery = 6;
const uint8_t _pin_selectorMultiPlex0 = 4;
const uint8_t _pin_selectorMultiPlex1 = 5;
const uint8_t _pin_selectorMultiPlex2 = 6;
const uint8_t _pin_selectorMultiPlex3 = 7;
const uint8_t _pin_rx = 3;
const uint8_t _pin_interrupt_to_attiny85 = 9;
const uint8_t _pin_dfMiniPlayer_rx = A1;
const uint8_t _pin_dfMiniPlayer_tx = A2;
const uint8_t _pin_dfMiniPlayer_volume = A3;
const uint8_t _pin_maxBatteryVoltageDifference = A4;
uint8_t total_takeovers = 0;
const uint8_t max_total_takeovers = 30;
const uint8_t demultiplexer_position_start = 0;
//-----------------------    ATTENZIONE PIN ASSEGNATI a scheda SD file excel !!!!!!!   -------------------------------
// Pin 11 MOSI	Pin 12 MISO		Pin 13 SCK
const float deltaVoltage[_numberOfBattery] = { 0.00, 0.00 , 0.00, 0.00, 0.00 ,0.00 };
float storedBatteryValues[_numberOfBattery] = { 0.00, 0.00 , 0.00, 0.00, 0.00 ,0.00};
float stored_ampere = 0.00;
float stored_watts = 0.00;
uint8_t _demultiplexerPosition = demultiplexer_position_start;
uint8_t fileNumber = 0;
uint8_t numeroDisallineamenti = 0;
char _idMessage[1] = { 'x' };
float batteryMaxLevel = 0.00f;
float batteryMinLevel = 0.00f;
const uint8_t max_files_numbers = 9;  //MAX 9
uint8_t ii = 0;
bool _is_card_writing_disable = false;
bool _is_DPlayer_disable = false;
char fileName[15] = {};
void setup(){
	analogReference(EXTERNAL);
	send_interrupt_to_all_attiny85();
#ifndef _IS_ON_VOLTAGE_TEST
	delay(5000);
#endif // !_IS_ON_VOLTAGE_TEST
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
#ifndef _IS_ON_VOLTAGE_TEST
	init_file_card();
#endif // !_IS_ON_VOLTAGE_TEST
#ifdef _DEBUG
	Serial.println(F("restart"));
#endif // _DEBUG
#ifndef _IS_ON_VOLTAGE_TEST
	play_message_on_DPlayer(AUDIO_SISTEMA_INIZIALIZZATO);
#endif // !_IS_ON_VOLTAGE_TEST
}
void init_file_card(){
	if (_is_card_writing_disable) return;
		if (SD.begin()){
			bool exit = false;
			uint8_t cicle = 0;
			//Serial.println(F("card ready"));
			while (cicle < max_files_numbers && !exit){
				strcpy(fileName, "batt");
				fileName[4] = (char)(cicle + 48);
				strcat(fileName, ".csv");
				fileName[9] = '\0';
#ifdef _DEBUG
				Serial.println(fileName);
#endif // _DEBUG
				if (SD.exists(fileName)){
#ifdef _DEBUG
					Serial.println(F("File esiste"));
#endif // _DEBUG
					cicle++;
					if (cicle == max_files_numbers){
						play_message_on_DPlayer(AUDIO_SCHEDA_MEM_PIENA);
						play_message_on_DPlayer(AUDIO_DELETE_FILES_30_SEC);
						delay(20);
						play_message_on_DPlayer(AUDIO_DELETE_FILES_10_SEC);
						delay(10);
						for (uint8_t i = 0; i < max_files_numbers; i++){
							strcpy(fileName, "batt");
							fileName[4] = (char)(i + 48);
							strcat(fileName, ".csv");
							fileName[9] = '\0';
							SD.remove(fileName);
						}
						play_message_on_DPlayer(AUDIO_ALL_FILES_DELETED);
						cicle = 0;
					}
				}
				else {
					exit = true;
				}
			}
			char headersText[37] = "IDMessage;Battery;Value;Delta;Origin";
			write_on_sd_card(headersText);
		}
		else{
			//buzzer_sensor_activity(5, 400, 1000, 500);
#ifdef _DEBUG
			Serial.println(F("SD failed"));
#endif // _DEBUG
			play_message_on_DPlayer(AUDIO_PROBLEMA_SCHEDA_MEMORIA);
			while (true) {};
		}
}
void loop(){
	/*play_message_on_DPlayer(AUDIO_INIZIO_STRESS_TEST);

	Serial.println("inizio del test"); 

	return;*/
	////volume test
	//Serial.print("volume : "); Serial.println(analogRead(A3));
	////volume test
	////percentage test
	//Serial.print("percentage : "); Serial.println(analogRead(A4));
	////percentage test
	//delay(500);
	//return;
	//send_interrupt_to_all_attiny85();
	//delay(1000);
	//return;
#ifdef _IS_ON_VOLTAGE_TEST
	_demultiplexerPosition = 0;
#endif // _IS_ON_VOLTAGE_TEST
	set_multiplexer(_demultiplexerPosition);
	char response[6] = {};
	get_data_from_serial_buffer(&response[0]);
#ifdef _IS_ON_VOLTAGE_TEST
	Serial.print(F("#")); Serial.print(response); Serial.println(F("#"));
	send_interrupt_to_all_attiny85();
	return;
#endif // _IS_ON_VOLTAGE_TEST
#ifdef _DEBUG
	Serial.print(F("#"));Serial.print(response);Serial.println(F("#"));
#endif
	uint8_t max_attempts = 0;
	//Attempts if number transformation fails.
	while ((!is_number(response) || response[0] == '.') && max_attempts < 5){
		get_data_from_serial_buffer(&response[0]);
#ifdef _DEBUG
		Serial.println(F("num.wrong"));
#endif
		max_attempts++;
	}
	//if number transformation was failed
	if (!is_number(response)){
		play_message_on_DPlayer(AUDIO_NUMERO_ERRATO);
#ifdef _DEBUG
		Serial.println(F("resp.not.num"));
#endif
		_demultiplexerPosition = demultiplexer_position_start;
		return;
	}
	if (_idMessage[0] != 'x'){
		if (_idMessage[0] != response[4]) {
			play_message_on_DPlayer(AUDIO_ID_MESSAGE_WRONG);
#ifdef _DEBUG
			Serial.println(F("idMess. problm"));
#endif // _DEBUG
			_demultiplexerPosition = demultiplexer_position_start;
			_idMessage[0] = 'x';
			return;
		}
	}
	else {
		_idMessage[0] = response[4];
	}
	char csv_battery_text_layout[21] = {};
	prepare_battery_sd_card_string(csv_battery_text_layout, response);
	if (is_battery_csv_text_layout_wrong(csv_battery_text_layout)){
		_demultiplexerPosition = demultiplexer_position_start;
		play_message_on_DPlayer(AUDIO_TRACCIA_ERRATA);
		return;
	}
	store_battery_value(response);
	write_on_sd_card(csv_battery_text_layout);
	if (_demultiplexerPosition == (_numberOfBattery - 1)){
#ifdef _DEBUG
		print_stored_battery_values_array();
#endif 
		check_activities();
		set_multiplexer(6);
		get_watts_from_serial_buffer();
		char csv_watts_layout[15]{};
		prepare_watts_sd_card_string(csv_watts_layout);
		char csv_amps_layout[15]{};
		prepare_ampere_sd_card_string(csv_amps_layout);
		write_on_sd_card(csv_watts_layout);
		write_on_sd_card(csv_amps_layout);
		_demultiplexerPosition = demultiplexer_position_start;
		_idMessage[0] = 'x';
		for (uint8_t i = 0; i < _numberOfBattery; i++) {
			storedBatteryValues[i] = 0.00;
		}
		total_takeovers++;
		if (total_takeovers == max_total_takeovers){
			total_takeovers = 0;
			play_message_on_DPlayer(AUDIO_AQUISIZIONE_DATI);
		}
		send_interrupt_to_all_attiny85();
	}
	else{
		_demultiplexerPosition++;
	}
}
bool is_battery_csv_text_layout_wrong(char* csv_text_layout){
	bool return_value = false;
	for (uint8_t i = 0; i < 20; i++){
		//Serial.println((char)csvTextLayOut[i]);
		if (((char)csv_text_layout[i] < 46 || (char)csv_text_layout[i] > 59) && (char)csv_text_layout[2] != 'B'){
#ifdef _DEBUG
			Serial.println(F("text.problem"));
#endif // _DEBUG
			return_value = true;
		}
	}
	return return_value;
}
void store_battery_value(char response[6]){
	float number = get_number(response);
	number = number + deltaVoltage[_demultiplexerPosition];
	storedBatteryValues[_demultiplexerPosition] = number;
}
void print_stored_battery_values_array(){
	for (uint8_t i = 0; i < _numberOfBattery; i++){
		Serial.println(storedBatteryValues[i]);
	}
}
void check_activities(){
	check_batteries_max_level(storedBatteryValues[0]);
	check_batteries_min_level(storedBatteryValues[0]);
#ifdef _DEBUG
	Serial.print(F("Mx.V:"));Serial.println(batteryMaxLevel);
#endif // _DEBUG
#ifdef _DEBUG
	Serial.print(F("Min.V:"));Serial.println(batteryMinLevel);
#endif // _DEBUG
	if (there_are_unbalanced_batteries()){
#ifdef _DEBUG
		Serial.println(F("Unbalanced batteries"));
#endif // _DEBUG
		play_message_on_DPlayer(AUDIO_DISLIVELLO_BATTERIE);
		//buzzer_sensor_activity(5, 2500, 80, 200);
	}
}
bool is_number(const String& s){
	char* end = nullptr;
	float val = strtod(s.c_str(), &end);
	return end != s.c_str() && *end == '\0' && val < 4.5 && val > 0.00;
}
bool there_are_unbalanced_batteries(){
    //https://www.desmos.com/calculator/wsfbcw9ffn
	//See math site for percentage calculate.
	//float maxPercentageForAlarm = analogRead(_pin_maxBatteryVoltageDifference) / (1024.00 / 15.00 /*<--max percentage*/);
	float x = 0.00f;
	for (int i = 0; i < _numberOfBattery; i++){
		x = x + storedBatteryValues[i];
	}
	x = x / _numberOfBattery;
	float maxPercentageForAlarm = -(8.60f * x) + 32.15f;
	float percentageValue = 100 - ((batteryMinLevel / batteryMaxLevel) * 100);
#ifdef _DEBUG
	Serial.print(F("% value : "));Serial.print(percentageValue);Serial.println(F("%"));
	Serial.print(F("% max : "));Serial.print(maxPercentageForAlarm);Serial.println(F("%"));
#endif // _DEBUG
	if (percentageValue > maxPercentageForAlarm){
		return true;
	}
	return false;
}
float get_number(char* response){
	return atof(response);
}
void get_data_from_serial_buffer(char* response){
	SoftwareSerial softwareSerial(_pin_rx, 99);
	softwareSerial.begin(600);
	while (!softwareSerial);
	delay(500);
	char trash[20]{};
	if (softwareSerial.available() > 0){
		softwareSerial.readBytesUntil('*', trash, 20);
	}
	if (softwareSerial.available() > 0){
		softwareSerial.readBytes(response, 6);
	}
	response[5] = '\0';
}
void get_watts_from_serial_buffer(){
	stored_ampere = 0.00f;
	stored_watts = 0.00f;
	SoftwareSerial softwareSerial(_pin_rx, 99);
	softwareSerial.begin(600);
	while (!softwareSerial);
	delay(800);
	char trash[20]{};
	if (softwareSerial.available() > 0){
		softwareSerial.readBytesUntil('*', trash, 19);
	}
	if (softwareSerial.available() > 0){
		stored_ampere = softwareSerial.parseFloat();
	}
	if (softwareSerial.available() > 0){
		stored_watts = softwareSerial.parseFloat();
	}
#ifdef _DEBUG
	Serial.print(F("Ampere :"));Serial.println(stored_ampere);
	Serial.print(F("Watts/h :"));Serial.println(stored_watts);
#endif // _DEBUG
}
//Serial.print("setMultiplexer channel : "); Serial.println(channel);
void set_multiplexer(int channel){
	uint8_t controlPin[4] = { _pin_selectorMultiPlex0, _pin_selectorMultiPlex1, _pin_selectorMultiPlex2, _pin_selectorMultiPlex3 };
	const uint8_t muxChannel[7][4] = {
		{0, 0, 0, 0}, // channel 0
		{1, 0, 0, 0}, // channel 1
		{0, 1, 0, 0}, // channel 2
		{1, 1, 0, 0}, // channel 3
		{0, 0, 1, 0}, // channel 4
		{1, 0, 1, 0}, // channel 5
		{0, 1, 1, 0}, // channel 6
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
	// loop through the 4 sig
	for (int i = 0; i < 4; i++){
		digitalWrite(controlPin[i], muxChannel[channel][i]);
	}
}
void prepare_battery_sd_card_string(char* csvTextLayOut, char response[6]){
	const char* idBattery[_numberOfBattery] = { "B0", "B1", "B2", "B3", "B4", "B5"};
	//const char* idBattery[_numberOfBattery] = { "B0", "B1", "B2" };
	//const char* idBattery[_numberOfBattery] = { "B0", "B1", "B2","B3"};
	char deltaVoltage_to_string[4] = {};
	response[4] = '\0';
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
}
void prepare_watts_sd_card_string(char* csv_text_layout){
	char watts[7];
	dtostrf(stored_watts, 7, 2, watts);
	strcpy(csv_text_layout, "watts");
	strcat(csv_text_layout, ";");
	strcat(csv_text_layout, ";");
	strcat(csv_text_layout, watts);
	strcat(csv_text_layout, ";");
	strcat(csv_text_layout, ";");
	csv_text_layout[20] = '\0';
#ifdef _DEBUG
	Serial.print(F("csv_watts_layout: ")); Serial.println(csv_text_layout);
#endif 
}
void prepare_ampere_sd_card_string(char* csv_text_layout){
	char amps[5];
	dtostrf(stored_ampere, 5, 2, amps);
	strcpy(csv_text_layout, "amps");
	strcat(csv_text_layout, ";");
	strcat(csv_text_layout, ";");
	strcat(csv_text_layout, amps);
	strcat(csv_text_layout, ";");
	strcat(csv_text_layout, ";");
	csv_text_layout[20] = '\0';
#ifdef _DEBUG
	Serial.print("csv_amps_layout: "); Serial.println(csv_text_layout);
#endif 
}
void write_on_sd_card(char* message){
	File myFile;
	if (_is_card_writing_disable)return;
	// Create/Open file
	// String fileName = "batt" + String(fileNumber) + ".csv";
	// Serial.print(F("Apro file:"));
	// Serial.println(fileName);
	myFile = SD.open(fileName, FILE_WRITE);
	if (myFile){
		myFile.println(message);
#ifdef _DEBUG
		Serial.println(F("write.SD"));
#endif // _DEBUG
		myFile.close();
	}
	else{
		//buzzer_sensor_activity(5, 400, 1000, 500);
#ifdef _DEBUG
		Serial.println(F("err.SD"));
#endif // _DEBUG
		play_message_on_DPlayer(AUDIO_PROBLEMA_SCHEDA_MEMORIA);
		myFile.close();
		while (true) {};
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
void send_interrupt_to_all_attiny85(){
	digitalWrite(_pin_interrupt_to_attiny85, HIGH);
	delay(200);
	digitalWrite(_pin_interrupt_to_attiny85, LOW);
#ifdef _DEBUG
	Serial.println(F("--  Inter. send --"));
#endif // _DEBUG
}
void buzzer_sensor_activity(uint8_t numberOfCicle, unsigned int frequency, unsigned long duration, uint16_t pause){
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
void check_batteries_max_level(float value){
	ii = 0;
	while (ii < _numberOfBattery){
		// Serial.println(ii);
		if (value >= storedBatteryValues[ii]){
#ifdef _DEBUG
			/*Serial.print(F("if ")); Serial.print(value); Serial.print(F(" maggiore o uguale a ")); Serial.println(storedBatteryValues[ii]);
			Serial.print(F("metto ")); Serial.print(value); Serial.println(F(" in batteryMaxLevel"));
			 delay(1000);*/
#endif // _DEBUG
			batteryMaxLevel = value;
			ii++;
		}
		else{
#ifdef _DEBUG
			// Serial.print(F("mando ")); Serial.print(storedBatteryValues[ii]); Serial.println(F(" in funzione"));
#endif // _DEBUG
			check_batteries_max_level(storedBatteryValues[ii]);
		}
	}
}
void check_batteries_min_level(float value){
	ii = 0;
	while (ii < _numberOfBattery){
		if (value <= storedBatteryValues[ii]){
#ifdef _DEBUG
			/*Serial.print(F("if ")); Serial.print(value); Serial.print(F(" maggiore o uguale a ")); Serial.println(storedBatteryValues[ii]);
			Serial.print(F("metto ")); Serial.print(value); Serial.println(F(" in batteryMaxLevel"));
			delay(1000);*/
#endif // _DEBUG
			batteryMinLevel = value;
			ii++;
		}
		else{
#ifdef _DEBUG
			// Serial.print(F("mando ")); Serial.print(storedBatteryValues[ii]); Serial.println(F(" in funzione"));
#endif // _DEBUG
			check_batteries_min_level(storedBatteryValues[ii]);
		}
	}
}
void play_message_on_DPlayer(uint8_t messageCode){
	if (_is_DPlayer_disable) return;
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
		//buzzer_sensor_activity(5, 2500, 80, 200);
		while (true);
	}
#ifdef _DEBUG
	Serial.println(F("DFPlayer Mini online."));
#endif // _DEBUG
	uint16_t volume = (30.00 / 1024.00) * analogRead(_pin_dfMiniPlayer_volume);
	//Serial.println(volume);
	myDFPlayer.volume(volume);  //Set volume value. From 0 to 30
	myDFPlayer.play(messageCode);  //Play next mp3 every 3 second.
	delay(5000);
}