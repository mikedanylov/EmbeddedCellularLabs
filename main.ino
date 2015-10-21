#include "gprs.h"

#define N_ENTRIES 2

char buffer[64]; // buffer array for data recieve over serial port
int count = 0;     // counter for buffer array 
String authorized_entries[N_ENTRIES] = {"Jalli", "401959797"};
bool is_authorized = false; 

void setup(){  
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
    Serial.println(buffer);

    // create actions based on data from GPRS
    
    // TODO
    // 1.
    // if there is a call attempt
    // check the number with verify_caller()
    // and if the entry is authorized
    // then perform some actions
    // display some info on LCD as well
    
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

bool verify_caller(char* buffer){
  for (int i = 0; i < N_ENTRIES; i++){
    // check if contains authorized entry
    if (strstr(buffer, authorized_entries[i]) != NULL){
      Serial.write("Authorized");
      is_authorized = true;
      break;
    }
  }
  return is_authorized;
}
