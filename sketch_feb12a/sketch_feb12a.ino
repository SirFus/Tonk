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

// --- UPDATED FOR YOUR SPECIFIC BOARD ---
// This specific L298P shield uses D3, D5, D6, D7 for motor configurations
const int ENA = 3;  // Motor A Speed (PWM)
const int ENB = 5;  // Motor B Speed (PWM) -> From the NO.3.5.6 PIN header
const int INA = 6;  // Motor A Direction
const int INB = 7;  // Motor B Direction -> From the NO.7 PIN header

int motorSpeed = 200; // Speed value between 0 and 255

// --- Ultrasonic Sensor Pins (Moved to avoid motor conflict) ---
const int trigPin = 2; // Placed on the dedicated NO.2 PIN header
const int echoPin = A0; // Moved to Analog 0 (acting as digital input)

const float SOUND_SPEED = 0.034;
float distance;
long duration;

// --- Servo ---
Servo servo1;
const float sensitivity = 3; 
float angle = 90;

// --- Accessory Pins (Moved to free Analog Pins) ---
const int buzzerPin = A1;      // Moved to Analog 1
const int laserPin = A2;       // Moved to Analog 2
const int distanceLedPin = A3; // Moved to Analog 3

const int MIN_DIST = 70;
const int MAX_DIST = 250;
bool canShoot = true;

// --- Timing Variables ---
unsigned long lastCommandTime = 0;
const unsigned long commandTimeout = 200; 

unsigned long laserShootTime = 0;
bool isShooting = false;

unsigned long lastDistanceTime = 0;
const unsigned long distanceInterval = 80; 

void setup() {
  // Note: The Bluetooth interface on this board uses standard hardware RX/TX.
  // When uploading code via USB, you MUST unplug the Bluetooth module, 
  // otherwise the upload will fail!
  Serial.begin(9600);

  initializeMotors();
  initializeUltrasonic();

  pinMode(laserPin, OUTPUT);
  pinMode(distanceLedPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  //servo1.attach(9); // Matches the "Servo Interface (GND, 5V, D9)" perfectly!
  servo1.write(angle);
}

void loop() {
  handleCommunication(); 
  handleServo();
  
  if (millis() - lastDistanceTime >= distanceInterval) {
    handleDistanceSensor();
    handleLed();
    lastDistanceTime = millis();
  }
  
  handleMotorTimeout();
  updateShootState(); 
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
      moveTank(HIGH, HIGH, motorSpeed); 
      break;
    case BACKWARD:
      moveTank(LOW, LOW, motorSpeed);  
      break;
    case LEFT:
      moveTank(LOW, HIGH, motorSpeed); 
      break;
    case RIGHT:
      moveTank(HIGH, LOW, motorSpeed); 
      break;
    case CIRCLE:
      angle += sensitivity;
      break;
    case SQUARE:
      angle -= sensitivity;
      break;
    case CROSS:
      startShoot();
      break;
  }
}

void handleMotorTimeout() {
  if (millis() - lastCommandTime > commandTimeout) {
    analogWrite(ENA, 0);
    analogWrite(ENB, 0);
  }
}

void handleDistanceSensor() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 20000); 

  if (duration > 0) {
    distance = duration * SOUND_SPEED / 2;
  } else {
    distance = 999; 
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

void startShoot() {
  if (!canShoot || isShooting) return;

  isShooting = true;
  laserShootTime = millis();
  
  digitalWrite(laserPin, HIGH);
  tone(buzzerPin, 1000);
}

void updateShootState() {
  if (isShooting && (millis() - laserShootTime >= 1000)) {
    noTone(buzzerPin);
    digitalWrite(laserPin, LOW);
    isShooting = false;
  }
}

void moveTank(int dirA, int dirB, int speed) {
  digitalWrite(INA, dirA);
  digitalWrite(INB, dirB);
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void initializeMotors() {
  pinMode(INA, OUTPUT);
  pinMode(INB, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

void initializeUltrasonic() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}
