# ESPHome Vevor Diesel Heater BLE Controller

This project provides an **ESP32-based Bluetooth Low Energy (BLE)
bridge** between a **Vevor diesel heater** and **Home Assistant**.
It enables full control and telemetry via ESPHome while decoding the
proprietary BLE frames emitted by the heater.

This repository contains the ESPHome YAML configuration that handles:

-   BLE connection and automatic polling
-   Full telemetry decoding (temperatures, voltage, altitude, modes, errors, etc.)
-   Power control, mode selection, setpoint control (temperature or level)
-   Publishing all values and states to Home Assistant

------------------------------------------------------------------------

# 🔧 Build & Installation Instructions

## 1. Install ESPHome

You can use: - ESPHome Dashboard
- The `esphome` CLI
- Home Assistant's ESPHome add-on


## 2. Create `secrets.yaml`

A `secrets.example.yaml` file is included. Create your own `secrets.yaml` in the same folder:

``` yaml
mac_address: ""
passkey: "1234"
wifi_ssid: ""
wifi_password: ""
api_encryption_key: "vHF/emJMoAZz3q8k4JySuSZnYEa43IujpwN9+fQkwYk="
ota_password: "09ec3c09b5847b5a4eeef2c15ce8fb00"
```

### Secret values:

| Field                  | Description                                                                                       |
| -----------------------|---------------------------------------------------------------------------------------------------|
| **mac_address**        | Bluetooth MAC address of the heater. You can obtain this using any BLE scanner app on your phone. |
| **passkey**            | The heater passkey set in the Vevor app. Default is **1234** unless changed.                      |
| **wifi_ssid**          | Credentials for the network the ESP32 should join.                                                |
| **wifi_password**      | Credentials for the network the ESP32 should join.                                                | 
| **api_encryption_key** | ESPHome API encryption key used by Home Assistant.                                                |
| **ota_password**       | Password required for over-the-air firmware updates.                                              |

### ⚠️ Important: Change Your API Encryption Key and OTA Password

The values in `secrets.example.yaml` come from the original project this was forked from.
They remain in place for ease of setup, but **should be replaced before deploying your device**.

#### Generate a new API key:

``` bash
openssl rand -base64 32
```

#### Choose any strong OTA password:

``` bash
openssl rand -hex 16
```

Update your `secrets.yaml` accordingly.

## 4. Update the Device Name (Optional)

Inside the YAML:

``` yaml
substitutions:
  name: "diesel-air-heater-ble"
  friendly_name: Diesel Air Heater
```

Modify these if you want your device to appear differently in Home Assistant.

## 5. Compile & Flash

### First-time USB flash:

``` bash
esphome run diesel-air-heater-ble.yaml
```

### After the ESP32 is flashed once, all future updates can be done **OTA** via WIFI:
``` bash
esphome clean diesel-air-heater-ble.yaml
esphome compile diesel-air-heater-ble.yaml
esphome upload diesel-air-heater-ble.yaml
```
_select the OTA option at the prompt_


# 📡 Features & Home Assistant Integration

This ESPHome configuration performs **bidirectional BLE communication** with the heater.

## ✓ Controls Available

### Power & Operation
| Control                     | Type     | Description                                     |
|-----------------------------|----------|-------------------------------------------------|
| **Power**                   | Switch   | Turn heater on/off                              |
| **Mode**                    | Select   | Level mode or Automatic (temperature) mode      |
| **Level**                   | Number   | Sets output level 1-10 (Level mode)             |
| **Temperature**             | Number   | Sets target temperature 8-36°C (Automatic mode) |

### Settings & Configuration
| Control                     | Type     | Description                                     |
|-----------------------------|----------|-------------------------------------------------|
| **Temperature Unit**        | Select   | Choose Celsius or Fahrenheit display            |
| **Language**                | Select   | Set display language (English, 简体中文, Русский, Disabled, Deutsch) |
| **Altitude Unit**           | Select   | Choose meters, feet, or kilometers              |
| **Temperature Compensation**| Number   | Fine-tune temperature readings (-10 to +10)     |
| **Display Brightness**      | Number   | Adjust screen brightness (0-5)                  |
| **Set Time**                | Button   | Sync heater clock with Home Assistant time      |

### Auto-On Scheduling
| Control                     | Type     | Description                                     |
|-----------------------------|----------|-------------------------------------------------|
| **Auto On Status**          | Switch   | Enable/disable scheduled auto-start             |
| **Auto On Start Time**      | Time     | Set time for automatic heater start             |
| **Auto On Run Time**        | Number   | Duration heater runs during auto-on (minutes)   |

# 📊 Telemetry & Sensors (Published to Home Assistant)

### Status Sensors

| Sensor                 | Type          | Description                                    |
|------------------------|---------------|------------------------------------------------|
| **Power**              | Binary Sensor | Whether heater is currently running            |
| **Mode**               | Text Sensor   | Current mode: "Level" or "Automatic"           |
| **Glow Plug Status**   | Text Sensor   | Operating state (Heating, Running, Cooling Down, Idle) |
| **Error Code**         | Sensor        | Current error code (0 = no error)              |

### Environmental Sensors

| Sensor                 | Unit | Description                                    |
|------------------------|------|------------------------------------------------|
| **Room Temperature**   | °C   | Ambient/cabin temperature                      |
| **Core Temperature**   | °C   | Heater core/exhaust temperature                |
| **Battery Voltage**    | V    | Supply voltage from vehicle battery            |
| **Altitude**           | m    | Current altitude setting                       |

### System Sensors

| Sensor                 | Description                                    |
|------------------------|------------------------------------------------|
| **ESP Uptime**         | Time since ESP32 last boot                     |
| **ESPHome Version**    | Installed ESPHome firmware version             |
| **WiFi Signal**        | WiFi signal strength (RSSI)                    |
| **IP Address**         | Current IP address on network                  |
| **Connected SSID**     | Name of WiFi network                           |

# 🙏 Credits & Related Projects

Huge thanks to the pioneers who decoded the heater protocol for
older models:

-   https://github.com/spin877/Bruciatore_BLE
-   https://github.com/Knutnoh/Bruciatore_BLE
-   https://community.home-assistant.io/t/vevor-diesel-heater-control-development-in-progress/832159/14
