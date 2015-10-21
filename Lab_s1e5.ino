#include <LiquidCrystal.h>
#include <Servo.h>

#define N_ENTRIES 2

// device phone number: 0466259105 
// SIM card pin: 1234

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
}

void loop() {
  serialEvent();
  lcd.home();
  if (stringComplete){ // when string is sent by pressing Enter
    for (int i = 0; i < N_ENTRIES; i++){
      Serial.println(authNumbers[i]); // debugging
      if (input == authNumbers[i]){
        lcd.print("Authorized");
        attempt = 0;
        lcd.setCursor(0,1);
        lcd.print("Attempt: ");
        lcd.print(attempt);
        digitalWrite(LEDPIN, HIGH);
        gate.write(gateMin);
        break; // no need to continue if matches at least one of numbers    
      }else {
        lcd.print("Unauthorized");
        lcd.setCursor(0,1);
        lcd.print("Attempt: ");
        attempt++;
        lcd.print(attempt);   
        digitalWrite(LEDPIN, LOW);
        gate.write(gateMax);
      }
    }
    stringComplete = false; 
    input = ""; 
  }
}

void serialEvent(){
  while(Serial.available()){
    char inChar = (char)Serial.read();
    input += inChar;
    if(inChar == '\n')
      stringComplete = true;
  }
}

