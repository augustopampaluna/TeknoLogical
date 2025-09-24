#pragma once
#include "rack.hpp"
#include "dsp/filter.hpp"
#include <algorithm>
#include <cmath>

using namespace rack;

// General DSP.
namespace DSPUtils {

    struct HP1 {
        float a = 0.f;   // coeficiente
        float y1 = 0.f;  // y[n-1]
        float x1 = 0.f;  // x[n-1]
        void setCutoff(float fc, float sampleRate) {
            fc = std::max(1.f, fc);
            float dt = 1.f / sampleRate;
            float RC = 1.f / (2.f * float(M_PI) * fc);
            a = RC / (RC + dt);
        }
        inline float process(float x) {
            float y = a * (y1 + x - x1);
            y1 = y;
            x1 = x;
            return y;
        }
        void reset(){ y1 = x1 = 0.f; }
    };

    struct LowPassFilter {
        float y = 0.f;
        float alpha = 1.f;

        void setCutoff(float cutoff, float sampleRate) {
            float rc = 1.f / (2.f * M_PI * cutoff);
            float dt = 1.f / sampleRate;
            alpha = dt / (rc + dt);
            alpha = clamp(alpha, 0.f, 1.f);
        }

        float process(float input) {
            y += alpha * (input - y);
            return y;
        }
    };

    struct HighPassFilter {
        float y = 0.f;
        float x = 0.f;
        float alpha = 1.f;

        void setCutoff(float cutoff, float sampleRate) {
            float rc = 1.f / (2.f * M_PI * cutoff);
            float dt = 1.f / sampleRate;
            alpha = rc / (rc + dt);
            alpha = clamp(alpha, 0.f, 1.f);
        }

        float process(float input) {
            float out = alpha * (y + input - x);
            x = input;
            y = out;
            return out;
        }
    };

    struct DecayEnvelope {
        float value = 0.f;
        float decayCoeff = 0.f;

        void trigger(float decayParam, float sampleRate) {
            // Mapear decayParam de [-10, +10] a [0.08, 0.8] s (logarítmico)
            const float minTime = 0.08f;
            const float maxTime = 0.8f;
            float normalized = (decayParam + 10.f) / 20.f;
            float decayTime = minTime * std::pow(maxTime / minTime, normalized);
            decayTime = std::max(decayTime, 0.001f);
            decayCoeff = std::exp(-1.f / (decayTime * sampleRate));
            value = 1.f;
        }

        float process() {
            value *= decayCoeff;
            return value;
        }

        bool isActive() const {
            return value > 0.001f;
        }
    };

    struct CachedLowPass {
        LowPassFilter filter;
        float lastParam = 999.f;
        float lastSampleRate = 0.f;

        float process(float input, float filterParam, float sampleRate) {
            if (filterParam < 0.f) {
                if (filterParam != lastParam || sampleRate != lastSampleRate) {
                    float cutoffFreq = std::pow(10.f, rescale(filterParam, -10.f, 0.f, std::log10(20.f), std::log10(20000.f)));
                    filter.setCutoff(cutoffFreq, sampleRate);
                    lastParam = filterParam;
                    lastSampleRate = sampleRate;
                }
                return filter.process(input);
            }
            return input;
        }
    };

    struct CachedHighPass {
        HighPassFilter filter;
        float lastParam = -999.f;
        float lastSampleRate = 0.f;

        float process(float input, float filterParam, float sampleRate) {
            if (filterParam > 0.f) {
                if (filterParam != lastParam || sampleRate != lastSampleRate) {
                    float cutoffFreq = std::pow(10.f, rescale(filterParam, 0.f, 10.f, std::log10(20.f), std::log10(20000.f)));
                    filter.setCutoff(cutoffFreq, sampleRate);
                    lastParam = filterParam;
                    lastSampleRate = sampleRate;
                }
                return filter.process(input);
            }
            return input;
        }
    };

    inline float applyVolume(float signal, float volumeParam) {
        float gain = clamp(volumeParam / 10.f, 0.f, 1.f);  // volumen normalizado [0..1]
        return signal * gain;
    }

    inline void applyPan(float input, float panParam, float& left, float& right) {
        // Pan lineal con pan en [-1, 1]
        float pan = clamp(panParam, -1.f, 1.f);
        left  = input * (pan <= 0.f ? 1.f : 1.f - pan);
        right = input * (pan >= 0.f ? 1.f : 1.f + pan);
    }

    inline float applyBoost(float signal, float push) {
        if (push == 1.0f) {
            float boosted = signal * 1.5f;
            return clamp(boosted, -1.f, 1.f);
        }
        return signal;
    }

    inline float applyLowPassFilter(float input, float filterParam, float sampleRate, LowPassFilter& filter) {
        if (filterParam < 0.f) {
            // Mapeo logarítmico: -10..0 → 20..20000 Hz
            float cutoffFreq = std::pow(10.f, rescale(filterParam, -10.f, 0.f, std::log10(20.f), std::log10(20000.f)));
            filter.setCutoff(cutoffFreq, sampleRate);
            return filter.process(input);
        }
        return input;
    }

    inline float applyHighPassFilter(float input, float filterParam, float sampleRate, HighPassFilter& filter) {
        if (filterParam > 0.f) {
            // Mapeo logarítmico: 0..+10 → 20..20000 Hz
            float cutoffFreq = std::pow(10.f, rescale(filterParam, 0.f, 10.f, std::log10(20.f), std::log10(20000.f)));
            filter.setCutoff(cutoffFreq, sampleRate);
            return filter.process(input);
        }
        return input;
    }

    inline void panMonoEqualPower(float in, float pan /* -1..1 */, float& l, float& r) {
        float theta = (pan * 0.5f + 0.5f) * (float)M_PI_2; // 0..PI/2
        float gl = std::cos(theta);
        float gr = std::sin(theta);
        l += in * gl;
        r += in * gr;
    }

    inline void balanceStereoEqualPower(float& l, float& r, float pan /* -1..1 */) {
        float theta = std::fabs(pan) * (float)M_PI_2; // 0..PI/2
        float g = std::cos(theta);
        if (pan > 0.f) l *= g;
        else if (pan < 0.f) r *= g;
    }

    inline float softLimit5V(float x) {
        return 5.f * std::tanh(x / 5.f);
    }

    inline float resolveVolume01(float knob0to10, bool cvConnected, float cvVolts0to10) {
        float volMax = clamp(knob0to10 / 10.f, 0.f, 1.f);
        if (!cvConnected) return volMax;
        float v = clamp(cvVolts0to10, 0.f, 10.f) / 10.f; // 0..1
        return volMax * v;
    }

    inline float resolvePanMinus1to1(float knobMinus1to1, bool cvConnected, float cvVoltsPlusMinus5) {
        if (!cvConnected) return clamp(knobMinus1to1, -1.f, 1.f);
        return clamp(cvVoltsPlusMinus5 / 5.f, -1.f, 1.f);
    }

}
