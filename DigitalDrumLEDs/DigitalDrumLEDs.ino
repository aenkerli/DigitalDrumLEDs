/*
 * Project: Digital Drum LEDs
 * Autor: Adrian Enkerli
 * Datum: 07.10.2016
 * Version: 0.1
 */

 /*
  * Do not make any changes at the includes
  */
  
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

/*
 * Congfiguration Section
 */

// Global Variables
// Define Analog Inputs
byte triggerBD = 0;
byte triggerMT = 1;
byte triggerST = 2;

// SleepTime in ms - max 255
byte sleepTime = 50;

// SoftHard Threshold 0 - 1
float hardHitThresh = 0.5;

// Threshold values BD=Bassdrum, MT=MidTom, SM=SmallTom
unsigned int thresholdBDmax = 750;
unsigned int thresholdBDmin = 350;
unsigned int thresholdMTmax = 750;
unsigned int thresholdMTmin = 350;
unsigned int thresholdSTmax = 750;
unsigned int thresholdSTmin = 350;


/*
 * Do not make any changes after this point
 */

// Hit variables
unsigned int hitBD = 512; // 0-Point is on 512
unsigned int hitMT = 512; // 0-Point is on 512
unsigned int hitST = 512; // 0-Point is on 512

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
 * Neopixel Vraiables
 */

// Define the Pixels
Adafruit_NeoPixel pixBD = Adafruit_NeoPixel(72, 2, NEO_RGBW + NEO_KHZ800);
Adafruit_NeoPixel pixST = Adafruit_NeoPixel(60, 3, NEO_RGBW + NEO_KHZ800);
Adafruit_NeoPixel pixMT = Adafruit_NeoPixel(60, 4, NEO_RGBW + NEO_KHZ800);
 
void setup() {

 //Serial.begin(9600);          //  setup serial

 // Initialize the Neopixels
  pixBD.begin(); // This initializes the NeoPixel library.
  pixBD.show(); // Initialize all pixels to 'off'
  
  pixST.begin(); // This initializes the NeoPixel library.
  pixST.show(); // Initialize all pixels to 'off'
  
  pixMT.begin(); // This initializes the NeoPixel library.
  pixMT.show(); // Initialize all pixels to 'off'
}

void loop() {
  // put your main code here, to run repeatedly:

  //Check BD
  hitStates[0] = hitdetection(triggerBD, thresholdBDmax, thresholdBDmin);
  hitStates[1] = hitdetection(triggerMT, thresholdMTmax, thresholdMTmin);
  hitStates[2] = hitdetection(triggerST, thresholdSTmax, thresholdSTmin);

  programmcontroller(hitStates);

}


/*
 * Hit-Detection
 * 
 * triggerSource = analog input
 * thresMax = maximum threshold for hit detection
 * thresMin = minimum threshold for hit detection
 * 
 * return hit
 * 0 = none
 * 1 = soft
 * 2 = hard
 * 
 */
byte hitdetection(byte triggerSource, unsigned int thresMax, unsigned int thresMin) {

  int unsigned triggerValue = 512;
  byte hit = 0;
  unsigned long now = millis();

/*
  Serial.print(triggerSource);Serial.print(":::");
  Serial.print(now);Serial.print(",");
  Serial.print(lastHitTime[triggerSource]);Serial.print(",");
*/
  
  //Check sleeptime
  if ( (lastHitTime[triggerSource] == 0) || ((now - lastHitTime[triggerSource]) > sleepTime)) {
    /*
     * Sleeptime not active, check hit
     */

    // Get analog input value
    unsigned int triggerValue = analogRead(triggerSource);

    // Check if a hit was hitten
    if ((triggerValue > thresMax) || (triggerValue < thresMin)) {
      /*
       * hit, detected, check the it's a hard hit else it's soft
       */

      // Callculate hard threshold
      unsigned int hardThreshMax = thresMax + ((1023 - thresMax) * hardHitThresh);
      unsigned int hardThreshMin = thresMin - (thresMin * hardHitThresh);
      
      if ((triggerValue > hardThreshMax) || (triggerValue < hardThreshMin)) {
        
        hit = 2;
        
      }
      else {
      
        hit = 1;
      
      }

      lastHitTime[triggerSource] = now;
    }
  
    return hit;
  
  }
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

/*
 * FX Section
 */

void FX_1(){

  pixST.setPixelColor(i, pixels.Color(0,0,255,0));
  
}


