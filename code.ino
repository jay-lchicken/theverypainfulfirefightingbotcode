// ------------------- Pin Definitions -------------------
#define LEFT_MOTOR_PWM 10   // Motor A Speed (PWM)
#define RIGHT_MOTOR_PWM 11  // Motor B Speed (PWM)
#define LEFT_MOTOR_EN 12    // Motor A Direction (EN)
#define RIGHT_MOTOR_EN 13   // Motor B Direction (EN)
#define RELAY_PIN 7         // Relay module
#define IR_LEFT_PIN 50      // Left IR sensor
#define IR_RIGHT_PIN 51     // Right IR sensor

// ------------------- Parameters -------------------
#define BASE_SPEED 210         // Base speed for forward motion
int offset = -23;              // Offset to balance motors (adjust if turning)
#define TURN_SPEED 200         // Speed to use when turning
#define SLOWER_TURN_SPEED 170  //SPeed when recovering from turn
bool turningLeft = false;
bool turningRight = false;
int distanceMeasure[3] = { 0, 0, 0 };
const int trigPinLeft = 26;
const int echoPinLeft = 27;
const int trigPinMiddle = 25;
const int echoPinMiddle = 24;
const int trigPinRight = 22;
const int echoPinRight = 23;

int IRReadInMiddle = false;
long readUltrasonic(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);  // 30 ms timeout
  long distance = duration * 0.034 / 2;
  return distance;
}


// ------------------- Setup -------------------
void setup() {
  pinMode(LEFT_MOTOR_PWM, OUTPUT);
  pinMode(RIGHT_MOTOR_PWM, OUTPUT);
  pinMode(LEFT_MOTOR_EN, OUTPUT);
  pinMode(RIGHT_MOTOR_EN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(IR_LEFT_PIN, INPUT);
  pinMode(IR_RIGHT_PIN, INPUT);
  pinMode(trigPinLeft, OUTPUT);
  pinMode(echoPinLeft, INPUT);
  pinMode(trigPinMiddle, OUTPUT);
  pinMode(echoPinMiddle, INPUT);
  pinMode(trigPinRight, OUTPUT);
  pinMode(echoPinRight, INPUT);

  digitalWrite(RELAY_PIN, LOW);  // Relay off at start
  Serial.begin(9600);
}

// ------------------- Motor Control -------------------

void moveBackward() {
  int leftSpeed = -BASE_SPEED + offset;
  int rightSpeed = -BASE_SPEED - offset;
  setLeftMotor(leftSpeed);
  setRightMotor(rightSpeed);
}
void setLeftMotor(int speed) {
  int setSpeed = constrain(abs(speed), 0, 255);
  if (speed >= 0) {
    digitalWrite(LEFT_MOTOR_EN, HIGH);
    analogWrite(LEFT_MOTOR_PWM, setSpeed);
  } else {
    digitalWrite(LEFT_MOTOR_EN, LOW);
    analogWrite(LEFT_MOTOR_PWM, setSpeed);
  }
}

void setRightMotor(int speed) {
  int setSpeed = constrain(abs(speed), 0, 255);
  if (speed >= 0) {
    digitalWrite(RIGHT_MOTOR_EN, HIGH);
    analogWrite(RIGHT_MOTOR_PWM, setSpeed);
  } else {
    digitalWrite(RIGHT_MOTOR_EN, LOW);
    analogWrite(RIGHT_MOTOR_PWM, setSpeed);
  }
}

void moveForward() {
  int leftSpeed = BASE_SPEED - offset;
  int rightSpeed = BASE_SPEED + offset;
  setLeftMotor(leftSpeed);
  setRightMotor(rightSpeed);
}

void stopMotors() {
  setLeftMotor(0);
  setRightMotor(0);
}

void turnLeft() {
  setLeftMotor(TURN_SPEED / 2);  // Slower left motor
  setRightMotor(TURN_SPEED);     // Faster right motor
}
void slowTurn(char direction) {
  if (direction == 'L') {
    // Gentle left turn: left motor slower, right full
    setLeftMotor(-TURN_SPEED);
    setRightMotor(TURN_SPEED);
  } else if (direction == 'R') {
    // Gentle right turn: left full, right slower
    setLeftMotor(TURN_SPEED);
    setRightMotor(-TURN_SPEED);
  }
}


void turnRight() {
  setLeftMotor(TURN_SPEED);       // Faster left motor
  setRightMotor(TURN_SPEED / 2);  // Slower right motor
}
int leftIR = digitalRead(IR_LEFT_PIN);
int rightIR = digitalRead(IR_RIGHT_PIN);
void loop() {
  if (IRReadInMiddle) {
    leftIR = digitalRead(IR_LEFT_PIN);
    rightIR = digitalRead(IR_RIGHT_PIN);

  } else {
    IRReadInMiddle = false;
  }



  Serial.print("IR Sensors - Left: ");
  Serial.print(leftIR);
  Serial.print(", Right: ");
  Serial.println(rightIR);

  if (turningRight) {
    Serial.println("=== TURNING RIGHT MODE ===");
    while (rightIR != 1) {
      Serial.println("Turning right until right IR detects line");
      slowTurn('R');
      rightIR = digitalRead(IR_RIGHT_PIN);
      Serial.print("Right IR reading: ");
      Serial.println(rightIR);
    }
    Serial.println("Right IR detected line, now checking left IR");

    leftIR = digitalRead(IR_LEFT_PIN);
    stopMotors();
    Serial.println("Motors stopped, starting fine adjustment");

    unsigned long startTime = millis();

    while (leftIR != 1) {
      Serial.println("Fine adjusting left motor");
      setLeftMotor(-SLOWER_TURN_SPEED);
      leftIR = digitalRead(IR_LEFT_PIN);
      Serial.print("Left IR reading: ");
      Serial.println(leftIR);

      if (millis() - startTime > 3000) {
        Serial.println("TIMEOUT: Fine adjustment took too long (3s)");
        break;
      }
    }

    turningRight = false;
    Serial.println("Turning right mode completed");

  } else if (turningLeft) {
    Serial.println("=== TURNING LEFT MODE ===");
    while (leftIR != 1) {
      Serial.println("Turning left until left IR detects line");
      slowTurn('L');
      leftIR = digitalRead(IR_LEFT_PIN);
      Serial.print("Left IR reading: ");
      Serial.println(leftIR);
    }
    Serial.println("Left IR detected line, now checking right IR");

    rightIR = digitalRead(IR_RIGHT_PIN);
    stopMotors();
    Serial.println("Motors stopped, starting fine adjustment");

    unsigned long startTime = millis();

    while (rightIR != 1) {
      Serial.println("Fine adjusting right motor");
      setRightMotor(-SLOWER_TURN_SPEED);
      rightIR = digitalRead(IR_RIGHT_PIN);
      Serial.print("Right IR reading: ");
      Serial.println(rightIR);

      if (millis() - startTime > 3000) {
        Serial.println("TIMEOUT: Fine adjustment took too long (3s)");
        break;
      }
    }

    turningLeft = false;
    Serial.println("Turning left mode completed");

  } else {
    Serial.println("=== NORMAL NAVIGATION MODE ===");

    if (leftIR == 1 && rightIR == 1) {
      Serial.println("BOTH IR SENSORS DETECTED LINE - ACTIVATING RELAY");
      stopMotors();
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("Relay ON for 4 seconds");
      delay(4000);
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("Relay OFF, backing up");
      delay(200);
      moveBackward();
      delay(1000);
      slowTurn("L");
      delay(700);
      stopMotors();
      Serial.println("Backup completed");

    } else if (leftIR == 0 && rightIR == 1) {
      Serial.println("RIGHT IR DETECTED LINE - Setting turning left mode");
      turningLeft = true;
      slowTurn('L');

    } else if (leftIR == 1 && rightIR == 0) {
      Serial.println("LEFT IR DETECTED LINE - Setting turning right mode");
      turningRight = true;
      slowTurn('R');

    } else {
      Serial.println("NO IR DETECTION - Using ultrasonic navigation");

      long leftDist = readUltrasonic(trigPinLeft, echoPinLeft);
      delay(50);
      long midDist = readUltrasonic(trigPinMiddle, echoPinMiddle);
      delay(50);
      long rightDist = readUltrasonic(trigPinRight, echoPinRight);

      Serial.print("Ultrasonic - Left: ");
      Serial.print(leftDist);
      Serial.print(" cm, Mid: ");
      Serial.print(midDist);
      Serial.print(" cm, Right: ");
      Serial.print(rightDist);
      Serial.println(" cm");

      if (midDist > 10) {
        Serial.println("PATH CLEAR AHEAD - Moving forward");
        while (midDist > 10) {
          leftIR = digitalRead(IR_LEFT_PIN);
          rightIR = digitalRead(IR_RIGHT_PIN);
          Serial.println("Moving forward (path clear)");
          moveForward();
          long leftDist = readUltrasonic(trigPinLeft, echoPinLeft);
          delay(50);
          long midDist = readUltrasonic(trigPinMiddle, echoPinMiddle);
          delay(50);
          long rightDist = readUltrasonic(trigPinRight, echoPinRight);
          //CHECK CIRCLE IN LOOP
          //NEED tobreak of while loop
          if (leftIR == 1 || rightIR == 1) {
            IRReadInMiddle = true;  //So that on next loop, they will not read the values again and use this value instead
            Serial.println("breaking");
            stopMotors();
            break;
          }


          if (leftDist <= 10 || rightDist <= 10) {
            Serial.println("Side obstacle detected - stopping forward movement");
            break;
          }
        }
        Serial.println("Forward movement stopped");
        stopMotors();

      }else if (midDist <= 10 && midDist != 0) {
        Serial.println("OBSTACLE AHEAD - Executing left bypass maneuver");
        slowTurn('L');
        delay(800);
        stopMotors();
        Serial.println("Turned left, moving forward");
        moveForward();
        delay(800);
        stopMotors();
        Serial.println("Moving forward complete, turning left again");
        slowTurn('R');
        delay(800);
        stopMotors();
        Serial.println("Left bypass maneuver completed");

      } else if (leftDist <= 10 && leftDist != 0 && midDist == 0 && rightDist == 0) {
        Serial.println("LEFT OBSTACLE ONLY - Executing right bypass");
        slowTurn('R');
        delay(200);
        stopMotors();
        Serial.println("Turned right, moving forward");
        moveForward();
        delay(800);
        stopMotors();
        Serial.println("Moving forward complete, correcting with left turn");
        slowTurn('L');
        delay(600);
        stopMotors();
        Serial.println("Right bypass with left correction completed");

      } else if (rightDist <= 10 && rightDist != 0 && midDist == 0 && leftDist == 0) {
        Serial.println("RIGHT OBSTACLE ONLY - Executing left bypass");
        slowTurn('L');
        delay(200);
        stopMotors();
        Serial.println("Turned left, moving forward");
        moveForward();
        delay(800);
        stopMotors();
        Serial.println("Moving forward complete, correcting with right turn");
        slowTurn('R');
        delay(600);
        stopMotors();
        Serial.println("Left bypass with right correction completed");

      } else if (rightDist <= 10 && midDist <= 10 && rightDist != 0 && midDist != 0) {
        Serial.println("RIGHT + MID OBSTACLES - Executing left avoidance");
        slowTurn('L');
        delay(400);
        stopMotors();
        Serial.println("Turned left, moving forward");
        moveForward();
        delay(800);
        stopMotors();
        Serial.println("Moving forward complete, correcting with right turn");
        slowTurn('R');
        delay(600);
        stopMotors();
        Serial.println("Left avoidance maneuver completed");
      }

      Serial.println("Ultrasonic navigation cycle completed");
      delay(300);
    }
  }

  Serial.println("--- Loop iteration completed ---\n");
}
