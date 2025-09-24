#pragma once
#include "rack.hpp"
#include "dsp/filter.hpp"
#include <algorithm>
#include <cmath>

using namespace rack;

namespace DSPUtils {

    inline bool changedEnough(float a, float b, float eps = 1e-4f) {
        return std::fabs(a - b) > eps;
    }

    inline void equalPowerGains(float pan, float& gL, float& gR) {
        float theta = (clamp(pan, -1.f, 1.f) * 0.5f + 0.5f) * (float)M_PI_2;
        gL = std::cos(theta);
        gR = std::sin(theta);
    }

    inline float equalPowerAttenuation(float pan /* -1..1 */) {
        float theta = std::fabs(clamp(pan, -1.f, 1.f)) * (float)M_PI_2;
        return std::cos(theta);
    }

    struct HP1 {
        float a = 0.f, y1 = 0.f, x1 = 0.f;
        void setCutoff(float fc, float sampleRate) {
            fc = std::max(1.f, fc);
            float dt = 1.f / sampleRate;
            float RC = 1.f / (2.f * float(M_PI) * fc);
            a = RC / (RC + dt);
        }
        inline float process(float x) {
            float y = a * (y1 + x - x1);
            y1 = y; x1 = x;
            return y;
        }
        void reset(){ y1 = x1 = 0.f; }
    };

    struct LowPassFilter {
        float b0=1.f, b1=0.f, b2=0.f, a1=0.f, a2=0.f;
        float z1=0.f, z2=0.f;

        void setCutoff(float cutoff, float sampleRate, float Q = 0.707f) {
            cutoff = std::max(20.f, cutoff);
            float w0 = 2.f * (float)M_PI * cutoff / sampleRate;
            float c = std::cos(w0);
            float s = std::sin(w0);
            float alpha = s / (2.f * std::max(0.0001f, Q));

            float b0n = (1.f - c) * 0.5f;
            float b1n = (1.f - c);
            float b2n = (1.f - c) * 0.5f;
            float a0  = 1.f + alpha;
            float a1n = -2.f * c;
            float a2n = 1.f - alpha;

            b0 = b0n / a0; b1 = b1n / a0; b2 = b2n / a0;
            a1 = a1n / a0; a2 = a2n / a0;
        }

        inline float process(float x) {
            float y = b0 * x + z1;
            z1 = b1 * x - a1 * y + z2;
            z2 = b2 * x - a2 * y;
            return y;
        }
    };

    struct HighPassFilter {
        float b0=1.f, b1=0.f, b2=0.f, a1=0.f, a2=0.f;
        float z1=0.f, z2=0.f;

        void setCutoff(float cutoff, float sampleRate, float Q = 0.707f) {
            cutoff = std::max(20.f, cutoff);
            float w0 = 2.f * (float)M_PI * cutoff / sampleRate;
            float c = std::cos(w0);
            float s = std::sin(w0);
            float alpha = s / (2.f * std::max(0.0001f, Q));

            float b0n = (1.f + c) * 0.5f;
            float b1n = -(1.f + c);
            float b2n = (1.f + c) * 0.5f;
            float a0  = 1.f + alpha;
            float a1n = -2.f * c;
            float a2n = 1.f - alpha;

            b0 = b0n / a0; b1 = b1n / a0; b2 = b2n / a0;
            a1 = a1n / a0; a2 = a2n / a0;
        }

        inline float process(float x) {
            float y = b0 * x + z1;
            z1 = b1 * x - a1 * y + z2;
            z2 = b2 * x - a2 * y;
            return y;
        }
    };

    struct DecayEnvelope {
        float value = 0.f, decayCoeff = 0.f;
        void trigger(float decayParam, float sampleRate) {
            const float minTime = 0.08f, maxTime = 0.8f;
            float normalized = (decayParam + 10.f) / 20.f;
            float decayTime = minTime * std::pow(maxTime / minTime, normalized);
            decayTime = std::max(decayTime, 0.001f);
            decayCoeff = std::exp(-1.f / (decayTime * sampleRate));
            value = 1.f;
        }
        float process() { value *= decayCoeff; return value; }
        bool isActive() const { return value > 0.001f; }
    };

    inline float mapLP_Cutoff(float filterParam /* -10..0 */, float sr) {
        return std::pow(10.f, rescale(filterParam, -10.f, 0.f, std::log10(20.f), std::log10(20000.f)));
    }
    inline float mapHP_Cutoff(float filterParam /* 0..+10 */, float sr) {
        return std::pow(10.f, rescale(filterParam, 0.f, 10.f, std::log10(20.f), std::log10(20000.f)));
    }
    inline float mapResonanceQ(float amount01) {
        const float Qmin = 0.707f;
        const float Qmax = 2.5f;
        const float shape = 1.35f;
        float t = std::pow(clamp(amount01, 0.f, 1.f), shape);
        return Qmin + (Qmax - Qmin) * t;
    }

    struct CachedLowPass {
        LowPassFilter filter;
        float lastParam = 999.f, lastSampleRate = 0.f;

        float process(float input, float filterParam, float sampleRate) {
            if (filterParam < 0.f) {
                if (filterParam != lastParam || sampleRate != lastSampleRate) {
                    float cutoff = mapLP_Cutoff(filterParam, sampleRate);
                    float amount = (-filterParam) / 10.f;  // 0..1
                    float Q = mapResonanceQ(amount);
                    filter.setCutoff(cutoff, sampleRate, Q);
                    lastParam = filterParam; lastSampleRate = sampleRate;
                }
                return filter.process(input);
            }
            return input;
        }
    };

    struct CachedHighPass {
        HighPassFilter filter;
        float lastParam = -999.f, lastSampleRate = 0.f;

        float process(float input, float filterParam, float sampleRate) {
            if (filterParam > 0.f) {
                if (filterParam != lastParam || sampleRate != lastSampleRate) {
                    float cutoff = mapHP_Cutoff(filterParam, sampleRate);
                    float amount = (filterParam) / 10.f; // 0..1
                    float Q = mapResonanceQ(amount);
                    filter.setCutoff(cutoff, sampleRate, Q);
                    lastParam = filterParam; lastSampleRate = sampleRate;
                }
                return filter.process(input);
            }
            return input;
        }
    };

    inline float applyVolume(float signal, float volumeParam) {
        float gain = clamp(volumeParam / 10.f, 0.f, 1.f);
        return signal * gain;
    }

    inline void applyPan(float input, float panParam, float& left, float& right) {
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
            float cutoff = mapLP_Cutoff(filterParam, sampleRate);
            float amount = (-filterParam) / 10.f; // 0..1
            float Q = mapResonanceQ(amount);
            filter.setCutoff(cutoff, sampleRate, Q);
            return filter.process(input);
        }
        return input;
    }

    inline float applyHighPassFilter(float input, float filterParam, float sampleRate, HighPassFilter& filter) {
        if (filterParam > 0.f) {
            float cutoff = mapHP_Cutoff(filterParam, sampleRate);
            float amount = (filterParam) / 10.f; // 0..1
            float Q = mapResonanceQ(amount);
            filter.setCutoff(cutoff, sampleRate, Q);
            return filter.process(input);
        }
        return input;
    }

    inline void panMonoEqualPower(float in, float pan /* -1..1 */, float& l, float& r) {
        float gl, gr; equalPowerGains(pan, gl, gr);
        l += in * gl; r += in * gr;
    }

    inline void balanceStereoEqualPower(float& l, float& r, float pan /* -1..1 */) {
        float g = equalPowerAttenuation(pan);
        if (pan > 0.f) l *= g;
        else if (pan < 0.f) r *= g;
    }

    inline float softLimit5V(float x) {
    #ifdef DSP_FAST_LIMIT
        float y = x - (x * x * x) / (75.f);
        return clamp(y, -5.f, 5.f);
    #else
        return 5.f * std::tanh(x / 5.f);
    #endif
    }

    inline float resolveVolume01(float knob0to10, bool cvConnected, float cvVolts0to10) {
        float volMax = clamp(knob0to10 / 10.f, 0.f, 1.f);
        if (!cvConnected) return volMax;
        float v = clamp(cvVolts0to10, 0.f, 10.f) / 10.f;
        return volMax * v;
    }

    inline float resolvePanMinus1to1(float knobMinus1to1, bool cvConnected, float cvVoltsPlusMinus5) {
        if (!cvConnected) return clamp(knobMinus1to1, -1.f, 1.f);
        return clamp(cvVoltsPlusMinus5 / 5.f, -1.f, 1.f);
    }

}
