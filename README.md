# ESP32 Smart Home Automation_5_Channel_Relays

ESP32-based smart home automation system using ESP RainMaker. Control lights and fans via Google Home, IR remote, and manual switches with DHT11 temperature/humidity monitoring.

## Features

### Multiple Control Methods
- **Google Home Integration**: Voice control through Google Assistant via ESP RainMaker
- **IR Remote Control**: Control devices using any IR remote (configurable hex codes)
- **Manual Switches**: Physical push button switches for direct control
- **Mobile App**: Control via ESP RainMaker mobile app (iOS/Android)

### Devices Controlled
- **2 Light Switches**: Independent control of two lighting circuits
- **Fan with 4-Speed Control**: 
  - Speed 0: Off
  - Speed 1-4: Variable speed levels
  - Smooth speed transitions

### Monitoring
- **Temperature Sensor**: Real-time temperature monitoring via DHT11
- **Humidity Sensor**: Real-time humidity monitoring via DHT11
- Sensor data updates every 30 seconds
- Values accessible through ESP RainMaker app

### Smart Features
- **State Persistence**: Device states saved in NVS memory (survives power loss)
- **Wi-Fi Provisioning**: Easy setup via BLE/SoftAP
- **OTA Updates**: Over-the-air firmware updates
- **WiFi Reset**: Long press BOOT button (3-10 seconds)
- **Factory Reset**: Very long press BOOT button (>10 seconds)

## Hardware Requirements

### Components
- **ESP32 Development Board** (any variant)
- **DHT11 Temperature & Humidity Sensor**
- **IR Receiver Module** (TSOP1738 or similar)
- **3x Relay Modules** for fan speed control
- **2x Relay Modules** for lights
- **4x Push Button Switches**
- **IR Remote** (any universal remote)
- **Power Supply** (5V for ESP32, appropriate voltage for relays)

## Pin Configuration

### ESP32 Pin Connections

#### Relay Outputs
| Device | ESP32 Pin | Function |
|--------|-----------|----------|
| Light 1 Relay | GPIO 23 (D23) | Light 1 Control |
| Light 2 Relay | GPIO 22 (D22) | Light 2 Control |
| Fan Relay 1 | GPIO 21 (D18) | Fan Speed Bit 0 |
| Fan Relay 2 | GPIO 19 (D5) | Fan Speed Bit 1 |
| Fan Relay 3 | GPIO 18 (D25) | Fan Speed Bit 2 |

#### Input Switches
| Switch | ESP32 Pin | Function |
|--------|-----------|----------|
| Switch 1 | GPIO 13 (D13) | Light 1 Control |
| Switch 2 | GPIO 12 (D12) | Light 2 Control |
| Fan Up | GPIO 33 (D33) | Increase Fan Speed |
| Fan Down | GPIO 32 (D32) | Decrease Fan Speed |

#### Sensors & Indicators
| Component | ESP32 Pin | Function |
|-----------|-----------|----------|
| DHT11 | GPIO 16 (RX2) | Temperature/Humidity |
| IR Receiver | GPIO 35 (D35) | IR Signal Input |
| WiFi LED | GPIO 2 (D2) | WiFi Status Indicator |
| Reset Button | GPIO 0 (BOOT) | WiFi/Factory Reset |

## Wiring Diagram

### Light Relays
```
ESP32 GPIO 23 → Relay 1 IN → Light 1
ESP32 GPIO 22 → Relay 2 IN → Light 2

Relay VCC → 5V
Relay GND → GND
Relay COM → AC Live/DC+
Relay NO → Light
```

### Fan Speed Control (3 Relay Configuration)
```
Fan Speed Truth Table:
Relay1 | Relay2 | Relay3 | Speed
  OFF  |  OFF   |  OFF   |  0 (OFF)
  ON   |  OFF   |  OFF   |  1
  OFF  |  ON    |  OFF   |  2
  ON   |  ON    |  OFF   |  3
  OFF  |  OFF   |  ON    |  4 (MAX)
```

### DHT11 Sensor
```
DHT11 VCC → 3.3V
DHT11 DATA → GPIO 16
DHT11 GND → GND
```

### IR Receiver
```
TSOP1738 VCC → 3.3V
TSOP1738 OUT → GPIO 35
TSOP1738 GND → GND
```

### Push Buttons
```
All buttons: One side to GPIO, other side to GND
(Internal pull-up resistors are enabled in code)
```

## Software Setup

### Required Libraries
Install these libraries via Arduino IDE Library Manager:
```
- ESP32 (by Espressif)  version 2.0.5
- IRremote (by shirriff)  version 3.6.1
- DHT sensor library (by Adafruit)  version 1.4.4
- SimpleTimer  version 1.0.0
- Preferences (built-in)  version 2.2.2
- AceButton  version 1.9.2
```

### Configuration Steps

1. **Clone or Download this repository**

2. **Update IR Remote Codes**
   - Open `esp_rainmaker.ino`
   - Find the IR button definitions:
   ```cpp
   #define IRButton1   0x1FEC13E  // Update with your remote's hex code
   #define IRButton2   0x1FECE31
   #define IRFanUp     0x1FE1CE3
   #define IRFanDown   0x1FE02FD
   #define IRAllOn     0x1FE12ED
   #define IRAllOff    0x1FE817E
   ```
   - Use the IRremote library examples to read your remote's codes

3. **Customize Device Names** (Optional)
   ```cpp
   char nodeName[] = "Your-Smart-Home";  // Node name in app
   char deviceName_1[] = "Living Room Light";
   char deviceName_2[] = "Bedroom Light";
   char deviceName_3[] = "Ceiling Fan";
   ```

4. **Update WiFi Provisioning Credentials**
   ```cpp
   const char *service_name = "YOUR_DEVICE_NAME";
   const char *pop = "YOUR_PASSWORD";  // Proof of Possession
   ```

5. **Upload the Code**
   - Select your ESP32 board in Arduino IDE
   - Select the correct COM port
   - Upload the sketch

## ESP RainMaker Setup

### 1. Download ESP RainMaker App
- **iOS**: [App Store Link](https://apps.apple.com/app/esp-rainmaker/id1497491540)
- **Android**: [Play Store Link](https://play.google.com/store/apps/details?id=com.espressif.rainmaker)

### 2. Provisioning Process
1. Open the ESP RainMaker app
2. Create an account or sign in
3. Tap "Add Device"
4. Scan the QR code displayed in Serial Monitor
   - Or enter the details manually
5. Connect to your WiFi network
6. Device will appear in your dashboard

### 3. Google Home Integration
1. Open Google Home app
2. Tap "+" → "Set up device" → "Works with Google"
3. Search for "ESP RainMaker"
4. Sign in with your ESP RainMaker credentials
5. Your devices will appear in Google Home
6. Control with voice: "Hey Google, turn on Living Room Light"

## Usage

### Voice Control Examples
```
"Hey Google, turn on the lights"
"Hey Google, turn off Bedroom Light"
"Hey Google, set Ceiling Fan to 75%"  (Speed 3)
"Hey Google, turn off all devices"
```

### IR Remote
- Program your IR remote buttons as per the hex codes
- Dedicated buttons for:
  - Light 1 toggle
  - Light 2 toggle
  - Fan speed up
  - Fan speed down
  - All devices ON
  - All devices OFF

### Manual Switches
- **Light Switches**: Press and hold for ON, release for OFF
- **Fan Speed**: 
  - Up button: Increase speed (0→1→2→3→4)
  - Down button: Decrease speed (4→3→2→1→0)

### Reset Functions
- **WiFi Reset** (3-10 second press on BOOT button):
  - Clears WiFi credentials
  - Device enters provisioning mode
  - Re-configure via app

- **Factory Reset** (>10 second press on BOOT button):
  - Clears all data
  - Device removed from RainMaker account
  - Complete re-setup required

## License
This project is open-source. Feel free to modify and distribute.

## Author
Developed by Nafizur Nayem
