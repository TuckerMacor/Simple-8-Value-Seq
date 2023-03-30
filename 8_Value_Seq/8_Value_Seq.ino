//Simple Sequencer | Tucker Macor | Created 2023-02-27 | Updated 2023-03-29

#define buttonCount 8
const int buttonPins[buttonCount] = {2, 3, 4, 5, 6, 7, 8, 9};   // connects straight to buttons no matrix or multiplexing
const int outputPins[buttonCount] = {A0, A1, A2, A3, A4, A5, 0, 1}; // connects to leds and right pin of each potentiometer
const int forwardPin = 10;  // forward step input pin
const int reversePin = 11;  // reverse step input pin
const int resetPin = 12;    // reset sequence input pin
const int gateOutPin = 13;  // output to gate out jack

bool buttonState[buttonCount];       // what state is the button at
bool buttonWasPressed[buttonCount];  // was the button pressed
bool lastButtonState[buttonCount];   // previous state of the button
unsigned long lastDebounceTime;      // last debounce event time
unsigned long debounceDelay = 30;    // how long to wait before a button is considered pressed

//variables used to debounce inputs
int forwardState;                  // the current reading from the input pin
int lastForwardState = LOW;        // the previous reading from the input pin
unsigned long lastForwardTime = 0; // last time the pin changed
int reverseState;                  // the current reading from the input pin               
int lastReverseState = LOW;        // the previous reading from the input pin
unsigned long lastReverseTime = 0; // last time the pin changed
int resetState;                    // the current reading from the input pin
int lastResetState = LOW;          // the previous reading from the input pin
unsigned long lastResetTime = 0;   // last time the pin changed

int reading; // for reading buttons

int currentStep = 0;    // which step to output
int buttonsPressed = 0; // how many buttons are being pressed

int state = 0;
// 0 = no sequance just step one by one (default)
// 1 = select sequence length
// 2 = input sequence
// 3 = play sequence

int sequence[32]; // in seqence mode the order is stored here
int sequenceLength = 2; // how many steps in the sequence
int positionToAdd = 0; // which position in the sequence should receive a new step
int sequencePlayPosition = 0; // which posistion in the seqence should be played

void setup() {
  // put your setup code here, to run once:

  // set pins as inputs and outputs
  for (byte i = 0; i < buttonCount; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    pinMode(outputPins[i], OUTPUT);
  }
  pinMode(forwardPin, INPUT);
  pinMode(reversePin, INPUT);
  pinMode(resetPin, INPUT);
  pinMode(gateOutPin, OUTPUT);

  blinkAll(1,500); // basically a "notice me senpai" to show that is functioning after power up
}

void loop() {
  // put your main code here, to run repeatedly:
  readButtons();
  readInputs();

  //loop (if past the end of the sequence go to the other end)
  if (currentStep > 7) {
    currentStep = 0;
  }
  if (currentStep < 0) {
    currentStep = 7;
  }

  // control gate
  if (digitalRead(forwardPin) || digitalRead(reversePin) || buttonsPressed > 0) {
    digitalWrite(gateOutPin, HIGH);
  }
  else {
    digitalWrite(gateOutPin, LOW);
  }

  // select output
  if (state == 0 || state == 2) {
    applyOutputPosition();
  }
  else if (state == 1) {
    for (byte i = 0; i < buttonCount; i++) {
      digitalWrite(outputPins[i], HIGH);
    }
  }
  else if (state == 3) {
    currentStep = sequence[sequencePlayPosition];
    applyOutputPosition();
  }
}

void readButtons() {//check if buttons got pressed
  buttonsPressed = 0;
  for (byte i = 0; i < buttonCount; i++) {
    int reading = digitalRead(buttonPins[i]);
    if (reading != lastButtonState[i]) {
      lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (reading != buttonState[i]) {
        buttonState[i] = reading;
        if (!buttonState[i]) {
          if (state == 0) {
            if (!buttonState[0] && !buttonState[7]) { // start programming a sequance
              state = 1;
            }
            else {
              currentStep = i; // set the output to be the step of the button you just pressed (keyboard mode)
            }
          }
          else if (state == 1) {
            positionToAdd = 0;
            sequenceLength = ((i + 1) * 4) - 1;
            state = 2;
            blinkAll(2,225);
          }
          else if (state == 2) {
            if (!buttonState[0] && !buttonState[7]) {
              state = 0;
            }
            currentStep = i;
            sequence[positionToAdd] = i;
            positionToAdd++;
            applyOutputPosition();
            if (positionToAdd > sequenceLength) { // if sequence is full then start playing it
              state = 3;
              blinkAll(3,225);
            }
          }
          else if (state == 3) {
              state = 0;            
          }
        }
      }
    }
    lastButtonState[i] = reading;
    if (buttonState[i] == LOW) {
      buttonsPressed++;
    }
  }
}

void readInputs() { // read the three input jacks ( i don't remember why each is seprate instead of just using a loop but it works so i'm not fixing it :) )
  reading = digitalRead(forwardPin);
  if (reading != lastForwardState) {
    lastForwardTime = millis();
  }
  if ((millis() - lastForwardTime) > debounceDelay) {
    if (reading != forwardState) {
      forwardState = reading;
      if (forwardState == HIGH) {
        if (state == 0) {
          currentStep++;
        }
        else if (state == 3) {
          sequencePlayPosition++;
          if (sequencePlayPosition > sequenceLength) {
            sequencePlayPosition = 0;
          }
        }
      }
    }
  }
  lastForwardState = reading;

  reading = digitalRead(reversePin);
  if (reading != lastReverseState) {
    lastReverseTime = millis();
  }
  if ((millis() - lastReverseTime) > debounceDelay) {
    if (reading != reverseState) {
      reverseState = reading;
      if (reverseState == HIGH) {
        if (state == 0) {
          currentStep--;
        }
        else if (state == 3) {
          sequencePlayPosition--;
          if (sequencePlayPosition < 0) {
            sequencePlayPosition = sequenceLength;
          }
        }
      }
    }
  }
  lastReverseState = reading;

  reading = digitalRead(resetPin);
  if (reading != lastResetState) {
    lastResetTime = millis();
  }
  if ((millis() - lastResetTime) > debounceDelay) {
    if (reading != resetState) {
      resetState = reading;
      if (resetState == HIGH) {
        if (state == 0 || state == 3) {
          currentStep = 0;
          sequencePlayPosition = 0;
        }
      }
    }
  }
  lastResetState = reading;
}

void blinkAll(int numberOfBlinks, int blinkDelay) { // blink leds to indicate stuff
  digitalWrite(gateOutPin, HIGH); // makes sound play during blinks
  for (byte i = 0; i < numberOfBlinks; i++) {
    for (byte i = 0; i < buttonCount; i++) {
      digitalWrite(outputPins[i], HIGH);
    }
    delay(blinkDelay);
    for (byte i = 0; i < buttonCount; i++) {
      digitalWrite(outputPins[i], LOW);
    }
    delay(blinkDelay);
  }
}

void applyOutputPosition() {  // AHHHHHHHHHHHH
  for (byte i = 0; i < buttonCount; i++) {
    if (i == currentStep) {
      digitalWrite(outputPins[i], HIGH);
    }
    else {
      digitalWrite(outputPins[i], LOW);
    }
  }
}
