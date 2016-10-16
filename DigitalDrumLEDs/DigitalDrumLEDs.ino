/*
 * Project: Digital Drum LEDs
 * Autor: Adrian Enkerli
 * Datum: 07.10.2016
 * Version: 0.2
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
byte triggerBD = 0;
byte triggerMT = 1;
byte triggerST = 2;

// Set Digital Output
byte pinOutBD = 2;
byte pinOutST = 3;
byte pinOutMT = 4;

// SleepTime in ms - max 255
byte sleepTime = 50;

// SoftHard Threshold 0 - 1
float hardHitThresh = 0.5;

// Threshold values BD=Bassdrum, MT=MidTom, SM=SmallTom

//Threshold in %
byte hitThresholdBD = 50;
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


/*
 * Classes Section
 */

//Class Hit
class Hit {
  byte inputTrigger;
  byte hitSleepTime;
  unsigned long lastHitMillis;
  byte hitThreshold;
  int maxThreshold;
  int minThreshold;
  int nullPoint;
  
  public: 
  Hit(byte inTrigger, byte inSleepTime, byte inThreshold) {
    inputTrigger = inTrigger;
    hitSleepTime = inSleepTime;
    hitThreshold = inThreshold;
    lastHitMillis = 0;
  }

  void initialize() {

  //Getnullpoint and callculate Threshold 
    /* Schleife 5 mal je nach 100 ms Wert asulesen und dann alle mitteln
    triggerValue = analogRead(inputTrigger);
    */
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
    unsigned int triggerValue = 512;
    byte hit = 0;
    unsigned long currentMillis = millis();
    
    /*
      Serial.print(triggerSource);Serial.print(":::");
      Serial.print(now);Serial.print(",");
      Serial.print(lastHitTime[triggerSource]);Serial.print(",");
    */
      
      //Check sleeptime
      if ( (lastHitTime == 0) || ((currentMillis - lastHitMillis) > sleepTime)) {
        /*
         * Sleeptime not active, check hit
         */
    
        // Get analog input value
        triggerValue = analogRead(inputTrigger);
    
        // Check if a hit was hitten
        if ((triggerValue > maxThreshold) || (triggerValue < minThreshold)) {
          /*
           * hit, detected, check the it's a hard hit else it's soft
           */
    
          // Callculate hard threshold
          unsigned int hardThreshMax = maxThreshold + ((1023 - maxThreshold) * hardHitThresh);
          unsigned int hardThreshMin = minThreshold - (minThreshold * hardHitThresh);
          
          if ((triggerValue > hardThreshMax) || (triggerValue < hardThreshMin)) {
            
            hit = 2;
            
          }
          else {
          
            hit = 1;
          
          }    
        }
        lastHitMillis = millis();
      }
      return hit;
  }
  
};

/*
 * Superclass Drum
 */
class Drum {

  public:

  Adafruit_NeoPixel pixels;
  
  Drum(){ }

  void FX1(){ }
  
  void start(){
    pixels.begin(); // This initializes the NeoPixel library.
    pixels.show(); // Initialize all pixels to 'off'
  }

  
};

class Tom : public Drum {
  int stepFX;
  int maxSteps;
  int totalLEDs;
  bool stateFX; // 1=on, 0=off
  unsigned long previousMillis;
  public:
  
  Tom(byte pin){

    totalLEDs = 60;
    pixels = Adafruit_NeoPixel(totalLEDs, pin, NEO_RGBW + NEO_KHZ800);   
    stepFX = 0;
    stateFX = 1;
    previousMillis = 0;
    
  }

/*
 * Effect with Steps
 */
  void FX1(unsigned long currentMillis){
  
  unsigned long waittime = 55;
  
    if ((stateFX == 1) && (currentMillis - previousMillis > waittime)) {
          if (stepFX < totalLEDs){
            pixels.setPixelColor(stepFX, pixels.Color(0,0,255,0));
            stepFX++;
          }
          else if (stepFX == totalLEDs) {   
            pixels.show();
            stepFX++;
            previousMillis = millis();   
          }
          else if ((stepFX > totalLEDs) && (currentMillis - previousMillis > waittime)){
            stepFX = 0;
            stateFX = 0;
          }
    }
    else if (stateFX == 0) {

      switch (stepFX) {
        case 15:
          if (currentMillis - previousMillis > waittime) {
            pixels.show();
            pixels.setPixelColor(stepFX, pixels.Color(0,0,0,0));
            stepFX++; 
          }
        break;
        case 30:
          if (currentMillis - previousMillis > waittime) {
            pixels.show();
            pixels.setPixelColor(stepFX, pixels.Color(0,0,0,0));
            stepFX++; 
          }
        break;
        case 45:
          if (currentMillis - previousMillis > waittime) {
            pixels.show();
            pixels.setPixelColor(stepFX, pixels.Color(0,0,0,0));
            stepFX++; 
          }
        break;
        case 60:
            if (currentMillis - previousMillis > waittime) {
            pixels.show();
            pixels.setPixelColor(stepFX, pixels.Color(0,0,0,0));
            stateFX = 1;
            stepFX = 0;  
            previousMillis = millis();
          }

        break;
        default:
          pixels.setPixelColor(stepFX, pixels.Color(0,0,0,0));
          stepFX++; 
          previousMillis = millis();
        break;
      }
    }
  }
};

class BD : public Drum {
  unsigned int stepFX;
  unsigned int stepFXColum;
  unsigned int stepFXRow;
  byte totalLEDs;
  byte totalLEDColums;
  byte totalLEDRows;
  unsigned long previousMillis;
  
  public:
  BD(byte pin){

    totalLEDs = 72;
    totalLEDColums = 9;
    totalLEDRows = 8;
    
    pixels = Adafruit_NeoPixel(totalLEDs, pin, NEO_RGBW + NEO_KHZ800);   
    
    stepFX = 0;
    stepFXColum = 0;
    stepFXRow = 0;
    previousMillis = 0;

  }

  void FX1(unsigned long currentMillis){
  
    unsigned int currentLED = 0;
    unsigned long waittime = 30;
  
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
    if ((stepFXColum < totalLEDColums) && (currentMillis - previousMillis > waittime)){
      
      if (stepFXRow < totalLEDRows){
  
        switch (stepFXColum){
          case 0:
            currentLED = (stepFXRow * totalLEDColums) + stepFXColum;
            pixels.setPixelColor(currentLED, pixels.Color(0,0,255,0));
          break;
  
          default:
            currentLED = (stepFXRow * totalLEDColums) + stepFXColum;
            pixels.setPixelColor(currentLED, pixels.Color(0,0,255,0));
  
            currentLED = (stepFXRow * totalLEDColums) + (stepFXColum - 1);
            pixels.setPixelColor(currentLED, pixels.Color(0,0,0,0));
  
          break;
        }
  
        pixels.show();
        stepFXRow++;
        
      }
      else {
       
        stepFXColum++;
        stepFXRow=0;
        previousMillis = millis();
  
      }
  
    } else if (stepFXColum == totalLEDColums) {
  
            //Deactivate the previous row
      if (stepFXRow < totalLEDRows){
  
            currentLED = (stepFXRow * totalLEDColums) + (stepFXColum - 1);
            pixels.setPixelColor(currentLED, pixels.Color(0,0,0,0));
            stepFXRow++;
      }
      else
      {
        stepFXColum=0;
        stepFXRow=0;
        pixels.show();
        previousMillis = millis();
      }
    }
  }
};


class ProgControl {
  public:
  ProgControl(){
    
  }

  void runProgTom(Tom inTom) {
    
  }
  
};

// Create the Toms
Tom smallTom(pinOutST);
Tom middleTom(pinOutMT);
BD bassdrum(pinOutBD);

// Create Hits
Hit hitBD(triggerBD,sleepTime,hitThresholdBD);
Hit hitST(triggerST,sleepTime,hitThresholdST);
Hit hitMT(triggerMT,sleepTime,hitThresholdMT);

void setup() {

 //Serial.begin(9600);          //  setup serial

 // Start the Neopixel
  smallTom.start();
  middleTom.start();
  bassdrum.start();
}

void loop() {
  // Get current Time
  unsigned long currentMillis = millis();

  //Check BD
  /*
  hitStates[0] = hitdetection(triggerBD, thresholdBDmax, thresholdBDmin);
  hitStates[1] = hitdetection(triggerMT, thresholdMTmax, thresholdMTmin);
  hitStates[2] = hitdetection(triggerST, thresholdSTmax, thresholdSTmin);
  */
  programmcontroller(hitStates);

}

void programmcontroller(byte hitState[3]){

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



