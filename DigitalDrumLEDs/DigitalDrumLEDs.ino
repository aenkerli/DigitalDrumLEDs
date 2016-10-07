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

// Hit variables
int hitBD = 512; // 0-Point is on 512
int hitMT = 512; // 0-Point is on 512
int hitST = 512; // 0-Point is on 512

// SleepTime in ms - max 255
byte sleepTime = 70;

// Threshold values BD=Bassdrum, MT=MidTom, SM=SmallTom
int thresholdBDmax = 750;
int thresholdBDmin = 250;
int thresholdMTmax = 750;
int thresholdMTmin = 250;
int thresholdSTmax = 750;
int thresholdSTmin = 250;

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

 
void setup() {

 Serial.begin(9600);          //  setup serial

}

void loop() {
  // put your main code here, to run repeatedly:

  //Check BD
  byte hitBD = hitdetection(triggerBD, thresholdBDmax, thresholdBDmin);
  byte hitMT = hitdetection(triggerMT, thresholdMTmax, thresholdMTmin);
  byte hitST = hitdetection(triggerST, thresholdSTmax, thresholdSTmin);
  Serial.print(hitBD);
  Serial.print(hitMT);
  Serial.println(hitST); 



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
byte hitdetection(byte triggerSource, int thresMax, int thresMin) {

  static int triggerValue = 512;
  static byte hit = 0;

  // Check hit
  triggerValue = analogRead(triggerSource);

  // Check if a hit was hitten
  if ((triggerValue > thresMax) || (triggerValue < thresMin)) {

    //check force
    //if ( triggerValue

    hit = 1;
        
  }

  return hit;
  
}

