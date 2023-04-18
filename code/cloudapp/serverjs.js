const axios = require('axios');
const express = require('express');
const SerialPort = require('serialport');
const Readline = require('@serialport/parser-readline');

const app = express();
app.use(express.json());

// use the built-in JSON middleware to parse incoming request bodies
app.use(express.json());

// use the built-in urlencoded middleware to parse incoming URL-encoded request bodies
app.use(express.urlencoded({ extended: true }));

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

// define the endpoint for setting the solenoid valve state
app.post('/api/set_valve_state', (req, res) => {
  const state = req.body.state;
  const url = 'http://<pin-ip>/api/set_valve_state';

  // send an HTTP POST request to the slave Mega with the desired valve state
  axios.post(url, { state })
    .then(response => {
      // send a JSON response indicating success
      res.json({ status: 'ok' });
    })
    .catch(error => {
      console.error(error);
      // send a JSON response indicating failure
      res.status(500).json({ status: 'error' });
    });
});

// define the endpoint for fetching flow sensor data
app.get('/api/get_flow_sensor_data', async (req, res) => {
  const url = 'http://<pin-ip>/api/get_flow_sensor_data';

  // send an HTTP GET request to the slave Mega to fetch the flow sensor data
  axios.get(url)
    .then(response => {
      // send the flow sensor data back as a JSON response
      res.json({ flow: response.data });
    })
    .catch(error => {
      console.error(error);
      // send a JSON response indicating failure
      res.status(500).json({ status: 'error' });
    });
});

// define the endpoint for fetching pressure sensor data
app.get('/api/get_pressure_sensor_data', async (req, res) => {
  const url = 'http://<pin-ip>/api/get_pressure_sensor_data';

  // send an HTTP GET request to the slave Mega to fetch the pressure sensor data
  axios.get(url)
    .then(response => {
      // send the pressure sensor data back as a JSON response
      res.json({ pressure: response.data });
    })
    .catch(error => {
      console.error(error);
      // send a JSON response indicating failure
      res.status(500).json({ status: 'error' });
    });
});

// start the server
app.listen(port, () => {
  console.log(`Server running at http://localhost:${port}`);
});
