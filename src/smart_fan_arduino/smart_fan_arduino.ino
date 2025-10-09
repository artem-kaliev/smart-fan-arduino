// ===== БАЗОВЫЕ НАСТРОЙКИ =====
#include "config.h"

// ===== БИБЛИОТЕКИ =====
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>

// Настройка датчика
#define ONE_WIRE_BUS 2  // Пин DATA датчика
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

LiquidCrystal_I2C lcd(DISPLAY_ADDRESS, DISPLAY_COLUMNS, DISPLAY_ROWS);  // Создаем объект дисплея

// ===== НАСТРОЙКА =====
void setup() {

  sensors.begin();            // Включить датчик температуры
  sensors.setResolution(10);  // Погрешность в 0.25°C, частота 187.5 ms

  /*
  Защита от случайного включения вентилятора.
  С началом работы датчика первое измерение выводит завышенную температуру.
  Небольшая задержка пропускает 1-ое измерение.
  */
  sensors.requestTemperatures();
  delay(750);

  lcd.init();       // Инициализация дисплея
  lcd.backlight();  // Включаем подсветку

  // Выводим приветственное сообщение
  lcd.setCursor(0, 0);  // Столбец 0, строка 0
  lcd.print("Smart Fan v1.0");
  lcd.setCursor(0, 1);  // Столбец 0, строка 1
  lcd.print("Initializing...");

  // Настройка пинов
  pinMode(FAN_PIN, OUTPUT);
  pinMode(LED_PIN_RED, OUTPUT);
  pinMode(LED_PIN_GREEN, OUTPUT);

  // Гарантированно выключаем все при запуске
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(LED_PIN_RED, LOW);
  digitalWrite(LED_PIN_GREEN, LOW);
}

// ===== ОСНОВНОЙ ЦИКЛ =====
void loop() {

  // 1. УСРЕДНЕННОЕ ИЗМЕРЕНИЕ ТЕМПЕРАТУРЫ (каждые 200мс)
  if ((unsigned long)(millis() - previousSampleTime) >= SAMPLE_DELAY) {
    previousSampleTime = millis();

    // Измерение температуры с датчика
    sensors.requestTemperatures();
    float temperature = sensors.getTempCByIndex(0);

    #ifdef ENABLE_TEMPERATURE_VALIDATION
    
    // Проверка температурных данных
    if (checkTemperatureValidity(temperature)) {
      // Данные достоверны - обрабатываем как обычно
      tempSum += temperature;
      sampleCount++;
    };

    #else
    
    // Обработка без проверок
    tempSum += temperature;
    sampleCount++;

    #endif

      // Когда набрали 5 измерений - вычисляем среднее
      if (sampleCount >= 5) {
        smoothedTemperature = tempSum / 5;
        tempSum = 0;
        sampleCount = 0;
      }
    }
  // 2. ОСНОВНОЕ УПРАВЛЕНИЕ ВЕНТИЛЯТОРОМ (каждые 1000мс)
  if ((unsigned long)(millis() - previousTempReadTime) >= TEMP_READ_INTERVAL) {
    previousTempReadTime = millis();
    controlFan(smoothedTemperature);
  }

  // 3. ОТЛАДОЧНЫЙ ВЫВОД В ДИСПЛЕЙ (каждые 1000мс)
  if ((unsigned long)(millis() - lastDisplayUpdate) >= DISPLAY_UPDATE_INTERVAL) {
    lastDisplayUpdate = millis();
    updateDisplay(smoothedTemperature, fanState);
  }
}

void updateDisplay(float temperature, bool fanState) {
  // Столбец 0, строка 0, очистка строки
  lcd.setCursor(0, 0);
  lcd.print("                ");

  // Столбец 0, строка 0, ввод температуры
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature, 1);
  lcd.print("C");
  lcd.print((char)223);

  // Столбец 0, строка 1, очистка строки
  lcd.setCursor(0, 1);
  lcd.print("                ");

  // Столбец 0, строка 1, ввод состояния вентилятора
  lcd.setCursor(0, 1);
  lcd.print("Fan: ");
  lcd.print(fanState ? "ON " : "OFF");
}

// ===== ФУНКЦИЯ УПРАВЛЕНИЯ ВЕНТИЛЯТОРОМ =====
void controlFan(float temperature) {
  bool newFanState = fanState;

  // Логика с гистерезисом для предотвращения частых включений/выключений
  if (temperature >= TEMP_THRESHOLD_ON) {
    newFanState = true;
  } else if (temperature <= TEMP_THRESHOLD_OFF) {
    newFanState = false;
  }
  // Между TEMP_THRESHOLD_OFF и TEMP_THRESHOLD_ON состояние не меняется

  // Применяем изменения только если состояние изменилось
  if (newFanState != fanState) {
    fanState = newFanState;
    digitalWrite(FAN_PIN, fanState ? HIGH : LOW);
    digitalWrite(LED_PIN_GREEN, fanState ? HIGH : LOW);
    digitalWrite(LED_PIN_RED, fanState ? LOW : HIGH);
  }
}

// ===== ФУНКЦИЯ ПРЕРЫВАНИЯ РАБОТЫ ВЕНТИЛЯТОРА =====
void emergencyShutdown(const char* reason) {
  // Выключаем все системы
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(LED_PIN_GREEN, LOW);

  // Выводим сообщение об ошибке на дисплей
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.print(reason);

  // Бесконечный цикл - система остановлена
  while (true) {
    digitalWrite(LED_PIN_RED, HIGH);
    delay(BLINK_INTERVAL);
    digitalWrite(LED_PIN_RED, LOW);
    delay(BLINK_INTERVAL);
  }
}

#ifdef ENABLE_TEMPERATURE_VALIDATION
/*
Функция проверяет измерительные данные датчика
и в случае некорректных показателей
приостанавливает работу программы
*/
bool checkTemperatureValidity(float temperature) {
  if (temperature == DEVICE_DISCONNECTED_C || temperature > TEMP_MAX_REASONABLE || temperature < TEMP_MIN_REASONABLE) {

    sensorErrorCount++;

    if (sensorErrorCount >= MAX_SENSOR_ERRORS) {
      emergencyShutdown("SENSOR ERROR");
      return false;  // Критическая ошибка, приостанавливаем программу
    }
    return false;  // Временная ошибка, пропуск измерения
  } else {
    sensorErrorCount = 0;  // Сброс счетчика при успешных измерениях
    return true;  // Данные достоверны
  }
}
#endif