#include <string.h>
#include <SoftwareSerial.h>

SoftwareSerial GPRS(7, 8);

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