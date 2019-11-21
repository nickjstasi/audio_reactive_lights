#include <FastLED.h>

#define LED_PIN_BASS 7
#define LED_PIN_MIDS 8
#define LED_PIN_HIGHS 9
#define NUM_LEDS 143
#define NUM_MODES 3
#define AUDIO_MODE 0
#define RAINBOW_MODE 1
#define WHITE_MODE 2
#define MAX_LOUDNESS 348
#define STROBE_PIN 13
#define RESET_PIN 12

int modes[NUM_MODES] = {};
CRGB leds[NUM_LEDS];
int colors[NUM_LEDS];
int msg_level[7];
int msgOutPin = A5;
int msgStrobePin = STROBE_PIN;
int msgResetPin = RESET_PIN;
int centerLed = NUM_LEDS / 2;
int buttonPin = 2;
int buttonState = 0;

void setup() {
  delay(3000);
  pinMode(2, INPUT); //button input
  pinMode(LED_BUILTIN,OUTPUT);
  attachInterrupt(0, buttonPressed, FALLING); //interrupt for when button is pressed
  //Serial.begin(9600); //for debug
  ///////////////////SETUP FAST LED///////////////////////////////
  FastLED.addLeds<WS2812, LED_PIN_BASS, GRB>(leds, NUM_LEDS);
  //FastLED.addLeds<WS2812, LED_PIN_MIDS, GRB>(leds, NUM_LEDS);
  //FastLED.addLeds<WS2812, LED_PIN_HIGHS, GRB>(leds, NUM_LEDS);
  LEDS.setBrightness(255);
  //////////////////////////////////////////////////////////////////

  ////////////////////SETUP MSGEQ7////////////////////////////////
  //defin pin types
  pinMode      (msgStrobePin, OUTPUT);
  pinMode      (msgResetPin,  OUTPUT);
  pinMode      (msgOutPin,    INPUT);
  //write initial states
  digitalWrite (msgResetPin,  LOW);
  digitalWrite (msgStrobePin, LOW);
  delay        (1);
  //reset the MSG chip
  digitalWrite (msgResetPin,  HIGH);
  delay        (1);
  digitalWrite (msgResetPin,  LOW);
  digitalWrite (msgStrobePin, HIGH); 
  delay        (1);
  /////////////////////////////////////////////////////////////////


  
}

void loop() {
   //Check the analog input
   //int loudness = analogRead(A0);
   //int highest_led = map(loudness, 0, 388, 0, NUM_LEDS);
   int level[7];
   
  // Cycle through each frequency band by pulsing the strobe.
  for (int i = 0; i < 7; i++) {
    digitalWrite       (msgStrobePin, LOW);
    delayMicroseconds  (40);                  // Delay necessary due to timing diagram
    level[i] =         analogRead (msgOutPin);
    digitalWrite       (msgStrobePin, HIGH);
    delayMicroseconds  (40);                    // Delay necessary due to timing diagram  
  }

  int loudDistance = map(level[0], 0, 1023, 0, NUM_LEDS/2); //get the distance from the center that the lights should flash based on loudness of level[2]
  //Serial.print("level2: "); Serial.print(level[2]); Serial.print(" loudDistance: ");  Serial.print(loudDistance); Serial.println("");

  //set the leds to light up
  int lowerBound = centerLed - loudDistance;
  int upperBound = centerLed + loudDistance;
  for(int i = lowerBound; i< upperBound; i++){
    leds[i] = CRGB::Green;
  }
  //set lights outside this range to be black
  for(int i = 0; i < lowerBound; i++)
  {
    leds[i] = CRGB::Black;
  }
  for(int i = upperBound; i < NUM_LEDS; i++){
    leds[i] = CRGB::Black;
  }
  FastLED.show();
  delay(25);//wait a lil
  
//  for (int i = 0; i < 7; i++) {
//    Serial.print       (level[i]);
//    Serial.print       ("   ");
//  }
//  Serial.println ();  
   







   
//   //highest_led = highest_led - 25;
//   //Serial.print("loudness: "); Serial.println(loudness); //this was messing with the pwm output
//   //Serial.print("highest_led: "); Serial.println(highest_led);  //seems to be working properly up to here
//   
//   //light up all led's up to the highest_led
//   int i;
//   for(i = 1; i < highest_led; i++){
//      if(i < (NUM_LEDS / 3)){
//        leds[i] = CRGB::Green;
//      }
//      else if(i < (2*NUM_LEDS / 3)){
//        leds[i] = CRGB::Yellow;
//      }
//      else {
//        leds[i] = CRGB::Red;
//      }
//   }
//   
//   //turn off leds higher than highest_led
//   int j = highest_led;
//   if(j == 0) j = 1;
//   for( ; j < NUM_LEDS; j++){
//      leds[j] = CRGB::Black;
//   }
//
//   FastLED.show();
//   delay(5); //wait a lil bit



   
   // Move a single white led 
//   for(int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1) {
//      // Turn our current led on to white, then show the leds
//      leds[whiteLed] = CRGB::White;
//
//      // Show the leds (only one of which is set to white, from above)
//      FastLED.show();
//
//      // Wait a little bit
//      delay(10);
//
//      // Turn our current led back to black for the next loop around
//      //leds[whiteLed] = CRGB::Black;
//   }
  }//loop

/**
 * Helper function used to get the user selected mode
 */
int checkMode(){
  //TODO
  return WHITE_MODE; //for testing
}

int clearStrip(){
  int i = 0;
  for(i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}

int buttonPressed(){
    buttonState++;
    //Serial.print(buttonState);
    //Serial.println("");
  
}//buttonPressed
