#include "plugin.hpp"
#include "../helpers/widgets/sliders.hpp"

#include <cmath>

struct TL_Pump : Module {
	enum ParamId {
		SHAPE_PARAM,
		TRIGGER_MANUAL_PARAM,
		DRYWET_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		TRIGGER_INPUT,
		IN_L_INPUT,
		IN_R_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_L_OUTPUT,
		OUT_R_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LED_A_LIGHT,
		LED_B_LIGHT,
		LED_C_LIGHT,
		LED_D_LIGHT,
		SHAPE_LED,
		TRIGGER_MANUAL_LED,
		LIGHTS_LEN
	};

	// ---- Runtime ----
	int shapeIndex = 0;

	dsp::SchmittTrigger shapeTrig;
	dsp::SchmittTrigger manualTrig;
	dsp::SchmittTrigger inputTrig;

	// Envelope state
	bool envActive = false;
	float envTime = 0.f; // seconds since trigger

	// Fixed depth (no control). 0.85 = duck fuerte pero musical.
	// gain = 1 - env * depth
	static constexpr float depth = 0.85f;

	struct ShapeDef {
		float attack;   // seconds
		float hold;     // seconds
		float release;  // seconds
		float aExp;     // attack curve exponent
		float rExp;     // release curve exponent
	};

	// 4 formas (ajustadas “tipo Kickstart” y una con attack más lento para dejar pasar el bombo)
	ShapeDef shapes[4] = {
		// A: classic pump (muy rápido abajo, subida media)
		{ 0.0015f, 0.0000f, 0.180f, 0.60f, 2.20f },
		// B: deep/long (rápido abajo + release largo)
		{ 0.0010f, 0.0150f, 0.320f, 0.55f, 2.60f },
		// C: let-kick-through (attack más lento, release medio)
		{ 0.0100f, 0.0000f, 0.200f, 1.40f, 2.10f },
		// D: snappy/gatey (rápido abajo + release corto)
		{ 0.0008f, 0.0000f, 0.090f, 0.50f, 2.00f }
	};

	TL_Pump() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configParam(SHAPE_PARAM, 0.f, 1.f, 0.f, "Shape");
		configParam(TRIGGER_MANUAL_PARAM, 0.f, 1.f, 0.f, "Trigger");
		configParam(DRYWET_PARAM, 0.f, 1.f, 1.f, "Dry/Wet"); // default full wet suele ser lo esperado

		configInput(TRIGGER_INPUT, "Trigger");
		configInput(IN_L_INPUT, "In L");
		configInput(IN_R_INPUT, "In R");

		configOutput(OUT_L_OUTPUT, "Out L");
		configOutput(OUT_R_OUTPUT, "Out R");
	}

	float envelopeValue(const ShapeDef& s, float t) {
		// env in [0..1], where 1 = maximum duck
		if (t < 0.f) return 0.f;

		const float a = s.attack;
		const float h = s.hold;
		const float r = s.release;

		// Attack: 0 -> 1
		if (t <= a) {
			float x = (a > 0.f) ? (t / a) : 1.f;
			x = clamp(x, 0.f, 1.f);
			// Curva: pow(x, aExp) (aExp < 1 más rápido al inicio)
			return std::pow(x, s.aExp);
		}

		// Hold: 1
		if (t <= a + h) {
			return 1.f;
		}

		// Release: 1 -> 0
		float tr = t - (a + h);
		if (tr <= r) {
			float x = (r > 0.f) ? (tr / r) : 1.f;
			x = clamp(x, 0.f, 1.f);
			// Queremos 1 -> 0: pow(1 - x, rExp)
			return std::pow(1.f - x, s.rExp);
		}

		// End
		return 0.f;
	}

	void startEnvelope() {
		envActive = true;
		envTime = 0.f;
	}

	void process(const ProcessArgs& args) override {
		// --- Shape button: cycle 4 shapes on rising edge
		if (shapeTrig.process(params[SHAPE_PARAM].getValue())) {
			shapeIndex = (shapeIndex + 1) & 3; // 0..3
		}

		// --- Trigger detection: manual OR input
		bool trig = false;
		if (manualTrig.process(params[TRIGGER_MANUAL_PARAM].getValue())) {
			trig = true;
		}
		if (inputs[TRIGGER_INPUT].isConnected()) {
			// SchmittTrigger espera señal unipolar razonable; tus triggers suelen ser 0..10V
			if (inputTrig.process(inputs[TRIGGER_INPUT].getVoltage())) {
				trig = true;
			}
		}

		if (trig) {
			startEnvelope();
		}

		// --- Envelope advance
		float env = 0.f;
		const ShapeDef& s = shapes[shapeIndex];

		if (envActive) {
			env = envelopeValue(s, envTime);
			envTime += args.sampleTime;

			// Auto-stop cuando ya terminó
			if (envTime > (s.attack + s.hold + s.release + 0.01f)) {
				envActive = false;
				envTime = 0.f;
				env = 0.f;
			}
		}

		// --- Gain from envelope
		float gain = 1.f - env * depth;
		gain = clamp(gain, 0.f, 1.f);

		// --- Audio IO
		float inL = inputs[IN_L_INPUT].getVoltage();
		float inR = inputs[IN_R_INPUT].isConnected() ? inputs[IN_R_INPUT].getVoltage() : inL;

		float wetL = inL * gain;
		float wetR = inR * gain;

		float dw = clamp(params[DRYWET_PARAM].getValue(), 0.f, 1.f);

		float outL = inL * (1.f - dw) + wetL * dw;
		float outR = inR * (1.f - dw) + wetR * dw;

		outputs[OUT_L_OUTPUT].setVoltage(outL);
		outputs[OUT_R_OUTPUT].setVoltage(outR);

		// --- Lights: indicate selected shape
		lights[LED_A_LIGHT].setBrightness(shapeIndex == 0 ? 1.f : 0.f);
		lights[LED_B_LIGHT].setBrightness(shapeIndex == 1 ? 1.f : 0.f);
		lights[LED_C_LIGHT].setBrightness(shapeIndex == 2 ? 1.f : 0.f);
		lights[LED_D_LIGHT].setBrightness(shapeIndex == 3 ? 1.f : 0.f);

		// Integrated button lights:
		// - SHAPE_LED: on when pressed (momentary feel) + a tiny "always on" so se ve
		float shapeLed = clamp(params[SHAPE_PARAM].getValue(), 0.f, 1.f);
		lights[SHAPE_LED].setBrightness(0.15f + 0.85f * shapeLed);

		// - TRIGGER_MANUAL_LED: show current envelope amount (nice feedback)
		lights[TRIGGER_MANUAL_LED].setBrightness(clamp(env, 0.f, 1.f));
	}
};


struct TL_PumpWidget : ModuleWidget {
	TL_PumpWidget(TL_Pump* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/TL_Pump.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createLightParamCentered<VCVLightButton<LargeSimpleLight<WhiteLight>>>(mm2px(Vec(5.724, 31.038)), module, TL_Pump::SHAPE_PARAM, TL_Pump::SHAPE_LED));
		addParam(createLightParamCentered<VCVLightButton<LargeSimpleLight<WhiteLight>>>(mm2px(Vec(24.749, 31.072)), module, TL_Pump::TRIGGER_MANUAL_PARAM, TL_Pump::TRIGGER_MANUAL_LED));
		addParam(createParamCentered<SmallHSliderBool>(mm2px(Vec(15.097, 110.059)), module, TL_Pump::DRYWET_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.096, 25.578)), module, TL_Pump::TRIGGER_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.415, 81.202)), module, TL_Pump::IN_L_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.415, 92.827)), module, TL_Pump::IN_R_INPUT));

		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(21.985, 81.188)), module, TL_Pump::OUT_L_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(21.976, 92.871)), module, TL_Pump::OUT_R_OUTPUT));

		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(3.51, 42.014)), module, TL_Pump::LED_A_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(27.02, 42.004)), module, TL_Pump::LED_B_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(3.498, 68.468)), module, TL_Pump::LED_C_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(27.023, 68.479)), module, TL_Pump::LED_D_LIGHT));
	}
};

Model* modelTL_Pump = createModel<TL_Pump, TL_PumpWidget>("TL_Pump");
