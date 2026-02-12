#include <Servo.h>

#define FORWARD 'F'
#define BACKWARD 'B'
#define LEFT 'L'
#define RIGHT 'R'
#define CIRCLE 'C'
#define CROSS 'X'
#define TRIANGLE 'T'
#define SQUARE 'S'
#define START 'A'
#define PAUSE 'P'

//Motor 1
const int M1pin1 = 2;
const int M1pin2 = 4;
// Motor 2
const int M2pin1 = 3;
const int M2pin2 = 5;

// ultrasonic sensor
const int trigPin = 9;
const int echoPin = 8;

const float SOUND_SPEED = 0.034;

float distance;
long duration;

// Servo
Servo servo1;
const float sensitivity = 1;
float angle = 90;

// Laser
const int laserPin;

// Buzzer
const int buzzerPin;

// Led
const int distanceLedPin;
const int MIN_DIST = 70;
const int MAX_DIST = 250;
bool canShoot = true;

void setup() {
  // Start serial communication
  Serial.begin(9600);
  
  initializeMotors();
  initializeUltrasonic();

  //Servo
  servo1.attach(10);
  servo1.write(90);
}

void loop() {
  handleCommunication();
  handleServo();
  handleDistanceSensor()
}

void handleCommunication() {
  if (Serial.available()) {
    char command = Serial.read();
    executeCommand(command);
  }
}

void handleServo() {
  servo1.write(angle);
}


void executeCommand(char command) {
  switch (command) {
    case FORWARD:
    powerSide1(true); 
    powerSide2(true);
      break;
    case BACKWARD:
    powerSide1(false); 
    powerSide2(false);
      break;
    case LEFT:
    powerSide1(true); 
    powerSide2(false);
      break;
    case RIGHT:
    powerSide1(false); 
    powerSide2(true);
      break;
    case CIRCLE:
      angle += sensitivity;
      break;
    case CROSS:
      shoot();
    case SQUARE:
      angle -= sensitivity;
      break;
  }
}

void handleDistanceSensor() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH, 25000);
  
  // Calculate the distance

  if (duration * SOUND_SPEED/2 != 0) {
    distance = duration * SOUND_SPEED/2;
  }
}

void handleLed() {
  if (MIN_DIST < distance < MAX_DIST) {
    digitalWrite(distanceLedPin, HIGH);
    canShoot = true;
  }
  else {
    digitalWrite(distanceLedPin, LOW);
    canShoot = false;
  }
}

void shoot() {
  if (!canShoot) return;

  digitalWrite(laserPin, HIGH);
  tone(buzzerPin, 1000);
  delay(1000)
  noTone(buzzerPin);
  digitalWrite(laserPin, LOW);
}

// Driver neutral steering
void powerSide1(bool param) 
{
  digitalWrite(M1pin1, param ? HIGH : LOW);
  digitalWrite(M1pin2, param ? LOW : HIGH); 
}

void powerSide2(bool param) 
{
  digitalWrite(M2pin1, param ? LOW : HIGH);
  digitalWrite(M2pin2, param ? HIGH : LOW); 
}

void initializeMotors() {
  pinMode(M1pin1, OUTPUT);
  pinMode(M1pin2, OUTPUT);
  pinMode(M2pin1, OUTPUT);
  pinMode(M2pin2, OUTPUT);
}

void initializeUltrasonic() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}