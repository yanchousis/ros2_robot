#!/usr/bin/env python3

import serial
import sys
import signal

running = True

def signal_handler(sig, frame):
    global running
    print("\nЗавершение программы...")
    running = False

signal.signal(signal.SIGINT, signal_handler)

def main():
    # Проверка аргументов командной строки
    if len(sys.argv) != 3:
        print("Использование: {} <порт> <скорость>".format(sys.argv[0]))
        print("Пример: {} /dev/ttyUSB0 115200".format(sys.argv[0]))
        sys.exit(1)

    port = sys.argv[1]
    baudrate = int(sys.argv[2])

    try:
        ser = serial.Serial(
            port=port,
            baudrate=baudrate,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=1
        )
        print(f"Порт {port} открыт. Ожидание данных... (Ctrl+C для выхода)")

        while running:
            data = ser.read(ser.in_waiting or 1)
            if data:
                # Вывод сырых данных в виде шестнадцатеричной строки
                # (без пробелов, слитно)
                # print(data.hex(), end=' ', flush=True)
                # Альтернативный вариант с пробелами для лучшей читаемости:
                print(' '.join(f'{b:02x}' for b in data), end=' ', flush=True)
                print('\n')
                


    except serial.SerialException as e:
        print(f"Ошибка открытия порта: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"Ошибка: {e}")
        sys.exit(1)
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("Порт закрыт.")

if __name__ == "__main__":
    main()