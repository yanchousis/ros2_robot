#include <Servo.h>

#define SERVO_PIN 9

Servo servo;  // Создаем объект

void setup() {
  servo.attach(SERVO_PIN);   // Указываем объекту класса Servo, что серво присоединен к пину

  servo.write(180);   // Выставляем начальное положение
  delay(2000);
//  servo.write(150); // Поворачиваем серво на 90 градусов
//  delay(2000);
//  servo.write(80);
//  delay(2000);
}

void loop() {
}