/*
 * Project XenonCentralBLE
 * Description:
 * Author:
 * Date:
 */

// System mode for better Bluetooth Performance
SYSTEM_MODE(MANUAL);

// List of UUIDs in String format
#define BAT_CHAR_UUID "2A19"
#define VOL_CHAR_UUID "1EDF9A1E-6426-4E87-9C6D-F7518017E1D7"
#define TYPE_CHAR_UUID "1EDF9A1D-6426-4E87-9C6D-F7518017E1D7"

// List of UUIDs
BleUuid sensorCharBat = BleUuid(BAT_CHAR_UUID);
BleUuid sensorCharVol = BleUuid(VOL_CHAR_UUID);
BleUuid sensorCharTyp = BleUuid(TYPE_CHAR_UUID);

// List of characteristics
BleCharacteristic batteryCharacteristic;
BleCharacteristic voltageCharacteristic;
BleCharacteristic typeCharacteristic;

// List of important objects for BLE operations
BleAddress sensorAddress = BleAddress("ff:ff:ff:ff:ff:ff");
BlePeerDevice sensor;

// Logger
SerialLogHandler logHandler(115200, LOG_LEVEL_ERROR, {{"app", LOG_LEVEL_INFO}});

void scanResultCallback(const BleScanResult *scanResult, void *context)
{
  // Filtering using name
  // Check if name matches SensorPad
  String name = scanResult->advertisingData.deviceName();

  char buf[128];
  name.toCharArray(buf, sizeof(buf));

  Log.info("Device name %s", buf);

  // Check if they're equal
  if (name == "SensorPad")
  {

    // Collect the uuids showing in the advertising data
    BleUuid uuids[20];
    int uuidsAvail = scanResult->advertisingData.serviceUUID(uuids, sizeof(uuids) / sizeof(BleUuid));

    // Used to store the string representation of the UUID recieved
    char uuidString[256];

    // Loop over all available UUIDs
    for (int i = 0; i < uuidsAvail; i++)
    {
      uuids[i].toString(uuidString, sizeof(uuidString));
      Log.info("UUID: %s", uuidString);
    }

    // Stop scannning
    BLE.stopScanning();

    // Set address
    sensorAddress = scanResult->address;
  }
}

void onDisconnect(const BlePeerDevice &peer, void *context)
{
  Log.info("Disconnected");

  // Reset address
  sensorAddress = BleAddress("ff:ff:ff:ff:ff:ff");
}

void getBatteryMeasurement()
{
  uint8_t data;
  batteryCharacteristic.getValue(&data);
  Log.info("Battery Percentage: %d", data);
}

void getVoltage()
{
  float data;
  String sensorData = "";
  voltageCharacteristic.getValue(&data);
  sensorData = sensorData + data;

  Log.info(sensorData);
}

void getType()
{
  char data[10];
  String sensorType = "";
  typeCharacteristic.getValue(&data);
  sensorType = sensorType + data;

  Log.info("Sensor Type: " + sensorType);
}

// setup() runs once, when the device is first turned on.
void setup()
{
  // Set disconnect handler
  BLE.onDisconnected(onDisconnect, NULL);
}

// loop() runs over and over again, as quickly as it can execute.
void loop()
{
  // Run the scan
  if (!(sensorAddress == BleAddress("ff:ff:ff:ff:ff:ff")) && !sensor.connected())
  {
    // Connnect!
    sensor = BLE.connect(sensorAddress);
  }
  else if (!sensor.connected())
  {
    // Scan
    BLE.scan(scanResultCallback, NULL);
  }

  // Validate characteristic
  if (sensor.connected() && !batteryCharacteristic.valid())
  {
    Log.info("Gettng battery characteristic.");

    // Get the char
    sensor.getCharacteristicByUUID(batteryCharacteristic, sensorCharBat);

    // Check if it's good!
    if (batteryCharacteristic.valid())
    {
      Log.info("Battery Valid!");
    }
    else
    {
      Log.info("Battery Invalid!");
      return;
    }
  }
  else if (sensor.connected() && batteryCharacteristic.valid())
  {
    getBatteryMeasurement();
  }

  if (sensor.connected() && !voltageCharacteristic.valid())
  {
    Log.info("Gettng voltage characteristic.");

    // Get the char
    sensor.getCharacteristicByUUID(voltageCharacteristic, sensorCharVol);

    // Check if it's good!
    if (voltageCharacteristic.valid())
    {
      Log.info("Voltage Valid!");
    }
    else
    {
      Log.info("Voltage Invalid!");
      return;
    }
  }
  else if (sensor.connected() && voltageCharacteristic.valid())
  {
    getVoltage();
  }

  if (sensor.connected() && !typeCharacteristic.valid())
  {
    Log.info("Gettng type characteristic.");

    // Get the char
    sensor.getCharacteristicByUUID(typeCharacteristic, sensorCharTyp);

    // Check if it's good!
    if (typeCharacteristic.valid())
    {
      Log.info("Type Valid!");
    }
    else
    {
      Log.info("Type Invalid!");
      return;
    }
  }
  else if (sensor.connected() && typeCharacteristic.valid())
  {
    getType();
  }
}