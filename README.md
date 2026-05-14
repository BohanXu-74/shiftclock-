# Seven Segment Clock

![Clock](images/1000004734.jpg)

A 4-digit seven segment clock built with an STC15W204S microcontroller and DS1302 RTC module. Uses shift registers and multiplexing to drive the display with minimal pins. Runs a state machine for time, alarm, and timer modes. Features custom debounce logic, a self-written stc15.h header, and a KiCad PCB design in progress.

---

## Hardware

| Component | Description |
|---|---|
| STC15W204S | Main microcontroller |
| DS1302 | Real-time clock module for timekeeping |
| 4-digit 7-segment display | Output display |
| Shift register | Used to load BCD segment data |
| Push buttons | Mode selection and value adjustment |

---

## How It Works

The MCU constantly refreshes the display by rapidly switching between each of the 4 digits while the shift register loads in the BCD code for the seven segment. The digits switch fast enough that all 4 appear to be on at the same time.

The clock runs on a state machine that cycles through three modes:

- **Time** — displays current time from the DS1302 RTC
- **Alarm** — supports up to 16 individually configurable alarms
- **Timer** — user-configurable countdown timer

Before entering each mode, the display shows the mode name first instead of instantly switching to numbers.

---

## Software

- Written in C
- Custom `stc15.h` header file written from scratch since no existing one was available for this MCU
- Custom button debounce logic to prevent multiple registrations from a single press
- Timing logic and display refresh written manually to eliminate flickering

---

## Files

| File | Description |
|---|---|
| `main.c` | Full source code |
| `clock.kicad_pcb` | KiCad PCB layout |
| `clock-cache.lib` | KiCad component cache |
| `stc15.h` | Custom SFR header for STC15W204S |

---

## Status

Currently working on a prototype breadboard/perf board setup. PCB design in KiCad is in progress. Final goal is a custom PCB version in an enclosure.

---

## Notes

- KiCad version used: v5.1 (older PC limitation)
- Hackaday project page: [DIY Seven Segment Clock](https://hackaday.io/project/205699-diy-seven-segment-clock)
