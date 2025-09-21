#include <Wire.h>
#include "RTClib.h"

RTC_DS1307 rtc;

// Пин динамика
const int BUZZER_PIN = 8;

// Пины и параметры драйвера и двигателя
const int DIR_PIN = 4;  // Пин направления
const int STEP_PIN = 3; // Пин шага
const int stepsPerRevolution = 200;  // 200 шагов на полный оборот, 1.8° один шаг
const int microstepping = 1;      // Если используются микрошаги (1, 2, 4, 8, 16)

// Время звонов
// * утреня
const int MORNING_HOUR = 7;
const int MORNING_MINUTE = 0;
// * вечеря
const int EVENING_HOUR = 19;
const int EVENING_MINUTE = 0;

// Флаг для предотвращения повторного срабатывания в ту же минуту
bool ringing_is_triggered = false;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  pinMode(BUZZER_PIN, OUTPUT);

  if (!rtc.begin()) {
    Serial.println("Ошибка: RTC не найден!");
    while (1);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC не запущен. Устанавливаю время компиляции...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);

  Serial.println("Система запущена.");
  playBeep();
}

void loop() {
  // Текущее время
  DateTime now = rtc.now();

  int current_hour = now.hour();
  int current_minute = now.minute();
  int current_second = now.second();

  // Отладочный вывод текущего времени
  Serial.print("Время: ");
  Serial.print(current_hour);
  Serial.print(":");
  Serial.print(current_minute);
  Serial.print(":");
  Serial.println(current_second);

  // Проверка расписания звонов
  if ((current_hour == MORNING_HOUR && current_minute == MORNING_MINUTE && current_second == 0) ||
      (current_hour == EVENING_HOUR && current_minute == EVENING_MINUTE && current_second == 0)) {
    
    if (!ringing_is_triggered) {
      ringing_is_triggered = true; // чтобы не повторять в ту же минуту
      ringTheBell();
    }
  } else {
    // Сброс флага, чтобы на следующую минуту снова сработало
    ringing_is_triggered = false;
  }

  delay(1000);
}

void playBeep() {
  tone(BUZZER_PIN, 1000);
  delay(250); // 1/4 секунды звука
  noTone(BUZZER_PIN);
}

void ringTheBell() {
  Serial.println("Звон запущен.");
  for (int i = 0; i < 40; i++) {
      //playBeep();
      digitalWrite(DIR_PIN, HIGH);
      rotateMotor(stepsPerRevolution/4 * microstepping);  // 1/4 оборота по часовой
      digitalWrite(DIR_PIN, LOW);
      rotateMotor(stepsPerRevolution/4 * microstepping);  // 1/4 оборота против часовой
      delay(2000); // 2 с интервал между ударами
  }
  Serial.println("Звон завершён.");
}

// Вращение валом шаговика на заданное число шагов
void rotateMotor(int steps) {
  for (int i = 0; i < steps; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(800);  // Контроль скорости
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(800);
  }
}