#include <Arduino.h>
#include <TM1637Display.h> //TM1637 by Avishay Orpaz in library manager

// LED display connection pins (Digital Pins)
#define CLK 2
#define DIO 3

#define BUTTON_PIN  4
#define FIRST_RELAY_PIN  5
#define LED_PIN     13

#define DELAY               12*60000 //in milliseconds
#define DELAY_RELAY_1_OFF   6*60000
#define DELAY_RELAY_2_ON    5*60000

#define SECOND              1000

#define PROGRAM_STATE_IDLE            0
#define PROGRAM_STATE_COUNTDOWN_TIMER 1

#define LONG_PRESS_TIME     1000

unsigned long pressedTime  = 0;
unsigned long releasedTime = 0;

char programState = 0;

bool buttonState = false;
bool lastButtonState = false;

unsigned long startTime = 0;

bool relay1State = false;
bool relay2State = false;

const uint8_t SEG_DONE[] = {
	SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
	SEG_C | SEG_E | SEG_G,                           // n
	SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
	};

const uint8_t SEG_IDLE[] = {
	SEG_G,           // -
	SEG_G,           // -
	SEG_G,           // -
	SEG_G            // -
	};

TM1637Display display(CLK, DIO);

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  pinMode(FIRST_RELAY_PIN, OUTPUT);
  pinMode(FIRST_RELAY_PIN + 1, OUTPUT);

  SetRelay(1, LOW);
  SetRelay(2, LOW);

  display.clear();
  display.setBrightness(0x0f);

  display.setSegments(SEG_IDLE);
}

void loop() {

  if (programState == PROGRAM_STATE_COUNTDOWN_TIMER) {
    unsigned long deltaTime = millis() - startTime;
    if ( deltaTime < DELAY) {
      int sec = abs((DELAY-deltaTime)/SECOND);
      int min = abs(sec/60);
      sec -= min * 60;
      int time = min*100 + sec;
      min > 0 ? display.showNumberDecEx(time, (0x80 >> 1), false) : display.showNumberDec(time, false);    

      runTimers(deltaTime);   
    } else {
      stopCountDownTimer();
    }
  }

  checkButton();

  if (programState == PROGRAM_STATE_IDLE && buttonState == HIGH) {
    startCountDownTimer();
  }
}

void runTimers(unsigned long deltaTime){
  if (deltaTime > DELAY_RELAY_1_OFF && relay1State){
    SetRelay(1, LOW);
  }  
  if (deltaTime > DELAY_RELAY_2_ON  && !relay2State){
    SetRelay(2, HIGH);
  }
}

void checkButton(){

  buttonState = digitalRead(BUTTON_PIN);

  //onboard indicator
  if (buttonState == HIGH) {
    digitalWrite(LED_PIN, HIGH);
  } else{
    digitalWrite(LED_PIN, LOW);
  }  

  // long press
  if(buttonState == HIGH && lastButtonState == LOW)        // button is pressed
    pressedTime = millis();
  else if(buttonState == LOW && lastButtonState == HIGH) { // button is released
    releasedTime = millis();
    long pressDuration = releasedTime - pressedTime;
    if( pressDuration > LONG_PRESS_TIME ){
      stopCountDownTimer();
      display.setSegments(SEG_IDLE);
    }
  }
  lastButtonState = buttonState;  
}

void startCountDownTimer(){
  startTime = millis();
  programState = PROGRAM_STATE_COUNTDOWN_TIMER;
  SetRelay(1, HIGH);
  SetRelay(2, LOW);

}

void stopCountDownTimer(){
  if( programState != PROGRAM_STATE_IDLE){
    SetRelay(1, LOW);
    SetRelay(2, LOW);  
    programState = PROGRAM_STATE_IDLE;
    display.setSegments(SEG_DONE);
  }
}

void SetRelay(int num, bool state){
  int pin = FIRST_RELAY_PIN + num - 1;
  digitalWrite(pin, state);
  relay1State = state;
}