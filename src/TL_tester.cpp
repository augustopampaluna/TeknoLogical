#include "plugin.hpp"


struct TL_tester : Module {
	enum ParamId {
		KNOB1_PARAM,
		KNOB2_PARAM,
		KNOB3_PARAM,
		KNOB4_PARAM,
		KNOB5_PARAM,
		KNOB6_PARAM,
		KNOB7_PARAM,
		KNOB8_PARAM,
		KNOB9_PARAM,
		KNOB10_PARAM,
		KNOB11_PARAM,
		KNOB12_PARAM,
		KNOB13_PARAM,
		KNOB14_PARAM,
		KNOB15_PARAM,
		KNOB16_PARAM,
		KNOB17_PARAM,
		KNOB18_PARAM,
		KNOB19_PARAM,
		KNOB20_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		PARAM1_INPUT,
		PARAM2_INPUT,
		PARAM3_INPUT,
		PARAM4_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUTPUTS_LEN
	};
	enum LightId {
		LED1_LIGHT,
		LED2_LIGHT,
		LED3_LIGHT,
		LED4_LIGHT,
		LED5_LIGHT,
		LED6_LIGHT,
		LED7_LIGHT,
		LED8_LIGHT,
		LED9_LIGHT,
		LED10_LIGHT,
		LED11_LIGHT,
		LED12_LIGHT,
		LIGHTS_LEN
	};

	TL_tester() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(KNOB1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB5_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB6_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB7_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB8_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB9_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB10_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB11_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB13_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB14_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB15_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB16_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB17_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB18_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB19_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB20_PARAM, 0.f, 1.f, 0.f, "");

		configInput(PARAM1_INPUT, "");
		configInput(PARAM4_INPUT, "");
		configInput(PARAM3_INPUT, "");
		configInput(PARAM2_INPUT, "");
	}

	void process(const ProcessArgs& args) override {
		lights[LED1_LIGHT].setBrightness(1.f);
		lights[LED2_LIGHT].setBrightness(1.f);
		lights[LED3_LIGHT].setBrightness(1.f);
		lights[LED4_LIGHT].setBrightness(1.f);
		lights[LED5_LIGHT].setBrightness(1.f);
		lights[LED6_LIGHT].setBrightness(1.f);
		lights[LED7_LIGHT].setBrightness(1.f);
		lights[LED8_LIGHT].setBrightness(1.f);
		lights[LED9_LIGHT].setBrightness(1.f);
		lights[LED10_LIGHT].setBrightness(1.f);
		lights[LED11_LIGHT].setBrightness(1.f);
		lights[LED12_LIGHT].setBrightness(1.f);
	}
};


struct TL_testerWidget : ModuleWidget {
	TL_testerWidget(TL_tester* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/TL_tester.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


		// INPUTS
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.991, 8.157)), module, TL_tester::PARAM1_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(22.353, 8.588)), module, TL_tester::PARAM2_INPUT));
		addInput(createInputCentered<CL1362Port>(mm2px(Vec(35.515, 8.373)), module, TL_tester::PARAM3_INPUT));
		addInput(createInputCentered<PJ3410Port>(mm2px(Vec(49.296, 8.114)), module, TL_tester::PARAM4_INPUT));



		// KNOBS / SWITCHES (3 filas distintas)
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(8.5, 25.0)), module, TL_tester::KNOB1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.5, 25.0)), module, TL_tester::KNOB2_PARAM));
		addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(36.5, 25.0)), module, TL_tester::KNOB3_PARAM));
		addParam(createParamCentered<RoundHugeBlackKnob>(mm2px(Vec(50.5, 25.0)), module, TL_tester::KNOB4_PARAM));

		addParam(createParamCentered<CKSS>(mm2px(Vec(8.5, 40.0)), module, TL_tester::KNOB5_PARAM));
		addParam(createParamCentered<VCVSlider>(mm2px(Vec(22.5, 40.0)), module, TL_tester::KNOB6_PARAM));
		addParam(createParamCentered<CKD6>(mm2px(Vec(36.5, 40.0)), module, TL_tester::KNOB7_PARAM));
		addParam(createParamCentered<SynthTechAlco>(mm2px(Vec(50.5, 40.0)), module, TL_tester::KNOB8_PARAM));

		addParam(createParamCentered<TL1105>(mm2px(Vec(8.5, 55.0)), module, TL_tester::KNOB9_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(22.5, 55.0)), module, TL_tester::KNOB10_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(36.5, 55.0)), module, TL_tester::KNOB11_PARAM));
		addParam(createParamCentered<NKK>(mm2px(Vec(50.5, 55.0)), module, TL_tester::KNOB12_PARAM));

		addParam(createParamCentered<Davies1900hLargeWhiteKnob>(mm2px(Vec(8.5, 70.0)), module, TL_tester::KNOB13_PARAM));
		addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(22.5, 70.0)), module, TL_tester::KNOB14_PARAM));
		addParam(createParamCentered<PB61303>(mm2px(Vec(36.5, 70.0)), module, TL_tester::KNOB15_PARAM));
		addParam(createParamCentered<CKSSThreeHorizontal>(mm2px(Vec(50.5, 70.0)), module, TL_tester::KNOB16_PARAM));

		addParam(createParamCentered<BefacoTinyKnob>(mm2px(Vec(8.5, 85.0)), module, TL_tester::KNOB17_PARAM));
		addParam(createParamCentered<Rogan5PSGray>(mm2px(Vec(22.5, 85.0)), module, TL_tester::KNOB18_PARAM));
		addParam(createParamCentered<Rogan1PSWhite>(mm2px(Vec(36.5, 85.0)), module, TL_tester::KNOB19_PARAM));
		addParam(createParamCentered<Rogan1PWhite>(mm2px(Vec(50.5, 85.0)), module, TL_tester::KNOB20_PARAM));




		// LEDS (3 filas distintos tipos)
		addChild(createLightCentered<TinyLight<RedLight>>(mm2px(Vec(8.5, 100.0)), module, TL_tester::LED1_LIGHT));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(22.5, 100.0)), module, TL_tester::LED2_LIGHT));
		addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(36.5, 100.0)), module, TL_tester::LED3_LIGHT));
		addChild(createLightCentered<LargeLight<WhiteLight>>(mm2px(Vec(50.5, 100.0)), module, TL_tester::LED4_LIGHT));

		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(8.5, 110.0)), module, TL_tester::LED5_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(22.5, 110.0)), module, TL_tester::LED6_LIGHT));
		addChild(createLightCentered<LargeLight<GreenLight>>(mm2px(Vec(36.5, 110.0)), module, TL_tester::LED7_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(50.5, 110.0)), module, TL_tester::LED8_LIGHT));

		addChild(createLightCentered<LargeLight<BlueLight>>(mm2px(Vec(8.5, 120.0)), module, TL_tester::LED9_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(22.5, 120.0)), module, TL_tester::LED10_LIGHT));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(36.5, 120.0)), module, TL_tester::LED11_LIGHT));
		addChild(createLightCentered<TinyLight<GreenLight>>(mm2px(Vec(50.5, 120.0)), module, TL_tester::LED12_LIGHT));

	}
};



Model* modelTL_tester = createModel<TL_tester, TL_testerWidget>("TL-tester");