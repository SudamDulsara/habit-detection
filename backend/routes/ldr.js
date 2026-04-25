const express = require('express');
const router  = express.Router();
const LdrReading = require('../model/ldrModel');

// POST /api/ldr
router.post('/', async (req, res) => {
  const { device, value, status } = req.body;

  if (value == null || status == null)
    return res.status(400).json({ error: 'Missing value or status field' });

  try {
    const entry = new LdrReading({ device, value, status, timestamp: new Date() });
    await entry.save();

    const emoji = status === 'Dark' ? '🌑' : '☀️';
    console.log(`${emoji} LDR — Value: ${value} | Status: ${status}`);

    res.status(201).json({ message: 'LDR reading saved!', data: entry });
  } catch (err) {
    console.error('❌ LDR DB error:', err.message);
    res.status(500).json({ error: 'Failed to save LDR reading' });
  }
});

// GET /api/ldr
router.get('/', async (req, res) => {
  try {
    const entries = await LdrReading.find().sort({ timestamp: -1 }).limit(20);
    res.json(entries);
  } catch (err) {
    res.status(500).json({ error: 'Failed to fetch LDR readings' });
  }
});

module.exports = router;
