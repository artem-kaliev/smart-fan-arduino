#pragma once

// ===== БАЗОВЫЕ НАСТРОЙКИ =====

// ===== ПИНЫ =====
const int FAN_PIN = 3;        // Вентилятор (через транзистор)
const int LED_PIN_RED = 7;    // Индикатор работы
const int LED_PIN_GREEN = 8;  // Индикатор работы

// ===== Температурные пороги (в °C) =====
const float TEMP_THRESHOLD_ON = 30.0;   // Включение вентилятора
const float TEMP_THRESHOLD_OFF = 28.0;  // Выключение вентилятора (гистерезис)
const float TEMP_MAX_REASONABLE = 80.0; // Максимум для помещения
const float TEMP_MIN_REASONABLE = -10.0;// Минимум для помещения

// ===== Интервалы времени (в миллисекундах) =====
const unsigned long TEMP_READ_INTERVAL = 1000;  // Основное измерение температуры
const unsigned long SAMPLE_DELAY = 200;         // Задержка между усреднениями
const int DISPLAY_UPDATE_INTERVAL = 1000;       // Обновлять дисплей каждые 1000ms
const int BLINK_INTERVAL = 500;                 // Как часто мигает светодиод в случае сбоя

// ===== ПЕРЕМЕННЫЕ ДЛЯ ТАЙМЕРОВ =====
unsigned long previousTempReadTime = 0;  // Время последнего измерения температуры
unsigned long previousSampleTime = 0;    // Время последнего усреднения
unsigned long lastDisplayUpdate = 0;     // Время последнего вывода в дисплей

// ===== ПЕРЕМЕННЫЕ ДЛЯ ИЗМЕРЕНИЙ =====
float smoothedTemperature = 0;  // Сглаженное значение температуры
bool fanState = false;          // Текущее состояние вентилятора
int sampleCount = 0;            // Счетчик измерений для усреднения
float tempSum = 0;              // Сумма температур для усреднения

// ===== ПЕРЕМЕННЫЕ ДЛЯ ДИСПЛЕЯ =====
const int DISPLAY_ADDRESS = 0x27; // Адрес обычно 0x27 или 0x3F
const int DISPLAY_COLUMNS = 16;   // Максимальный размер столбцов дисплея
const int DISPLAY_ROWS = 2;       // Максимальный размер строк дисплея

// ===== ПРОВЕРКА НА СБОИ ДАТЧИКА =====
bool sensorError = false;
int sensorErrorCount = 0;
const int MAX_SENSOR_ERRORS = 10; // Число ошибок, после которого прерывается система
