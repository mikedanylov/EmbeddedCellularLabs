#include <stdio.h>
#include <stdlib.h> 
#include <LiquidCrystal.h>
#include <Servo.h>
#include <SoftwareSerial.h>

// typedef struct{
//   String name;
//   String phone_number;
// } Contact;

// void constact_set_name(String name, Contact* contact){
//   contact->name = name;
// }

// void constact_set_phone_num(String number, Contact* contact){
//   contact->phone_number = number;
// }

void GPRS_init(){
  // Enter PIN code: AT+CPIN=<pin>
  Serial.write("AT+CPIN=1234\r");
  delay(3000); // takes time to accept PIN
}

// Send SMS Message
void GPRS_sms_send(String text){
  Serial.write("AT+CMGF=1\r");
  delay(3000);
  Serial.write("AT+CMGS=\"0401959797\",129\r");
  delay(3000);
  Serial.write("Hello, World\r");
  delay(3000);
}

// Select Phonebook Memory Storage
void GPRS_phonebook_memory(){
  Serial.write("AT+CPBS?\r");
  delay(3000);
}

// Write Phonebook Entry
void GPRS_phonebook_write(String text){
  Serial.write("AT+CPBW=<index1>[, <number>,[<type>,[<text>]]]\r");
  delay(3000); 
}

// Read Current Phonebook Entries
void GPRS_phonebook_read(){
  Serial.write("AT+CPBR=?\r");
  delay(3000); 
}

#define N_ENTRIES 2

// device phone number: 0466259105 

SoftwareSerial GPRS(7, 8);
unsigned char buffer[64]; // buffer array for data recieve over serial port
int count = 0;     // counter for buffer array 
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
Servo gate;

String authNumbers[] = {"0405369467\n", "0401959797\n"};
String input = "";
bool stringComplete = false;
int attempt = 0;
int LEDPIN = 13;
int gateMin = 45;
int gateMax = 135;

void setup() {
  Serial.begin(9600);
  lcd.begin(16,2);
  input.reserve(200);
  pinMode(LEDPIN, HIGH);
  gate.attach(9);

  GPRS.begin(19200);
  delay(5000);
  GPRS_init();
}

void loop() {

  // serialEvent();
  // lcd.home();
  // if (stringComplete){ // when string is sent by pressing Enter
  //   for (int i = 0; i < N_ENTRIES; i++){
  //     Serial.println(authNumbers[i]); // debugging
  //     if (input == authNumbers[i]){
  //       lcd.print("Authorized");
  //       attempt = 0;
  //       lcd.setCursor(0,1);
  //       lcd.print("Attempt: ");
  //       lcd.print(attempt);
  //       digitalWrite(LEDPIN, HIGH);
  //       gate.write(gateMin);
  //       break; // no need to continue if matches at least one of numbers    
  //     }else {
  //       lcd.print("Unauthorized");
  //       lcd.setCursor(0,1);
  //       lcd.print("Attempt: ");
  //       attempt++;
  //       lcd.print(attempt);   
  //       digitalWrite(LEDPIN, LOW);
  //       gate.write(gateMax);
  //     }
  //   }
  //   stringComplete = false; 
  //   input = ""; 
  // }

  // write everything from SW serial to HW serial
  if (GPRS.available()){
    while(GPRS.available()){
      buffer[count++]=GPRS.read();
      if(count >= 64)break;
    }
    Serial.write(buffer,count);
    clearBufferArray();
    count = 0;
  }

  // write everything from HW serial to SW serial
  if (Serial.available())
    GPRS.write(Serial.read());
}

void serialEvent(){
  while(Serial.available()){
    char inChar = (char)Serial.read();
    input += inChar;
    if(inChar == '\n')
      stringComplete = true;
  }
}

void clearBufferArray(){
  for (int i=0; i<count;i++)
    buffer[i]=NULL;
}
