const mqtt = require('mqtt');
const { handleSensorData } = require('../handlers/sensor');
require('dotenv').config();

const client = mqtt.connect(process.env.MQTT_BROKER, {
    username: process.env.MQTT_USER,
    password: process.env.MQTT_PASS,
});

client.on('connect', () => {
    console.log('Connected to MQTT broker');
    client.subscribe('devices/esp32-1'); // change topic
});

client.on('message', (topic, message) => {
    let payload;
    try {
        payload = JSON.parse(message.toString());
    } catch {
        payload = message.toString();
    }
    handleSensorData(topic, payload);
});