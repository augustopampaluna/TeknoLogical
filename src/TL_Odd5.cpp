#include "plugin.hpp"


struct TL_Odd5 : Module {
	enum ParamId {
		KNOB_31_PARAM,
		KNOB_32_PARAM,
		KNOB_33_PARAM,
		KNOB_51_PARAM,
		KNOB_52_PARAM,
		KNOB_53_PARAM,
		KNOB_54_PARAM,
		KNOB_55_PARAM,
		KNOB_71_PARAM,
		KNOB_72_PARAM,
		KNOB_73_PARAM,
		KNOB_74_PARAM,
		KNOB_75_PARAM,
		KNOB_76_PARAM,
		KNOB_77_PARAM,
		
		RANDOM_PARAM,
		SPREAD_PARAM,
		
		PARAMS_LEN
	};
	enum InputId {
		IN_3_INPUT,
		IN_5_INPUT,
		IN_7_INPUT,
		
		INPUTS_LEN
	};
	enum OutputId {
		OUT_3_OUTPUT,
		OUT_5_OUTPUT,
		OUT_7_OUTPUT,
		
		OUTPUTS_LEN
	};
	enum LightId {
		LED_31_LIGHT,
		LED_32_LIGHT,
		LED_33_LIGHT,
		LED_51_LIGHT,
		LED_52_LIGHT,
		LED_53_LIGHT,
		LED_54_LIGHT,
		LED_55_LIGHT,
		LED_71_LIGHT,
		LED_72_LIGHT,
		LED_73_LIGHT,
		LED_74_LIGHT,
		LED_75_LIGHT,
		LED_76_LIGHT,
		LED_77_LIGHT,
		
		LIGHTS_LEN
	};

	int currentStep3 = 0; // 0..2
	int currentStep5 = 0; // 0..4
	int currentStep7 = 0; // 0..6

	dsp::SchmittTrigger trig3;
	dsp::SchmittTrigger trig5;
	dsp::SchmittTrigger trig7;

	TL_Odd5() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		// Params (alfabético)
		configParam(KNOB_31_PARAM, -1.f, 1.f, 0.f, "Step 1");
		configParam(KNOB_32_PARAM, -1.f, 1.f, 0.f, "Step 2");
		configParam(KNOB_33_PARAM, -1.f, 1.f, 0.f, "Step 3");

		configParam(KNOB_51_PARAM, -1.f, 1.f, 0.f, "Step 1");
		configParam(KNOB_52_PARAM, -1.f, 1.f, 0.f, "Step 2");
		configParam(KNOB_53_PARAM, -1.f, 1.f, 0.f, "Step 3");
		configParam(KNOB_54_PARAM, -1.f, 1.f, 0.f, "Step 4");
		configParam(KNOB_55_PARAM, -1.f, 1.f, 0.f, "Step 5");

		configParam(KNOB_71_PARAM, -1.f, 1.f, 0.f, "Step 1");
		configParam(KNOB_72_PARAM, -1.f, 1.f, 0.f, "Step 2");
		configParam(KNOB_73_PARAM, -1.f, 1.f, 0.f, "Step 3");
		configParam(KNOB_74_PARAM, -1.f, 1.f, 0.f, "Step 4");
		configParam(KNOB_75_PARAM, -1.f, 1.f, 0.f, "Step 5");
		configParam(KNOB_76_PARAM, -1.f, 1.f, 0.f, "Step 6");
		configParam(KNOB_77_PARAM, -1.f, 1.f, 0.f, "Step 7");

		configParam(SPREAD_PARAM, 1.f, 5.f, 1.f, "Spread");
		configParam(RANDOM_PARAM, 0.f, 1.f, 0.f, "Random");

		// Inputs
		configInput(IN_3_INPUT, "Seq3");
		configInput(IN_5_INPUT, "Seq5");
		configInput(IN_7_INPUT, "Seq7");

		// Outputs
		configOutput(OUT_3_OUTPUT, "Seq3");
		configOutput(OUT_5_OUTPUT, "Seq5");
		configOutput(OUT_7_OUTPUT, "Seq7");
	}

	void process(const ProcessArgs& args) override {
		const float spread = params[SPREAD_PARAM].getValue();          // 0..9
		const bool randomMode = (params[RANDOM_PARAM].getValue() > 0.5f);

		// Mapeos (paramId por paso) para leer rápido el knob correspondiente al step actual
		static const int seq3Params[3] = { KNOB_31_PARAM, KNOB_32_PARAM, KNOB_33_PARAM };
		static const int seq5Params[5] = { KNOB_51_PARAM, KNOB_52_PARAM, KNOB_53_PARAM, KNOB_54_PARAM, KNOB_55_PARAM };
		static const int seq7Params[7] = { KNOB_71_PARAM, KNOB_72_PARAM, KNOB_73_PARAM, KNOB_74_PARAM, KNOB_75_PARAM, KNOB_76_PARAM, KNOB_77_PARAM };

		// LEDs asociados (lightId por paso)
		static const int seq3Leds[3] = { LED_31_LIGHT, LED_32_LIGHT, LED_33_LIGHT };
		static const int seq5Leds[5] = { LED_51_LIGHT, LED_52_LIGHT, LED_53_LIGHT, LED_54_LIGHT, LED_55_LIGHT };
		static const int seq7Leds[7] = { LED_71_LIGHT, LED_72_LIGHT, LED_73_LIGHT, LED_74_LIGHT, LED_75_LIGHT, LED_76_LIGHT, LED_77_LIGHT };

		// -------------------- Advance steps on triggers --------------------
		if (inputs[IN_3_INPUT].isConnected()) {
			if (trig3.process(inputs[IN_3_INPUT].getVoltage())) {
				if (randomMode) currentStep3 = (int) (random::u32() % 3);
				else currentStep3 = (currentStep3 + 1) % 3;
			}
		}

		if (inputs[IN_5_INPUT].isConnected()) {
			if (trig5.process(inputs[IN_5_INPUT].getVoltage())) {
				if (randomMode) currentStep5 = (int) (random::u32() % 5);
				else currentStep5 = (currentStep5 + 1) % 5;
			}
		}

		if (inputs[IN_7_INPUT].isConnected()) {
			if (trig7.process(inputs[IN_7_INPUT].getVoltage())) {
				if (randomMode) currentStep7 = (int) (random::u32() % 7);
				else currentStep7 = (currentStep7 + 1) % 7;
			}
		}

		// -------------------- Outputs (CV) --------------------
		// Cada knob está en -1..+1, lo escalamos por SPREAD => -spread..+spread volts
		const float out3 = params[seq3Params[currentStep3]].getValue() * spread;
		const float out5 = params[seq5Params[currentStep5]].getValue() * spread;
		const float out7 = params[seq7Params[currentStep7]].getValue() * spread;

		outputs[OUT_3_OUTPUT].setVoltage(out3);
		outputs[OUT_5_OUTPUT].setVoltage(out5);
		outputs[OUT_7_OUTPUT].setVoltage(out7);

		// -------------------- LEDs --------------------
		for (int i = 0; i < 3; i++)
			lights[seq3Leds[i]].setBrightnessSmooth(i == currentStep3 ? 1.f : 0.f, args.sampleTime);

		for (int i = 0; i < 5; i++)
			lights[seq5Leds[i]].setBrightnessSmooth(i == currentStep5 ? 1.f : 0.f, args.sampleTime);

		for (int i = 0; i < 7; i++)
			lights[seq7Leds[i]].setBrightnessSmooth(i == currentStep7 ? 1.f : 0.f, args.sampleTime);
	}

};



struct TL_Odd5Widget : ModuleWidget {
	TL_Odd5Widget(TL_Odd5* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/TL_Odd5.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Rogan1PWhite>(mm2px(Vec(10.15, 15.19)), module, TL_Odd5::SPREAD_PARAM));
		addParam(createParamCentered<NKK>(mm2px(Vec(50.629, 15.184)), module, TL_Odd5::RANDOM_PARAM));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(20.382, 29.606)), module, TL_Odd5::KNOB_31_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(30.437, 28.548)), module, TL_Odd5::KNOB_32_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(40.491, 29.606)), module, TL_Odd5::KNOB_33_PARAM));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(12.445, 53.419)), module, TL_Odd5::KNOB_51_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(21.441, 52.36)), module, TL_Odd5::KNOB_52_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(30.437, 51.302)), module, TL_Odd5::KNOB_53_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(39.432, 52.36)), module, TL_Odd5::KNOB_54_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(48.428, 53.419)), module, TL_Odd5::KNOB_55_PARAM));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(5.037, 78.29)), module, TL_Odd5::KNOB_71_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(13.503, 76.702)), module, TL_Odd5::KNOB_72_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(21.97, 75.644)), module, TL_Odd5::KNOB_73_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(30.437, 74.585)), module, TL_Odd5::KNOB_74_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(38.903, 75.644)), module, TL_Odd5::KNOB_75_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(47.37, 76.702)), module, TL_Odd5::KNOB_76_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(55.837, 78.29)), module, TL_Odd5::KNOB_77_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(19.424, 87.195)), module, TL_Odd5::IN_3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.774, 97.223)), module, TL_Odd5::IN_5_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.039, 107.268)), module, TL_Odd5::IN_7_INPUT));

		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(41.86, 87.409)), module, TL_Odd5::OUT_3_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(46.457, 97.364)), module, TL_Odd5::OUT_5_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(51.236, 107.484)), module, TL_Odd5::OUT_7_OUTPUT));

		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(23.63, 19.822)), module, TL_Odd5::LED_31_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(30.509, 18.764)), module, TL_Odd5::LED_32_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(37.388, 19.822)), module, TL_Odd5::LED_33_LIGHT));

		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(16.75, 43.635)), module, TL_Odd5::LED_51_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(23.63, 42.577)), module, TL_Odd5::LED_52_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(30.509, 41.518)), module, TL_Odd5::LED_53_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(37.388, 42.577)), module, TL_Odd5::LED_54_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(44.267, 43.635)), module, TL_Odd5::LED_55_LIGHT));

		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(9.871, 68.506)), module, TL_Odd5::LED_71_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(16.75, 66.918)), module, TL_Odd5::LED_72_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(23.63, 65.86)), module, TL_Odd5::LED_73_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(30.509, 64.802)), module, TL_Odd5::LED_74_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(37.388, 65.86)), module, TL_Odd5::LED_75_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(44.267, 66.918)), module, TL_Odd5::LED_76_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(51.146, 68.506)), module, TL_Odd5::LED_77_LIGHT));
	}
};


Model* modelTL_Odd5 = createModel<TL_Odd5, TL_Odd5Widget>("TL_Odd5");