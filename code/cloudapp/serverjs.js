const express = require('express');
const SerialPort = require('serialport');
const Readline = require('@serialport/parser-readline');

const app = express();
app.use(express.json());

const port = 3001;
const arduinoPort = '/dev/ttyACM0'; // Replace with your Arduino's serial port

const serialPort = new SerialPort(arduinoPort, { baudRate: 9600 });
const parser = serialPort.pipe(new Readline({ delimiter: '\n' }));

// API endpoint to fetch flow sensor data
app.get('/api/get_flow_sensor_data', async (req, res) => {
  // Send command to Arduino to read flow sensor data
  serialPort.write('READ_FLOW_SENSOR\n');
  
  // Wait for Arduino to respond with the data
  parser.once('data', (data) => {
    res.json(Number(data));
  });
});

// API endpoint to fetch pressure sensor data
app.get('/api/get_pressure_sensor_data', async (req, res) => {
  // Send command to Arduino to read pressure sensor data
  serialPort.write('READ_PRESSURE_SENSOR\n');
  
  // Wait for Arduino to respond with the data
  parser.once('data', (data) => {
    res.json(Number(data));
  });
});

// API endpoint to set the solenoid valve state
app.post('/api/set_valve_state', (req, res) => {
  const state = req.body.state;
  const command = state ? 'VALVE_ON\n' : 'VALVE_OFF\n';

  // Send command to Arduino to set the solenoid valve state
  serialPort.write(command);
  res.json({ status: 'ok' });
});

app.listen(port, () => {
  console.log(`Server running at http://localhost:${port}`);
});
