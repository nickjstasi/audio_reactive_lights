#include <FastLED.h>

#define LED_PIN_BASS 6 //(atmega pin 12) 
#define LED_PIN_MIDS 5 //atmega pin 11
#define LED_PIN_HIGHS 7 // atmega pin 13
#define NUM_LEDS 143
#define NUM_MODES 3
#define AUDIO_MODE 0
#define RAINBOW_MODE 1
#define WHITE_MODE 2
#define MAX_LOUDNESS 348
#define STROBE_PIN 9 //atmega pin 15
#define RESET_PIN 10 //atmega pin 16
#define BASE_BRIGHTNESS 150

int modes[NUM_MODES] = {};
CRGB ledsBass[NUM_LEDS];
CRGB ledsMids[NUM_LEDS];
CRGB ledsHighs[NUM_LEDS];
int* prev_level;
int msgOutPin = A5; //atmega pin 28
int msgStrobePin = STROBE_PIN;
int msgResetPin = RESET_PIN;
int buttonPin = 4; //atmega pin 6
int buttonState = 1;
int noise[7] = {0}; // used to hold noise measurement at startup
 
void setup() {
  analogReference(DEFAULT);
  delay(3000);
  pinMode(buttonPin, INPUT_PULLUP); //button input
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonPressed, CHANGE); //interrupt for when button is pressed
  //Serial.begin(9600); //for debug
  ///////////////////SETUP FAST LED///////////////////////////////
  FastLED.addLeds<WS2812, LED_PIN_BASS, GRB>(ledsBass, NUM_LEDS);
  FastLED.addLeds<WS2812, LED_PIN_MIDS, GRB>(ledsMids, NUM_LEDS);
  FastLED.addLeds<WS2812, LED_PIN_HIGHS, GRB>(ledsHighs, NUM_LEDS);
  LEDS.setBrightness(BASE_BRIGHTNESS);
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

  //////////////////Sample noise//////////////////////////////////

  for(int i = 0; i < 10; i++){ //take 10 measurements
     for (int j = 0; j < 7; j++) { //read every band
      digitalWrite       (msgStrobePin, LOW);
      delayMicroseconds  (40);                  // Delay necessary due to timing diagram
      noise[j] +=         analogRead (msgOutPin); //read the level
      digitalWrite       (msgStrobePin, HIGH);
      delayMicroseconds  (40);                  // Delay necessary due to timing diagram  
    }
    delay(50); //wait 50 millis 
  }

  for(int i = 0; i < 7; i++){ //divide by 10 for average
    noise[i] = noise[i] / 10; 
  }
 ///////////////////////////////////////////////////////////////////
}//setup

void loop() {
   //Check the analog input
   //int loudness = analogRead(A0);
   //int highest_led = map(loudness, 0, 388, 0, NUM_LEDS);
   int level[7]; //array to hold values from MSGEQ7
   
  // Cycle through each frequency band by pulsing the strobe.
  for (int i = 0; i < 7; i++) {
    digitalWrite       (msgStrobePin, LOW);
    delayMicroseconds  (40);                  // Delay necessary due to timing diagram
    level[i] =         analogRead (msgOutPin) - noise[i]; //read the level correct for noise
    digitalWrite       (msgStrobePin, HIGH);
    delayMicroseconds  (40);                    // Delay necessary due to timing diagram  
  }

  //allInOne(level);
  //peakBounce(level);
  //debug();
  //allWhite();
  switch(buttonState){
    case 1: 
        midBounce(level);
        break;
    case 2:
        peakBounce(level);
        break;
    case 3:
        allWhite();
        break;
    default:
        midBounce(level);
  }
  prev_level = level; //set the previous level array
  FastLED.show();
  delay(25);//wait a lil
}//loop


///////////////////////////////////LIGHT MODES///////////////////////////////////////////////////
int clearStrip(){
  int i = 0;
  for(i = 0; i < NUM_LEDS; i++){
    ledsBass[i] = CRGB::Black;
    ledsMids[i] = CRGB::Black;
    ledsHighs[i] = CRGB::Black;
  }
}

void midBounce(int* level){
  //get the distance from the center that the lights
  int bassDistance = map(getBassLevel(level), 0, 1023, 0, NUM_LEDS/2);
  int midsDistance = map(getMidLevel(level), 0, 1023, 0, NUM_LEDS/2);
  int highsDistance = map(getHighLevel(level), 0, 1023, 0, NUM_LEDS/2);
  
  int lowerBoundBass = NUM_LEDS/2 - bassDistance;
  int upperBoundBass = NUM_LEDS/2 + bassDistance;
  int lowerBoundMids = NUM_LEDS/2 - midsDistance;
  int upperBoundMids = NUM_LEDS/2 + midsDistance;
  int lowerBoundHighs = NUM_LEDS/2 - highsDistance;
  int upperBoundHighs = NUM_LEDS/2 + highsDistance;
  for(int i = lowerBoundBass; i< upperBoundBass; i++){
    ledsBass[i] = CRGB::Red;
  }
  for(int i = lowerBoundMids; i< upperBoundMids; i++){
    ledsMids[i] = CRGB::Green;
  }
  for(int i = lowerBoundHighs; i< upperBoundHighs; i++){
    ledsHighs[i] = CRGB::Blue;
  }
  
  //set lights outside this range to be black
  for(int i = 0; i < lowerBoundBass; i++){
    ledsBass[i] = CRGB::Black;
  }
  for(int i = upperBoundBass; i < NUM_LEDS; i++){
    ledsBass[i] = CRGB::Black;
  }
  for(int i = 0; i < lowerBoundMids; i++){
    ledsMids[i] = CRGB::Black;
  }
  for(int i = upperBoundMids; i < NUM_LEDS; i++){
    ledsMids[i] = CRGB::Black;
  }
  for(int i = 0; i < lowerBoundHighs; i++){
    ledsHighs[i] = CRGB::Black;
  }
  for(int i = upperBoundHighs; i < NUM_LEDS; i++){
    ledsHighs[i] = CRGB::Black;
  }
}//midBounce

void peakBounce(int* level){
   //TODO: decide how highest_led should be calculated
   int tot = maxLevel(level);
   int bassLevel = getBassLevel(level);
   int highestBassLed = map(bassLevel, 0, 1023, 0, NUM_LEDS);
   int highest_led = map(tot, 0, 1023, 0, NUM_LEDS);
   ledsBass[0] = CRGB::Green;
   ledsMids[0] = CRGB::Green;
   ledsHighs[0] = CRGB::Green;

   for(int i = 1; i < highest_led; i++){
      if(i < (NUM_LEDS / 3)){
        ledsBass[i] = CRGB::Green;
        ledsMids[i] = CRGB::Green;
        ledsHighs[i] = CRGB::Green;
      }
      else if(i < (2*NUM_LEDS / 3)){
        ledsBass[i] = CRGB::Yellow;
        ledsMids[i] = CRGB::Yellow;
        ledsHighs[i] = CRGB::Yellow;
      }
      else {
        ledsBass[i] = CRGB::Red;
        ledsMids[i] = CRGB::Red;
        ledsHighs[i] = CRGB::Red;
      }
   }
   
   //turn off leds higher than highest_led
   int j = highest_led;
   if(j == 0) j = 1;
   for( ; j < NUM_LEDS; j++){
      ledsBass[j] = CRGB::Black;
      ledsMids[j] = CRGB::Black;
      ledsHighs[j] = CRGB::Black;
   }
}//peakBounce

void allWhite(){
  for(int i = 0; i < NUM_LEDS; i++){
    ledsBass[i] = CRGB::White;
    ledsMids[i] = CRGB::White;
    ledsHighs[i] = CRGB::White;
  }
}

void allInOne(int* level){
  int tot = maxLevel(level); //get the maximum loudness
  int brightnessBoost = map(tot, 0, 1023, 0, 255 - BASE_BRIGHTNESS); //get the brightness boost
  int hs = map(tot, 0, 1023, 0, 255); //get hue and saturation
  int midDist = map(tot, 0, 1023, 0, NUM_LEDS / 2); //get the distance from the middle
  int bassLoudness = level[0] + level[1];
  int redBoost = map(bassLoudness, 0, 2046, 0, 255); //how much to boost the red color
  int midLoudness = level[2] + level[3];
  int greenBoost = map(midLoudness, 0, 2046, 0, 255);
  int trebBoost = level[4] + level[5] + level[6];
  int blueBoost = map(trebBoost, 0, 3069, 0, 255);

  int lowerBound = (NUM_LEDS/2) - midDist;
  int upperBound = (NUM_LEDS/2) + midDist;
  for(int i = lowerBound; i < upperBound; i++){
    ledsBass[i] = CHSV(hs, hs, BASE_BRIGHTNESS + brightnessBoost);
    ledsMids[i] = CHSV(hs, hs, BASE_BRIGHTNESS + brightnessBoost);
    ledsHighs[i] = CHSV(hs, hs, BASE_BRIGHTNESS + brightnessBoost);
//    ledsBass[i].setRGB(redBoost, 75, blueBoost);
//    ledsMids[i].setRGB(redBoost, 75, blueBoost);
//    ledsHighs[i].setRGB(redBoost, 75, blueBoost);
  }
  for(int i = 0; i < lowerBound; i++){
    ledsBass[i] = CRGB::Black;
    ledsMids[i] = CRGB::Black;
    ledsHighs[i] = CRGB::Black;
  }
  for(int i = upperBound; i < NUM_LEDS; i++){
    ledsBass[i] = CRGB::Black;
    ledsMids[i] = CRGB::Black;
    ledsHighs[i] = CRGB::Black;    
  } 
}//allInOne()

void buttonPressed(){
    buttonState++; //increment state
    if(buttonState == NUM_MODES){ //roll over
      buttonState = 0;
    }
    allWhite();
    FastLED.show();
    delay(1000);
}//buttonPressed

///////////////////////////////////////////API's//////////////////////////////////////////////
/**
 * Helper method to find the highest level
 * reported in level[]
 */
int maxLevel(int* level){
  int maximum = 0;
  for(int i = 0; i < 7; i++){
    //Serial.print(level[i]); Serial.print(" ");
    if(level[i] > maximum){
      maximum = level[i];
    }
  }
  //get second highest value
  int secondHighest = 0;
  for(int i = 0; i < 7; i++){
    if(level[i] > secondHighest && level[i] < maximum){
      secondHighest = level[i];
    }
  }
  //Serial.println();
  return maximum;
}

/**
 * Get the average level of bass
 */
int getBassLevel(int* level){
  int sum = 0;
  for(int i = 0; i < 3; i++){
    sum += level[i];
  }
  sum /= 3; //turn sum into average
  return sum;
}

/**
 * Get the average level of mids
 */
int getMidLevel(int* level){
  int sum = 0;
  for(int i = 2; i < 5; i++){ //level[2 - 4]
    sum += level[i];
  }
  sum /= 3; // turn sum into average
  return sum;
}

/**
 * Get the average level of mids
 */
int getHighLevel(int* level){
  int sum = 0;
  for(int i = 4; i < 7; i++){ //level[4 - 6]
    sum += level[i];
  }
  sum /= 3; //turn sum into average
  return sum;
}
