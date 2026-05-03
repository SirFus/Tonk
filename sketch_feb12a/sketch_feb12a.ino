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

// Motor 1
const int M1pin1 = 2;
const int M1pin2 = 4;

// Motor 2
const int M2pin1 = 3;
const int M2pin2 = 5;

// Ultrasonic sensor
const int trigPin = 9;
const int echoPin = 8;

const float SOUND_SPEED = 0.034;

float distance;
long duration;

// Servo
Servo servo1;
const float sensitivity = 1;
float angle = 90;

const int laserPin;
const int buzzerPin;
const int distanceLedPin;

const int MIN_DIST = 70;
const int MAX_DIST = 250;
bool canShoot = true;

// Command timeout
unsigned long lastCommandTime = 0;
const unsigned long commandTimeout = 200; // milliseconds

void setup() {
  Serial.begin(9600);

  initializeMotors();
  initializeUltrasonic();

  servo1.attach(10);
  servo1.write(90);
}

void loop() {
  handleCommunication();
  handleServo();
  handleDistanceSensor();
  handleLed();
  handleMotorTimeout();
}

void handleCommunication() {
  if (Serial.available()) {
    char command = Serial.read();
    lastCommandTime = millis();
    executeCommand(command);
  }
}

void handleServo() {
  angle = constrain(angle, 0, 180);
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
      break;

    case SQUARE:
      angle -= sensitivity;
      break;
  }
}

void handleMotorTimeout() {
  if (millis() - lastCommandTime > commandTimeout) {
    digitalWrite(M1pin1, LOW);
    digitalWrite(M1pin2, LOW);
    digitalWrite(M2pin1, LOW);
    digitalWrite(M2pin2, LOW);
  }
}

void handleDistanceSensor() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 25000);

  if (duration > 0) {
    distance = duration * SOUND_SPEED / 2;
  }
}

void handleLed() {
  if (distance > MIN_DIST && distance < MAX_DIST) {
    digitalWrite(distanceLedPin, HIGH);
    canShoot = true;
  } else {
    digitalWrite(distanceLedPin, LOW);
    canShoot = false;
  }
}

void shoot() {
  if (!canShoot) return;

  digitalWrite(laserPin, HIGH);
  tone(buzzerPin, 1000);
  delay(1000);
  noTone(buzzerPin);
  digitalWrite(laserPin, LOW);
}

// Motor control
void powerSide1(bool forward) {
  digitalWrite(M1pin1, forward ? HIGH : LOW);
  digitalWrite(M1pin2, forward ? LOW : HIGH);
}

void powerSide2(bool forward) {
  digitalWrite(M2pin1, forward ? LOW : HIGH);
  digitalWrite(M2pin2, forward ? HIGH : LOW);
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
