//Cesar Zavala
//ECE 241 Final Proyect

#include <LiquidCrystal.h> //LCD library
#include <Servo.h> //Servo library
#define INTERVAL 1000 //Second long interval.
Servo servo; //Servo setup
LiquidCrystal lcd(11, 9, 5, 6, 7, 8); //LCD setup.
int Hours, Minutes, Seconds; //Clock setup.
int encoderP; //Current position of the encoder.
int clkset; //Integer affected every 4 clicks of the encoder.
unsigned long ButtonTime; //Timer for the button state machine.
unsigned long SecondsTimer; //Timer for the seconds.
unsigned long Timer; //Timer
int count = 0; //Utilized to separate states of the time-setting process.
int BlinkFlag; //LED blink state.
int DataFlag; //Flag utilized to retain the previous time when setting the clock.
int servoPin = 10; //PWM pin utilized by the motor.
int angle; //Angle of the motor.
int interruptPin1 = 2; //Interrupt pin 1 setup.
int interruptPin2 = 3; //Interrupt pin 2 setup.
int sensorPin = A0; //First photocell.
int sensor1 = 0; //Variable to store the value coming from the sensor.
int sensorPin2 = A1; //Second photocell.
int sensor2 = 0; //Variable to store the value coming from the second sensor.
int Daytime = false; //Daytime flag.

void MonitorA() { //ISR to control the encoder.
  if ( digitalRead(2) == digitalRead(3)) { //If the pin 2 value is equal to pin 3 value.
    encoderP ++;
  }
  else {
    encoderP --;
  }
}
void MonitorB() { //ISR to control the encoder.
  if ( digitalRead(2) ==  digitalRead(3)) { //If the pin 2 value is equal to pin 3 value.
    encoderP --;
  }
  else {
    encoderP ++;
  }
}

enum ButtonState { //Button state machine
  Low,
  Idle,
  Wait
};
ButtonState Buttonswitch;
int ButtonNextState(int input) {
  switch (Buttonswitch) { //State switch setup
    case Idle: //Idle scenario, waiting for a button press.
      if ( input == LOW) {
        ButtonTime = millis();
        Buttonswitch = Wait;
      }
      break; //Breaks should be placed outside the statements!!!

    case Wait: //Wait scenario, checking 5ms button press.
      if ( input == HIGH) { //Return to Idle if button is released.
        Buttonswitch = Idle;
      }
      else if ( millis() - ButtonTime >= 200 ) { //If pressed for more than 5 seconds, and proceed to Low.
        Buttonswitch = Low;
        return 1;
      }
      break; //Breaks should be placed outside the statements!!!

    case Low: //Low scenario, button press check passed.
      if (input == HIGH) {
        count ++;
        Buttonswitch = Idle;
        encoderP = 0;
        DataFlag = false;
        lcd.clear();
      }
      break; //Breaks should be placed outside the statements!!!
  }
  return 0; //The return 0 command should be outside the states, for it to return 0 if nothing happened in the switch.
}

void UpdateClock() { //Script to make the clock work.
  // Check if Seconds not at wrap point.
  if (Seconds < 59)
  {
    Seconds++; // Move seconds ahead.
  }
  else
  {
    Seconds = 0; // Reset Seconds
    // and check Minutes for wrap.
    if (Minutes < 59)
    {
      Minutes++; // Move seconds ahead.
    }
    else
    {
      Minutes = 0; // Reset Minutes
      // check Hours for wrap
      if (Hours < 23)
      {
        Hours++;// Move Hours ahead.
      }
      else
      {
        Hours = 0;// Reset Hours
      }// End of Hours test.
    } // End of Minutes test
  } // End of Seconds test
} // end of UpdateClock()


void SendClock() //Clock LCD update function.
{
  lcd.setCursor(0, 1); //Set cursor to line 2.
  // Check if leading zero needs to be sent
  if (Hours < 10)
  {
    lcd.print("0");
  }
  lcd.print(Hours); // Then send hours
  lcd.print(":"); // And separator
  // Check for leading zero on Minutes.
  if (Minutes < 10)
  {
    lcd.print("0");
  }
  lcd.print(Minutes); // Then send Minutes
  lcd.print(":"); // And separator
  // Check for leading zero needed for Seconds.
  if (Seconds < 10)
  {
    lcd.print("0");
  }
  lcd.print(Seconds); // Then send Seconds
} // End of SendClock()


void setup() {

  pinMode(interruptPin1, INPUT_PULLUP); //Encoder ISR setup.
  pinMode(interruptPin2, INPUT_PULLUP); //Encoder ISR setup.
  pinMode(13, OUTPUT); //LED pin setup.
  pinMode(sensorPin, INPUT); //Photocell setup.
  pinMode(sensorPin2, INPUT);//Photocell setup.
  angle = 0; //Initial angle.
  servo.attach(servoPin); //Servo motor setup.
  attachInterrupt(digitalPinToInterrupt(interruptPin1), MonitorA, CHANGE); //Attach MonitorA to ISR pin 2.
  attachInterrupt(digitalPinToInterrupt(interruptPin2), MonitorB, CHANGE); //Attach MonitorB to ISR pin 3.
  Buttonswitch = Idle; //State machine setup.
  Serial.begin(9600); //Serial setup for debugging purposes
  SecondsTimer = millis(); //Timer setup.
  Timer = millis(); //Timer setup.
  lcd.begin(16, 2); //LCD setup
  Hours = 13; //Clock setup.
  Minutes = 0;
  Seconds = 0;
  count = 0;
  BlinkFlag = false; //LED blinker setup.
  lcd.clear();
}

void loop() {

  if ( ButtonNextState(digitalRead(4)) ) { //Button call function.
    Serial.println("BUTTON PRESSED"); //debug
  }

  if (count == 1) {
    if (clkset > 23) { //Clock boundaries
      clkset = 0;
    }
    if (clkset < 0) { //Clock boundaries
      clkset = 23;
    }
  }
  else {
    if (clkset > 59) { //Clock boundaries
      clkset = 0;
    }
    if (clkset < 0) { //Clock boundaries
      clkset = 59;
    }
  }
  if (encoderP > 3) { //Clock boundaries
    encoderP = 0;
    clkset ++;
    lcd.clear();
  }
  if (encoderP < -3) { //Clock boundaries
    encoderP = 0;
    clkset --;
    lcd.clear();
  }
  if (count >= 4) { //End of adjustments, reset count to 0.
    count = 0;
    lcd.clear();
  }
  if (count == 3) { //Clock seconds adjustment.
    if (DataFlag == false) {
      clkset = Seconds;
      DataFlag = true;
    }
    else {
      Seconds = clkset;
    }
  }
  if (count == 2) {//Clock minutes adjustment.
    if (DataFlag == false) {
      clkset = Minutes;
      DataFlag = true;
    }
    else {
      Minutes = clkset;
    }
  }
  if (count == 1) {//Clock hours adjustment.
    if (DataFlag == false) {
      clkset = Hours;
      DataFlag = true;
    }
    else {
      Hours = clkset;
    }
  }

  if (millis() - Timer >= 100) { //Every 100ms.
    SendClock(); //Refresh clock on LCD.
    sensor1 = analogRead(sensorPin); // read the value from the sensor
    Serial.print("Sensor 1: "); //debug
    Serial.print(sensor1); //debug
    sensor2 = analogRead(sensorPin2); // read the value from the sensor
    Serial.print("  Sensor 2: "); //debug
    Serial.println(sensor2); //debug
    lcd.setCursor(0, 0);
    lcd.print("Angle:"); //Displaying angle on LCD.
    lcd.print(angle);
    if (Daytime == true) { //Runs only during daytime.
      if (sensor1 < 100 && sensor2 > 130 ) { //Turn left towards 0 degrees.
        angle -= 5; //Reduce angle by 5 degrees.
      }
      if (sensor1 < 300 && sensor2 > 230 && sensor2 < 320 && sensor1 > 150) {// Turn right towards 180 degrees.
        angle += 5; //Increase angle by 5 degrees.
      }
      if (sensor1 > 200 && sensor2 > 350) { //Positioning according to time, if light is insufficient.
        int x = Hours - 6;
        angle = (15 * x);
      }
    }
    else { //During nighttime, the servo resets to 90 degrees and displays "N" on the LCD.
      angle = 90;
      lcd.setCursor(11, 0);
      lcd.print("N");
    }
    servo.write(angle); // Send current position to the servo.
    Timer += 100;
  }
  if (millis() - SecondsTimer >= INTERVAL) { //Every 1 second...
    if (count == 0) { //If the clock is not being modified...
      UpdateClock(); //Update clock.
      digitalWrite(13, LOW);

    }
    if (count != 0) { //Blinker script, indicates the clock is being adjusted.
      if (BlinkFlag == true) {
        digitalWrite(13, LOW);
        BlinkFlag = false;
      }
      else {
        digitalWrite(13, HIGH);
        BlinkFlag = true;
      }
    }
    SecondsTimer += INTERVAL;
  }
  if (Hours > 18) { //If time goes beyond 18:00, set Daytime flag to false.
    Daytime = false;
  }
  else if (Hours < 6) {//If time is before 6:00, set Daytime flag to false.
    Daytime = false;
  }
  else { //If not, set Daytime flag to true.
    Daytime = true;
  }
  if (angle > 180) {
    angle = 180;  // reset to 180 if it goes higher
  }
  if (angle < 0) {
    angle = 0;  // reset to 0 if it goes lower
  }
}
