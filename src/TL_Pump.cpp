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

	// ---------- Runtime ----------
	int shapeIndex = 0;

	dsp::SchmittTrigger shapeTrig;
	dsp::SchmittTrigger manualTrig;
	dsp::SchmittTrigger inputTrig;

	bool envActive = false;
	float envTime = 0.f; // seconds since trigger
	static constexpr float minGain = 0.03f; // ~ -30 dB

	struct ShapeDef {
		// Shape 1/2/4: hold + release
		float hold;        // seconds (espera en el fondo)
		float release;     // seconds (subida)
		float releasePow;  // curva de subida (1 = lineal; <1 sube rápido y se aplana; >1 tarda y sube al final)

		// Shape 3: fade-out only
		float fade;        // seconds (1 -> 0)
		float fadePow;     // curva de caída
		bool  latchZero;   // si true: al llegar a 0 se queda ahí hasta el próximo trigger
	};

	ShapeDef shapes[4] = {
		// hold, release, releasePow, fade, fadePow, latchZero
		{ 0.040f, 0.280f, 0.70f,   0.000f, 1.0f,  false }, // 1: sube relativamente rápido al inicio y se aplana (parecido a “curva suave”)
		{ 0.040f, 0.200f, 2.40f,   0.000f, 1.0f,  false }, // 2: retorno “de golpe” (se queda bajo y pega el salto más hacia el final)
		{ 0.000f, 0.000f, 1.00f,   0.350f, 1.10f, true  }, // 3: fade-out y latch en cero
		{ 0.020f, 0.140f, 0.75f,   0.000f, 1.0f,  false }, // 4: release rápido
	};

	TL_Pump() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configParam(SHAPE_PARAM, 0.f, 1.f, 0.f, "Shape");
		configParam(TRIGGER_MANUAL_PARAM, 0.f, 1.f, 0.f, "Trigger");
		configParam(DRYWET_PARAM, 0.f, 1.f, 1.f, "Dry/Wet");

		configInput(TRIGGER_INPUT, "Trigger");
		configInput(IN_L_INPUT, "In L");
		configInput(IN_R_INPUT, "In R");

		configOutput(OUT_L_OUTPUT, "Out L");
		configOutput(OUT_R_OUTPUT, "Out R");
	}

	void startEnvelope() {
		envActive = true;
		envTime = 0.f;
	}

	float gainFromShape(int idx, float t) {
		const ShapeDef& s = shapes[idx];

		// ---- Shape 3: fade-out y latch ----
		if (idx == 2) {
			// Antes del trigger, envActive=false -> devolvemos 1 en process()
			float x = (s.fade > 0.f) ? (t / s.fade) : 1.f;
			x = clamp(x, 0.f, 1.f);

			// 1 -> 0 con curva
			float g = 1.f - std::pow(x, s.fadePow);
			g = clamp(g, 0.f, 1.f);

			// Latch en cero
			if (s.latchZero && x >= 1.f)
				return 0.f;

			return g;
		}

		// ---- Shapes 1/2/4: caída inmediata al mínimo, hold, luego release hacia 1 ----
		if (t <= s.hold) {
			return minGain;
		}

		float tr = t - s.hold;
		float x = (s.release > 0.f) ? (tr / s.release) : 1.f;
		x = clamp(x, 0.f, 1.f);

		// minGain -> 1 con curva
		float shaped = std::pow(x, s.releasePow);
		float g = minGain + (1.f - minGain) * shaped;
		return clamp(g, minGain, 1.f);
	}

	void process(const ProcessArgs& args) override {
		// --- Shape: ciclo 4 formas por click (flanco ascendente)
		if (shapeTrig.process(params[SHAPE_PARAM].getValue())) {
			shapeIndex = (shapeIndex + 1) & 3;
		}

		// --- Trigger: manual OR input
		bool trig = false;
		if (manualTrig.process(params[TRIGGER_MANUAL_PARAM].getValue()))
			trig = true;

		if (inputs[TRIGGER_INPUT].isConnected()) {
			if (inputTrig.process(inputs[TRIGGER_INPUT].getVoltage()))
				trig = true;
		}

		if (trig) {
			startEnvelope();
		}

		// --- Gain envelope
		float gain = 1.f;

		if (envActive) {
			gain = gainFromShape(shapeIndex, envTime);
			envTime += args.sampleTime;

			const ShapeDef& s = shapes[shapeIndex];

			// Auto-stop:
			if (shapeIndex == 2) {
				if (!s.latchZero && envTime > (s.fade + 0.01f)) {
					envActive = false;
					envTime = 0.f;
				}
				// si latchZero: no apagamos envActive; queda en 0 hasta nuevo trigger
			} else {
				if (envTime > (s.hold + s.release + 0.01f)) {
					envActive = false;
					envTime = 0.f;
					gain = 1.f;
				}
			}
		}

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

		// --- Shape indicator lights (A/B/C/D)
		lights[LED_A_LIGHT].setBrightness(shapeIndex == 0 ? 1.f : 0.f);
		lights[LED_B_LIGHT].setBrightness(shapeIndex == 1 ? 1.f : 0.f);
		lights[LED_C_LIGHT].setBrightness(shapeIndex == 2 ? 1.f : 0.f);
		lights[LED_D_LIGHT].setBrightness(shapeIndex == 3 ? 1.f : 0.f);

		// --- Button lights:
		lights[SHAPE_LED].setBrightness(clamp(params[SHAPE_PARAM].getValue(), 0.f, 1.f));

		// TRIGGER_MANUAL_LED: feedback del gain
		// 1 = máximo pump, 0 = sin pump
		float pumpAmt = 1.f - clamp(gain, 0.f, 1.f);
		lights[TRIGGER_MANUAL_LED].setBrightness(clamp(pumpAmt, 0.f, 1.f));
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
