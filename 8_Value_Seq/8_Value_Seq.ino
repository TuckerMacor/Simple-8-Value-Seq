//Simple Sequencer | Tucker Macor | Created 2023-02-27 | Updated 2023-03-01
//this code works but is relatively lacking in comments so have fun figuring out whats going on!

#define buttonCount 8
const int buttonPins[buttonCount] = {2, 3, 4, 5, 6, 7, 8, 9};
const int outputPins[buttonCount] = {A0, A1, A2, A3, A4, A5, 0, 1};
const int forwardPin = 10;
const int reversePin = 11;
const int resetPin = 12;
const int gateOutPin = 13;

bool buttonState[buttonCount];       // what state is the button at
bool buttonWasPressed[buttonCount];  // was the button pressed
bool lastButtonState[buttonCount];   // previous state of the button
unsigned long lastDebounceTime;      // last debounce event time
unsigned long debounceDelay = 30;    // how long to wait before a button is considered pressed

int forwardState;             // the current reading from the input pin
int lastForwardState = LOW;   // the previous reading from the input pin
unsigned long lastForwardTime = 0;
int reverseState;             // the current reading from the input pin
int lastReverseState = LOW;   // the previous reading from the input pin
unsigned long lastReverseTime = 0;
int resetState;             // the current reading from the input pin
int lastResetState = LOW;   // the previous reading from the input pin
unsigned long lastResetTime = 0;

int reading;
int currentStep = 0;
int buttonsPressed = 0;

int state = 0;
int sequence[32];
int sequenceLength = 2;
int positionToAdd = 0;
int sequencePlayPosition = 0;

void setup() {
  // put your setup code here, to run once:
  for (byte i = 0; i < buttonCount; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    pinMode(outputPins[i], OUTPUT);
  }
  pinMode(forwardPin, INPUT);
  pinMode(reversePin, INPUT);
  pinMode(resetPin, INPUT);
  pinMode(gateOutPin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  readButtons();
  readInputs();

  //loop
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
            if (!buttonState[0] && !buttonState[7]) {
              state = 1;
            }
            else {
              currentStep = i;
            }
          }
          else if (state == 1) {
            positionToAdd = 0;
            sequenceLength = ((i + 1) * 4) - 1;
            state = 2;
          }
          else if (state == 2) {
            currentStep = i;
            sequence[positionToAdd] = i;
            positionToAdd++;
            applyOutputPosition();
            if (positionToAdd > sequenceLength) {
              state = 3;
            }
          }
          else if (state == 3) {
            if (i==7) {
              state = 0;
            }
            else{
              state = 1;
            }
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

void readInputs() {
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

void applyOutputPosition() {
  for (byte i = 0; i < buttonCount; i++) {
    if (i == currentStep) {
      digitalWrite(outputPins[i], HIGH);
    }
    else {
      digitalWrite(outputPins[i], LOW);
    }
  }
}
