const express = require('express');
const router  = express.Router();
const PowerReading = require('../model/acsModel');

// POST /api/acs712
router.post('/', async (req, res) => {
  const { voltage, current, wattage, fanOn, applianceState } = req.body;

  if (current == null || wattage == null)
    return res.status(400).json({ error: 'Missing current or wattage field' });

  try {
    const entry = new PowerReading({
      voltage, current, wattage, fanOn, applianceState,
      timestamp: new Date()
    });
    await entry.save();

    const emoji = fanOn ? '\u26A1' : '\uD83D\uDD0C';
    console.log(`${emoji} ACS712 — ${current.toFixed(3)}A | ${wattage.toFixed(1)}W | Fan: ${applianceState}`);

    res.status(201).json({ message: 'Power reading saved!', data: entry });
  } catch (err) {
    console.error('❌ ACS712 DB error:', err.message);
    res.status(500).json({ error: 'Failed to save power reading' });
  }
});

// GET /api/acs712
router.get('/', async (req, res) => {
  try {
    const entries = await PowerReading.find().sort({ timestamp: -1 }).limit(20);
    res.json(entries);
  } catch (err) {
    res.status(500).json({ error: 'Failed to fetch power readings' });
  }
});

module.exports = router;