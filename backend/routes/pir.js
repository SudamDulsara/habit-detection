const express = require('express');
const router  = express.Router();
const Motion  = require('../models/pirModel');

// POST /api/pir
router.post('/', async (req, res) => {
  const { motion, occupancy } = req.body;

  if (motion == null || occupancy == null)
    return res.status(400).json({ error: 'Missing motion or occupancy field' });

  if (motion !== 0 && motion !== 1)
    return res.status(400).json({ error: 'motion must be 0 or 1' });

  if (!['Occupied', 'Unoccupied'].includes(occupancy))
    return res.status(400).json({ error: 'occupancy must be "Occupied" or "Unoccupied"' });

  try {
    const entry = new Motion({ motion, occupancy, timestamp: new Date() });
    await entry.save();

    const emoji = motion === 1 ? '👤' : '🚫';
    console.log(`${emoji} PIR — Room is ${occupancy}`);

    res.status(201).json({ message: 'Occupancy data saved!', data: entry });
  } catch (err) {
    console.error('❌ PIR DB error:', err.message);
    res.status(500).json({ error: 'Failed to save PIR reading' });
  }
});

// GET /api/pir
router.get('/', async (req, res) => {
  try {
    const entries = await Motion.find().sort({ timestamp: -1 }).limit(20);
    res.json(entries);
  } catch (err) {
    res.status(500).json({ error: 'Failed to fetch PIR readings' });
  }
});

module.exports = router;