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
CRGB ledsBass[NUM_LEDS];
CRGB ledsMids[NUM_LEDS];
CRGB ledsHighs[NUM_LEDS];
int colors[NUM_LEDS];
int msg_level[7];
int msgOutPin = A5;
int msgStrobePin = STROBE_PIN;
int msgResetPin = RESET_PIN;
int centerLed = NUM_LEDS / 2;
int buttonPin = 2;
int buttonState = 1;

void setup() {
  delay(3000);
  pinMode(2, INPUT); //button input
  pinMode(LED_BUILTIN,OUTPUT);
  attachInterrupt(0, buttonPressed, FALLING); //interrupt for when button is pressed
  //Serial.begin(9600); //for debug
  ///////////////////SETUP FAST LED///////////////////////////////
  FastLED.addLeds<WS2812, LED_PIN_BASS, GRB>(ledsBass, NUM_LEDS);
  FastLED.addLeds<WS2812, LED_PIN_MIDS, GRB>(ledsMids, NUM_LEDS);
  FastLED.addLeds<WS2812, LED_PIN_HIGHS, GRB>(leds, NUM_LEDS);
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
   int level[7]; //array to hold values from MSGEQ7
   
  // Cycle through each frequency band by pulsing the strobe.
  for (int i = 0; i < 7; i++) {
    digitalWrite       (msgStrobePin, LOW);
    delayMicroseconds  (40);                  // Delay necessary due to timing diagram
    level[i] =         analogRead (msgOutPin); //read the level
    digitalWrite       (msgStrobePin, HIGH);
    delayMicroseconds  (40);                    // Delay necessary due to timing diagram  
  }

//TODO get rid of magic numbers
  switch(buttonState){
    case 1: 
        midBounce(level);
        break;
    case 2:
        peakBounce(level);
        break;
    case 3:
        allWhite(level);
        break;
    default:
        midBounce(level);
  }

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
    ledsBass[i] = CRGB::Black;
    ledsMids[i] = CRGB::Black;
    ledsHighs[i] = CRGB::Black;
  }
  FastLED.show();
}

int buttonPressed(){
    buttonState++; //increment state
    if(buttonState == NUM_MODES){ //roll over
      buttonState = 0;
    }
    //Serial.print(buttonState);
    //Serial.println("");
  
}//buttonPressed

void midBounce(int* level){
  int loudDistance = map(level[0], 0, 1023, 0, NUM_LEDS/2); //get the distance from the center that the lights should flash based on loudness of level[2]
  
  //set the leds to light up
  int lowerBound = centerLed - loudDistance;
  int upperBound = centerLed + loudDistance;
  for(int i = lowerBound; i< upperBound; i++){
    ledsBass[i] = CRGB::Blue;
    ledsMids[i] = CRGB::Green;
  }
  //set lights outside this range to be black
  for(int i = 0; i < lowerBound; i++)
  {
    ledsBass[i] = CRGB::Black;
    ledsMids[i] = CRGB::Black;
  }
  for(int i = upperBound; i < NUM_LEDS; i++){
    ledsBass[i] = CRGB::Black;
    ledsMids[i] = CRGB::Black;
  }
  FastLED.show();
  delay(25);//wait a lil
}//midBounce

void peakBounce(int* level){
   //TODO: decide how highest_led should be calculated
   int highest_led = 0;
   //light up all led's up to the highest_led
   int i;
   for(i = 1; i < highest_led; i++){
      if(i < (NUM_LEDS / 3)){
        leds[i] = CRGB::Green;
      }
      else if(i < (2*NUM_LEDS / 3)){
        leds[i] = CRGB::Yellow;
      }
      else {
        leds[i] = CRGB::Red;
      }
   }
   
   //turn off leds higher than highest_led
   int j = highest_led;
   if(j == 0) j = 1;
   for( ; j < NUM_LEDS; j++){
      leds[j] = CRGB::Black;
   }

   FastLED.show();
   delay(25); //wait a lil bit

}//peakBounce

void allWhite(){
  for(int i = 0; i < NUM_LEDS; i++){
    ledsBass[i] = CRGB::White;
    ledsMids[i] = CRGB::White;
    ledsHighs[i] = CRGB::White;
  }
  FastLED.show()
  delay(25);

}
