import React, { useState, useEffect } from 'react';
import { StyleSheet, Text, View, Button, FlatList, TouchableOpacity, PermissionsAndroid, Platform } from 'react-native';
import { BleManager } from 'react-native-ble-plx';
import { encode } from 'base-64';

export const manager = new BleManager();

export default function App() {
  const [isScanning, setIsScanning] = useState(false);
  const [devices, setDevices] = useState([]);
  const [connectedDevice, setConnectedDevice] = useState(null);
  const [logs, setLogs] = useState([]);

  const addLog = (msg) => setLogs(prev => [...prev, msg].slice(-10));

  const requestPermissions = async () => {
    if (Platform.OS === 'android') {
      await PermissionsAndroid.requestMultiple([
        PermissionsAndroid.PERMISSIONS.BLUETOOTH_SCAN,
        PermissionsAndroid.PERMISSIONS.BLUETOOTH_CONNECT,
        PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
      ]);
    }
  };

  useEffect(() => {
    requestPermissions();
    return () => manager.destroy();
  }, []);

  const startScan = () => {
    if (!isScanning) {
      setDevices([]);
      setIsScanning(true);
      addLog('Scanning for SuperBadgeOS...');
      manager.startDeviceScan(null, null, (error, device) => {
        if (error) {
          addLog('Scan error: ' + error.message);
          setIsScanning(false);
          return;
        }
        if (device && device.name === 'SuperBadgeOS') {
          addLog('Found SuperBadgeOS!');
          setDevices(prev => {
            if (!prev.find(d => d.id === device.id)) {
              return [...prev, device];
            }
            return prev;
          });
        }
      });
      setTimeout(() => {
        manager.stopDeviceScan();
        setIsScanning(false);
        addLog('Scan stopped.');
      }, 5000);
    }
  };

  const connectDevice = async (device) => {
    manager.stopDeviceScan();
    setIsScanning(false);
    addLog(`Connecting to ${device.id}...`);
    try {
      const connected = await device.connect();
      addLog('Connected!');
      setConnectedDevice(connected);
      await connected.discoverAllServicesAndCharacteristics();
      addLog('Services discovered.');
    } catch (e) {
      addLog('Connect error: ' + e.message);
    }
  };

  const sendCommand = async (cmd) => {
    if (!connectedDevice) return;
    try {
      const payload = JSON.stringify({ cmd, args: {} }) + "\n";
      const base64Payload = encode(payload);
      await connectedDevice.writeCharacteristicWithResponseForService(
        "6e400001-b5a3-f393-e0a9-e50e24dcca9e",
        "6e400002-b5a3-f393-e0a9-e50e24dcca9e",
        base64Payload
      );
      addLog(`Sent: ${cmd}`);
    } catch (e) {
      addLog('Send error: ' + e.message);
    }
  };

  return (
    <View style={styles.container}>
      <Text style={styles.title}>Super Badge OS Controller</Text>
      
      {!connectedDevice ? (
        <View style={styles.scanContainer}>
          <Button title={isScanning ? "Scanning..." : "Scan for Badge"} onPress={startScan} disabled={isScanning} />
          <FlatList
            data={devices}
            keyExtractor={item => item.id}
            renderItem={({ item }) => (
              <TouchableOpacity style={styles.deviceBtn} onPress={() => connectDevice(item)}>
                <Text style={styles.deviceText}>Connect: {item.name}</Text>
              </TouchableOpacity>
            )}
          />
        </View>
      ) : (
        <View style={styles.controlContainer}>
          <Text style={styles.connectedText}>Connected to Badge!</Text>
          <Button title="Home Screen" onPress={() => sendCommand('welcome')} />
          <View style={{height: 10}} />
          <Button title="Lock Badge" onPress={() => sendCommand('lock')} color="red" />
          <View style={{height: 10}} />
          <Button title="Disconnect" onPress={() => { connectedDevice.cancelConnection(); setConnectedDevice(null); }} />
        </View>
      )}

      <View style={styles.logs}>
        {logs.map((l, i) => <Text key={i} style={styles.logText}>{l}</Text>)}
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
  container: { flex: 1, padding: 40, backgroundColor: '#f5f5f5' },
  title: { fontSize: 24, fontWeight: 'bold', marginBottom: 20, textAlign: 'center' },
  scanContainer: { flex: 1 },
  controlContainer: { flex: 1, justifyContent: 'center' },
  deviceBtn: { backgroundColor: '#ddd', padding: 15, marginVertical: 10, borderRadius: 8 },
  deviceText: { fontSize: 18, textAlign: 'center' },
  connectedText: { fontSize: 20, color: 'green', marginBottom: 20, textAlign: 'center' },
  logs: { height: 150, backgroundColor: '#eee', padding: 10, borderRadius: 8 },
  logText: { fontFamily: 'monospace', fontSize: 12 }
});
