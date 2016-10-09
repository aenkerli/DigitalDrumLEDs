/*
 * Project: Digital Drum LEDs
 * Autor: Adrian Enkerli
 * Datum: 07.10.2016
 * Version: 0.1
 */

// Global Variables
// Define Analog Inputs
byte triggerBD = 0;
byte triggerMT = 1;
byte triggerST = 2;

// SleepTime in ms - max 255
unsigned int sleepTime = 2000;

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
byte lastStateBD = 0;
byte lastStateMT = 0;
byte lastStateST = 0;

// Last hit time
unsigned long lastHitTime[3] = {0,0,0};





 
void setup() {

 Serial.begin(9600);          //  setup serial

}

void loop() {
  // put your main code here, to run repeatedly:

  //Check BD
  byte hitBD = hitdetection(triggerBD, thresholdBDmax, thresholdBDmin);
  byte hitMT = hitdetection(triggerMT, thresholdMTmax, thresholdMTmin);
  byte hitST = hitdetection(triggerST, thresholdSTmax, thresholdSTmin);

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

