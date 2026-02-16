#include "plugin.hpp"

struct TL_VULR : Module {
	enum InputId {
		IN_L_INPUT,
		IN_R_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUTPUTS_LEN
	};
	enum ParamId {
		PARAMS_LEN
	};
	enum LightId {
		// Interleaved L/R so we can index lights as (segment-1)*2 + channel
		LED_L_1_LIGHT,
		LED_R_1_LIGHT,
		LED_L_2_LIGHT,
		LED_R_2_LIGHT,
		LED_L_3_LIGHT,
		LED_R_3_LIGHT,
		LED_L_4_LIGHT,
		LED_R_4_LIGHT,
		LED_L_5_LIGHT,
		LED_R_5_LIGHT,
		LED_L_6_LIGHT,
		LED_R_6_LIGHT,
		LED_L_7_LIGHT,
		LED_R_7_LIGHT,
		LED_L_8_LIGHT,
		LED_R_8_LIGHT,
		LED_L_9_LIGHT,
		LED_R_9_LIGHT,
		LED_L_10_LIGHT,
		LED_R_10_LIGHT,
		LED_L_11_LIGHT,
		LED_R_11_LIGHT,
		LED_L_12_LIGHT,
		LED_R_12_LIGHT,
		LED_L_13_LIGHT,
		LED_R_13_LIGHT,
		LED_L_14_LIGHT,
		LED_R_14_LIGHT,
		LIGHTS_LEN
	};

	// Fast attack peak with exponential release
	float peakL = 0.f;
	float peakR = 0.f;
	float releaseCoeff = 0.9995f;

	static constexpr int kSegments = 14;
	static constexpr float kFullScaleV = 5.f; // VCV audio nominal peak (+/-5V), 10Vpp

	TL_VULR() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(IN_L_INPUT, "L");
		configInput(IN_R_INPUT, "R");
	}

	void onSampleRateChange() override {
		// ~300ms release (optimized: one coeff, no extra filters)
		const float releaseTimeS = 0.30f;
		releaseCoeff = std::exp(-1.f / (releaseTimeS * APP->engine->getSampleRate()));
	}

	void onReset() override {
		peakL = 0.f;
		peakR = 0.f;
	}

	void process(const ProcessArgs& args) override {
		const float inL = inputs[IN_L_INPUT].getVoltage();
		const float inR = inputs[IN_R_INPUT].getVoltage();

		const float aL = std::fabs(inL);
		const float aR = std::fabs(inR);

		// Peak follower: immediate rise, exponential fall
		peakL = (aL > peakL) ? aL : (peakL * releaseCoeff);
		peakR = (aR > peakR) ? aR : (peakR * releaseCoeff);

		const float levelL = clamp(peakL / kFullScaleV, 0.f, 1.f);
		const float levelR = clamp(peakR / kFullScaleV, 0.f, 1.f);

		// Light i is on if level >= i/kSegments
		for (int i = 1; i <= kSegments; ++i) {
			const float t = (float)i / (float)kSegments;
			const int base = (i - 1) * 2;

			lights[base + 0].setBrightness(levelL >= t ? 1.f : 0.f);
			lights[base + 1].setBrightness(levelR >= t ? 1.f : 0.f);
		}
	}
};

struct TL_VULRWidget : ModuleWidget {
	TL_VULRWidget(TL_VULR* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/TL_VULR.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.116, 99.129)), module, TL_VULR::IN_L_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.226, 107.559)), module, TL_VULR::IN_R_INPUT));

		addChild(createLightCentered<LargeLight<WhiteLight>>(mm2px(Vec(3.983, 88.849)), module, TL_VULR::LED_L_1_LIGHT));
		addChild(createLightCentered<LargeLight<WhiteLight>>(mm2px(Vec(11.392, 88.849)), module, TL_VULR::LED_R_1_LIGHT));

		addChild(createLightCentered<LargeLight<WhiteLight>>(mm2px(Vec(3.976, 80.901)), module, TL_VULR::LED_L_2_LIGHT));
		addChild(createLightCentered<LargeLight<WhiteLight>>(mm2px(Vec(11.385, 80.901)), module, TL_VULR::LED_R_2_LIGHT));

		addChild(createLightCentered<LargeLight<WhiteLight>>(mm2px(Vec(3.977, 72.952)), module, TL_VULR::LED_L_3_LIGHT));
		addChild(createLightCentered<LargeLight<WhiteLight>>(mm2px(Vec(11.385, 72.952)), module, TL_VULR::LED_R_3_LIGHT));

		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(3.496, 66.004)), module, TL_VULR::LED_L_4_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(11.969, 65.998)), module, TL_VULR::LED_R_4_LIGHT));

		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(2.961, 60.184)), module, TL_VULR::LED_L_5_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(12.497, 60.178)), module, TL_VULR::LED_R_5_LIGHT));

		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(2.965, 54.348)), module, TL_VULR::LED_L_6_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(12.49, 54.354)), module, TL_VULR::LED_R_6_LIGHT));

		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(2.597, 49.239)), module, TL_VULR::LED_L_7_LIGHT));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(12.724, 49.180)), module, TL_VULR::LED_R_7_LIGHT));

		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(2.459, 44.429)), module, TL_VULR::LED_L_8_LIGHT));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(12.954, 44.417)), module, TL_VULR::LED_R_8_LIGHT));

		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(2.457, 39.664)), module, TL_VULR::LED_L_9_LIGHT));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(12.964, 39.652)), module, TL_VULR::LED_R_9_LIGHT));

		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(2.215, 35.425)), module, TL_VULR::LED_L_10_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(13.222, 35.351)), module, TL_VULR::LED_R_10_LIGHT));

		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(1.938, 31.646)), module, TL_VULR::LED_L_11_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(13.453, 31.637)), module, TL_VULR::LED_R_11_LIGHT));

		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(1.931, 27.944)), module, TL_VULR::LED_L_12_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(13.447, 27.935)), module, TL_VULR::LED_R_12_LIGHT));

		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(1.934, 24.233)), module, TL_VULR::LED_L_13_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(13.452, 24.233)), module, TL_VULR::LED_R_13_LIGHT));

		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(1.934, 20.529)), module, TL_VULR::LED_L_14_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(13.446, 20.526)), module, TL_VULR::LED_R_14_LIGHT));

	}
};

Model* modelTL_VULR = createModel<TL_VULR, TL_VULRWidget>("TL_VULR");
