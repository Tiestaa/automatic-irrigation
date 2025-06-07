# Automatic irrigator

The automatic irrigator uses a Telegram bot to setup the system and being notified.  

## Before you run it

Before you run it, you must include a file named "secrets.h" containing:

```c
#define BOT_TOKEN "YOUR_BOT_TOKEN"
#define CHAT_ID "YOUR_TELEGRAM_CHAT_ID"
#define OPENPLANT_API_KEY String("YOUR_OPENPLANT_API_KEY")

#define SSID_WIFI "YOUR_WIFI_SSID"
#define PASS_WIFI "YOUR_WIFI_PASSWORD"
```

The second step is to change pin macros for sensors in `sensors.cpp`. 
> _NOTE:_  based on your bucket/water container you must adapt values of ultrasonic sensor (SENSOR_HEIGHT in `sensors.cpp` and MIN_WATER_FOR_PUMP in `sensors.h`). 