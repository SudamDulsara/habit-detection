const mongoose = require('mongoose');

const dht22Schema = new mongoose.Schema({
  temperature:   { type: Number, required: true },
  humidity:      { type: Number, required: true },
  comfortStatus: { type: String },
  timestamp:     { type: Date, default: Date.now }
});

module.exports = mongoose.model('DHT22Reading', dht22Schema, 'temp_sensor');