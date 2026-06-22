// PaidingAttention 2.0 - AZL Node Backend
const express = require('express');
const cors = require('cors');

const app = express();
const PORT = process.env.PORT || 8080;

app.use(cors({
  origin: '*',
  methods: ['GET', 'POST', 'OPTIONS'],
  allowedHeaders: ['Content-Type', 'Authorization']
}));
app.use(express.json({ limit: '1mb' }));

app.get('/', (req, res) => {
  res.json({ service: 'PaidingAttention 2.0', status: 'ok', node: 'AZL' });
});

// Register / Anchor
app.post('/api/ai-register', (req, res) => {
  const { node_id, manifest, fw_version } = req.body || {};
  console.log(`[AZL] Register: ${node_id || 'unknown'} fw=${fw_version || 'n/a'}`);
  res.json({
    status: 'ok',
    anchored: true,
    node_id: node_id || 'LAT-MILTG6042BP',
    timestamp: new Date().toISOString(),
    lattice: 'ABSZERO'
  });
});

// Tier Ingest
app.post('/api/azl-ingest', (req, res) => {
  const { node_id, tier1, tier2, tier3, tier4, tier5, tier6 } = req.body || {};
  console.log(`[AZL] Ingest ${node_id || 'unknown'}:`, { tier1, tier2, tier3, tier4, tier5, tier6 });
  res.json({
    status: 'ok',
    node_id: node_id || null,
    received: 6,
    timestamp: new Date().toISOString()
  });
});

app.options('*', cors());
app.listen(PORT, '0.0.0.0', () => {
  console.log(`PaidingAttention 2.0 AZL backend listening on ${PORT}`);
});
