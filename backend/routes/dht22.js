const express = require('express');
const router  = express.Router();
const DHT22Reading = require('../models/dht22Model');

// POST /api/dht22
router.post('/', async (req, res) => {
  const { temperature, humidity, comfortStatus } = req.body;

  if (temperature == null || humidity == null)
    return res.status(400).json({ error: 'Missing temperature or humidity' });

  if (temperature < -40 || temperature > 80)
    return res.status(400).json({ error: 'Temperature out of valid range (-40 to 80 °C)' });

  if (humidity < 0 || humidity > 100)
    return res.status(400).json({ error: 'Humidity out of valid range (0–100 %)' });

  try {
    const reading = new DHT22Reading({ temperature, humidity, comfortStatus });
    await reading.save();
    console.log(`🌡️  DHT22 — Temp: ${temperature}°C | Humidity: ${humidity}% | ${comfortStatus}`);
    res.status(201).json({ message: 'DHT22 reading saved!', data: reading });
  } catch (err) {
    console.error('DHT22 DB error:', err.message);
    res.status(500).json({ error: 'Failed to save DHT22 reading' });
  }
});

// GET /api/dht22
router.get('/', async (req, res) => {
  try {
    const readings = await DHT22Reading.find().sort({ timestamp: -1 }).limit(20);
    res.json(readings);
  } catch (err) {
    res.status(500).json({ error: 'Failed to fetch DHT22 readings' });
  }
});

module.exports = router;