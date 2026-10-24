# Wireless Smart Night Light System

A smart, motion-sensing LED night light that runs straight from the mains, keeps working through power cuts on its own battery, and is controlled by a paired wireless remote.

I designed and built both custom PCBs end to end: schematic, PCB layout, power design, component selection and firmware for **both** the light and the remote.



![Fabricated main PCB, front view](Images/Frontt%20of%20PCB%20%28Fabricated%29.jpeg)

## Highlights

- **Mains to light, all on one board:** 100 to 240 VAC input, isolated 5 V conversion, battery charging, 3.3 V logic, RF, sensing and a six-LED lighting array.
- **Seamless battery backup:** an 18650 Li-ion cell takes over automatically during a power outage, designed for about 24 hours of operation.
- **Smart behaviour:** the light turns on only when it is dark **and** someone moves, then stays on while motion continues.
- **Secure pairing:** a remote pairs with the light using a random 32-bit token, then talks to it on a private radio address that only that light listens to.
- **Hardware-level battery protection:** a nanopower comparator with hysteresis shuts the 3.3 V rail down at low battery, without depending on firmware.
- **Battery-friendly remote:** the remote sleeps between button presses and wakes on demand, so a small battery goes a long way.

## Hardware

### Main unit (the night light)

| Block | Part | Role |
| --- | --- | --- |
| Mains conversion | HLK-5M05 | Isolated 100 to 240 VAC to 5 V module, 5 W |
| Charger and power path | BQ24074RGT | Charges the battery, powers the system from the supply, switches to battery during an outage |
| Battery | 18650 Li-ion, 3000 mAh | Backup energy storage |
| Low-battery switching | TLV3012 comparator + AO3400A + AO3407A | Hardware cutoff with hysteresis |
| 3.3 V regulator | TLV75533 | Clean rail for MCU, RF and PIR |
| Microcontroller | STM32G031K8T6 | State machine, RF, sensing, LED control |
| Wireless | nRF24L01+ | 2.4 GHz link to the remote |
| Motion sensing | BISS0001 + PIR element | Presence detection |
| Light sensing | GL5528 LDR | Day/night detection via the ADC |
| Lighting | 6x Samsung LM281B+ (SPMWH1228MD5WATUVK) | Illumination, driven through an AO3400A MOSFET |
| Programming | SWD Tag-Connect | Flash and debug the STM32 |

![Main PCB layout](Images/Main%20PCB%20layout.png)

### Wireless remote

| Block | Part | Role |
| --- | --- | --- |
| Microcontroller | ATtiny1616 | Remote controller |
| Wireless | nRF24L01+ | 2.4 GHz link to the light |
| Inputs | 3 buttons | Power, Sensor On, Sensor Off |
| Power | 2x AAA | Direct 3 V operation, no regulator needed |

![Remote PCB layout](Images/Remote%20PCB%20layout.png)

### Board views

| | Design | Fabricated |
|---|---|---|
| **Front** | ![Front of the PCB, design](Images/Front%20of%20PCB.jpeg) | ![Front of the fabricated PCB](Images/Frontt%20of%20PCB%20%28Fabricated%29.jpeg) |
| **Back** | ![Back of the PCB, design](Images/Back%20of%20PCB.jpeg) | ![Back of the fabricated PCB](Images/Back%20of%20PCB%20%28Fabricated%29.jpeg) |

### Schematic

[![Schematic](Images/Night%20light%20PCB.svg)](Images/Night%20light%20PCB.svg)

## Engineering decisions

### Charger, power path and input limiting

The BQ24074 is set for a **500 mA** charge current (1.8 k ISET resistor) and a **600 mA input current limit** (2.7 k ILIM resistor). The input limit is deliberate: it keeps total draw within what the HLK-5M05 can supply and prevents the module from overheating inside a compact enclosure.

### Low-battery cutoff in hardware, with hysteresis

A TLV3012 nanopower comparator with a built-in reference watches the battery through a 174 k / 100 k divider. A 1 M feedback resistor adds **hysteresis**, so the circuit turns the system **off at about 3.4 V** and only turns it **back on at about 3.6 V**. The comparator drives an N-MOSFET (AO3400A) that controls a high-side P-MOSFET (AO3407A) feeding the 3.3 V regulator. The result is a clean shutdown with no oscillation, and no dependence on firmware to protect the battery.

### Low-noise LED drive

Dedicated constant-current ICs (such as NSI45020 and AL5809) were evaluated and rejected: they need voltage headroom that is not available as the battery falls from 4.2 V towards 3.4 V. Instead, the six LEDs each have a 33 ohm series resistor and are switched through a MOSFET. It works across the full battery range, adds no switching noise and keeps the part count low.

### A 3.3 V rail chosen for the radio

The TLV75533 was picked for its fast transient response, which handles the current spikes of the nRF24L01+, and for its low output noise, which keeps the PIR and RF stable.

### Isolation from the mains

Mains conversion is handled by a pre-built isolated AC-DC module, so everything downstream is low voltage. It also keeps the design simpler and lowers certification risk compared with a discrete flyback.

### Battery connector protection

A P-MOSFET (AO3407A) sits in series with the battery connector, which provides reverse-polarity protection.

### Light sensor is read only while the light is off

If the LDR were read while the LEDs are on, the light would be measuring its own output and the system would flip-flop between states. The firmware therefore reads the LDR only when the light is off, and waits for it to settle after the light switches off before sampling.

## Design evolution

I started this project with a written system architecture, and the final board differs from it in three places. I made each change deliberately during development.

| Area | Initial architecture | Final design | Why it changed |
| --- | --- | --- | --- |
| PIR front-end | NCS36000 | BISS0001 | The original chip could not be sourced, so I found a suitable replacement and redesigned the sensor circuit around it to keep the build moving without compromising the design. |
| Remote buttons | 2 buttons | 3 buttons (Power, Sensor On, Sensor Off) | Improved during development. Separate Sensor On and Sensor Off buttons make the remote clearer and more direct to use, while Power handles toggling, pairing and reset. |
| Pairing | Remote sends a unique ID, which the light stores | Random 32-bit token, stored with the remote's ID, then a private radio address | Chosen for security and for a better experience for users and engineers (see below). |

### Why pairing was redesigned

The first plan relied on a fixed unique ID from the remote. That works, but it leaves a reusable identifier that never changes, and it keeps all traffic on one shared address where neighbouring units can interfere with each other.

The final scheme is more robust:

- **A fresh secret for every pairing.** The light generates a 32-bit token each time a remote is paired, so there is no fixed identifier to copy or reuse. Every command afterwards must carry that token, and the light ignores anything that does not match.
- **A private address after pairing.** Once paired, commands travel on a private radio address that belongs to that remote and that only its light listens to. The shared public address is used only for pairing and factory reset, which keeps everyday traffic off it and reduces interference between nearby units.
- **A short, clear pairing window.** Pairing is only open for 60 seconds after power-up, with LED feedback at every step, so users and installers always know what state the light is in.
- **A safe way out.** A 6 second factory reset clears the stored pairing, so a lost or replaced remote never leaves the light locked to a remote nobody has.

## How it behaves

### Light modes

| Mode | What the light does |
| --- | --- |
| Smart Idle | Light is off. The LDR is read to check if the room is dark. If it is dark and the PIR sees motion, the light turns on and moves to Smart Active. If the room is bright, motion is ignored. |
| Smart Active | Light is on. The LDR is no longer read, so room lights or sunrise do not switch it off. Every movement restarts a 60 second countdown, and when 60 seconds pass with no motion the light turns off and returns to Smart Idle. |
| Forced On | Power pressed while sensors are enabled. The light turns on and stays on. Motion and the LDR are ignored and the 60 second timer is paused. |
| Manual Mode | Sensor Off pressed. Both sensors are disabled and the light is a plain remote-controlled lamp: Power turns it on, Power again turns it off. |

### Remote buttons

| Button | Action |
| --- | --- |
| Power | Tap: toggle the light. Hold 3 seconds: start pairing. Hold 6 seconds: factory reset. |
| Sensor On | Tap: re-enable the motion and light sensors. |
| Sensor Off | Tap: disable the sensors and switch to manual mode. |

### Pairing, step by step

1. On first power-up the light enters pairing mode for up to 60 seconds. Its LED blinks slowly (1 second on, 2 seconds off) while it listens on the public address.
2. On the remote I hold Power for 3 seconds. The remote sends a pairing request and blinks its LED (1 second on, 2 seconds off) while it waits.
3. The light generates a random 32-bit token, stores it with the remote's address in permanent memory and replies with a confirmation.
4. The remote receives the confirmation, blinks twice quickly and switches to its private address. The light confirms with a long double blink (3 seconds on, 2 off, 3 on), and normal operation starts.
5. If nobody pairs within 60 seconds, the light stops blinking and runs in smart (sensor) mode. Until a remote is paired, it cannot be switched on or off or have its sensor mode changed.

To pair a new remote later, for example after losing the first, I hold Power for 6 seconds. This sends a factory reset, which clears the stored pairing; the light blinks 6 times and goes back into pairing mode. Factory reset is the only command the light accepts on the public address after pairing.

### LED feedback

The light's LED only blinks to report:

- Slow blink (1 second on, 2 seconds off): pairing mode, waiting for a remote.
- Long double blink (3 seconds on, 2 off, 3 on): paired successfully.
- One short flash: sensors turned on.
- Two short flashes: sensors turned off.

The remote LED responds to every button press: one blink means the light received the command, three fast blinks mean it failed.

### Remote power saving

The remote does not run a loop. It sleeps fully between presses, drawing under a microamp, and wakes in microseconds when any button is pressed. It then sends its command, waits for the acknowledgement, blinks its LED and goes straight back to sleep, so two AAA cells go a long way.

## Firmware and radio link

Both units run PlatformIO projects using the Arduino framework with the RF24 library for the nRF24L01+. The firmware is layered so that hardware drivers, the radio, pairing and the application logic are kept in separate modules.

### Radio settings

| Setting | Value |
| --- | --- |
| Frequency channel | 76 (2.476 GHz) |
| Data rate | 250 kbps |
| Transmit power | PA_LOW |
| Auto-retry | up to 15 retries (delay setting 5) |
| Public address (pipe 0) | `E7:E7:E7:E7:E7`, used only for pairing and factory reset |
| Private address (pipe 1) | the remote's own unique 5-byte address, assigned at pairing |

250 kbps gives the best receiver sensitivity and range on the nRF24L01+, and low transmit power keeps current draw down on the remote's two AAA cells.

### Packet format

Every message is a fixed 11 byte packet:

```c
struct Packet {
    uint8_t  cmd;            // command code
    uint32_t token;          // pairing token
    uint8_t  remote_addr[5]; // remote's private address
    uint8_t  nonce;          // per-message value
};
```

| Command | Code | Sent on |
| --- | --- | --- |
| CMD_PAIR | 0x01 | public address |
| CMD_PAIR_ACK | 0x02 | public address |
| CMD_FACTORY_RESET | 0x03 | public address |
| CMD_POWER_ON | 0x10 | private address |
| CMD_POWER_OFF | 0x11 | private address |
| CMD_SENSOR_ON | 0x12 | private address |
| CMD_SENSOR_OFF | 0x13 | private address |

### How the light handles a packet

- On the public address (pipe 0), a paired light accepts only a factory reset.
- On the private address (pipe 1), the light checks the token first. A packet with a wrong token is dropped; a valid one goes to the light controller, which acts on the command.
- The pairing token is built from the lowest bit of 32 LDR ADC readings, mixed with the system timer, so it does not depend on a fixed seed. It is stored in EEPROM so pairing survives power cuts and battery loss.
- The bulb runs the independent watchdog (IWDG) so that a stuck firmware loop resets itself.

## Manufacturing

BOM, Gerber and pick-and-place files for JLCPCB / LCSC.

## Repository layout

```text
.
├── README.md
├── Night light PCB.kicad_pro      # KiCad project
├── Night light PCB.kicad_sch      # schematic
├── Night light PCB.kicad_pcb      # PCB layout
├── fp-lib-table                   # footprint library paths
├── sym-lib-table                  # symbol library paths
├── Footprint/                     # custom footprint libraries
├── Symbol/                        # custom symbol libraries
├── 3D Model/                      # 3D models used by the footprints
├── Night light PCB (Kicad and production files)/
│   └── Design filess/
│       ├── Night light PCB 3D model/
│       │   ├── Night Light board.step
│       │   ├── Night Light remote board.step
│       │   └── Night light PCB.step
│       ├── production/            # JLCPCB manufacturing outputs
│       │   ├── backups/
│       │   ├── bom.csv
│       │   ├── designators.csv
│       │   ├── netlist.ipc
│       │   ├── Night_light_PCB.zip
│       │   └── positions.csv
│       ├── Night light PCB BOM .csv
│       ├── Night light PCB-F_Fab.svg
│       ├── Night light PCB-NPTH-drl_map.pdf
│       ├── Night light PCB-PTH-drl_map.pdf
│       └── Night light Schematics.pdf
├── Firmware source file/          # firmware source projects
├── firmwares/                     # firmware builds
├── Documents/                     # project documents
└── Images/                        # photos, layouts and schematic used in this README
```

## Author

Hakeem Oyebisi · [oyebisihakeem@gmail.com](mailto:oyebisihakeem@gmail.com) · [GitHub](https://github.com/oyeniyihakeem08-blip)
