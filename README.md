# Pressure8266
Version 0.0.1
Дата: 10.11.2025

Свойства проекта:
- работает только с ESP8266 
- есть фильтр сигнала от датчика
- есть вебстраница, где отображаются данные от датчика и переключатель отключения измерения
- данные отображаются по MQTT на сайте Home Assistant
- параметры подключения WiFi указыватся в коде приложения - ssid, password (файл PressureHA-V1-WiFi.cpp)
- параметры подключения к MQTT - файл SD_OpenTherm.hpp (стр. 38-45)

# Настройка проекта
- нужно настроить переменные трансляции в файле Config.h
- параметры Wi-Fi
  - #define LASER
  - 
    //#define DIR620
    
- подключаемые датчики
- один датчик температуры
-   //#define IF_PRESSURESENSOR
-   
    #define IF_TEMPERATURESENSOR
    
- оба датчика

  #define IF_PRESSURESENSOR
  
    #define IF_TEMPERATURESENSOR

  ## Установка и запуск

### Команды для загрузки проекта в контроллер ESP32

Перейдите в директорию проекта и выполните следующие команды:

```bash
cd "Перейдите директория проекта где расположен файл platformio.ini, например"
cd /home/user/hotbed
# Загрузка через USB и сразу Мониторинг последовательного порта
pio run --target upload && pio device monitor
```
Дополнительные команды PlatformIO
```bash
# Очистка проекта
pio run --target clean

```

## Поддерживаемые датчики
Датчик давления KY-3V3-IIC-16Bar-B
https://ali.click/rk5qog

