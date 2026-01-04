# TL-Odd5

<img width="361" height="758" alt="Captura de pantalla 2025-12-28 205419" src="https://github.com/user-attachments/assets/223f5412-6eda-4add-8398-42c21999d3ec" />

A **triple odd-step voltage sequencer** for VCV Rack 2.  
TL-Odd5 runs **three independent sequences of fixed odd lengths (3, 5, and 7 steps)**, designed to generate evolving, non-repeating patterns for minimal and hypnotic techno.

Each row outputs a stepped CV sequence driven by its own trigger input, making TL-Odd5 suitable for melodic sequencing, modulation, and polyrhythmic control.

---

## Controls

### Global

| Control | Description |
|--------|-------------|
| **SPREAD** | Global voltage spread. Scales the range of all step voltages simultaneously, from subtle modulation to wide melodic movement. |
| **RANDOM** | Enables controlled random variation on step voltages while preserving the overall structure defined by SPREAD and the knob settings. |

---

### Step controls

Each row has a fixed odd length and its own per-step voltage knobs:

- **Row 1:** 3 steps  
- **Row 2:** 5 steps  
- **Row 3:** 7 steps  

Step LEDs indicate the currently active position in each row.

---

## Inputs

- **TRIGGERS (3x)** — One trigger input per row. Each rising edge advances that row by one step. Rows run fully independently.

---

## Outputs

- **SEQUENCES (3x)** — Stepped CV output for each row, reflecting the voltage of the active step.

---

## Behavior Notes

- Each row advances only on its own trigger input.
- Sequences loop continuously within their fixed odd length.
- All step voltages are scaled by the global **SPREAD** control.
- **RANDOM** introduces variation without breaking the underlying pattern structure.
- No reset input: sequences are intended to free-run and naturally phase against each other.

---

## Typical Use

1. Patch clocks or triggers into each **TRIGGERS** input.
2. Patch **SEQUENCES** outputs to pitch, filter cutoff, or modulation destinations.
3. Set per-step voltages with the knobs.
4. Adjust **SPREAD** to define the musical range.
5. Enable **RANDOM** for subtle live variation.
6. Let the odd step lengths phase over time for evolving patterns.

---

## Notes

- Fixed odd step lengths are intentional and non-configurable.
- Designed for musical drift rather than bar-aligned sequencing.
- Pairs especially well with even-step sequencers for long-form variation.

---

[⬅ Back to Module Index](../README.md)
