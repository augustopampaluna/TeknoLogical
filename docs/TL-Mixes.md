# TL-Mixes

<img width="553" height="581" alt="docu_mixes" src="https://github.com/user-attachments/assets/f4b45c7f-c55b-43fe-9d27-dc20cbc95213" />

A 7-channel mono/stereo audio mixer for VCV Rack 2. Each channel provides High-Pass (CUT), Pan, Volume, Mute, and Solo, with stereo summing, a soft output limiter, a post-fader Master control, and stereo VU meters on the main output.

---

## Controls (per channel)

| Control | Description |
|---|---|
| CUT | Enables a gentle high-pass filter on the channel. LED indicates on/off. |
| Pan | -1..+1 pan. With mono sources, TL-Mixes uses constant-power panning. With stereo sources, it acts as a balance control (attenuates the opposite side). The Pan CV input (P) replaces the knob with a range of -5..+5 V. |
| Vol | Channel volume. The knob sets the maximum (0..100%). The Volume CV input (V) applies an absolute 0..1 value within that maximum, expected CV range 0..10 V. This makes each channel usable as a simple VCA. |
| Mute | Silences the channel. LED indicates state. |
| Solo | Isolates the channel. If any Solo is active, only soloed channels are heard. LED indicates state. |

### Master controls

- Master: Global post-fader level (0..100%). The summed signal passes a soft limiter (tanh) that holds around +/-5 V to avoid hard clipping. The VU meters show post-limiter level.

---

## Inputs (per channel)

- L / R: Audio inputs. If only one side is connected (L or R), the channel runs in mono and is copied to the other side. If both are connected, the channel is treated as stereo.
- V (Vol CV): 0..10 V. Absolute volume control inside the maximum set by the knob.
- P (Pan CV): -5..+5 V. Replaces the pan knob. With mono sources it performs constant-power panning; with stereo sources it works as balance.

---

## Outputs

- Stereo Out (L / R): Post-limiter stereo mix.

---

## Indicators

- CUT LED (per channel): High-pass state.
- MUTE / SOLO LEDs (per channel): Channel state.
- VU L / R (5 segments): Post-limiter level indication. Top segment lights near full scale.

---

## Notes

- The high-pass filter is recalculated when the sample rate changes and its internal state is cleared on module reset.
- Mono pan uses a constant-power law; with stereo, pan behaves as a balance control.
- The soft limiter (tanh) keeps peaks musical and prevents hard digital clipping.

---

## Quick start

1) Patch your sources into each channel L/R input. For mono sources, one jack is enough.
2) Set channel Vol and, if you want CV control, send 0..10 V to the V input.
3) Adjust Pan with the knob or drive it from the P input (-5..+5 V). With stereo sources, Pan behaves as balance.
4) Use Mute and Solo to manage the mix. If any Solo is on, only soloed channels pass.
5) Raise Master to taste and watch the VU meters. The last LED indicates a level close to full scale.

---

[Back to module index](../README.md)

