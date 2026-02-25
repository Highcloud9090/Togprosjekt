require('dotenv').config();
require('./mqtt/client'); // starts the MQTT listener
console.log('IoT backend running');