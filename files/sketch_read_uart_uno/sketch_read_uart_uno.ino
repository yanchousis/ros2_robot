#include <SoftwareSerial.h>

// Программный порт: RX = пин 10, TX = пин 11 (TX не используется)
SoftwareSerial lidarSerial(10, 11);

void setup() {
  Serial.begin(9600);                // связь с компьютером
  lidarSerial.begin(1);         // скорость лидара (измените!)
  Serial.println("Uno готова, слушаем пин 10...");
}

void loop() {
  if (lidarSerial.available()) {
    while (lidarSerial.available()) {
      byte b = lidarSerial.read();
//      Serial.write(b);                // отправляем в монитор порта
            // Для отладки можно добавить вывод в HEX:
       if (b < 0x10) Serial.print('0');
       Serial.print(b, HEX);
       Serial.print(' ');
//       Serial.print('\n');
    }
  }
}
