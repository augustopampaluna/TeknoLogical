# TL-Reseter

<img width="66" height="582" alt="docu_reseter" src="https://github.com/user-attachments/assets/55cd2374-f72e-4c5d-a9e4-db1361fd3193" />

A **dual reset expander** for TL-Seq4. Provides manual buttons and gate inputs to reset **Channel A** and **Channel B** independently. Each channel can target the **left** or **right** neighboring TL-Seq4 via the internal expander bus. Perfect for bar-aligned patterns, fills, and live performance control.

---

## 🎛️ Controls (per channel)

| Control | Description |
|--------|-------------|
| **Push** | Momentary button. Sends a one-shot reset to the selected side. The LED lights while the button (or its input) is active. |
| **Side** | Chooses the destination TL-Seq4: **Left** or **Right** neighbor. A and B can target different sides simultaneously. |

---

## 🔌 Inputs

- **IN** – Gate/trigger input for the channel. A rising edge resets the target TL-Seq4 channel (works with pulses above ~1 V; negative-going pulses above that magnitude also work).

---

## 🔈 Outputs

- *None.* TL-Reseter communicates resets through the **expander bus** only.

---

## 🧩 Expander Behavior

- Place TL-Reseter **immediately to the left or right** of a TL-Seq4 to control it.
- **A** and **B** are independent. You can send **A → Left** and **B → Right**, or both to the same side.
- Resets are **edge-triggered**: holding the button or a high gate keeps the LED on but **sends only one reset** until it returns low and rises again.
- If there is no TL-Seq4 on the chosen side, the command is ignored safely.
- On TL-Seq4, a reset returns the playhead to the **first step** (or the **last step** when `Reverse` is enabled).

---

## 🛠️ Notes

- Designed for TL-Seq4; other modules will ignore the expander messages.
- Inputs are tolerant of both positive and negative triggers (by magnitude).
- No audio or CV is passed through; this module only issues reset messages.

---

## 🔁 Typical Use

1. Place TL-Reseter next to a TL-Seq4 (or between two TL-Seq4s).
2. Set **Side** per channel to the desired destination(s).
3. Patch your transport/bar-start pulse to **IN A** (and/or **IN B**).
4. Press **Push** during performance to inject manual resets for fills and variations.

---

[⬅ Back to Module Index](../README.md)
