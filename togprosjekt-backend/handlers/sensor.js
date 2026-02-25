const db = require('../db/connection');

async function handleSensorData(topic, payload) {
    console.log(`[${topic}]`, payload);

    await db.execute(
        'INSERT INTO readings (topic, ts, accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z, ts_backend) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)',
        [topic, payload.ts, payload.accel.x, payload.accel.y, payload.accel.z, payload.gyro.x, payload.gyro.y, payload.gyro.z, Date.now()]
    );
};

module.exports = { handleSensorData };