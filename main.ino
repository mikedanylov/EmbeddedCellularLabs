#include <string.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

SoftwareSerial GPRS(7, 8);
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);;

#define N_ENTRIES 2

char buffer[64]; // buffer array for data recieve over serial port
int count = 0;     // counter for buffer array
char authorized_entries[2][15] = {"Jalli", "401959797"};
//strcpy(authorized_entries[0], "Jalli\0");
//strcpy(authorized_entries[1], "401959797\0");
bool is_authorized = false;
char* caller;
boolean caller_data_next = false;

void setup(){
  lcd.begin(16, 2);
  Serial.begin(19200);
  GPRS_init();
}
 
void loop(){
  if (GPRS.available()){
    while(GPRS.available()){
      buffer[count++]=GPRS.read();
      if(count >= 64)
        break;
  }
  //Serial.println(buffer);

    // create actions based on data from GPRS
    
    // TODO
    // 1.
    // if there is a call attempt
    // check the number with verify_caller()
    // and if the entry is authorized
    // then perform some actions
    // display some info on LCD as well
    if (caller_data_next){
      caller = verify_caller(buffer);
      strcat(caller, "\0");
      if (caller != NULL){
        //Serial.println("Authorized!");
        lcd.home();
        lcd.print("Authorized!");
        lcd.setCursor(0, 1);
        lcd.print(caller);
        GPRS_call_hangup();
        delay(5000);
        lcd.clear();
        caller_data_next = false;      
      }
    }
    
    if (is_ringing(buffer)){
      Serial.print("You have a phone call!");
      caller_data_next = true;
    }
    
    // 2.
    // if there is new message received
    // check the number
    // and if the entry is authorized
    // then perform some actions
    // display some info on LCD as well

    clearBufferArray();
    count = 0;
  }
  // if (Serial.available())
  //   GPRS.write(Serial.read());
  
  is_authorized = false;
}

void clearBufferArray(){
  for (int i = 0; i < count; i++)
    buffer[i] = NULL;
}

char* verify_caller(char* buffer){
  char* tmp = NULL;
  for (int i = 0; i < N_ENTRIES; i++){
    // check if contains authorized entry
    if (strstr(buffer, authorized_entries[i]) != NULL){
//      Serial.write("Authorized");
      is_authorized = true;
      tmp = authorized_entries[i];
      break;
    }
  }
  return tmp;
}

void GPRS_init(){
  GPRS.begin(19200);
  delay(3000);
  GPRS.write("AT+CPIN=\"1234\"\r");
  delay(3000);
}

// Send SMS Message
void GPRS_sms_send(char* text, char* number, SoftwareSerial GPRS){
  char* cmd;
  strcpy(cmd, "AT+CMGS=");
  strcpy(cmd, number);
  strcpy(cmd, "\",129\r");

  GPRS.write("AT+CMGF=1\r");
  delay(3000);
  GPRS.write(cmd);
  delay(3000);
  GPRS.write("Hello, World\r");
  delay(3000);
}

// Read last SMS Message
void GPRS_sms_read_last(){
  GPRS.write("AT+CMGR=1\r");
  delay(3000);
}

// List all SMS messages
void GPRS_sms_read_all(){
  GPRS.write("AT+CMGL=\"ALL\"\r");
  delay(3000);
}

// Delete All SMS
void GPRS_sms_delall(){
  GPRS.write("AT+CMGDA=\"DEL ALL\"\r");
  delay(3000); 
}

// Select Phonebook Memory Storage
void GPRS_phonebook_memory(){
  GPRS.write("AT+CPBS?\r");
  delay(3000);
}

// Read Current Phonebook Entries
void GPRS_phonebook_read(){
  GPRS.write("AT+CPBR=?\r");
  delay(3000); 
}

// Receive Call
void GPRS_call_receive(){
  GPRS.write("ATA\r");
  delay(3000);
}

// Calling Line Identification Presentation
void GPRS_call_info_enable(){
  GPRS.write("AT+CLIP=1\r");
  delay(3000);
}

// Hang up Call
void GPRS_call_hangup(){
  GPRS.write("ATH\r");
  delay(3000);
}

boolean is_ringing(char* buff){
  // check if GPRS serial contains RING
  if (strstr(buffer, "RING") != NULL)
    return true;
  return false;
}
