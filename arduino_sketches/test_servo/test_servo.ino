#include <Servo.h>

#define SERVO_PIN 6

Servo servo;

void setup() {
  servo.attach(SERVO_PIN);

  int start_pos = 120;
  servo.write(start_pos);
  delay(1000);
}

void loop() {
}
