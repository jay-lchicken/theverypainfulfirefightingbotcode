// ------------------- Pin Definitions -------------------
#define LEFT_MOTOR_PWM   10   // Motor A Speed (PWM)
#define RIGHT_MOTOR_PWM  11   // Motor B Speed (PWM)
#define LEFT_MOTOR_EN    12   // Motor A Direction (EN)
#define RIGHT_MOTOR_EN   13   // Motor B Direction (EN)
#define RELAY_PIN         7   // Relay module
#define IR_LEFT_PIN      50   // Left IR sensor
#define IR_RIGHT_PIN     51   // Right IR sensor

// ------------------- Parameters -------------------
#define BASE_SPEED       210    // Base speed for forward motion
int offset = -23;               // Offset to balance motors (adjust if turning)
#define TURN_SPEED       200    // Speed to use when turning
#define SLOWER_TURN_SPEED 170 //SPeed when recovering from turn
bool turningLeft = false;
bool turningRight = false;


// ------------------- Setup -------------------
void setup() {
  pinMode(LEFT_MOTOR_PWM, OUTPUT);
  pinMode(RIGHT_MOTOR_PWM, OUTPUT);
  pinMode(LEFT_MOTOR_EN, OUTPUT);
  pinMode(RIGHT_MOTOR_EN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(IR_LEFT_PIN, INPUT);
  pinMode(IR_RIGHT_PIN, INPUT);

  digitalWrite(RELAY_PIN, LOW); // Relay off at start
  Serial.begin(9600);
}

// ------------------- Motor Control -------------------
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
  setLeftMotor(TURN_SPEED / 2);    // Slower left motor
  setRightMotor(TURN_SPEED);       // Faster right motor
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
  setLeftMotor(TURN_SPEED);        // Faster left motor
  setRightMotor(TURN_SPEED / 2);   // Slower right motor
}

void loop() {
  int leftIR = digitalRead(IR_LEFT_PIN);
  int rightIR = digitalRead(IR_RIGHT_PIN);
  Serial.println(leftIR);
  Serial.println(rightIR);
  if (turningRight){
    while ( rightIR != 1){
        slowTurn('R'); 
        rightIR = digitalRead(IR_RIGHT_PIN);  


    }
    leftIR = digitalRead(IR_LEFT_PIN);
    stopMotors();
    unsigned long startTime = millis();  // Record the start time

while (leftIR != 1) {
    setLeftMotor(-SLOWER_TURN_SPEED);
    leftIR = digitalRead(IR_LEFT_PIN);

    if (millis() - startTime > 3000) {  // 3 seconds timeout
        break;
    }
}

    turningRight = false;


  }else if (turningLeft){
    while ( leftIR != 1){
        slowTurn('L');   
        leftIR = digitalRead(IR_LEFT_PIN);


    }
    rightIR = digitalRead(IR_RIGHT_PIN);
    stopMotors();
    unsigned long startTime = millis();  // Record the start time

while (rightIR != 1) {
    setRightMotor(-SLOWER_TURN_SPEED);
    rightIR = digitalRead(IR_RIGHT_PIN);

    if (millis() - startTime > 3000) {  // 3000ms = 3s
        break;  // Exit the loop after 3 seconds
    }
}

    turningLeft = false;

  }
  else{
    
  Serial.println(leftIR + rightIR);

  if (leftIR == 1 && rightIR == 1) {
    stopMotors();
    digitalWrite(RELAY_PIN, HIGH);
    delay(4000);
    digitalWrite(RELAY_PIN, LOW);
    delay(200);
    moveBackward();
    delay(1000);
  } else if (leftIR == 0 && rightIR == 1) {
    turningLeft = true;

      slowTurn('L');   
} else if (leftIR == 1 && rightIR == 0) {
      turningRight = true;



  slowTurn('R');  
}else {
    moveForward();
  }
  }
}

void moveBackward() {
  int leftSpeed = -BASE_SPEED + offset;
  int rightSpeed = -BASE_SPEED - offset;
  setLeftMotor(leftSpeed);
  setRightMotor(rightSpeed);
}
