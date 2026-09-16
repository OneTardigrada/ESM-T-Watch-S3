# ESM Smartwatch Data Loader

![Platform](https://img.shields.io/badge/platform-Web-blue?style=flat-square)
![Browser](https://img.shields.io/badge/browser-Chrome%20%7C%20Edge-4285F4?style=flat-square&logo=googlechrome&logoColor=white)
![Web Serial](https://img.shields.io/badge/Web%20Serial-API-FF6F00?style=flat-square)
![Hardware](https://img.shields.io/badge/hardware-LilyGo%20T--Watch%20S3-1565C0?style=flat-square)
![No Install](https://img.shields.io/badge/install-none-success?style=flat-square)
![Offline](https://img.shields.io/badge/offline-100%25-brightgreen?style=flat-square)
![License](https://img.shields.io/badge/license-MIT-lightgrey?style=flat-square)

Browser-based configuration and data export tool for the ESM smartwatch (LilyGo T-Watch S3). Runs fully locally in the browser, no installation required.

## Features

- **Create & transfer configuration**: edit questionnaires, items, time windows and notifications via UI and push them to the watch.
- **Export responses**: download collected CSV data from the watch and save it locally.
- **Sync time**: set the watch's RTC from the browser's current time.
- **Self-test**: trigger the watch's hardware diagnostics from the browser.

## Requirements

- **Chrome** or **Edge** (uses the Web Serial API; Firefox and Safari are not supported).
- USB cable to the watch.
- Open the page via `file://` or `https://` (Web Serial does not work over plain `http://`).

## Usage

1. Open `index.html` in your browser (a double-click is enough).
2. Connect the watch via USB and power it on.
3. Click **"Connect"**, select the watch's serial port, and the connection is established at 115200 baud.
4. Choose the desired action (load configuration, export responses, set time, ...).

## File overview

| File | Purpose |
|------|---------|
| `index.html` | Main application (single self-contained HTML/CSS/JS file). |
| `sample_config.json` | Example configuration to use as a template. |

## Notes

- No data is ever sent to a server. Everything runs in the browser.
- If the connection drops, briefly unplug and reconnect the watch, then connect again.
