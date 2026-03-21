// ЛЕВЫЙ мотор (колесо)
#define EN_LEFT 11  // ШИМ-пин для регулировки скорости левого мотора
#define IN1 9      // Направление вращения левого мотора (HIGH/LOW)
#define IN2 8      // Направление вращения левого мотора (HIGH/LOW)

// ПРАВЫЙ мотор (колесо)
#define EN_RIGHT 10 // ШИМ-пин для регулировки скорости правого мотора
#define IN3 12     // Направление вращения правого мотора (HIGH/LOW)
#define IN4 13       // Направление вращения правого мотора (HIGH/LOW)

#define ENCL_A 2
#define ENCL_B 4

#define ENCR_A 3
#define ENCR_B 5

#define WHEEL_DIAMETER 70
#define WHEEL_BASE 210
#define WHEEL_OFFSET 41  // смещение колёс назад от центра (мм), изменить после измерения
#define TICKS_PER_REV 467
#define DEADZONE 15
#define ACCEL_ALPHA 0.1  // 0.05-0.2, меньше = плавнее

#define TICKS_TO_MM (PI * WHEEL_DIAMETER / TICKS_PER_REV)

float x = 0, y = 0, theta = 0;
long last_enc1 = 0, last_enc2 = 0;

float v_left = 0, v_right = 0;

float target_v = 0;
float target_w = 0;

float filtered_v = 0;
float filtered_w = 0;

float Kp = 0.6;
float Ki = 1.0;

float errL_i = 0;
float errR_i = 0;

volatile long enc1 = 0;
volatile long enc2 = 0;

unsigned long last_time = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {}  // Ждём установления соединения
  delay(500);         // Даём время на стабилизацию

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

void updateOdometry(float dt) {
  long d1 = enc1 - last_enc1;
  long d2 = enc2 - last_enc2;

  last_enc1 = enc1;
  last_enc2 = enc2;

  float distL = d1 * TICKS_TO_MM;
  float distR = d2 * TICKS_TO_MM;

  v_left = distL / dt;
  v_right = distR / dt;

  float ds = (distL + distR) / 2.0;
  float dtheta = (distR - distL) / (WHEEL_BASE + 2 * WHEEL_OFFSET);

  theta += dtheta;
  x += ds * cos(theta + dtheta * WHEEL_OFFSET / WHEEL_BASE);
  y += ds * sin(theta + dtheta * WHEEL_OFFSET / WHEEL_BASE);
}

void setVelocity(float linear, float angular) {
  target_v = linear;   // мм/с
  target_w = angular;  // рад/с
}

void computePID(float dt) {
  filtered_v += ACCEL_ALPHA * (target_v - filtered_v);
  filtered_w += ACCEL_ALPHA * (target_w - filtered_w);

  float targetL = filtered_v - filtered_w * WHEEL_BASE / 2.0;
  float targetR = filtered_v + filtered_w * WHEEL_BASE / 2.0;

  if (abs(targetL) < 0.1 && abs(targetR) < 0.1) {
    errL_i = 0;
    errR_i = 0;
  }

  float errL = targetL - v_left;
  float errR = targetR - v_right;

  errL_i += errL * dt;
  errR_i += errR * dt;

  errL_i = constrain(errL_i, -100, 100);
  errR_i = constrain(errR_i, -100, 100);

  float outL = Kp * errL + Ki * errL_i;
  float outR = Kp * errR + Ki * errR_i;

  if (abs(outL) < DEADZONE) outL = 0;
  if (abs(outR) < DEADZONE) outR = 0;

  outL = constrain(outL, -255, 255);
  outR = constrain(outR, -255, 255);

  setMotor(EN_LEFT, IN1, IN2, outL);
  setMotor(EN_RIGHT, IN3, IN4, outR);
}

void setMotor(int en, int in1, int in2, float speed) {
  int pwm = constrain(abs(speed), 0, 255);

  analogWrite(en, pwm);

  if (speed > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  } else if (speed < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
}

// энкодер 1
void readEnc1() {
  if (digitalRead(ENCL_A) == digitalRead(ENCL_B)) enc1++;
  else enc1--;
}

// энкодер 2
void readEnc2() {
  if (digitalRead(ENCR_A) == digitalRead(ENCR_B)) enc2--;
  else enc2++;
}

void stopRobot() {
  analogWrite(EN_LEFT, 0);
  analogWrite(EN_RIGHT, 0);

  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

void send_odometry(float dt) {
  Serial.print("ODOM ");
  Serial.print(x / 1000.0, 3);
  Serial.print(" ");
  Serial.print(y / 1000.0, 3);
  Serial.print(" ");
  Serial.print(theta, 3);
  Serial.print(" ");

  Serial.print(v_left / 1000.0, 3);
  Serial.print(" ");
  Serial.print(v_right / 1000.0, 3);
  Serial.print(" ");

  Serial.println(dt, 3);
}

void readCommand() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');

    if (cmd.startsWith("CMD")) {
      int i1 = cmd.indexOf(' ');
      int i2 = cmd.lastIndexOf(' ');

      float v = cmd.substring(i1 + 1, i2).toFloat();
      float w = cmd.substring(i2 + 1).toFloat();

      setVelocity(v * 1000.0, w); // м/с → мм/с

      Serial.println("OK");
    } else if (cmd.length() > 0) {
      Serial.print("ERR:");
      Serial.println(cmd);
    }
  }
}

void loop() {
  readCommand();

  unsigned long now = millis();
  float dt = (now - last_time) / 1000.0;

  if (dt <= 0 || dt > 0.1) {
    last_time = now;
    return;
  }

  last_time = now;

  updateOdometry(dt);
  computePID(dt);

  send_odometry(dt);

//  Serial.println(enc2);
  delay(50); // 20 Гц
}
