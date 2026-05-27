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

// --- L298P Motor Shield Pin Designations (Fixed by Shield Hardware) ---
const int INA = 12; // Motor A Direction
const int INB = 13; // Motor B Direction
const int ENA = 10; // Motor A Speed (PWM)
const int ENB = 11; // Motor B Speed (PWM)

int motorSpeed = 200; // Speed value between 0 and 255

// --- Ultrasonic Sensor Pins ---
const int trigPin = 2;
const int echoPin = 3; // CHANGED from 4 because 4 is used by the Shield Buzzer

const float SOUND_SPEED = 0.034;
float distance;
long duration;

// --- Servo ---
Servo servo1;
const float sensitivity = 3; // Step amount for servo rotation per button press
float angle = 90;

// --- Accessory Pins ---
const int buzzerPin = 4;      // Fixed by L298P Shield hardware
const int laserPin = 5;       // Assigned to Digital Pin 5
const int distanceLedPin = 6; // Assigned to Digital Pin 6

const int MIN_DIST = 70;
const int MAX_DIST = 250;
bool canShoot = true;

// --- Timing Variables (Non-blocking controls) ---
unsigned long lastCommandTime = 0;
const unsigned long commandTimeout = 200; // Stop motors if no command in 200ms

unsigned long laserShootTime = 0;
bool isShooting = false;

unsigned long lastDistanceTime = 0;
const unsigned long distanceInterval = 80; // Only check distance every 80ms to prevent lag

void setup() {
  Serial.begin(9600);

  initializeMotors();
  initializeUltrasonic();

  pinMode(laserPin, OUTPUT);
  pinMode(distanceLedPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  servo1.attach(9); // Servo Signal Pin
  servo1.write(angle);
}

void loop() {
  handleCommunication(); // Checked constantly for instant response
  handleServo();
  
  // Periodically checks distance so pulseIn() doesn't bottleneck the Bluetooth serial buffer
  if (millis() - lastDistanceTime >= distanceInterval) {
    handleDistanceSensor();
    handleLed();
    lastDistanceTime = millis();
  }
  
  handleMotorTimeout();
  updateShootState(); // Manages firing laser/buzzer timing
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
      moveTank(HIGH, HIGH, motorSpeed); // Both tracks forward
      break;

    case BACKWARD:
      moveTank(LOW, LOW, motorSpeed);  // Both tracks backward
      break;

    case LEFT:
      moveTank(LOW, HIGH, motorSpeed); // Pivot Left (Track A back, Track B forward)
      break;

    case RIGHT:
      moveTank(HIGH, LOW, motorSpeed); // Pivot Right (Track A forward, Track B back)
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
    // Cut PWM speed to both motors safely if Bluetooth disconnects or button released
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

  // 20000ms timeout prevents the Arduino from freezing if the sensor misses an echo
  duration = pulseIn(echoPin, HIGH, 20000); 

  if (duration > 0) {
    distance = duration * SOUND_SPEED / 2;
  } else {
    distance = 999; // Clear path / out of range
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

// Starts the shooting process cleanly
void startShoot() {
  if (!canShoot || isShooting) return;

  isShooting = true;
  laserShootTime = millis();
  
  digitalWrite(laserPin, HIGH);
  tone(buzzerPin, 1000);
}

// Automatically turns off laser and buzzer after 1000ms has elapsed
void updateShootState() {
  if (isShooting && (millis() - laserShootTime >= 1000)) {
    noTone(buzzerPin);
    digitalWrite(laserPin, LOW);
    isShooting = false;
  }
}

// --- Shield Motor Driving Logic ---
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
