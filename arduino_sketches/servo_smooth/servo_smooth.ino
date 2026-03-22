#include <Servo.h>

#define SERVO_PIN_BASE 6
#define SERVO_PIN_GRIPPER 7

#define POS_START 80
#define POS_GRIP  178
#define OPEN_GRIP_POS 38
#define CLOSE_GRIP_POS 65

Servo servo1;
Servo servo2;

int current_pos = POS_GRIP;

void setup() {
  Serial.begin(115200);
  
  servo1.attach(SERVO_PIN_BASE);
  servo2.attach(SERVO_PIN_GRIPPER);
  
  servo1.write(current_pos);
  servo2.write(OPEN_GRIP_POS);
  delay(500);
  
  Serial.println("READY");
}

void read_serial() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.length() == 0) return;

    if (cmd == "1" || cmd == "G") {
      if (current_pos == POS_GRIP) {
        Serial.println("ALREADY GRIP");
      } else {
        for (int i = current_pos; i <= POS_GRIP; i += 1) {
          servo1.write(i);
          delay(30);
          current_pos = i;
        }
        Serial.println("DONE");
      }
    }
    else if (cmd == "2" || cmd == "S") {
      if (current_pos == POS_START) {
        Serial.println("ALREADY START");
      } else {
        for (int i = current_pos; i >= POS_START; i -= 1) {
          servo1.write(i);
          delay(30);
          current_pos = i;
        }
        Serial.println("DONE");
              }
    }
    else if (cmd == "P") {
      Serial.print("POS:");
      Serial.println(current_pos);
    }
  }
}

void loop() {
  read_serial();
}