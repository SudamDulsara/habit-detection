require('dotenv').config();

const express  = require('express');
const mongoose = require('mongoose');
const cors     = require('cors');

const dht22Routes = require('./routes/dht22');
const pirRoutes   = require('./routes/pir');

const app = express();
app.use(express.json());
app.use(cors());

const MONGO_URI = process.env.MONGO_URI;

//Routes
app.use('/api/dht22', dht22Routes);
app.use('/api/pir',   pirRoutes);

app.get('/', (req, res) => {
  res.json({
    status: 'Backend is Live',
    endpoints: {
      dht22: 'POST /api/dht22  |  GET /api/dht22',
      pir:   'POST /api/pir    |  GET /api/pir',
    }
  });
});

mongoose.connect(MONGO_URI)
  .then(() => {
    console.log('Connected to MongoDB Atlas');
    const PORT = process.env.PORT || 3000;
    app.listen(PORT, '0.0.0.0', () => {
      console.log(`Server running on port ${PORT}`);
      console.log(`DHT22 → POST http://localhost:${PORT}/api/dht22`);
      console.log(`PIR   → POST http://localhost:${PORT}/api/pir`);
    });
  })
  .catch(err => console.error('MongoDB connection failed:', err.message));

process.on('unhandledRejection', err => {
  console.error('Unhandled error:', err.message);
});