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
| **RANDOM** | Changes the step advance behavior. When enabled, each trigger advances the sequence to a **random step** instead of moving to the next step in order. |

---

### Step controls

Each row has a fixed odd length and its own per-step voltage knobs:

- **Row 1:** 3 steps  
- **Row 2:** 5 steps  
- **Row 3:** 7 steps  

Step LEDs indicate the currently active position in each row.

---

## Inputs

- **TRIGGERS (3x)** — One trigger input per row. Each rising edge advances that row.  
  - **RANDOM off:** steps advance sequentially.  
  - **RANDOM on:** the next step is chosen randomly within the row length.

Rows run fully independently.

---

## Outputs

- **SEQUENCES (3x)** — Stepped CV output for each row, reflecting the voltage of the active step.

---

## Behavior Notes

- Each row advances only on its own trigger input.
- Sequences loop continuously within their fixed odd length.
- All step voltages are scaled by the global **SPREAD** control.
- **RANDOM** affects step order only; step voltages remain unchanged.
- No reset input: sequences are intended to free-run and naturally phase against each other.

---

## Typical Use

1. Patch clocks or triggers into each **TRIGGERS** input.
2. Patch **SEQUENCES** outputs to pitch, filter cutoff, or modulation destinations.
3. Set per-step voltages with the knobs.
4. Adjust **SPREAD** to define the musical range.
5. Enable **RANDOM** to introduce non-linear step movement.
6. Let the odd step lengths phase over time for evolving patterns.

---

## Notes

- Fixed odd step lengths are intentional and non-configurable.
- RANDOM introduces variation through step order, not voltage randomization.
- Designed for musical drift rather than bar-aligned sequencing.
- Pairs especially well with even-step sequencers for long-form variation.

---

[⬅ Back to Module Index](../README.md)

