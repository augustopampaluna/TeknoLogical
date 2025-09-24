# TL-Seq4

<img width="412" height="583" alt="docu_seq4" src="https://github.com/user-attachments/assets/a454ecbe-6cd2-4d8a-8617-de1156a016d9" />

A **two-channel step trigger sequencer** for VCV Rack. Channel A runs **4 or 8 steps**, and Channel B runs **8 or 16 steps**. Each step can be toggled on/off. Perfect for programming drum hits, accents, and rhythmic events with clean trigger outputs.

---

## 🎛️ Controls (per channel)

| Control | Description |
|--------|-------------|
| **Steps** | Sequence length selector. **A = 4/8** steps, **B = 8/16** steps. Can also be toggled by CV (see Inputs). |
| **Reverse** | Reverses the play direction. Can also be toggled by CV (see Inputs). |
| **Step buttons 1…N** | Latching on/off per step (with LED). When the playhead lands on an enabled step, a trigger is fired at the output. |

---

## 🔌 Inputs

- **STEP IN** – Clock/trigger input for the channel. Each rising edge advances one step (or moves one step backward when `Reverse` is enabled).
- **CV Steps** – Toggles the sequence length by **rising edge** (A: 4↔8, B: 8↔16).
- **CV Reverse** – Toggles `Reverse` by **rising edge**. The panel switch updates to reflect the current state.

---

## 🔈 Outputs

- **OUT** – Short trigger of **~10 V / ~1 ms** whenever the playhead hits an enabled step.

---

## 💡 Indicators

- **Circular step ring (A & B)** – Shows the **current step position**.
- **LEDs under each step button** – Show the **latch state** (on = step enabled).

---

## 🔄 Reset (expander)

TL-Seq4 can receive **external resets** from the **TL-Reseter** module when placed to the **left or right as an expander**.

- A reset pulse for **A** or **B** returns that channel to the **first step** (or the **last step** when `Reverse` is enabled).
- Resets are read from either neighbor so fast pulses aren’t missed.

> Note: There is no front-panel reset jack; resets arrive only via the TL-Reseter expander.

---

## 🛠️ Notes

- Channel ranges: **A = 4/8 steps**, **B = 8/16 steps**.
- `CV Steps` and `CV Reverse` are **toggle** controls triggered on rising edges. The corresponding panel switches follow the CV state.
- In `Reverse`, the `STEP IN` clock moves the playhead **backward** through the sequence. Resets position to the appropriate end for the current direction.

---

## 🔁 Typical Use

1. Patch a clock to each channel’s **STEP IN**.
2. Choose the sequence length with **Steps** and optionally enable **Reverse**.
3. Enable the **step buttons** where you want triggers.
4. Patch **OUT** to drum modules, envelopes, logic, etc.
5. For bar-aligned patterns, add **TL-Reseter** as an expander and send it bar-start reset pulses.

---

[⬅ Back to Module Index](../README.md)

