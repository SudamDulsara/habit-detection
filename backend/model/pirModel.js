const mongoose = require('mongoose');

const motionSchema = new mongoose.Schema({
  device: {
    type: String,
    default: 'ESP32_PIR_Sensor',
  },
  // 0 = no motion, 1 = motion detected
  motion: {
    type: Number,
    required: true,
  },
  // "Occupied" or "Unoccupied"
  occupancy: {
    type: String,
    required: true,
  },
  timestamp: {
    type: Date,
    default: Date.now,
  },
});

module.exports = mongoose.model('Motion', motionSchema, 'motion_sensor');