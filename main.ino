#include <string.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <string.h>
#include <Timer.h>
#include <LcdBarGraph.h>
#include <Unauthorized.h>

#define N_ENTRIES 2
#define BUFFER_SIZE 64
#define DELAY 3000

SoftwareSerial GPRS(7, 8);
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
char lcdNumCols = 3;    //size of bargraph
LcdBarGraph lbg(&lcd, lcdNumCols, 0, 0);
char authorized_entries[2][15] = {"Jalli"};
Unauthorized unauthorized_entries[MAX_UNAUTH];
int current_unauthorized_index = 0; // keep track of unauthorized entries
int n_attempts = 0;
char str_attempts[5];
char sms_attempts[5];
char caller_phone_number[PHONE];
char sms_bouncer_number[PHONE]; // for sending sms back
char caller_name[20];
bool isClock = false;
bool isSignalQuality = false;
bool isOperatorName = false;
int loopCount;
bool caller_data_next = false; // flag for incomming notification
bool message_data_next = false; // flag for new msg notification
bool send_sms_next = false; // track when to send sms back to sender
bool sms_text_next = false;
char sms_text[160]; // content of sms message
char buffer[BUFFER_SIZE]; // putput from GPRS
char sms_buffer[160];
int count = 0; // count chars in buffer
int idle_data_display = 0; // switch between signal/clock/operator
int unauthorized_attempts_total = 0;

void setup(){
  lcd.begin(16, 2);
  Serial.begin(9600);
  GPRS_init(9600);

  unauthorized_init(unauthorized_entries);
}
 
void loop(){
    
    if (GPRS.available()){
        while(GPRS.available()){
          buffer[count++]=GPRS.read();
          if(count >= BUFFER_SIZE)break;
        }

        Serial.print("buffer: ");
        Serial.print(buffer);
        Serial.print("\n");

        if (is_containing(buffer, "RDY")){
            // input pin
            delay(DELAY);
            GPRS.write("AT+CPIN=\"1234\"\r");
            delay(DELAY);
        }

        // start sms message if flag is set
        if (send_sms_next){
            GPRS_sms_send(sms_bouncer_number);
            send_sms_next = false;
            sms_text_next = true;
        }

        // type text of sms message
        if (sms_text_next){
            GPRS.write(sms_text);
            GPRS.write((char)26); // ctrl+Z
            delay(DELAY);
            sms_text_next = false;
        }

        // create actions based on data from GPRS
        // if there is a call attempt get the phone number and if
        // the entry is authorized then get contact name as well
        if (caller_data_next){
            // here buffer data is caller info
            // parse buffer and get phone number
            get_caller_number(buffer, caller_phone_number);
            Serial.println(caller_phone_number);
            if (is_authorized(caller_phone_number)){
                // get contact name as well
                get_caller_name(buffer, caller_name);
                lcd.home();
                lcd.print("Authorized!");
                lcd.setCursor(0, 1);
                lcd.print(caller_name);
                GPRS_call_hangup();
                delay(5000);
                lcd.clear();
                caller_data_next = false;
            }
            else{
                lcd.home();
                lcd.print("Unauthorized!");
                lcd.setCursor(0, 1);
                lcd.print(caller_phone_number);
                GPRS_call_hangup();
                // save unauthorized entry if isn't known already
                if (is_known(caller_phone_number, unauthorized_entries)){
                    // increment attempts for this number
                    increment_attempts(caller_phone_number, unauthorized_entries);
                }
                else{
                    strcpy(unauthorized_entries[current_unauthorized_index].phone_number, caller_phone_number);
                    unauthorized_entries[current_unauthorized_index].attempts = 1;
                }

                lcd.clear();
                lcd.home();
                lcd.print("Attempts: ");
                sprintf(str_attempts,"%d", get_attempts(caller_phone_number, unauthorized_entries));
                lcd.print(str_attempts);
                delay(DELAY);
                lcd.clear();
                caller_data_next = false;
            }
        }
        
        // check if there is incoming call 
        if (is_containing(buffer, "RING")){
            Serial.println("You have a phone call!");
            caller_data_next = true;
        }
        
        // check if buffer should contain new message to read
        if (message_data_next){
            // get message content
            if (is_containing(buffer, "getlist")){
                send_sms_next = true;
                // if contains command then perform some actions
                Serial.print("Do stuff after 'getlist' command is received!\n");
                
                // Heading of sms report
                strcpy(sms_text, "UNAUTHORIZED ATTEMPTS: ");
                unauthorized_attempts_total = 0;
                for (int i = 0; i < MAX_UNAUTH; i++)
                    unauthorized_attempts_total += unauthorized_entries[i].attempts;
                sprintf(sms_attempts,"%d", unauthorized_attempts_total);
                strcat(sms_text, sms_attempts);
                strcat(sms_text, "\r");

                for (int i = 0; i < MAX_UNAUTH; i++){
                    if (unauthorized_entries[i].attempts != 0){
                        // compose sms back to sender
                        strcat(sms_text, "Phone: ");
                        strcat(sms_text, unauthorized_entries[i].phone_number);
                        strcat(sms_text, "\r");
                        strcat(sms_text, "Tries: ");
                        sprintf(sms_attempts,"%d", unauthorized_entries[i].attempts);
                        strcat(sms_text, str_attempts);
                        strcat(sms_text, "\r");
                    }
                }
            }
            message_data_next = false;
            GPRS_sms_delall();
        }

        // check if new message notification received
        if (is_containing(buffer, "+CMT:")){
            if (is_authorized(caller_phone_number)){
                // check for new message
                GPRS_sms_read_last();
                strcpy(sms_bouncer_number, caller_phone_number);
                message_data_next = true;
            }   
        }
        clear_buffer(buffer);
        count = 0;
    }
}

void clear_buffer(char* buff){
  for (int i = 0; i < BUFFER_SIZE; i++)
    buff[i] = NULL;
}

void GPRS_init(int baud){
  GPRS.begin(baud);
  digitalWrite(9, LOW); // reboot GPRS without pressing button
  delay(DELAY);
  digitalWrite(9, HIGH);
  delay(DELAY);
  // GPRS.write("ATE0\r"); // disable GPRS echo
  // delay(DELAY);
}

// Send SMS Message
void GPRS_sms_send(char* number){
  char cmd[160];
  strcpy(cmd, "AT+CMGS=\"");
  strcat(cmd, number);
  strcat(cmd, "\",145\r");
  GPRS.write(cmd);
  delay(DELAY);
}

// Read last SMS Message
void GPRS_sms_read_last(){
  GPRS.write("AT+CMGR=1\r");
  delay(DELAY);
}

// List all SMS messages
void GPRS_sms_read_all(){
  GPRS.write("AT+CMGL=\"ALL\"\r");
  delay(DELAY);
}

// Delete All SMS
void GPRS_sms_delall(){
  GPRS.write("AT+CMGDA=\"DEL ALL\"\r");
  delay(DELAY); 
}

// Receive Call
void GPRS_call_receive(){
  GPRS.write("ATA\r");
  delay(DELAY);
}

// Calling Line Identification Presentation
void GPRS_call_info_enable(){
  GPRS.write("AT+CLIP=1\r");
  delay(DELAY);
}

// Hang up Call
void GPRS_call_hangup(){
  GPRS.write("ATH\r");
  delay(DELAY);
}

// Read clock
void GPRS_clock_read() {
  GPRS.write("AT+CCLK?\r");
  delay(DELAY);
}

// Sets clock
void GPRS_clock_set(char* time) {
  char* cmd;
  strcpy(cmd, "AT+CCLK=\r ");
  strcat(cmd, time);
  GPRS.write(cmd);
  delay(DELAY);
}

// Read signal quality
void GPRS_signalQuality() {
  GPRS.write("AT+CSQ\r");
  delay(DELAY);
}

// Read operator name
void GPRS_operator_read() {
  GPRS.write("AT+COPS?\r");
  delay(DELAY);
}

bool is_containing(char* buff, char* str){
    if (strstr(buffer, str) != NULL){
        return true;
    }
    return false;
}

// get the caller phone number
void get_caller_number(char* buffer, char* number){
    char* pch;
    char buff[BUFFER_SIZE];
    // copy buffer in order not to modify it
    strcpy(buff, buffer);
    pch = strtok (buff, "\"");
    while (pch != NULL){
        if (strstr(pch, "358")){
            // Serial.println(pch);
            strncpy(number, pch, 14);
            number[13] = '\0';
            // Serial.println(number);
        }
        pch = strtok (NULL, "\"");
    }
}

// get authorized caller name
void get_caller_name(char* buffer, char* name){
    char* pch;
    char buff[BUFFER_SIZE];
    int counter = 0;
    // copy buffer in order not to modify it
    strcpy(buff, buffer);
    pch = strtok (buff, "\"");
    while (pch != NULL){
        if (counter == 4){
            strcpy(name, pch);
            strcat(name, "\0");
        }
        pch = strtok (NULL, "\"");
        counter++;
    }
}

// check if number authorized
bool is_authorized(char* caller_phone_number){
    for (int i = 0; i < N_ENTRIES; i++){
        if ( strcmp(caller_phone_number, authorized_entries[i]) == 0)
            return true;
    }
    return false;
}

bool is_signalQuality(char* buff) {
  // check if GPRS serial contains CSQ
  if (strstr(buffer, "CSQ") != NULL)
    return true;
  return false;
}

bool is_clock(char* buff) {
  // check if GPRS serial contains CSQ
  if (strstr(buffer, "CCLK") != NULL)
    return true;
  return false;
}

bool is_operatorName(char* buff) {
  //check GPRS serial contains COPS
  if (strstr(buffer, "COPS") != NULL)
    return true;
  return false;
}

// Phone Activity Status
void GPRS_activity_status(void){
    GPRS.write("AT+CPAS\r");
    delay(DELAY);
}

// get current status of GPRS shield
void GPRS_get_state(char* buffer, char* state){
    char* pch;
    char buff[BUFFER_SIZE];
    // copy buffer in order not to modify it
    strcpy(buff, buffer);
    pch = strtok (buff, "\n");
    while (pch != NULL){
        if (strstr(pch, "CPAS: 0")){
            strcpy(state, "idle");
            state[7] = '\0';
        }
        else if (strstr(pch, "CPAS: 3")){
            strcpy(state, "ringing");
            state[7] = '\0';
        }
        pch = strtok (NULL, "\n");
    }
}
