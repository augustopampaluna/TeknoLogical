#include "plugin.hpp"
#include "../helpers/dsp_utils.hpp"

using namespace rack;
using namespace DSPUtils;

// -----------------------------------------------------------------------------
// PolyBLEP oscillator (one advance() per sample; shape reads do not advance).
// Maintains main phase and a sub-octave phase. Caches dt to avoid recompute.
// -----------------------------------------------------------------------------
struct PolyBLEPOsc {
	float phase = 0.f;     // [0,1)
	float subPhase = 0.f;  // sub at -1 octave
	float freq = 100.f;    // Hz
	float sr = 44100.f;
	float lastDt = 100.f / 44100.f; // cached phase increment

	void setSampleRate(float s) {
		sr = s;
		lastDt = freq / sr; // keep coherent with freq
	}

	void setFreq(float f) {
		freq = clamp(f, 10.f, 12000.f);
		lastDt = freq / sr; // keep dt updated on freq change
	}

	void resetPhase() { phase = 0.f; subPhase = 0.f; }

	// PolyBLEP step for discontinuity correction (static: used by const reads).
	static inline float polyblep(float t, float dt) {
		if (t < dt) { t /= dt; return t + t - t * t - 1.f; }
		if (t > 1.f - dt) { t = (t - 1.f) / dt; return t * t + t + t + 1.f; }
		return 0.f;
	}

	// Single-advance per sample (main + sub octave).
	inline void advance() {
		float dt = lastDt;
		phase += dt; if (phase >= 1.f) phase -= 1.f;
		float subDt = 0.5f * dt;
		subPhase += subDt; if (subPhase >= 1.f) subPhase -= 1.f;
	}

	// Readouts (no phase advance).
	inline float sine() const {
		return std::sin(2.f * M_PI * phase);
	}

	// Naive triangle
	inline float triangle() const {
		float t = 2.f * phase - 1.f;
		return 2.f * std::fabs(t) - 1.f;
	}

	// BLEP saw
	inline float sawBLEP() const {
		float dt = lastDt;
		float x = 2.f * phase - 1.f;
		return x - polyblep(phase, dt);
	}

	// BLEP square with PWM (two BLEP edges)
	inline float squareBLEP(float pwm = 0.5f) const {
		pwm = clamp(pwm, 0.05f, 0.95f);
		float dt = lastDt;
		float y = (phase < pwm) ? 1.f : -1.f;
		y += polyblep(phase, dt);
		float t = phase - pwm; // manual wrap
		if (t < 0.f) t += 1.f;
		y -= polyblep(t, dt);
		return y;
	}

	inline float subSine() const {
		return std::sin(2.f * M_PI * subPhase);
	}
};

// -----------------------------------------------------------------------------
// Module: mono bass voice with macro DJ-style filter and two timbres.
// - PolyBLEP core, decay envelope, anti-click micro-attack.
// - Macro filter: center=bypass; left=LP; right=HP (with dynamic-Q in DSP).
// -----------------------------------------------------------------------------
struct TL_Bass : Module {
	enum ParamId {
		BTN_TRIG_PARAM,
		FILTER_KNOB_PARAM,     // -10..+10  (0=bypass; <0 LP; >0 HP)
		DECAY_KNOB_PARAM,      // -10..+10
		TIMBRE_SELECTOR_PARAM, // “1 / 2”
		PARAMS_LEN
	};
	enum InputId {
		TRIGGER_JACK_INPUT,
		VOCT_JACK_INPUT,       // limited to ±2 oct
		FILTER_JACK_INPUT,     // sums with knob, same -10..+10 range
		DECAY_JACK_INPUT,
		INPUTS_LEN
	};
	enum OutputId { OUT_MONO_OUTPUT, OUTPUTS_LEN };
	enum LightId { BTN_TRIG_LIGHT, LIGHTS_LEN };

	// --- DSP state (triggers, oscillator) ---
	dsp::SchmittTrigger trigIn, trigBtn;
	PolyBLEPOsc osc;

	// --- Envelopes / anti-click ---
	DecayEnvelope env; // main D-envelope
	float atkEnv = 1.f; // micro attack state
	float atkCoeff = 1.f;
	float env2 = 0.f;     // reserved
	float env2Coeff = 0.999f;

	// --- Macro filter stages (cached) ---
	CachedLowPass  lowFilter;
	CachedHighPass highFilter;

	// --- Timbre 2 tone stages (fixed) ---
	HighPassFilter preT2HP;   // tighten < ~50 Hz
	LowPassFilter  postT2LP;  // soften > ~6 kHz

	// --- DC blocker ---
	HighPassFilter dcBlock;

	// --- V/Oct handling ---
	bool voctWasConnected = false;

	// 0 V = 440 Hz; default pitch offset so C2 ~ 65.4 Hz
	static constexpr float BASE_V_DEFAULT = -2.75f;

	// Trigger LED pulse holder
	dsp::PulseGenerator trigLed;

	TL_Bass() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(BTN_TRIG_PARAM, 0.f, 1.f, 0.f, "Trigger");
		configParam(FILTER_KNOB_PARAM, -10.f, 10.f, 0.f, "Filter");
		configParam(DECAY_KNOB_PARAM, -10.f, 10.f, 0.f, "Decay");
		configSwitch(TIMBRE_SELECTOR_PARAM, 0.f, 1.f, 1.f, "Timbre", {"2", "1"}); // panel labels

		configInput(TRIGGER_JACK_INPUT, "Trigger");
		configInput(VOCT_JACK_INPUT,    "V/Oct (±2 oct)");
		configInput(FILTER_JACK_INPUT,  "Filter CV");
		configInput(DECAY_JACK_INPUT,   "Decay CV");
		configOutput(OUT_MONO_OUTPUT,   "Mono");
	}

	// -------------------------------------------------------------------------
	// SR-dependent setup (oscillator rate, DC blocker, time constants, tone).
	// -------------------------------------------------------------------------
	void onSampleRateChange() override {
		float sr = APP->engine->getSampleRate();
		osc.setSampleRate(sr);

		// DC-block ~20 Hz
		dcBlock.setCutoff(20.f, sr);

		// Anti-click micro attack ~0.5 ms
		atkCoeff = std::exp(-1.f / (0.0005f * sr));

		// Timbre 2: fixed tone stages
		preT2HP.setCutoff(45.f, sr);    // trim ultra-low rumble
		postT2LP.setCutoff(6000.f, sr); // round the harsh top

		// Reserved envelope (precomputed coeff)
		env2Coeff = std::exp(-1.f / (0.003f * sr));
	}

	// -------------------------------------------------------------------------
	// Process: trigger handling, pitch, oscillator, timbre, macro filter, VCA.
	// -------------------------------------------------------------------------
	void process(const ProcessArgs& args) override {
		const float sr = args.sampleRate;

		// --- Trigger from jack and button (edge detection) ---
		bool fired = false;
		const bool trigConnected = inputs[TRIGGER_JACK_INPUT].isConnected();
		if (trigConnected) {
			// Conservative thresholds to avoid false retriggers
			fired |= trigIn.process(inputs[TRIGGER_JACK_INPUT].getVoltage(), 0.1f, 1.f);
		}
		// Button uses similar thresholds (0..1 V)
		fired |= trigBtn.process(params[BTN_TRIG_PARAM].getValue(), 0.1f, 1.f);

		// Retrigger on V/Oct unplug event (restore default note)
		const bool voctNow = inputs[VOCT_JACK_INPUT].isConnected();
		if (voctWasConnected && !voctNow) {
			fired = true;
		}
		voctWasConnected = voctNow;

		// Short LED pulse on trigger
		if (fired) trigLed.trigger(1e-3f);
		lights[BTN_TRIG_LIGHT].setBrightness(trigLed.process(args.sampleTime) ? 1.f : 0.f);

		// --- Decay (knob + CV) ---
		const bool decayCvConnected = inputs[DECAY_JACK_INPUT].isConnected();
		float decayParam = params[DECAY_KNOB_PARAM].getValue()
			+ (decayCvConnected ? inputs[DECAY_JACK_INPUT].getVoltage() : 0.f);
		decayParam = clamp(decayParam, -10.f, 10.f);

		if (fired) {
			env.trigger(decayParam, sr);
			atkEnv = 0.f; // start micro attack
		}

		// --- Pitch (1 V/oct, clamped ±2 oct) ---
		const float pitchIn = voctNow ? clamp(inputs[VOCT_JACK_INPUT].getVoltage(), -2.f, 2.f) : 0.f;
		const float pitchV  = BASE_V_DEFAULT + pitchIn;

		// Fast 2^x and Nyquist-safe ceiling
		float freq = 440.f * dsp::exp2_taylor5(pitchV);
		const float nyqSafe = 0.45f * sr;
		if (freq > nyqSafe) freq = nyqSafe;
		osc.setFreq(freq);

		// --- Oscillator: advance once, read all shapes ---
		osc.advance();
		const float s   = osc.sine();
		const float tr  = osc.triangle();
		const float sub = osc.subSine();
		const float sq  = osc.squareBLEP(0.48f);
		const float sw  = osc.sawBLEP();

		// Panel switch: > 0.5 => timbre "1" (clean)
		const bool pos1_clean = params[TIMBRE_SELECTOR_PARAM].getValue() > 0.5f;

		// --- Timbre paths (pre) ---
		float pre = 0.f;
		if (pos1_clean) {
			// Timbre 1: near-sine with slight soft drive
			const float clean = 0.90f * s + 0.10f * tr;
			pre = std::tanh(1.05f * clean);
		} else {
			// Timbre 2: aggressive “MM”-style blend + tone shaping
			float mixCore = 0.55f * sq + 0.45f * sw;
			mixCore = preT2HP.process(mixCore);
			mixCore = 0.85f * mixCore + 0.15f * sub;
			const float shaped = std::tanh(1.40f * mixCore) * 0.9f + 0.1f * mixCore;
			pre = postT2LP.process(shaped);
		}

		// --- Envelopes / anti-click ---
		const float e = env.process();
		atkEnv = 1.f - (1.f - atkEnv) * atkCoeff;

		// --- Macro filter (LP<0 | bypass=0 | >0 HP) ---
		const bool filtCvConnected = inputs[FILTER_JACK_INPUT].isConnected();
		float filterVal = params[FILTER_KNOB_PARAM].getValue()
			+ (filtCvConnected ? inputs[FILTER_JACK_INPUT].getVoltage() : 0.f);
		filterVal = clamp(filterVal, -10.f, 10.f);

		float x = pre;
		x = lowFilter.process(x,  filterVal, sr);
		x = highFilter.process(x, filterVal, sr);

		// --- VCA + anti-click, DC block, scaling, output ---
		float out = x * e * atkEnv;
		out = dcBlock.process(out);
		out = clamp(out * 5.f, -11.7f, 11.7f);

		outputs[OUT_MONO_OUTPUT].setVoltage(out);
	}
};

// -----------------------------------------------------------------------------
// Widget (panel & controls layout).
// -----------------------------------------------------------------------------
struct TL_BassWidget : ModuleWidget {
	TL_BassWidget(TL_Bass* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/TL_Bass.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createLightParamCentered<VCVLightButton<LargeSimpleLight<WhiteLight>>>(mm2px(Vec(15.142, 24.404)), module, TL_Bass::BTN_TRIG_PARAM, TL_Bass::BTN_TRIG_LIGHT));
		addParam(createParamCentered<Rogan1PSWhite>(mm2px(Vec(15.056, 46.994)), module, TL_Bass::FILTER_KNOB_PARAM));
		addParam(createParamCentered<Rogan1PSWhite>(mm2px(Vec(14.933, 68.928)), module, TL_Bass::DECAY_KNOB_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(7.957, 109.690)), module, TL_Bass::TIMBRE_SELECTOR_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.297, 28.152)), module, TL_Bass::TRIGGER_JACK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.206, 28.152)), module, TL_Bass::VOCT_JACK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.891, 92.414)), module, TL_Bass::FILTER_JACK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.650, 92.422)), module, TL_Bass::DECAY_JACK_INPUT));

		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(22.638, 108.322)), module, TL_Bass::OUT_MONO_OUTPUT));
	}
};

Model* modelTL_Bass = createModel<TL_Bass, TL_BassWidget>("TL_Bass");
