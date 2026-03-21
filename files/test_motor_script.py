import serial
import time

ser = serial.Serial("/dev/ttyACM0", 115200, timeout=1)
time.sleep(2)

# --- Движение вперёд со скоростью 100 мм/с ---
print("Робот едет вперёд...")
ser.write(f"CMD 0.2 0.0".encode())
time.sleep(2)

# --- Движение назад со скоростью -100 мм/с ---
print("Робот едет назад...")
ser.write(f"CMD -0.2 0.0".encode())
time.sleep(2)

# --- Поворот на месте вправо с угловой скоростью 1 рад/с ---
print("Робот поворачивает направо...")
ser.write(f"CMD 0.0 1.0".encode())
time.sleep(2)

# --- Поворот на месте влево с угловой скоростью -1 рад/с ---
print("Робот поворачивает налево...")
ser.write(f"CMD 0.0 -1.0".encode())
time.sleep(2)

# --- Остановка робота ---
print("Робот останавливается.")
ser.write(f"CMD 0.0 0.0".encode())
time.sleep(4)

# Закрываем соединение
ser.close()

