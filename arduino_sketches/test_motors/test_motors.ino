#define EN_LEFT 11  // ШИМ-пин для регулировки скорости левого мотора
#define IN1 9      // Направление вращения левого мотора (HIGH/LOW)
#define IN2 8      // Направление вращения левого мотора (HIGH/LOW)

#define EN_RIGHT 10 // ШИМ-пин для регулировки скорости правого мотора
#define IN3 12     // Направление вращения правого мотора (HIGH/LOW)
#define IN4 13       // Направление вращения правого мотора (HIGH/LOW)

#define ENCL_A 2
#define ENCL_B 4

#define ENCR_A 3
#define ENCR_B 5

volatile long enc1 = 0;
volatile long enc2 = 0;

unsigned long last_time = 0;

void setup() {
  Serial.begin(115200);

  pinMode(EN_LEFT, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(EN_RIGHT, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(ENCL_A, INPUT);
  pinMode(ENCL_B, INPUT);
  pinMode(ENCR_A, INPUT);
  pinMode(ENCR_B, INPUT);

  attachInterrupt(digitalPinToInterrupt(ENCL_A), readEnc1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCR_A), readEnc2, CHANGE);

  last_time = millis();
}

void moveForward(int speed) {
  analogWrite(EN_LEFT, speed);
  analogWrite(EN_RIGHT, speed);

  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void moveBackward(int speed) {
  analogWrite(EN_LEFT, speed);
  analogWrite(EN_RIGHT, speed);

  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

// энкодер 1
void readEnc1() {
  if (digitalRead(ENCL_A) == digitalRead(ENCL_B)) enc1++;
  else enc1--;
}

// энкодер 2
void readEnc2() {
  if (digitalRead(ENCR_A) == digitalRead(ENCR_B)) enc2++;
  else enc2--;
}

//// Функция: поворот налево (правое колесо вперёд, левое — остановлено)
//void turnLeft(int speed) {
//  analogWrite(EN_RIGHT, speed);
//  analogWrite(EN_LEFT, 0); // Левое колесо остановлено
//
//  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); // Правое — вперёд
//  digitalWrite(IN1, LOW);  digitalWrite(IN2, LOW); // Левое — блокировка
//}

//// Функция: поворот направо (левое колесо вперёд, правое — остановлено)
//void turnRight(int speed) {
//  analogWrite(EN_LEFT, speed);
//  analogWrite(EN_RIGHT, 0);
//
//  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); // Левое — вперёд
//  digitalWrite(IN3, LOW);  digitalWrite(IN4, LOW); // Правое — блокировка
//}

void stopRobot() {
  analogWrite(EN_LEFT, 0);
  analogWrite(EN_RIGHT, 0);

  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

void send_odometry() {
  unsigned long current_time = millis();
  float dt = (current_time - last_time) / 1000.0;
  last_time = current_time;

  noInterrupts();
  long e1 = enc1;
  long e2 = -enc2;
  interrupts();

  Serial.print("ODOM ");
  Serial.print(e1);
  Serial.print(",");
  Serial.print(e2);
  Serial.print(",");
  Serial.print(dt, 4);
  Serial.println("");
}

void loop() {
  moveForward(50);

  send_odometry();
  delay(50);
}
