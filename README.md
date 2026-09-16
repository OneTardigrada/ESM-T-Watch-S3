# ESM-T-Watch-S3

A firmware to facilitate studies using in-situ self-reports (Experience Sampling
Method, ESM) on the [LilyGo T-Watch S3](https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library).

This firmware lets researchers define questionnaires and notification schedules
through a simple JSON configuration. The watch runs fully offline — Wi-Fi and
Bluetooth stay disabled during studies. Configuration is transferred and
collected responses are exported over USB using a browser-based tool (no app
installation and no network connection required). Responses and logs are stored
on the watch's internal flash (LittleFS) and can be exported as CSV.

This project is a port and further development of the original
[ESM Firmware for the T-Watch 2020 V2](https://github.com/KL-Psychological-Methodology/TWatch-2020-ESM)
for the newer ESP32-S3 based T-Watch S3 hardware.

## Questionnaire Features

Multiple questionnaires can be defined, each with multiple items. Questionnaires
can be set to always be selectable by participants (event-based, e.g., to be
filled out in response to a real-life event) or made accessible via scheduled
notifications (see Notification Features).

Each item has an item text, a type-specific input, and a Continue button that
advances the questionnaire (or completes it on the last item). The following
item types are available:

- **Likert Scale** — a scale with a configurable number of points and text
  labels at both ends.
- **Visual Analogue Scale (VAS)** — a continuous slider with text labels at both
  ends.
- **Options** — a single choice from a scrolling list. A multi-select variant
  (`multi_options`) is also available.
- **Numeric** — numeric input with a configurable value range.
- **Text** — a pseudo-item without input, used to display additional context for
  the following items.

## Notification Features

Multiple notifications can be defined per questionnaire. A triggered notification
wakes the device, alerts the participant via the vibration motor, and shows a
message that lets the participant proceed to the associated questionnaire. When
defining notification triggers, the following options are available:

- **Scheduled times** — notifications trigger at defined times of day.
- **Expiry time** — if the watch is woken before an unanswered notification has
  expired, it is displayed again (useful if the participant did not notice it
  right away or could not respond immediately).
- **Reminder** — unanswered notifications can be re-elicited after they would
  otherwise expire.

## Configuration & Data Transfer (Data Loader)

Instead of a micro-SD card, this firmware uses a browser-based **Data Loader**
that communicates with the watch over USB via the Web Serial API. It runs fully
locally in the browser (Chrome or Edge), with no installation and no server.

With the Data Loader you can:

- create and push the study configuration to the watch,
- export collected responses as CSV,
- set the watch's real-time clock from the browser,
- trigger the on-device hardware self-test.

The tool is maintained as a separate project — see the
[LilyGo-Dataloader](https://github.com/OneTardigrada/LilyGo-Dataloader) repository
for the application, usage instructions, and an example configuration.

## Privacy

The device does not connect to any network during studies (Wi-Fi and Bluetooth
are disabled). All configuration and data transfer happens locally over USB, and
no data is ever sent to a server.

## Building & Flashing

The project is built with [PlatformIO](https://platformio.org/) using the Arduino
framework (via the pioarduino ESP32 platform). A custom board definition for the
T-Watch S3 is included in [`boards/`](boards/).

```sh
# Build
pio run -e twatch-s3

# Build and upload to the watch
pio run -e twatch-s3 -t upload

# Serial monitor
pio device monitor
```

## Acknowledgements

Based on the [ESM Firmware for the T-Watch 2020 V2](https://github.com/KL-Psychological-Methodology/TWatch-2020-ESM)
by the KL Psychological Methodology group, described in:

> Volsa, S., Batinic, B., & Stieger, S. (2022). Self-Reports in the Field Using
> Smartwatches: An Open-Source Firmware Solution. *Sensors, 22*(5), 1980.
> https://doi.org/10.3390/s22051980

## License

Released under the [MIT License](LICENSE).
