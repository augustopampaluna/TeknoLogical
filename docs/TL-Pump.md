# TL-Pump

A **trigger-driven volume shaper / ducking module** for VCV Rack 2.  
TL-Pump applies predefined sidechain-style gain curves to incoming stereo audio. It is designed for tight, musical pumping effects in minimal and techno contexts.

Instead of a traditional compressor with threshold and ratio controls, TL-Pump provides **four fixed envelope shapes** optimized for common kick lengths and groove styles.

---

## Overview

TL-Pump is not a compressor.  
It is a **triggered gain envelope** that multiplies the incoming audio by a predefined curve each time it receives a trigger.

The goal is simple:

- Clean  
- Predictable  
- Musically tuned  
- Performance-focused  

No threshold. No ratio. No attack tweaking.  
Just controlled rhythmic pumping.

---

## 🎛️ Controls

| Control | Description |
|----------|-------------|
| **SHAPE** | Selects one of four predefined volume curves. Each press advances sequentially through the shapes. The LED remains softly lit to indicate the active state. |
| **TRIG (button)** | Manual trigger. Fires the envelope exactly like the trigger input. |
| **DRY/WET** | Crossfade between original signal (Dry) and processed signal (Wet). Default = **100% Wet**. |

---

## 🔌 Inputs

- **L / R** — Stereo audio input.  
- **TRIG** — Trigger/gate input. A rising edge activates the selected envelope shape.

---

## 🔈 Outputs

- **L / R** — Stereo processed output.

---

## 📐 Envelope Shapes

The four shapes are fixed and carefully tuned.  
If white noise is passed through TL-Pump and observed on a scope, the amplitude contour will match these curves exactly.

### Shape 1 — Immediate Attack / Delayed Release (Long Kick)

- Instant drop in volume  
- Short hold at minimum  
- Smooth, musical return  
- Ideal for longer kick tails  

### Shape 2 — Immediate Attack / Snap Release

- Similar to Shape 1  
- Volume returns more abruptly  
- Produces a sharper groove feel  

### Shape 3 — One-Way Fade Out

- Starts at full volume  
- Fades smoothly to zero  
- Does **not** return to full volume until the next trigger  
- Useful for rhythmic gating and cuts  

### Shape 4 — Immediate Attack / Fast Release (Short Kick)

- Very quick drop  
- Faster release than Shape 1  
- Designed for tight, short kicks  

---

## ⚙️ Behavior Notes

- Envelopes trigger on **rising edge**.  
- The envelope restarts immediately on retrigger.  
- No threshold or audio detection is used — pumping is entirely trigger-based.  
- DRY/WET performs a linear crossfade between original and shaped signal.  
- Default state is fully Wet (100% effect).  
- Designed for consistent rhythmic shaping rather than dynamic compression.

---

## 🔁 Typical Use

1. Patch stereo audio into **L/R**.  
2. Send a kick trigger (or clocked trigger) into **TRIG**.  
3. Select a **SHAPE** that matches your groove.  
4. Keep **DRY/WET at 100%** for classic sidechain pumping.  
5. Reduce DRY/WET for subtler movement.

---

## 🛠️ Notes

- TL-Pump is intentionally minimal and non-CV controlled.  
- There is no threshold, ratio, or attack parameter by design.  
- Works best when triggered by kick patterns or rhythmic pulses.  
- Designed for live performance reliability and groove consistency.  

---

[⬅ Back to Module Index](../README.md)
