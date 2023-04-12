import React, { useState } from 'react';
import axios from 'axios';

function App() {
  const [flowSensorData, setFlowSensorData] = useState(null);
  const [pressureSensorData, setPressureSensorData] = useState(null);
  const [valveState, setValveState] = useState(false);

  const getFlowSensorData = async () => {
    const response = await axios.get('/api/get_flow_sensor_data');
    setFlowSensorData(response.data);
  };

  const getPressureSensorData = async () => {
    const response = await axios.get('/api/get_pressure_sensor_data');
    setPressureSensorData(response.data);
  };

  const setValveStateOn = async () => {
    await axios.post('/api/set_valve_state', { state: true });
    setValveState(true);
  };

  const setValveStateOff = async () => {
    await axios.post('/api/set_valve_state', { state: false });
    setValveState(false);
  };

  return (
    <div>
      <h1>Board Control</h1>
      <div>
        <h2>Flow Sensor Data</h2>
        <button onClick={getFlowSensorData}>Get Flow Sensor Data</button>
        <div>{flowSensorData !== null ? flowSensorData : 'No data'}</div>
      </div>
      <div>
        <h2>Pressure Sensor Data</h2>
        <button onClick={getPressureSensorData}>Get Pressure Sensor Data</button>
        <div>{pressureSensorData !== null ? pressureSensorData : 'No data'}</div>
      </div>
      <div>
        <h2>Solenoid Valve State</h2>
        <button onClick={setValveStateOn} disabled={valveState}>Turn On</button>
        <button onClick={setValveStateOff} disabled={!valveState}>Turn Off</button>
        <div>Current state: {valveState ? 'On' : 'Off'}</div>
      </div>
    </div>
  );
}

export default App;
