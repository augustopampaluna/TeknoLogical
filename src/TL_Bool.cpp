#include "plugin.hpp"
#include "../helpers/widgets/sliders.hpp"


struct TL_Bool : Module {
	enum ParamId {
		PROBABILITY_PARAM,
		TEST_A_PARAM,
		TEST_B_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		JACK_A_INPUT,
		JACK_B_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_AND_OUTPUT,
		OUT_NAND_OUTPUT,
		OUT_OR_OUTPUT,
		OUT_NOR_OUTPUT,
		OUT_XOR_OUTPUT,
		OUT_XNOR_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LED_A_LIGHT,
		LED_B_LIGHT,

		LED_AND_LIGHT,
		LED_NAND_LIGHT,
		LED_OR_LIGHT,
		LED_NOR_LIGHT,
		LED_XOR_LIGHT,
		LED_XNOR_LIGHT,

		LIGHTS_LEN
	};

	bool prevRaw[OUTPUTS_LEN] = {};
	bool passLatch[OUTPUTS_LEN] = { true, true, true, true, true, true };

	TL_Bool() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		// PROBABILITY: default 100%
		configParam(PROBABILITY_PARAM, 0.f, 1.f, 1.f, "Probability", "%", 0.f, 100.f);

		// Testing switches A/B: 0/1
		configSwitch(TEST_A_PARAM, 0.f, 1.f, 0.f, "Test A", {"Off", "On"});
		configSwitch(TEST_B_PARAM, 0.f, 1.f, 0.f, "Test B", {"Off", "On"});

		configInput(JACK_A_INPUT, "A");
		configInput(JACK_B_INPUT, "B");

		configOutput(OUT_AND_OUTPUT, "AND");
		configOutput(OUT_NAND_OUTPUT, "NAND");
		configOutput(OUT_OR_OUTPUT, "OR");
		configOutput(OUT_NOR_OUTPUT, "NOR");
		configOutput(OUT_XOR_OUTPUT, "XOR");
		configOutput(OUT_XNOR_OUTPUT, "XNOR");
	}

	bool applyProbabilityLatch(int outId, bool rawTrue, float p) {
		if (rawTrue && !prevRaw[outId]) {
			passLatch[outId] = (random::uniform() < p);
		}

		if (!rawTrue) {
			passLatch[outId] = true;
		}

		prevRaw[outId] = rawTrue;
		return rawTrue && passLatch[outId];
	}

	void process(const ProcessArgs& args) override {
		// Standard gate threshold
		constexpr float gateThreshold = 1.0f;

		// Test switches (absolute priority)
		const bool swA = params[TEST_A_PARAM].getValue() > 0.5f;
		const bool swB = params[TEST_B_PARAM].getValue() > 0.5f;

		// Jack inputs (only used if the switch is OFF)
		const bool jackA = inputs[JACK_A_INPUT].isConnected()
			&& (inputs[JACK_A_INPUT].getVoltage() >= gateThreshold);
		const bool jackB = inputs[JACK_B_INPUT].isConnected()
			&& (inputs[JACK_B_INPUT].getVoltage() >= gateThreshold);

		// Final logical inputs:
		// switch ON forces TRUE, otherwise the jack defines the state
		const bool A = swA || jackA;
		const bool B = swB || jackB;

		// Tiny LEDs: indicate ONLY the switch state
		lights[LED_A_LIGHT].setBrightness(swA ? 1.f : 0.f);
		lights[LED_B_LIGHT].setBrightness(swB ? 1.f : 0.f);

		// Raw logic evaluation
		const bool rawAND  = (A && B);
		const bool rawNAND = !rawAND;
		const bool rawOR   = (A || B);
		const bool rawNOR  = !rawOR;
		const bool rawXOR  = (A != B);
		const bool rawXNOR = !rawXOR;

		// Probability amount (0..1)
		const float p = clamp(params[PROBABILITY_PARAM].getValue(), 0.f, 1.f);

		// Apply probability on rising edges only (latched per output)
		const bool outAND  = applyProbabilityLatch(OUT_AND_OUTPUT,  rawAND,  p);
		const bool outNAND = applyProbabilityLatch(OUT_NAND_OUTPUT, rawNAND, p);
		const bool outOR   = applyProbabilityLatch(OUT_OR_OUTPUT,   rawOR,   p);
		const bool outNOR  = applyProbabilityLatch(OUT_NOR_OUTPUT,  rawNOR,  p);
		const bool outXOR  = applyProbabilityLatch(OUT_XOR_OUTPUT,  rawXOR,  p);
		const bool outXNOR = applyProbabilityLatch(OUT_XNOR_OUTPUT, rawXNOR, p);

		// Helper: set gate voltage and corresponding LED
		auto setGate = [&](int outId, int lightId, bool high) {
			outputs[outId].setVoltage(high ? 10.f : 0.f);
			lights[lightId].setBrightness(high ? 1.f : 0.f);
		};

		setGate(OUT_AND_OUTPUT,  LED_AND_LIGHT,  outAND);
		setGate(OUT_NAND_OUTPUT, LED_NAND_LIGHT, outNAND);
		setGate(OUT_OR_OUTPUT,   LED_OR_LIGHT,   outOR);
		setGate(OUT_NOR_OUTPUT,  LED_NOR_LIGHT,  outNOR);
		setGate(OUT_XOR_OUTPUT,  LED_XOR_LIGHT,  outXOR);
		setGate(OUT_XNOR_OUTPUT, LED_XNOR_LIGHT, outXNOR);
	}
};

struct TL_BoolWidget : ModuleWidget {
	TL_BoolWidget(TL_Bool* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/TL_Bool.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


		addParam(createParamCentered<SmallHSliderBool>(mm2px(Vec(15.1, 98.464 + 11.113)), module, TL_Bool::PROBABILITY_PARAM));

		addParam(createParamCentered<NKK>(mm2px(Vec(7.592, 16.478 + 11.113)), module, TL_Bool::TEST_A_PARAM));
		addParam(createParamCentered<NKK>(mm2px(Vec(22.657, 16.505 + 11.113)), module, TL_Bool::TEST_B_PARAM));


		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.367, 33.455 + 11.113)), module, TL_Bool::JACK_A_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.077, 33.469 + 11.113)), module, TL_Bool::JACK_B_INPUT));


		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(7.358, 50.603 + 11.113)), module, TL_Bool::OUT_AND_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(23.123, 50.51 + 11.113)), module, TL_Bool::OUT_NAND_OUTPUT));

		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(7.377, 65.343 + 11.113)), module, TL_Bool::OUT_OR_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(23.142, 65.325 + 11.113)), module, TL_Bool::OUT_NOR_OUTPUT));

		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(7.358, 80.232 + 11.113)), module, TL_Bool::OUT_XOR_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(23.123, 80.177 + 11.113)), module, TL_Bool::OUT_XNOR_OUTPUT));


		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(7.617, 11.119 + 11.113)), module, TL_Bool::LED_A_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(22.67, 11.071 + 11.113)), module, TL_Bool::LED_B_LIGHT));

		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(12.13, 57.482 + 11.113)), module, TL_Bool::LED_AND_LIGHT));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(18.053, 57.483 + 11.113)), module, TL_Bool::LED_NAND_LIGHT));

		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(12.133, 72.3 + 11.113)), module, TL_Bool::LED_OR_LIGHT));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(18.052, 72.3 + 11.113)), module, TL_Bool::LED_NOR_LIGHT));

		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(12.133, 87.114 + 11.113)), module, TL_Bool::LED_XOR_LIGHT));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(18.054, 87.116 + 11.113)), module, TL_Bool::LED_XNOR_LIGHT));
	}
};

Model* modelTL_Bool = createModel<TL_Bool, TL_BoolWidget>("TL_Bool");
