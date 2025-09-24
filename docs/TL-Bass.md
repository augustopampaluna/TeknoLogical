# TL-Bass

<img width="136" height="581" alt="docu_bass" src="https://github.com/user-attachments/assets/5289203c-840d-498c-b15e-f5150527e039" />

**Minimal / psy bass voice for VCV Rack.** One-shot trigger, **1 V/Oct** input, **timbre 1/2** selector, and a **bipolar filter** where **0 = bypass**, **negative = low-pass**, **positive = high-pass**. The engine includes a short anti-click micro-attack and a DC-block on the output.

---

## Controls

| Control | Description |
| --- | --- |
| **TRIG (button with LED)** | Fires a one-shot note. Same behavior as a rising edge at the **TRIG** input. |
| **FILTER** | Bipolar macro (**−10…0 = LP**, **0…+10 = HP**, **0 = bypass**). **FILTER** CV sums 1:1 with the knob over the same range. |
| **DECAY** | VCA envelope decay time. **DECAY** CV sums 1:1 with the knob (range **−10…+10**). |
| **1 / 2** | **Timbre selector**: <br>**1 – Clean**: near-sine blend (sine + a touch of triangle) with minimal drive for round subs. <br>**2 – Music-Man-like**: **polyBLEP square+saw** blend, gentle **HP ~45 Hz** pre-stage, subtle **sub** (−1 oct), **tanh** waveshaper, and **LP ~6 kHz** post-stage for a warm, thick character. |

---

## Inputs

- **TRIG** — Triggers the note on a rising edge (from jack or the front-panel button).  
- **V/OCT** — 1 V/Oct **clamped to ±2 octaves** around the base note. If the cable is **unplugged**, the module **re-triggers** and returns to the default note. Internal reference uses **0 V = 440 Hz** with an offset to place the base pitch.  
- **FILTER (CV)** — Adds to the **FILTER** knob in the **−10…+10** range.  
- **DECAY (CV)** — Adds to the **DECAY** knob (range **−10…+10**).  

---

## Output

- **OUT** — Mono **±5 V** (10 Vpp), **DC-blocked** and normalized. The engine applies a short anti-click micro-attack (~0.5 ms) before the VCA.

---

## Tuning & Range

- **Default note (V/OCT unpatched)**: around **C2 ≈ 65.4 Hz** (derived from **0 V = 440 Hz** with **offset ≈ −2.75 V**).  
- **V/Oct range**: input is **clamped to ±2 oct** (total span of 4 octaves).  

---

## Bipolar Filter

Behavior of the filter macro (knob + CV):

- **0** → **Bypass**  
- **−10…0** → **Low-Pass** (more negative = darker)  
- **0…+10** → **High-Pass** (more positive = tighter low end)

Implemented with LP/HP stages controlled by the same macro; at **0** both are bypassed.

---

## Implementation Notes / Key Changes

- **Free phase**: triggers **do not** reset oscillator phase (for natural variation).  
- **Anti-click**: internal exponential attack ~**0.5 ms** from 0→1 to avoid start clicks.  
- **Timbre 2**: pre **HP ~45 Hz**, **polyBLEP square+saw** mix, subtle sub, **tanh** shaper, and post **LP ~6 kHz**.  
- **Safe output**: audio is **DC-filtered ~20 Hz** and limited to **±5 V**.  

---

## Typical Use

1. Patch a clock/trigger to **TRIG** or tap the button to audition.  
2. For melodic lines, patch **V/OCT** (remember: ±2 oct max).  
3. Set **DECAY** to taste (from percussive to sustained).  
4. Choose **1/2** and sculpt with **FILTER** (negative = round LP, positive = tight HP).  
5. Send **OUT** to your mixer/FX; external compression/saturation pairs nicely. *(Typical Rack audio levels: ±5 V).*  

---

## Tips

- In **mode 1 (Clean)**, keep **FILTER** near **0** or slightly negative for maximum sub punch.  
- In **mode 2**, a gentle **HP** (**+1…+3**) tightens the low end and leaves room for the kick; the post **LP** already smooths top end.  

---

*[⬅ Back to the modules index](../README.md)*
