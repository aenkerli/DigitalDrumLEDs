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



// SleepTime in ms - max 255
byte sleepTime = 50;

// SoftHard Threshold 0 - 1
float hardHitThresh = 0.5;

// Threshold values BD=Bassdrum, MT=MidTom, SM=SmallTom

//Threshold in %
byte hitThresholdBD = 2;
byte hitThresholdST = 5;
byte hitThresholdMT = 5;

/*
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * 
 * Do not make any changes after this point
 * 
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

// Initialization on startup
bool initialize = 0;

/*
 * ProgrammModus
 * 
 * 0 = Blink
 * 1 = Scene
 * 2 = Pattern
 */
 byte selectedMode = 0;
 
/*
 * FX
 * 
 * 0 = Flash
 * 1 = FX1
 * 2 = FX2
 */
 byte selectedFX = 1;

/*
 * Scene
 * 
 * 0 = Scene 1
 * 1 = Scene 2
 * 
 */
 byte selectedScene = 0;
 
 /*
 * Pattern
 * 
 * 0 = Pattern 1
 * 1 = Pattern 2
 * 
 */
 byte selectedPattern = 0;

 uint32_t colorLED;

Adafruit_NeoPixel pixelBD = Adafruit_NeoPixel(totalLEDsBD, pinOutBD, NEO_RGBW + NEO_KHZ800);
Adafruit_NeoPixel pixelST = Adafruit_NeoPixel(totalLEDsST, pinOutST, NEO_RGBW + NEO_KHZ800);
Adafruit_NeoPixel pixelMT = Adafruit_NeoPixel(totalLEDsMT, pinOutMT, NEO_RGBW + NEO_KHZ800);


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

// Superclass Drum
class Drum {

  public:
 
  Drum(){ }
  
  virtual void FX1() = 0;
  virtual bool getFXState() = 0;
  virtual void Flash() = 0;
  virtual void FX2() = 0;
 
  void start(){ }
};

class Tom : public Drum {
  
  int totalLEDs;
  int stepFX;
  bool stateFX; // 1=on, 0=off
  bool intStateFX; // 1=on, 0=off
  unsigned long previousMillis;
  Adafruit_NeoPixel pixelsTom;

  public:
  Tom(Adafruit_NeoPixel pixels){
    totalLEDs = totalLEDsST;
    pixelsTom = pixels;
    previousMillis = 0;
    stepFX = 0;
    stateFX = 0;
    intStateFX = 1; 
  }

/*
 * Effect with Steps
 */
  void Flash(){
    
  }
  void FX1(){
  
  stateFX = 1;
  unsigned long currentMillis = millis();
  unsigned long waittime = 55;
  
    if ((intStateFX == 1) && (currentMillis - previousMillis >= waittime)) {
          if (stepFX < totalLEDs){
            pixelsTom.setPixelColor(stepFX, pixelsTom.Color(0,0,255,0));
            stepFX++;
          }
          else if (stepFX == totalLEDs) {   
            pixelsTom.show();
            stepFX++;
            previousMillis = currentMillis;   
          }
          else if ((stepFX > totalLEDs) && (currentMillis - previousMillis >= waittime)){
            stepFX = 0;
            intStateFX = 0;
          }
    }
    else if (intStateFX == 0) {

      switch (stepFX) {
        case 15:
          if (currentMillis - previousMillis >= waittime) {
            pixelsTom.show();
            pixelsTom.setPixelColor(stepFX, pixelsTom.Color(0,0,0,0));
            stepFX++; 
          }
        break;
        case 30:
          if (currentMillis - previousMillis >= waittime) {
            pixelsTom.show();
            pixelsTom.setPixelColor(stepFX, pixelsTom.Color(0,0,0,0));
            stepFX++; 
          }
        break;
        case 45:
          if (currentMillis - previousMillis >= waittime) {
            pixelsTom.show();
            pixelsTom.setPixelColor(stepFX, pixelsTom.Color(0,0,0,0));
            stepFX++; 
          }
        break;
        case 60:
            if (currentMillis - previousMillis >= waittime) {
            pixelsTom.show();
            pixelsTom.setPixelColor(stepFX, pixelsTom.Color(0,0,0,0));
            intStateFX = 1;
            stepFX = 0;  
            previousMillis = currentMillis;
            stateFX = 0;
          }

        break;
        default:
          pixelsTom.setPixelColor(stepFX, pixelsTom.Color(0,0,0,0));
          stepFX++; 
          previousMillis = currentMillis;
        break;
      }
    }
  }
  void FX2(){
    
  }
  void start(){
    pixelsTom.begin(); // This initializes the NeoPixel library.
    pixelsTom.show(); // Initialize all pixels to 'off'
  }
  bool getFXState(){
    return stateFX;
  }
};

class BD : public Drum{
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
  void Flash(){
    
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
  void FX2(){
    
  }
  void start(){
    pixelsDB.begin(); // This initializes the NeoPixel library.
    pixelsDB.show(); // Initialize all pixels to 'off'
  }
};

// Create the Toms
Tom smallTomObj(pixelST);
Tom middleTomObj(pixelMT);
BD bassdrumObj(pixelBD);

//Create Pointer
Drum *bassdrum = &bassdrumObj;
Drum *middleTom = &middleTomObj;
Drum *smallTom = &smallTomObj;

// Create Hits
Hit hitBD(triggerBD,sleepTime,hitThresholdBD);
Hit hitST(triggerST,sleepTime,hitThresholdST);
Hit hitMT(triggerMT,sleepTime,hitThresholdMT);


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

  //Serial.println(hitBD.detectHit());
  programmcontroller(hitBD.detectHit(), bassdrum);
  programmcontroller(hitST.detectHit(), smallTom);
  programmcontroller(hitMT.detectHit(), middleTom);

}
void programmcontroller(byte inHit, Drum *inDrum){

  bool isRunning = inDrum->getFXState();
  byte hitState = inHit;

  switch (selectedMode) {
    case 0: // Blink
    
      if ((isRunning == 1) || (hitState > 0)){

        switch (selectedFX) {
          case 0: // Flash
            inDrum->Flash();
          break;
          case 1: // FX1
            inDrum->FX1();
          break;
          case 2: // FX2
            inDrum->FX2();
          break;
          default:
          break;
        }
      }


    

    break;
    case 1: // Scene
    break;
    case 2: // Pattern
    break;
    default:
    break; 
  }
}



