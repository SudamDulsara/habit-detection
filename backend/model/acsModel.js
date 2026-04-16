const mongoose = require('mongoose');

const acs712Schema = new mongoose.Schema({
  device: {
    type: String,
    default: 'ESP32_ACS712_Sensor',
  },
  voltage: {
    type: Number,
    required: true,
  },
  current: {
    type: Number,
    required: true,
  },
  wattage: {
    type: Number,
    required: true,
  },
  fanOn: {
    type: Boolean,
    required: true,
  },
  applianceState: {
    type: String,
    required: true,
  },
  timestamp: {
    type: Date,
    default: Date.now,
  },
});

module.exports = mongoose.model('PowerReading', acs712Schema, 'power_sensor');