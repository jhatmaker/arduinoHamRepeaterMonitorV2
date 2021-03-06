/*
 * Ham_RepeaterMonitor
 * 
 * 
 * https://www.arduino.cc/en/Reference/LiquidCrystalConstructor
 * 
 * 
 * The Circuit:
 * 
 * LCD RS pin to digital pin 
 * LCD 
 * PIN (CTRL) -> connection PIN (LABEL)
 *  1 (Vss Ground) -> Arduino P4 (GND)
 *  2 (Vdd 5v PS) ->  Arduino P3 (5v)
 *  3 (VO Contrast Adjust)  -> center pin of pot
 *  4 (RS Data/Instruction select signal) -> Arduino Digital 7
 *  5 (R/W ) -> ground
 *  6 (E Enable)  -> Arduino Digital 8
 *  7 (DB0 Databus) -> X 
 *  8 (DB1 Databus) -> X
 *  9 (DB2 Databus) -> X
 * 10 (DB3 Databus) -> X
 * 11 (DB4 Databus) -> Arduino Digital 9
 * 12 (DB5 Databus) -> Arduino Digital 10
 * 13 (DB6 Databus) -> Arduino Digital 11
 * 14 (DB7 Databus) -> Arduino Digital 12
 * 15 (A PS for B/L+) -> 5v Arduino P3 (5v)
 * 16 (R PS for B/L RED) -> Arduino Digital 3
 * 17 (G PS for B/L GREEN) -> Arduino Digital 5
 * 18 (B PS for B/L BLUE) -> Arduino Digital 6
 *  
 *  
 *  Arduino View/Pinout
 *  Power P1 (Reset) -> X
 *  Power P2 (3.3v)  -> X
 *  Power P3 (5v) -> To Display
 *  Power P4 (GND) -> To Display
 *  Power P5 (GND) -> X
 *  Power P6 (Vin) -> X
 *  
 *  Analog A0 -> X
 *  Analog A1 -> X
 *  Analog A2 -> X
 *  Analog A3 -> FWD voltage reading
 *  Analog A4 -> X
 *  Analog A5 -> REV voltage reading
 *  
 *  AREF -> X
 *  Ground (GND) -> to control relay -
 *  Digital D13 -> X
 *  Digital D12 -> Display (DB7) 14 
 *  Digital D11 -> Display (DB6) 13
 *  Digital D10 -> Display (DB5) 12
 *  Digital D9 ->  Display (DB4) 11
 *  Digital 08 -> Display (E) PIN 6
 *  Digital 07 -> Display (RS) PIN 4
 *  Digital 06 -> Display (B) BLUE PIN 18
 *  Digital 05 -> Display (G) GREEN PIN 17
 *  Digital 04 -> X
 *  Digital 03 -> Display (R) RED PIN 16
 *  Digital 02 -> to control relay +
 *  Digital 01 -> X
 *  Digital 00 -> X
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
#define RELAYPIN_OUT 2

#if LCD_LINES == 2 
#define LABEL_LINE1 "Status: "
#define PWRHIGHMSG "Trigger HIGH "
#define PWRLOWMSG "Trigger LOW "
#define MAXFWDLEN 6
#define MAXREVLEN 6
String repeaterStatusLabel = LABEL_LINE1;
String fwdPowerLabel = "F "
String revPowerLabel = "R "

  
#elif LCD_LINES == 4
#define LABEL_LINE1 "Repeater Watchdog v2"
#define LABEL_LINE2 "Status: "
#define PWRHIGHMSG "Trigger HIGH "
#define PWRLOWMSG "Trigger LOW "

//String titleLabel = "Repeater Watchdog v2";
String titleLabel = LABEL_LINE1;
String repeaterStatusLabel = LABEL_LINE2;
String fwdPowerLabel = "FWD Power: ";
String revPowerLabel = "REV Power: ";

      

#else

#endif

void displayStatus( String );
void displayPower(String , String );
void displayPowerCycle(String);


// initialize the library with the numbers of the interface pins
// rs, enable, d,d,d,d)
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

   int brightness = 255;
   int relayStatus = 0;
   int analogPinFWD = FWDPOWER_IN;
   int analogPinREV = REVPOWER_IN;
   int digitalPinPowerCtrl = RELAYPIN_OUT; 
   int refreshTime = 100; // MilliSeconds
   // Testing value 
   unsigned long maxSeconds = 5;
   // Real Value
   //unsigned long maxSeconds = 300;
   unsigned long countDownSeconds = maxSeconds;
   unsigned long visualWarningSeconds = maxSeconds/4;
   unsigned long startTime = 0;
   int resetTime = 10;  // I can not remember what this was for
   int fwdReading = 0;
   int revReading = 0;
   int voltageThreshold = 30;
   int inTransmit = 0 ; 
   int powerCycleDelay = 5000; // wait for 5 seconds
   int loopCount = 0;
   int displayCount = 100;
   String statusMsg = "";



   
void setup() {
   Serial.begin(9600);
   lcd.begin(CHARS_PER_LINE, LCD_LINES);
   // Set PIN 13 to LOW (OUTPUT) to turn off the LED
   pinMode(13, OUTPUT);
   digitalWrite(13, LOW);
   pinMode(digitalPinPowerCtrl, OUTPUT);
   pinMode(REDLITE, OUTPUT);
   pinMode(GREENLITE, OUTPUT);
   pinMode(BLUELITE, OUTPUT);
   digitalWrite(digitalPinPowerCtrl, LOW); // default to low for on.
   initDisplayLabels();
   
}

void powerCycle(){
  setBacklight(255,0,0);
  displayPowerCycle((String)PWRHIGHMSG );
  digitalWrite(digitalPinPowerCtrl,HIGH); // 
  Serial.println((String) "Power off at " + millis());
  Serial.flush();
  delay(powerCycleDelay);
  displayPowerCycle((String)PWRLOWMSG );
  digitalWrite(digitalPinPowerCtrl,LOW);
  Serial.println((String) "Power on at " + millis());
  delay(1000);
  initDisplayLabels();
  inTransmit=0;  
  startTime=0;
}


void loop() {
  loopCount++;
  fwdReading = analogRead(analogPinFWD);
  revReading = analogRead(analogPinREV);
  Serial.print((String) "FWD: " + fwdReading + " / REV: " + revReading);
  Serial.print((String) " / inTX: " + inTransmit + " / StartTime: " + startTime);
  Serial.println((String) " Timer: " + countDownSeconds);
  Serial.flush();
  if(loopCount = displayCount){
  loopCount = 0;
  // TESTING
  displayPower((String)toWatts(fwdReading), (String)toWatts(revReading));
  
  

  }
  if(fwdReading > voltageThreshold) {
    if(startTime <= 0){
            startTime=millis();
    }
    if(!inTransmit){  //start timer
      inTransmit++;
      setBacklight(0,50,255);  // BLUE in TX 
      startTime=millis();
      displayStatus((String)"Tx " + maxSeconds);
    }else{        // check Timer
      countDownSeconds = maxSeconds - ((millis()-startTime)/1000);
      if(countDownSeconds < visualWarningSeconds ){
        setBacklight(255,255,0);
      }
      if(countDownSeconds < 3 ){
        setBacklight(255,0,0);
      }
      // TESTING DISPLAY
      displayStatus((String)"Tx " + countDownSeconds);
      
      //clearLine(1,repeaterStatusLabel.length());
      //lcd.print((String)"Tx " + countDownSeconds);
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
   displayStatus((String)"Idle");
   //clearLine(1,repeaterStatusLabel.length());
   //lcd.print((String)"Idle");
   
   
}

void clearLine(int lineIDX){
  lcd.setCursor(0,lineIDX);
  for(int i=0;i<CHARS_PER_LINE;i++){
    lcd.print(" ");
  }
  lcd.setCursor(0,lineIDX);

}

void clearLine(int lineIDX, int charIDX){
  lcd.setCursor(charIDX,lineIDX);
  for(int offset=0;offset < CHARS_PER_LINE-charIDX;offset++){
    lcd.print(" ");
  }
  lcd.setCursor(charIDX,lineIDX);
}

void clearLine(int lineIDX, int charIDX, int charLEN){
  lcd.setCursor(charIDX,lineIDX);
  int maxChar = (CHARS_PER_LINE-charIDX) < charLEN ? CHARS_PER_LINE-charIDX : charLEN;
  for(int offset=0;offset < maxChar;offset++){
    lcd.print(" ");
  }
  lcd.setCursor(charIDX,lineIDX);
  
}

double toWatts(int a){
  if(a>0)
    return pow(10,a/66.0);
  return 0.00;
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


#if LCD_LINES == 2 

void initDisplayLabels() {
  clearLine(0);
   lcd.print((String)repeaterStatusLabel + "Wait...");
   clearLine(1);
   lcd.print((String)fwdPowerLabel);
   clearLine(1,10);
   lcd.print((String)revPowerLabel);
}
void displayPower(fVal, rVal) {
  clearLine(1,fwdPowerLabel.length(), MAXFWDLEN);
  lcd.print((String)fwd);
  clearLine(1,fwdPowerLabel.length() + MAXFWDLEN + revPowerLabel.length(), MAXREVLEN);
  lcd.print((String)rev);
}
void displayPowerCycle() {
     clearLine(0);
    lcd.print((String)msg);  
  
}
void displayStatus(String statusMsg) {
   clearLine(1,repeaterStatusLabel.length());
   lcd.print((String)statusMsg);
}
  
#elif LCD_LINES == 4

void initDisplayLabels() {
  // If 4 line display
  clearLine(0);
  lcd.print(titleLabel);
   clearLine(1);
   lcd.print((String)repeaterStatusLabel + "Checking...");
   clearLine(2);
   lcd.print((String)fwdPowerLabel);
   clearLine(3);
   lcd.print((String)revPowerLabel);
}
void displayPowerCycle(String msg) {
    clearLine(1);
    lcd.print((String)msg);  
}
void displayPower(String fwd, String rev) {
  clearLine(2,fwdPowerLabel.length());
  lcd.print((String)fwd);
  clearLine(3,revPowerLabel.length());
  lcd.print((String)rev);
}

void displayStatus( String statusMsg) {
   clearLine(1,repeaterStatusLabel.length());
   lcd.print((String)statusMsg);
}
      

#else
void initDisplayLabels() {
}
void diplayPowerCycle() {
  
}
void displayPower() {
  
}
void displayStatus((String)statusMsg){
  // nothing;
}
#endif
