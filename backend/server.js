require('dotenv').config();

const express  = require('express');
const mongoose = require('mongoose');
const cors     = require('cors');

const dht22Routes  = require('./routes/dht22');
const pirRoutes    = require('./routes/pir');
const acs712Routes = require('./routes/acs712');
const ldrRoutes    = require('./routes/ldr');

const app = express();
app.use(express.json());
app.use(cors());

const MONGO_URI = process.env.MONGO_URI;

//Routes
app.use('/api/dht22',  dht22Routes);
app.use('/api/pir',    pirRoutes);
app.use('/api/acs712', acs712Routes);
app.use('/api/ldr32',  ldrRoutes);

app.get('/', (req, res) => {
  res.json({
    status: 'Backend is Live',
    endpoints: {
      dht22:  'POST /api/dht22   |  GET /api/dht22',
      pir:    'POST /api/pir     |  GET /api/pir',
      acs712: 'POST /api/acs712  |  GET /api/acs712',
      ldr32:  'POST /api/ldr32   |  GET /api/ldr32',
    }
  });
});

mongoose.connect(MONGO_URI)
  .then(() => {
    console.log('Connected to MongoDB Atlas');
    const PORT = process.env.PORT || 3000;
    app.listen(PORT, '0.0.0.0', () => {
      console.log(`Server running on port ${PORT}`);
      console.log(`DHT22  → POST http://localhost:${PORT}/api/dht22`);
      console.log(`PIR    → POST http://localhost:${PORT}/api/pir`);
      console.log(`ACS712 → POST http://localhost:${PORT}/api/acs712`);
      console.log(`LDR32  → POST http://localhost:${PORT}/api/ldr32`);
    });
  })
  .catch(err => console.error('MongoDB connection failed:', err.message));

process.on('unhandledRejection', err => {
  console.error('Unhandled error:', err.message);
});