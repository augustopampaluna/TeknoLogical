# TL-Bool

<img width="212" height="900" alt="Captura de pantalla 2026-01-04 133232" src="https://github.com/user-attachments/assets/f8840884-8ec7-4ef3-b9bc-09f06b081e52" />


A **dual-input logic gate processor with probability control** for VCV Rack 2.  
TL-Bool takes two gate inputs (**A** and **B**) and computes multiple logical combinations simultaneously, with an optional **probability stage** to introduce controlled rhythmic variation.

It is designed for trigger and gate processing in live techno contexts, where strict logic can be softened without becoming unpredictable.

---

## Overview

TL-Bool outputs the six most common Boolean logic functions at the same time:  
**AND, NAND, OR, NOR, XOR, XNOR**.

Each output can be used independently. A global **PROBABILITY** control optionally drops triggers in a musically consistent way, making rigid patterns breathe without losing structure.

---

## Inputs

| Input | Description |
|------|-------------|
| **A** | Gate/trigger input A. |
| **B** | Gate/trigger input B. |

If an input is not connected, its corresponding front-panel switch can be used to force it high.

---

## Controls

| Control | Description |
|--------|-------------|
| **A (switch)** | Forces input A high when enabled, ignoring the jack input. |
| **B (switch)** | Forces input B high when enabled, ignoring the jack input. |
| **PROBABILITY** | Sets the chance (0–100%) that a valid logic result will pass to the outputs. At 100%, all logic events pass. At lower values, events are randomly suppressed. |

---

## Outputs

Each output emits a gate whenever its logical condition is met **and** passes the probability test.

| Output | Logic |
|------|-------|
| **AND** | High when A and B are high. |
| **NAND** | High when NOT (A and B). |
| **OR** | High when A or B is high. |
| **NOR** | High when neither A nor B is high. |
| **XOR** | High when A and B differ. |
| **XNOR** | High when A and B are equal. |

All outputs operate in parallel.

---

## Behavior Notes

- Inputs are treated as logic-high above the internal gate threshold.
- Probability is evaluated **per event**, not per clock cycle.
- Probability affects all logic outputs equally.
- With **PROBABILITY = 100%**, TL-Bool behaves as a pure logic module.
- With lower probability values, TL-Bool introduces controlled trigger thinning without timing drift.

---

## Typical Use

1. Patch clocks, triggers, or gates into **A** and **B**.
2. Take different logic outputs to drive drums, envelopes, or sequencers.
3. Use the **A/B switches** to test or force logic states during performance.
4. Lower **PROBABILITY** to introduce variation and prevent repetitive patterns.
5. Combine multiple outputs for dense but evolving rhythmic structures.

---

## Notes

- TL-Bool is intended for **trigger and gate signals**, not audio-rate logic.
- Probability is global and non-CV-controlled by design, favoring simplicity and live reliability.
- Works especially well before drum triggers or reset inputs.

---

[⬅ Back to Module Index](../README.md)
