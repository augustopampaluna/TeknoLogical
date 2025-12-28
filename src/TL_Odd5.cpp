#include "plugin.hpp"


struct TL_Odd5 : Module {
	enum ParamId {
		SPREAD_PARAM,
		RANDOM_PARAM,
		KNOB_32_PARAM,
		KNOB_31_PARAM,
		KNOB_33_PARAM,
		KNOB_53_PARAM,
		KNOB_52_PARAM,
		KNOB_54_PARAM,
		KNOB_51_PARAM,
		KNOB_55_PARAM,
		KNOB_74_PARAM,
		KNOB_73_PARAM,
		KNOB_75_PARAM,
		KNOB_72_PARAM,
		KNOB_76_PARAM,
		KNOB_71_PARAM,
		KNOB_77_PARAM,
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
		LED_32_LIGHT,
		LED_31_LIGHT,
		LED_33_LIGHT,
		LED_53_LIGHT,
		LED_52_LIGHT,
		LED_54_LIGHT,
		LED_51_LIGHT,
		LED_55_LIGHT,
		LED_74_LIGHT,
		LED_73_LIGHT,
		LED_75_LIGHT,
		LED_72_LIGHT,
		LED_76_LIGHT,
		LED_71_LIGHT,
		LED_77_LIGHT,
		LIGHTS_LEN
	};

	TL_Odd5() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(SPREAD_PARAM, 0.f, 1.f, 0.f, "");
		configParam(RANDOM_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB_32_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB_31_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB_33_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB_53_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB_52_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB_54_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB_51_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB_55_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB_74_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB_73_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB_75_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB_72_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB_76_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB_71_PARAM, 0.f, 1.f, 0.f, "");
		configParam(KNOB_77_PARAM, 0.f, 1.f, 0.f, "");
		configInput(IN_3_INPUT, "");
		configInput(IN_5_INPUT, "");
		configInput(IN_7_INPUT, "");
		configOutput(OUT_3_OUTPUT, "");
		configOutput(OUT_5_OUTPUT, "");
		configOutput(OUT_7_OUTPUT, "");
	}

	void process(const ProcessArgs& args) override {
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