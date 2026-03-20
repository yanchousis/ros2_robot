#include <Servo.h>

Servo servo_base;
Servo servo_elbow;
Servo servo_gripper;

#define SERVO_BASE_PIN 9
#define SERVO_ELBOW_PIN 10
#define SERVO_GRIPPER 11


// Base
#define SERVO_BASE_NULL_POS
#define SERVO_BASE_GRIP_POS
#define SERVO_BASE_RELEASE_POS


// Elbow
#define SERVO_ELBOW_NULL_POS
#define SERVO_ELBOW_GRIP_POS
#define SERVO_ELBOW_RELEASE_POS

// Gripper
#define SERVO_GRIPPER_OPEN_POS
#define SERVO_GRIPPER_CUBE_POS
#define SERVO_GRIPPER_CYLINDER_POS
#define SERVO_GRIPPER_DUCK_POS

String input_string = "";
double RADUIS_OBJECT = 0.0;

void setup() {
  Serial.begin(115200); // CHANGE
  
  servo_base.attach(SERVO_BASE_PIN);
  servo_elbow.attach(SERVO_ELBOW_PIN);
  servo_gripper.attach(SERVO_GRIPPER);

  Serial.println("READY");
}

void loop() {
  read_serial();
}

void read_serial() {
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n') {
      process_command(input_string);
      input_string = "";
    } else {
      input_string += c;
    }
  }
}

void process_command(String cmd) {
  for (int i = 0; i < cmd.length(); ++i) {
    char part = cmd[i];
    char value = cmd[i + 1];

    if (part == 'B') {
      move_base(value);
    }

    if (part == 'E') {
      move_elbow(value);
    }

    if (part == 'G') {
      move_gripper(value);
    }

    ++i;
  }

  Serial.println("OK");
}

void move_base(char v) {
  if (v == '0') servo_base.write();
  if (v == '1') servo_base.write();
  if (v == '2') servo_base.write();
}

void move_elbow(char v) {
  if (v == '0') servo_base.write();
  if (v == '1') servo_base.write();
  if (v == '2') servo_base.write();
}

void move_gripper(char v) {
  if (v == '0') servo_base.write();
  if (v == '1') servo_base.write();
  if (v == '2') servo_base.write();
  if (v == '3') servo_gripper.write();
}
