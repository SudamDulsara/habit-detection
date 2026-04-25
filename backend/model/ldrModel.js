const mongoose = require('mongoose');

const ldrSchema = new mongoose.Schema({
  device: {
    type: String,
    default: 'ESP32_LDR_Sensor',
  },
  value: {
    type: Number,
    required: true,
  },
  status: {
    type: String,
    required: true,
  },
  timestamp: {
    type: Date,
    default: Date.now,
  },
});

module.exports = mongoose.model('LdrReading', ldrSchema, 'ldr_sensor');
