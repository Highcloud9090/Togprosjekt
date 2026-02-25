# togprosjekt-backend

Backend med Node.js for togprosjektet til gruppe 6, MJM solutions, i IELS2001.

Sensor data kommer fra esp32 med MQTT til serveren og lagres i en SQL database.

---

## env config
```env
#mqtt (mosquitto)
MQTT_BROKER=mqtt://localhost
MQTT_USER=
MQTT_PASS=

#database (SQL)
DB_HOST=localhost
DB_USER=
DB_PASS=
DB_NAME=
```
