/*
 * Project: Digital Drum LEDs
 * Autor: Adrian Enkerli
 * Datum: 20.10.2016
 * Version: 0.7
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
byte totalLEDColumsBD = 9;
byte totalLEDRowsBD = 8;

byte totalLEDsST = 60;
byte totalLEDColumsST = 15;
byte totalLEDRowsST = 4;

byte totalLEDsMT = 60;
byte totalLEDColumsMT = 15;
byte totalLEDRowsMT = 4;



// SleepTime in ms - max 255
byte sleepTime = 80;

// SoftHard Threshold 0 - 1
float hardHitThresh = 0.5;

// Threshold values BD=Bassdrum, MT=MidTom, SM=SmallTom

//Threshold in %
byte hitThresholdBD = 2;
byte hitThresholdST = 5;
byte hitThresholdMT = 5;

/*
 * Define Patterns using DrumType
 * 
 * DrumType: b=Bassdrum, s=SmallTom, m=MiddleTom
 * 
 */
char pattern1[5] = "ssss";
char pattern2[5] = "bsmm";

byte midiChannelOn = "Ax90";
byte midiChannelOff = "Ax80";
byte midiNoteBD = "0x36";
byte midiNoteST = "0x50";
byte midiNoteMT = "0x47";
byte midiVelocityOff = "0x00";
byte midiVelocitySoft = "0x20";
byte midiVelocityHard = "0x46";

/*
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * 
 * Do not make any changes after this point
 * 
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

// Initialization on startup
bool initialized = false;
bool isRunning = false;
bool isRunningBD = false;
bool isRunningST = false;
bool isRunningMT = false;
/*
 * ProgrammModus
 * 
 * 0 = Blink
 * 1 = Scene
 * 2 = Pattern
 */
 byte selectedMode = 1;
 
/*
 * FX
 * 
 * 0 = Flash
 * 1 = FX1
 * 2 = FX2
 */
 byte selectedFX = 0;

/*
 * Scene
 * 
 * 0 = Scene 1
 * 1 = Scene 2
 * 
 */
 byte selectedScene = 0;
 int stepScene = 0;
 bool stateScene = false;
 bool sceneFirstTime = true;
 unsigned long previousMillis = 0;
 /*
 * Pattern
 * 
 * 0 = Pattern 1
 * 1 = Pattern 2
 * 
 */
 byte selectedPattern = 0;
 bool patternFirstRound = true;
 char patternCue1[5];
 char patternCue2[5];
 char patternCue3[5];
 char patternCue4[5];
 bool patternMatch = false;
 byte stepPattern = 0;

 Adafruit_NeoPixel pixelBD = Adafruit_NeoPixel(totalLEDsBD, pinOutBD, NEO_RGBW + NEO_KHZ800);
 Adafruit_NeoPixel pixelST = Adafruit_NeoPixel(totalLEDsST, pinOutST, NEO_RGBW + NEO_KHZ800);
 Adafruit_NeoPixel pixelMT = Adafruit_NeoPixel(totalLEDsMT, pinOutMT, NEO_RGBW + NEO_KHZ800);

 uint32_t colorLED = pixelBD.Color(90,0,200,0);
 uint32_t colorLEDBlack = pixelBD.Color(0,0,0,0);

/*
 * Console Buttons and LED Pins
 */

 byte pinBtnConfig = 34;
 byte pinLEDConfig = 22;
 byte pinBtnMode = 24;
 byte pinLEDModeBlink = 25;
 byte pinLEDModeScene = 26;
 byte pinLEDModePattern = 27;

/*
 * Configuration
 */
 bool lastBtnConfigState;
 bool btnConfigState;
 bool lastBtnModeState;
 bool btnModeState;
 bool configMode = false;

/*
 * Ext. Interfaces
 */
 byte lastMidiState = 0;


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
  byte cntInit;
  
  public: 
  Hit(byte inTrigger, byte inSleepTime, byte inThreshold) {
    inputTrigger = inTrigger;
    hitSleepTime = inSleepTime;
    hitThreshold = inThreshold;
    lastHitMillis = 0;
  }

  void initialize() {
    cntInit=0;
    while (cntInit<5){
      triggerValue += analogRead(inputTrigger);
      //Serial.print(triggerValue);Serial.print(":::");
      delay (100);
      cntInit++;
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

  protected:
  char typeDrum;
  bool stateFX;
  bool intStateFX;
  int stepFX;
  unsigned long previousMillis;
  byte totalLEDs;
  byte totalLEDColums;
  byte totalLEDRows;
  Adafruit_NeoPixel pixels;
  byte midiNote;
  byte lastMidiState;
  
  public:
  Drum(){ }

  virtual void FX1() = 0;
  virtual void FX2() = 0;
  
  byte getDrumType() {
    return typeDrum;
  }
  
  bool getFXState() {
    return stateFX;
  }

  void start(){
    pixels.begin();
    pixels.show();
  }

  void Flash(){
    stateFX = 1;
    unsigned long currentMillis = millis();
    unsigned long waittime = 55;
    
    if (intStateFX == 1) {
      if (stepFX < totalLEDs){
        pixels.setPixelColor(stepFX, colorLED);
        stepFX++;
      }
      else if (stepFX == totalLEDs) {   
        pixels.show();
        stepFX++;
        previousMillis = currentMillis;   
      }
      else if ((stepFX > totalLEDs) && (currentMillis - previousMillis >= waittime)){
        stepFX = 0;
        intStateFX = 0;
      }
    }
    else if (intStateFX == 0) {

      if (stepFX < totalLEDs){
        pixels.setPixelColor(stepFX, colorLEDBlack);
        stepFX++;
      }
      else if (stepFX == totalLEDs) {   
        pixels.show();
        stepFX = 0;
        intStateFX = 1;
        stateFX = 0;
      }
    }
  }
  void sendMidi(byte inHit){
  byte midiState = inHit;
  
  // mIdi chanel 10 
  // Note 50 High Tom 1
  // Note 36 Bass Drum 1
  // Note 47 Mid Tom 1
  
  if (midiState != lastMidiState) {
    switch (midiState) {
      case 0: // no Hit - tunr Midi note off
        Serial.write(midiChannelOff);
        Serial.write(midiNote);
        Serial.write(midiVelocityOff);
      break;
      case 1: // soft hit - turn Midi note on
        Serial.write(midiChannelOn);
        Serial.write(midiNote);
        Serial.write(midiVelocitySoft); //mf
      break;
      case 2: // hard Hit - tunr Midi note on
        Serial.write(midiChannelOn);
        Serial.write(midiNote);
        Serial.write(midiVelocityHard); //fff
      break;
    }
    lastMidiState = midiState;
  }
}


};

class Tom : public Drum {

  public:
  Tom(Adafruit_NeoPixel inPixels, char inTypeDrum, byte inTotalLEDs, byte inTotalLEDColums, byte inTotalLEDRows,  byte inMidiNote){
    pixels = inPixels;
    totalLEDs = inTotalLEDs;
    totalLEDColums = inTotalLEDColums;
    totalLEDRows = inTotalLEDRows;

    previousMillis = 0;
    stepFX = 0;
    stateFX = 0;
    intStateFX = 1;
    typeDrum = inTypeDrum;
    midiNote = inMidiNote;
  }

/*
 * Effect with Steps
 */ 
  void FX1(){
  
  stateFX = 1;
  unsigned long currentMillis = millis();
  unsigned long waittime = 55;
  
    if ((intStateFX == 1) && (currentMillis - previousMillis >= waittime)) {
          if (stepFX < totalLEDs){
            pixels.setPixelColor(stepFX, pixels.Color(0,0,255,0));
            stepFX++;
          }
          else if (stepFX == totalLEDs) {   
            pixels.show();
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
            pixels.show();
            pixels.setPixelColor(stepFX, colorLEDBlack);
            stepFX++; 
          }
        break;
        case 30:
          if (currentMillis - previousMillis >= waittime) {
            pixels.show();
            pixels.setPixelColor(stepFX, colorLEDBlack);
            stepFX++; 
          }
        break;
        case 45:
          if (currentMillis - previousMillis >= waittime) {
            pixels.show();
            pixels.setPixelColor(stepFX, colorLEDBlack);
            stepFX++; 
          }
        break;
        case 60:
            if (currentMillis - previousMillis >= waittime) {
            pixels.show();
            pixels.setPixelColor(stepFX, colorLEDBlack);
            intStateFX = 1;
            stepFX = 0;  
            previousMillis = currentMillis;
            stateFX = 0;
          }

        break;
        default:
          pixels.setPixelColor(stepFX, colorLEDBlack);
          stepFX++; 
          previousMillis = currentMillis;
        break;
      }
    }
  }
  void FX2(){
    
  }

};

class BD : public Drum{
  unsigned int stepFXColum;
  unsigned int stepFXRow;
  
  public:
  BD(Adafruit_NeoPixel inPixels, char inTypeDrum, byte inTotalLEDs, byte inTotalLEDColums, byte inTotalLEDRows, byte inMidiNote){

    pixels = inPixels;
    totalLEDs = inTotalLEDs;
    totalLEDColums = inTotalLEDColums;
    totalLEDRows = inTotalLEDRows;

    stepFXColum = 0;
    stepFXRow = 0;
    previousMillis = 0;
    stateFX = 0;
    intStateFX = 1;
    stepFX = 0;
    typeDrum = inTypeDrum;
    midiNote = inMidiNote;

  }


  void FX1(){

    
    unsigned long currentMillis = millis();
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
    if ((stepFXColum < totalLEDColums) && (currentMillis - previousMillis > waittimeBD)){
      
      if (stepFXRow < totalLEDRows){
  
        switch (stepFXColum){
          case 0:
            currentLED = (stepFXRow * totalLEDColums) + stepFXColum;
            pixels.setPixelColor(currentLED, pixels.Color(0,255,0,0));
          break;
  
          default:
            currentLED = (stepFXRow * totalLEDColums) + stepFXColum;
            pixels.setPixelColor(currentLED, pixels.Color(0,255,0,0));
  
            currentLED = (stepFXRow * totalLEDColums) + (stepFXColum - 1);
            pixels.setPixelColor(currentLED, colorLEDBlack);
  
          break;
        }
  
        pixels.show();
        stepFXRow++;
        
      }
      else {
       
        stepFXColum++;
        stepFXRow=0;
        previousMillis = currentMillis;
  
      }
  
    } else if (stepFXColum == totalLEDColums) {
  
            //Deactivate the previous row
      if (stepFXRow < totalLEDRows){
  
            currentLED = (stepFXRow * totalLEDColums) + (stepFXColum - 1);
            pixels.setPixelColor(currentLED, colorLEDBlack);
            stepFXRow++;
      }
      else
      {
        stepFXColum=0;
        stepFXRow=0;
        pixels.show();
        previousMillis = currentMillis;
        stateFX = 0;
      }
    }
  }
  void FX2(){
    
  }
};

// Create the Toms
BD bassdrumObj(pixelBD,'b',totalLEDsBD,totalLEDColumsBD,totalLEDRowsBD,midiNoteBD);
Tom smallTomObj(pixelST,'s',totalLEDsST,totalLEDColumsST,totalLEDRowsST,midiNoteST);
Tom middleTomObj(pixelMT,'m',totalLEDsMT,totalLEDColumsMT,totalLEDRowsMT,midiNoteMT);


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

 /*
  * Setup Serial for Midi
  */
 Serial2.begin(31250);

 /*
  * PinMode
  */
  
  pinMode(pinBtnConfig, INPUT);
  pinMode(pinLEDConfig, OUTPUT);
  pinMode(pinBtnMode, INPUT);
  pinMode(pinLEDModeBlink, OUTPUT);
  pinMode(pinLEDModeScene, OUTPUT);
  pinMode(pinLEDModePattern, OUTPUT);

   
  bassdrum->start();
  middleTom->start();
  smallTom->start();

}

void loop() {

  if (initialized == false){
    //Serial.println("Start Initialize");
    hitBD.initialize();
    hitST.initialize();
    hitMT.initialize();
    initialized = true;
  }

  /*
   * Check Config Mode button
   */
  btnConfigState = digitalRead(pinBtnConfig);

  if (btnConfigState != lastBtnConfigState){
    if (btnConfigState == LOW){
      if (configMode == true) {
        // trun off config Mode
        configMode = false;
        digitalWrite(pinLEDConfig, LOW);
      }
      else {
        // trun on config Mode
        configMode = true;
        digitalWrite(pinLEDConfig, HIGH);
      }     
    }
    delay(50);
  }
  lastBtnConfigState = btnConfigState;

  /*
   * Get into configuration()
   */
  if (configMode == true) {
    configuration();
  }
  else {
    byte hitDBnow = hitBD.detectHit();
    byte hitSTnow = hitST.detectHit();
    byte hitMTnow = hitMT.detectHit();
    programmcontroller(hitDBnow, bassdrum);
    bassdrum->sendMidi(hitDBnow);
    programmcontroller(hitSTnow, smallTom);
    smallTom->sendMidi(hitDBnow);
    programmcontroller(hitMTnow, middleTom);
    middleTom->sendMidi(hitDBnow);
  }
}

void configuration() {
  unsigned long currentMillis = millis();
  
  btnModeState = digitalRead(pinBtnMode);

  if (btnModeState != lastBtnModeState){
    if (btnModeState == LOW){
      switch (selectedMode) {
        case 0:
          selectedMode = 1;
          digitalWrite(pinLEDModeBlink, LOW);
          digitalWrite(pinLEDModeScene, HIGH);
        break;
        case 1:
          selectedMode = 2;
          digitalWrite(pinLEDModeScene, LOW);
          digitalWrite(pinLEDModePattern, HIGH);
        break;
        case 2:
          selectedMode = 0;
          digitalWrite(pinLEDModePattern, LOW);
          digitalWrite(pinLEDModeBlink, HIGH);
        break;
        default:
        break;
      }
    }
    delay(50);
  }
  // Enter Testmode if button is active more then 3sek
  else if ((btnModeState == LOW) && (currentMillis - previousMillis > 3000)) {
    testMode();
  }
  lastBtnModeState = btnModeState;
}
void programmcontroller(byte inHit, Drum *inDrum){

  
  byte hitState = inHit;
  

  switch (selectedMode) {
    case 0: // Blink
      isRunning = inDrum->getFXState();
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
      //Start Scene
      if (( stateScene != 1) && (hitState > 0)) {
        stateScene = 1;
        sceneFirstTime = true;
      }

      if ( stateScene == 1) {
        switch (selectedScene) {  
          case 0: // Scene1
            scene1();    
          break;
          case 1: // Scene2
            scene1();
          break;
          default:
          break;
        }  
      }
   
    break;
    
    
    case 2: // Pattern
      isRunning = inDrum->getFXState();
      char currentDrumType;
      currentDrumType = inDrum->getDrumType();
      
      if (hitState > 0){
        switch (selectedPattern) {
          case 0: // Pattern 1
            patternMatch = pattern(currentDrumType, pattern1);
          break;
          case 1: // Pattern 2
            patternMatch = pattern(currentDrumType, pattern2);
          break;
          default:
          break;
        }
      }
 
      if ((patternMatch == true) || ( stateScene == 1)) {
        stateScene = 1;
        patternMatch = false;
        scene1();
      }
    
    break;
    default:
    break; 
  }
}

void scene1() {

  isRunningBD = bassdrum->getFXState();
  isRunningST = smallTom->getFXState();
  isRunningMT = middleTom->getFXState();
  unsigned long currentMillis = millis();
 
  switch (stepScene) {
    case 0:
      if ((isRunningBD == true) || (isRunningST == true) || (isRunningMT == true) || (sceneFirstTime == true)) {
  
        if ((isRunningBD == true) || (sceneFirstTime == true)) {
          bassdrum->FX1();
        }
  
        if ((isRunningST == true) || (sceneFirstTime == true)) {
          smallTom->FX1();
        }
        if ((isRunningMT == true) || (sceneFirstTime == true)) {
          middleTom->FX1();
        }
        sceneFirstTime = false;
      }
      else {
        stepScene++;
        sceneFirstTime = true;
        previousMillis = currentMillis;
      }
    break;

    //Pause
    case 1:
      if ( currentMillis - previousMillis >= 1000 ) {
        stepScene++;
      }
    break;
    case 2:
      if ((isRunningBD == true) || (isRunningST == true) || (isRunningMT == true) || (sceneFirstTime == true)) {

        if ((isRunningBD == true) || (sceneFirstTime == true)) {
          bassdrum->Flash();
        }

        if ((isRunningST == true) || (sceneFirstTime == true)) {
          smallTom->Flash();
        }
        if ((isRunningMT == true) || (sceneFirstTime == true)) {
          middleTom->Flash();
        }
        sceneFirstTime = false;
      }
      else {
        stepScene++;
        sceneFirstTime = true;
        previousMillis = currentMillis;
      }
    break;

    //Pause
    case 3:
      if ( currentMillis - previousMillis >= 1000 ) {
        stepScene++;
      }
    break;
        
    case 4:
      if ((isRunningBD == true) || (isRunningST == true) || (isRunningMT == true) || (sceneFirstTime == true)) {

        if ((isRunningBD == true) || (sceneFirstTime == true)) {
          bassdrum->FX1();
        }

        if ((isRunningST == true) || (sceneFirstTime == true)) {
          smallTom->FX1();
        }
        if ((isRunningMT == true) || (sceneFirstTime == true)) {
          middleTom->FX1();
        }
        sceneFirstTime = false;
      }
      else {
        stepScene++;
        sceneFirstTime = true;
      }
    break;
    
    default:
      stepScene = 0;
      sceneFirstTime = true;
      stateScene = 0;
    break;
    }

}
void scene2() {

  isRunningBD = bassdrum->getFXState();
  isRunningST = smallTom->getFXState();
  isRunningMT = middleTom->getFXState();
  unsigned long currentMillis = millis();
 
  switch (stepScene) {
     case 0:
      if ((isRunningBD == true) || (isRunningST == true) || (isRunningMT == true) || (sceneFirstTime == true)) {

        if ((isRunningBD == true) || (sceneFirstTime == true)) {
          bassdrum->Flash();
        }

        if ((isRunningST == true) || (sceneFirstTime == true)) {
          smallTom->Flash();
        }
        if ((isRunningMT == true) || (sceneFirstTime == true)) {
          middleTom->Flash();
        }
        sceneFirstTime = false;
      }
      else {
        stepScene++;
        sceneFirstTime = true;
        previousMillis = currentMillis;
      }
    break;
    
    //Pause
    case 1:
      if ( currentMillis - previousMillis >= 500 ) {
        stepScene++;
      }
    break;
    
    case 2:
      if ((isRunningBD == true) || (isRunningST == true) || (isRunningMT == true) || (sceneFirstTime == true)) {

        if ((isRunningBD == true) || (sceneFirstTime == true)) {
          bassdrum->Flash();
        }

        if ((isRunningST == true) || (sceneFirstTime == true)) {
          smallTom->Flash();
        }
        if ((isRunningMT == true) || (sceneFirstTime == true)) {
          middleTom->Flash();
        }
        sceneFirstTime = false;
      }
      else {
        stepScene++;
        sceneFirstTime = true;
        previousMillis = currentMillis;
      }
    break;
    
    //Pause
    case 3:
      if ( currentMillis - previousMillis >= 1000 ) {
        stepScene++;
      }
    break;
    
    case 4:
      if ((isRunningBD == true) || (isRunningST == true) || (isRunningMT == true) || (sceneFirstTime == true)) {
  
        if ((isRunningBD == true) || (sceneFirstTime == true)) {
          bassdrum->FX1();
        }
  
        if ((isRunningST == true) || (sceneFirstTime == true)) {
          smallTom->FX1();
        }
        if ((isRunningMT == true) || (sceneFirstTime == true)) {
          middleTom->FX1();
        }
        sceneFirstTime = false;
      }
      else {
        stepScene++;
        sceneFirstTime = true;
      }
    break;
   
    default:
      stepScene = 0;
      sceneFirstTime = true;
      stateScene = 0;
    break;
    }
}
void testMode() {
  // TestMode
}

bool pattern(char inDrumType, char *inPattern) {
  switch (stepPattern) {
    case 0:
      if (patternFirstRound == true) {
        patternCue1[0] = inDrumType;   
      }
      else {
        patternCue1[0] = inDrumType; 
        patternCue2[3] = inDrumType;
        patternMatch = checkPattern(patternCue2, inPattern);
        patternCue3[2] = inDrumType; 
        patternCue4[1] = inDrumType;
      }
      stepPattern++;
    break;
    case 1:
      if (patternFirstRound == true) {
        patternCue1[1] = inDrumType; 
        patternCue2[0] = inDrumType;
      }
      else {
        patternCue1[1] = inDrumType; 
        patternCue2[0] = inDrumType; 
        patternCue3[3] = inDrumType;
        patternMatch = checkPattern(patternCue3, inPattern); 
        patternCue4[2] = inDrumType;
      }
      stepPattern++;
    break;
    case 2:
      if (patternFirstRound == true) {
        patternCue1[2] = inDrumType; 
        patternCue2[1] = inDrumType; 
        patternCue3[0] = inDrumType;
        patternFirstRound = false;
      }
      else {
        patternCue1[2] = inDrumType; 
        patternCue2[1] = inDrumType; 
        patternCue3[0] = inDrumType; 
        patternCue4[3] = inDrumType;
        patternMatch = checkPattern(patternCue4, inPattern);
      }
      stepPattern++; 
    break;
    case 3:
      patternCue1[3] = inDrumType;
      patternMatch = checkPattern(patternCue1, inPattern); 
      patternCue2[2] = inDrumType; 
      patternCue3[1] = inDrumType; 
      patternCue4[0] = inDrumType; 
      stepPattern = 0;
      
    break;
    default:
    break;
  }
  Serial.print("patternFirstRound:::");Serial.print(patternFirstRound);
  Serial.print("PatternMatch:::");Serial.print(patternMatch);
  return patternMatch;
}

bool checkPattern(char *inPatternCue, char *inPattern){
  patternMatch = false;
  byte n;
  n = memcmp ( inPatternCue, inPattern, 4 );
  if (n == 0){
    patternMatch = true;
  }
  Serial.print("inPatternCue:::");Serial.print(inPatternCue);Serial.print(":::END::inPatternCue:::");
  Serial.print("inPattern:::");Serial.print(inPattern);Serial.print(":::END::inPattern:::");
  
  Serial.print(n);
  memset(inPatternCue,0,5);
  return patternMatch;
}


