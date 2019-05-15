/*
 * Ham_RepeaterMonitor
 *   This version reduces the flicker of the labels by only updating the changing values. 
 * 
 * https://www.arduino.cc/en/Reference/LiquidCrystalConstructor
 * 
 * 
 * The Circuit:
 *  TBD 
 *
 * LCD RS pin to digital pin 
 */


#include <LiquidCrystal.h>
#include <Wire.h>


#define CHARS_PER_LINE 20
#define LCD_LINES 4

#define FWDPOWER_IN A3
#define REVPOWER_IN A5
#define REDLITE 3
#define GREENLITE 5
#define BLUELITE 6
#define RELAYPIN_OUT 13


// initialize the library with the numbers of the interface pins
// rs, enable, d,d,d,d)
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
   String titleLabel = "Repeater Watchdog v2";
   String repeaterStatusLabel = "Status: ";
   String fwdPowerLabel = "Last FWD Power: ";
   String revPowerLabel = "Last REV Power: ";
   
   int brightness = 255;
   int relayStatus = 0;
   int analogPinFWD = FWDPOWER_IN;
   int analogPinREV = REVPOWER_IN;
   int digitalPinPowerCtrl = RELAYPIN_OUT; 
   int refreshTime = 100; // MilliSeconds
   unsigned long maxSeconds = 16;
   unsigned long countDownSeconds = maxSeconds;
   unsigned long visualWarningSeconds = maxSeconds/4;
   unsigned long startTime = 0;
   int resetTime = 10;  // I can not remember what this was for
   int fwdReading = 0;
   int revReading = 0;
   int voltageThreshold = 400;
   int inTransmit = 0 ; 
   int powerCycleDelay = 5000; // wait for 5 seconds



   
void setup() {
   Serial.begin(9600);
   lcd.begin(CHARS_PER_LINE, LCD_LINES);
   lcd.print(titleLabel);
   pinMode(digitalPinPowerCtrl, OUTPUT);
   pinMode(REDLITE, OUTPUT);
   pinMode(GREENLITE, OUTPUT);
   pinMode(BLUELITE, OUTPUT);
   digitalWrite(digitalPinPowerCtrl, LOW); // default to low for on.
   clearLine(1);
   lcd.print((String)repeaterStatusLabel + "Checking...");
   clearLine(2);
   lcd.print((String)fwdPowerLabel);
   clearLine(3);
   lcd.print((String)revPowerLabel);
}

void powerCycle(){
  setBacklight(255,0,0);
  clearLine(1);
  lcd.print((String)"Power: Set CTRL HIGH");
  digitalWrite(digitalPinPowerCtrl,HIGH); // 
  Serial.println((String) "Power off at " + millis());
  Serial.flush();
  delay(powerCycleDelay);
  clearLine(1);
  lcd.print("Power: Set CTRL LOW ");
  digitalWrite(digitalPinPowerCtrl,LOW);
  Serial.println((String) "Power on at " + millis());
  delay(1000);
  clearLine(1);
  lcd.print((String)repeaterStatusLabel + "Checking...");
  inTransmit=0;  
  startTime=0;
}


void loop() {
  fwdReading = analogRead(analogPinFWD);
  revReading = analogRead(analogPinREV);
  Serial.print((String) "FWD: " + fwdReading + " / REV: " + revReading);
  Serial.print((String) " / inTX: " + inTransmit + " / StartTime: " + startTime);
  Serial.println((String) " Timer: " + countDownSeconds);
  Serial.flush();
  clearLine(2,fwdPowerLabel.length());
  lcd.print((String)fwdReading);
  //lcd.print((String)"Last FWD Power: " + fwdReading );
  clearLine(3,revPowerLabel.length());
  lcd.print((String)revReading);
  //lcd.print((String)"Last REV Power: " + revReading );
  if(fwdReading > voltageThreshold) {
    if(startTime <= 0){
            startTime=millis();
    }
    if(!inTransmit){  //start timer
      inTransmit++;
      setBacklight(0,50,255);  // BLUE in TX 
      startTime=millis();
      clearLine(1,repeaterStatusLabel.length());
      lcd.print((String)"Tx: " + maxSeconds);
    }else{        // check Timer
      countDownSeconds = maxSeconds - ((millis()-startTime)/1000);
      if(countDownSeconds < visualWarningSeconds ){
        setBacklight(255,255,0);
      }
      if(countDownSeconds < 3 ){
        setBacklight(255,0,0);
      }
      clearLine(1,repeaterStatusLabel.length());
      lcd.print((String)"Tx: " + countDownSeconds);
      Serial.println((String) " Timer: " + countDownSeconds);

      if(countDownSeconds <= 0 ){
        powerCycle();
        resetTimer();
      }
    }
    
  }else{ // Repeater is NOT transmitting now
    resetTimer();
  }
  delay(refreshTime);
}

void resetTimer(){
   countDownSeconds = maxSeconds;
   inTransmit=0;
   startTime = 0;
   setBacklight(0,255,0); // Not in TX. GREEN
   clearLine(1,repeaterStatusLabel.length());
   lcd.print((String)"Idle");
   
   
}

void clearLine(int lineIDX){
  clearLine(lineIDX,0);
}

void clearLine(int lineIDX, int charIDX){
  lcd.setCursor(charIDX,lineIDX);
  for(int offset=0;offset < CHARS_PER_LINE-charIDX;offset++){
    lcd.print(" ");
  }
  lcd.setCursor(charIDX,lineIDX);
  
}

void setBacklight(uint8_t r, uint8_t g, uint8_t b) {
  // normalize the red LED - its brighter than the rest!
  r = map(r, 0, 255, 0, 100);
  g = map(g, 0, 255, 0, 150);
 
  r = map(r, 0, 255, 0, brightness);
  g = map(g, 0, 255, 0, brightness);
  b = map(b, 0, 255, 0, brightness);
 
  // common anode so invert!
  r = map(r, 0, 255, 255, 0);
  g = map(g, 0, 255, 255, 0);
  b = map(b, 0, 255, 255, 0);
  Serial.print("R = "); Serial.print(r, DEC);
  Serial.print(" G = "); Serial.print(g, DEC);
  Serial.print(" B = "); Serial.println(b, DEC);
  analogWrite(REDLITE, r);
  analogWrite(GREENLITE, g);
  analogWrite(BLUELITE, b);
}
