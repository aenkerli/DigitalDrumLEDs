/*
 * Project: Digital Drum LEDs
 * Autor: Adrian Enkerli
 * Datum: 17.10.2016
 * Version: 0.3
 */

 /*
  * Do not make any changes at the includes
  */
  
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

/*
 * Global Congfiguration Section
 */

// Global Variables
// Set Analog Inputs
byte triggerBD = 2;
byte triggerMT = 1;
byte triggerST = 0;

// Set Digital Output
byte pinOutBD = 2;
byte pinOutST = 3;
byte pinOutMT = 4;

byte totalLEDsBD = 72;
byte totalLEDsST = 60;
byte totalLEDsMT = 60;

Adafruit_NeoPixel pixelBD = Adafruit_NeoPixel(totalLEDsBD, pinOutBD, NEO_RGBW + NEO_KHZ800);
Adafruit_NeoPixel pixelST = Adafruit_NeoPixel(totalLEDsST, pinOutST, NEO_RGBW + NEO_KHZ800);
Adafruit_NeoPixel pixelMT = Adafruit_NeoPixel(totalLEDsMT, pinOutMT, NEO_RGBW + NEO_KHZ800);

// SleepTime in ms - max 255
byte sleepTime = 50;

// SoftHard Threshold 0 - 1
float hardHitThresh = 0.5;

// Threshold values BD=Bassdrum, MT=MidTom, SM=SmallTom

//Threshold in %
byte hitThresholdBD = 2;
byte hitThresholdST = 50;
byte hitThresholdMT = 50;

unsigned int thresholdBDmax = 750;
unsigned int thresholdBDmin = 350;
unsigned int thresholdMTmax = 750;
unsigned int thresholdMTmin = 350;
unsigned int thresholdSTmax = 750;
unsigned int thresholdSTmin = 350;


/*
 * Do not make any changes after this point
 */

// Initialization on startup
bool initialize = 0;

/*
 * States:
 * 0 = none
 * 1 = soft
 * 2 = hard
 * 3 = sleep
 */
// Hit State
byte hitStates[3] = {0,0,0};

// Last hit time
unsigned long lastHitTime[3] = {0,0,0};

/*
 * ProgrammModus
 * 
 * 0 = Blink
 * 1 = Szene
 * 2 = Pattern
 */
 byte selectedModus = 0;

/*
 * runState
 * 
 * 0 = norun
 * 1 = running
 */
 byte runState[2] = {0,0};
 bool runStateBD = 0;
 bool runStateST = 0;
 bool runStateMT = 0;



/*
 * Classes Section
 */

//Class Hit
class Hit {
  byte inputTrigger;
  unsigned long hitSleepTime;
  unsigned long lastHitMillis;
  byte hitThreshold;
  int maxThreshold;
  int minThreshold;
  int hardThreshMax;
  int hardThreshMin;
  int nullPoint;
  int triggerValue;
  byte i;
  
  public: 
  Hit(byte inTrigger, byte inSleepTime, byte inThreshold) {
    inputTrigger = inTrigger;
    hitSleepTime = inSleepTime;
    hitThreshold = inThreshold;
    lastHitMillis = 0;
  }

  void initialize() {
    i=0;
    while (i<5){
      triggerValue += analogRead(inputTrigger);
      //Serial.print(triggerValue);Serial.print(":::");
      delay (100);
      i++;
    }

    nullPoint = triggerValue / 5;
    Serial.print(nullPoint);Serial.print(":::");
    maxThreshold = nullPoint + (1023 - nullPoint) / 100 * hitThreshold;
    Serial.print(maxThreshold);Serial.print(":::");
    minThreshold = nullPoint - (1023 - nullPoint) / 100 * hitThreshold;
    Serial.println(minThreshold);

    // Callculate hard threshold
    hardThreshMax = maxThreshold + ((1023 - maxThreshold) * hardHitThresh);
    hardThreshMin = minThreshold - (minThreshold * hardHitThresh);
  
  }

  byte detectHit() {
    
    /*
     * Hit-Detection
     * 
     * return hit
     * 0 = none
     * 1 = soft
     * 2 = hard
     * 
     */

    byte hit = 0;
    unsigned long currentMillis = millis();
    
    /*
      Serial.print(triggerSource);Serial.print(":::");
      Serial.print(now);Serial.print(",");
      Serial.print(lastHitTime[triggerSource]);Serial.print(",");
    */
      
      //Check sleeptime
      if ((currentMillis - lastHitMillis) >= sleepTime) {
        /*
         * Sleeptime not active, check hit
         */
    
        // Get analog input value
        triggerValue = analogRead(inputTrigger);
        //Serial.print(inputTrigger); Serial.print(":::");
        //Serial.println(triggerValue);
    
        // Check if a hit was hitten
        if ((triggerValue > maxThreshold) || (triggerValue < minThreshold)) {
          /*
           * hit, detected, check the it's a hard hit else it's soft
           */
           
          if ((triggerValue > hardThreshMax) || (triggerValue < hardThreshMin)) {
            
            hit = 2;
            
          }
          else {
          
            hit = 1;
          
          }    
        }
        lastHitMillis = currentMillis;
      }
      return hit;
  }
  
};


class SmallTom {
  
  int totalLEDs;
  int stepFX;
  bool stateFX; // 1=on, 0=off
  unsigned long previousMillis;
  Adafruit_NeoPixel pixelsSmallTom;


  public:
  SmallTom(Adafruit_NeoPixel pixels){
    totalLEDs = totalLEDsST;
    pixelsSmallTom = pixels;
    previousMillis = 0;
    stepFX = 0;
    stateFX = 1;
    
  }

/*
 * Effect with Steps
 */
  void FX1(){

  unsigned long currentMillis = millis();
  unsigned long waittime = 55;
  
    if ((stateFX == 1) && (currentMillis - previousMillis >= waittime)) {
          if (stepFX < totalLEDs){
            pixelsSmallTom.setPixelColor(stepFX, pixelsSmallTom.Color(0,0,255,0));
            stepFX++;
          }
          else if (stepFX == totalLEDs) {   
            pixelsSmallTom.show();
            stepFX++;
            previousMillis = currentMillis;   
          }
          else if ((stepFX > totalLEDs) && (currentMillis - previousMillis >= waittime)){
            stepFX = 0;
            stateFX = 0;
          }
    }
    else if (stateFX == 0) {

      switch (stepFX) {
        case 15:
          if (currentMillis - previousMillis >= waittime) {
            pixelsSmallTom.show();
            pixelsSmallTom.setPixelColor(stepFX, pixelsSmallTom.Color(0,0,0,0));
            stepFX++; 
          }
        break;
        case 30:
          if (currentMillis - previousMillis >= waittime) {
            pixelsSmallTom.show();
            pixelsSmallTom.setPixelColor(stepFX, pixelsSmallTom.Color(0,0,0,0));
            stepFX++; 
          }
        break;
        case 45:
          if (currentMillis - previousMillis >= waittime) {
            pixelsSmallTom.show();
            pixelsSmallTom.setPixelColor(stepFX, pixelsSmallTom.Color(0,0,0,0));
            stepFX++; 
          }
        break;
        case 60:
            if (currentMillis - previousMillis >= waittime) {
            pixelsSmallTom.show();
            pixelsSmallTom.setPixelColor(stepFX, pixelsSmallTom.Color(0,0,0,0));
            stateFX = 1;
            stepFX = 0;  
            previousMillis = currentMillis;
          }

        break;
        default:
          pixelsSmallTom.setPixelColor(stepFX, pixelsSmallTom.Color(0,0,0,0));
          stepFX++; 
          previousMillis = currentMillis;
        break;
      }
    }
  }
  void start(){
    pixelsSmallTom.begin(); // This initializes the NeoPixel library.
    pixelsSmallTom.show(); // Initialize all pixels to 'off'
  }
};

class MiddleTom {
  
  int totalLEDs;
  int stepFX;
  bool stateFX; // 1=on, 0=off
  unsigned long previousMillis;
  Adafruit_NeoPixel pixelsMiddleTom;


  public:
  MiddleTom(Adafruit_NeoPixel pixels){
    totalLEDs = totalLEDsMT;
    pixelsMiddleTom = pixels;
    previousMillis = 0;
    stepFX = 0;
    stateFX = 1;
    
  }

/*
 * Effect with Steps
 */
  void FX1(){

  unsigned long currentMillis = millis();
  unsigned long waittime = 55;
  
    if ((stateFX == 1) && (currentMillis - previousMillis >= waittime)) {
          if (stepFX < totalLEDs){
            pixelsMiddleTom.setPixelColor(stepFX, pixelsMiddleTom.Color(255,0,0,0));
            stepFX++;
          }
          else if (stepFX == totalLEDs) {   
            pixelsMiddleTom.show();
            stepFX++;
            previousMillis = currentMillis;   
          }
          else if ((stepFX > totalLEDs) && (currentMillis - previousMillis >= waittime)){
            stepFX = 0;
            stateFX = 0;
          }
    }
    else if (stateFX == 0) {

      switch (stepFX) {
        case 15:
          if (currentMillis - previousMillis >= waittime) {
            pixelsMiddleTom.show();
            pixelsMiddleTom.setPixelColor(stepFX, pixelsMiddleTom.Color(0,0,0,0));
            stepFX++; 
          }
        break;
        case 30:
          if (currentMillis - previousMillis >= waittime) {
            pixelsMiddleTom.show();
            pixelsMiddleTom.setPixelColor(stepFX, pixelsMiddleTom.Color(0,0,0,0));
            stepFX++; 
          }
        break;
        case 45:
          if (currentMillis - previousMillis >= waittime) {
            pixelsMiddleTom.show();
            pixelsMiddleTom.setPixelColor(stepFX, pixelsMiddleTom.Color(0,0,0,0));
            stepFX++; 
          }
        break;
        case 60:
            if (currentMillis - previousMillis >= waittime) {
            pixelsMiddleTom.show();
            pixelsMiddleTom.setPixelColor(stepFX, pixelsMiddleTom.Color(0,0,0,0));
            stateFX = 1;
            stepFX = 0;  
            previousMillis = currentMillis;
          }

        break;
        default:
          pixelsMiddleTom.setPixelColor(stepFX, pixelsMiddleTom.Color(0,0,0,0));
          stepFX++; 
          previousMillis = currentMillis;
        break;
      }
    }
  }
  void start(){
    pixelsMiddleTom.begin(); // This initializes the NeoPixel library.
    pixelsMiddleTom.show(); // Initialize all pixels to 'off'
  }
};

class BD {
  unsigned int stepFXColum;
  unsigned int stepFXRow;
  byte totalLEDs;
  byte totalLEDColums;
  byte totalLEDRows;
  unsigned long previousMillisDB;
  Adafruit_NeoPixel pixelsDB;
  bool stateFX;
  
  public:
  BD(Adafruit_NeoPixel pixels){

    totalLEDs = 72;
    totalLEDColums = 9;
    totalLEDRows = 8;
    
    pixelsDB = pixels;   

    stepFXColum = 0;
    stepFXRow = 0;
    previousMillisDB = 0;
    stateFX = 0;

  }
  bool getFXState(){
    return stateFX;
  }
  void FX1(){

    
    unsigned long currentMillisDB = millis();
    unsigned int currentLED = 0;
    unsigned long waittimeBD = 30;
    stateFX = 1;
  
  /*
   * Rows: Colums from - to
   * 0: 0-8
   * 1: 9-17
   * 2: 18-26
   * 3: 27-35
   * 4: 36-44
   * 5: 45-53
   * 6: 54-62
   * 7: 63-71
   */
    if ((stepFXColum < totalLEDColums) && (currentMillisDB - previousMillisDB > waittimeBD)){
      
      if (stepFXRow < totalLEDRows){
  
        switch (stepFXColum){
          case 0:
            currentLED = (stepFXRow * totalLEDColums) + stepFXColum;
            pixelsDB.setPixelColor(currentLED, pixelsDB.Color(0,255,0,0));
          break;
  
          default:
            currentLED = (stepFXRow * totalLEDColums) + stepFXColum;
            pixelsDB.setPixelColor(currentLED, pixelsDB.Color(0,255,0,0));
  
            currentLED = (stepFXRow * totalLEDColums) + (stepFXColum - 1);
            pixelsDB.setPixelColor(currentLED, pixelsDB.Color(0,0,0,0));
  
          break;
        }
  
        pixelsDB.show();
        stepFXRow++;
        
      }
      else {
       
        stepFXColum++;
        stepFXRow=0;
        previousMillisDB = currentMillisDB;
  
      }
  
    } else if (stepFXColum == totalLEDColums) {
  
            //Deactivate the previous row
      if (stepFXRow < totalLEDRows){
  
            currentLED = (stepFXRow * totalLEDColums) + (stepFXColum - 1);
            pixelsDB.setPixelColor(currentLED, pixelsDB.Color(0,0,0,0));
            stepFXRow++;
      }
      else
      {
        stepFXColum=0;
        stepFXRow=0;
        pixelsDB.show();
        previousMillisDB = currentMillisDB;
        stateFX = 0;
      }
    }
  }
  void start(){
    pixelsDB.begin(); // This initializes the NeoPixel library.
    pixelsDB.show(); // Initialize all pixels to 'off'
  }
};


class ProgControl {
  public:
  ProgControl(){
    
  }

  void runProgTom() {
    
  }
  
};

// Create the Toms
SmallTom smallTom(pixelST);
MiddleTom middleTom(pixelMT);
BD bassdrum(pixelBD);

// Create Hits
Hit hitBD(triggerBD,sleepTime,hitThresholdBD);
Hit hitST(triggerST,sleepTime,hitThresholdST);
Hit hitMT(triggerMT,sleepTime,hitThresholdMT);
int cnt = 0;


void setup() {

 //Serial.begin(9600);          //  setup serial

  pixelBD.begin();
  pixelBD.show();
  pixelST.begin();
  pixelST.show();
  pixelMT.begin();
  pixelMT.show();

}

void loop() {
  // Get current Time
  //unsigned long currentMillis = millis();
  if (initialize == 0){
    //Serial.println("Start Initialize");
    hitBD.initialize();
    hitST.initialize();
    hitMT.initialize();
    initialize = 1;
  }


  //Check BD
  /*
  hitStates[0] = hitdetection(triggerBD, thresholdBDmax, thresholdBDmin);
  hitStates[1] = hitdetection(triggerMT, thresholdMTmax, thresholdMTmin);
  hitStates[2] = hitdetection(triggerST, thresholdSTmax, thresholdSTmin);
  */
  //Serial.println(hitBD.detectHit());
  programmcontroller(hitBD.detectHit());
/*
  if (hitBD != 0){
    bassdrum.FX1();
  }
    if (hitST != 0){
    bassdrum.FX1();
  }
    if (hitMT != 0){
    bassdrum.FX1();
  }
  smallTom.FX1();
  middleTom.FX1();
  */
}
void programmcontroller(byte inHit){

  bool isRunningBD = bassdrum.getFXState();
  byte hitStateDB = inHit;
  if ((isRunningBD == 1) || (hitStateDB > 0)){
    bassdrum.FX1();
  }
}
/*void programmcontroller(byte hitState[3]){

// first select programm

  switch (selectedModus) {

    case 0: //blink
      for (int trigger = 0; trigger < 3; trigger++){
        
        if (runState[trigger] == 0){ //not Running
          
          if (hitState[trigger] == 0){// no hit detected
            break;
          }
          //start new programm with step 1
          
        }
        else{ // running
          // next Step
        }
      }
    break;
    
    case 1: //szene
    break;
    
    case 2: //pattern
    break;
    
    default:
    break;
  }
}
*/


