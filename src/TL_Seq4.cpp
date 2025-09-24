#include "plugin.hpp"
#include "../helpers/messages.hpp"


extern Model* modelTL_Reseter;

// Main module class for a dual trigger sequencer (A: 4/8 steps, B: 8/16 steps).
struct TL_Seq4 : Module {
// --------------------   UI enums / parameter & I/O indices  --------------------
	enum ParamId {
		LENGTH_1_PARAM,
		REVERSE_1_PARAM,

		STEP_A1_PARAM,
		STEP_A2_PARAM,
		STEP_A3_PARAM,
		STEP_A4_PARAM,
		STEP_A5_PARAM,
		STEP_A6_PARAM,
		STEP_A7_PARAM,
		STEP_A8_PARAM,

		LENGTH_2_PARAM,
		REVERSE_2_PARAM,

		STEP_B1_PARAM,
		STEP_B2_PARAM,
		STEP_B3_PARAM,
		STEP_B4_PARAM,
		STEP_B5_PARAM,
		STEP_B6_PARAM,
		STEP_B7_PARAM,
		STEP_B8_PARAM,
		STEP_B9_PARAM,
		STEP_B10_PARAM,
		STEP_B11_PARAM,
		STEP_B12_PARAM,
		STEP_B13_PARAM,
		STEP_B14_PARAM,
		STEP_B15_PARAM,
		STEP_B16_PARAM,

		PARAMS_LEN
	};
	enum InputId {
		IN_STEP_1_INPUT,
		LENGTH_1_INPUT,
		REVERSE_1_INPUT,

		IN_STEP_2_INPUT,
		LENGTH_2_INPUT,
		REVERSE_2_INPUT,

		INPUTS_LEN
	};
	enum OutputId {
		OUT_1_OUTPUT,
		OUT_2_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		// A ring LEDs
		LED_A1_LIGHT,
		LED_A2_LIGHT,
		LED_A3_LIGHT,
		LED_A4_LIGHT,
		LED_A5_LIGHT,
		LED_A6_LIGHT,
		LED_A7_LIGHT,
		LED_A8_LIGHT,

		// A step latch LEDs
		STEP_A1_LED,
		STEP_A2_LED,
		STEP_A3_LED,
		STEP_A4_LED,
		STEP_A5_LED,
		STEP_A6_LED,
		STEP_A7_LED,
		STEP_A8_LED,

		// A mini activity LEDs
		MINILED_A1_LIGHT,
		MINILED_A2_LIGHT,
		MINILED_A3_LIGHT,
		MINILED_A4_LIGHT,
		MINILED_A5_LIGHT,
		MINILED_A6_LIGHT,
		MINILED_A7_LIGHT,
		MINILED_A8_LIGHT,

		// B ring LEDs
		LED_B1_LIGHT,
		LED_B2_LIGHT,
		LED_B3_LIGHT,
		LED_B4_LIGHT,
		LED_B5_LIGHT,
		LED_B6_LIGHT,
		LED_B7_LIGHT,
		LED_B8_LIGHT,
		LED_B9_LIGHT,
		LED_B10_LIGHT,
		LED_B11_LIGHT,
		LED_B12_LIGHT,
		LED_B13_LIGHT,
		LED_B14_LIGHT,
		LED_B15_LIGHT,
		LED_B16_LIGHT,
		
		// B step latch LEDs
		STEP_B1_LED,
		STEP_B2_LED,
		STEP_B3_LED,
		STEP_B4_LED,
		STEP_B5_LED,
		STEP_B6_LED,
		STEP_B7_LED,
		STEP_B8_LED,
		STEP_B9_LED,
		STEP_B10_LED,
		STEP_B11_LED,
		STEP_B12_LED,
		STEP_B13_LED,
		STEP_B14_LED,
		STEP_B15_LED,
		STEP_B16_LED,

		// B mini activity LEDs
		MINILED_B1_LIGHT,
		MINILED_B2_LIGHT,
		MINILED_B3_LIGHT,
		MINILED_B4_LIGHT,
		MINILED_B5_LIGHT,
		MINILED_B6_LIGHT,
		MINILED_B7_LIGHT,
		MINILED_B8_LIGHT,
		MINILED_B9_LIGHT,
		MINILED_B10_LIGHT,
		MINILED_B11_LIGHT,
		MINILED_B12_LIGHT,
		MINILED_B13_LIGHT,
		MINILED_B14_LIGHT,
		MINILED_B15_LIGHT,
		MINILED_B16_LIGHT,

		LIGHTS_LEN
	};

// --------------------   Runtime state / edge detectors  ------------------------
	dsp::BooleanTrigger resetATrigger;
	dsp::BooleanTrigger resetBTrigger;

	// Sequencer A state.
	bool input_a = false;
	bool latch_a[8];
	bool length_a = false, reverse_a = false;
	bool cv_len_a = false, cv_rev_a = false;
	int currentStepA = 0;
	dsp::SchmittTrigger clockTriggerA;
	dsp::PulseGenerator gatePulseA;
	bool reverseA = false, lengthA = false;
	bool reverse_a_state = false;              // Current reverse switch state (A)
	dsp::SchmittTrigger reverse_a_cv_trigger;  // Edge detector for reverse CV (A)
	bool manual_reverse_a = false;
	bool length_a_state = false;               // Current length switch state (A)
	dsp::SchmittTrigger length_a_cv_trigger;   // Edge detector for length CV (A)
	bool manual_length_a = false;

	
	// Sequencer B state.
	bool input_b = false;
	bool latch_b[16];
	bool length_b = false, reverse_b = false;
	bool cv_len_b = false, cv_rev_b = false;
	int currentStepB = 0;
	dsp::SchmittTrigger clockTriggerB;
	dsp::PulseGenerator gatePulseB;
	bool reverseB = false, lengthB = false;
	bool reverse_b_state = false;              // Current reverse switch state (B)
	dsp::SchmittTrigger reverse_b_cv_trigger;  // Edge detector for reverse CV (B)
	bool manual_reverse_b = false;
	bool length_b_state = false;               // Current length switch state (B)
	dsp::SchmittTrigger length_b_cv_trigger;   // Edge detector for length CV (B)
	bool manual_length_b = false;
	
	int totalStepsA, totalStepsB;
	bool latchStates[24];
	float gateOut;

	// --- Expander: input pulses from neighbor modules (TL_Reseter) -------------
    bool resetAPulse = false;
    bool resetBPulse = false;

	ReseterMessage leftBuf[2];
	ReseterMessage rightBuf[2];

	// Read expander messages from left/right neighbors and convert to 1-frame pulses.
	inline void readExpanderResets() {
		bool a = false, b = false;

		// Read producer messages directly (ensures catching pulses even if flips are desynced).
		if (leftExpander.module && leftExpander.module->model == modelTL_Reseter) {
			auto* p = (ReseterMessage*) leftExpander.module->rightExpander.producerMessage;
			if (p) { a |= p->aGate; b |= p->bGate; }
		}
		if (rightExpander.module && rightExpander.module->model == modelTL_Reseter) {
			auto* p = (ReseterMessage*) rightExpander.module->leftExpander.producerMessage;
			if (p) { a |= p->aGate; b |= p->bGate; }
		}

		// Also read my consumer messages and request a flip (official expander path).
		if (leftExpander.module && leftExpander.module->model == modelTL_Reseter) {
			auto* c = (ReseterMessage*) leftExpander.consumerMessage;
			a |= c->aGate;
			b |= c->bGate;
			leftExpander.requestMessageFlip();
		} else {
			((ReseterMessage*) leftExpander.consumerMessage)->aGate = false;
			((ReseterMessage*) leftExpander.consumerMessage)->bGate = false;
		}

		if (rightExpander.module && rightExpander.module->model == modelTL_Reseter) {
			auto* c = (ReseterMessage*) rightExpander.consumerMessage;
			a |= c->aGate;
			b |= c->bGate;
			rightExpander.requestMessageFlip();
		} else {
			((ReseterMessage*) rightExpander.consumerMessage)->aGate = false;
			((ReseterMessage*) rightExpander.consumerMessage)->bGate = false;
		}

		// Convert combined levels into single-frame pulses.
		resetAPulse = resetATrigger.process(a);
		resetBPulse = resetBTrigger.process(b);
	}


// --------------------   Constructor / configuration  ---------------------------
	TL_Seq4() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		static const std::vector<std::string> on_off_labels = {"OFF", "ON"};
		
		// Sequencer A controls.
		static const std::vector<std::string> steps_labels_a = {"4", "8"};
		configSwitch(LENGTH_1_PARAM, 0.f, 1.f, 0.f, "Steps", steps_labels_a);
		configSwitch(REVERSE_1_PARAM, 0.f, 1.f, 0.f, "Reverse", on_off_labels);
		configInput(IN_STEP_1_INPUT, "Trigger A");
		configInput(LENGTH_1_INPUT, "CV Steps");
		configInput(REVERSE_1_INPUT, "CV Reverse");
		
		configSwitch(STEP_A1_PARAM, 0.f, 1.f, 0.f, "1", on_off_labels);
		configSwitch(STEP_A2_PARAM, 0.f, 1.f, 0.f, "2", on_off_labels);
		configSwitch(STEP_A3_PARAM, 0.f, 1.f, 0.f, "3", on_off_labels);
		configSwitch(STEP_A4_PARAM, 0.f, 1.f, 0.f, "4", on_off_labels);
		configSwitch(STEP_A5_PARAM, 0.f, 1.f, 0.f, "5", on_off_labels);
		configSwitch(STEP_A6_PARAM, 0.f, 1.f, 0.f, "6", on_off_labels);
		configSwitch(STEP_A7_PARAM, 0.f, 1.f, 0.f, "7", on_off_labels);
		configSwitch(STEP_A8_PARAM, 0.f, 1.f, 0.f, "8", on_off_labels);
		
		configOutput(OUT_1_OUTPUT, "Seq A");
		
		// Sequencer B controls.
		static const std::vector<std::string> steps_labels_b = {"8", "16"};
		configSwitch(LENGTH_2_PARAM, 0.f, 1.f, 0.f, "Steps", steps_labels_b);
		configSwitch(REVERSE_2_PARAM, 0.f, 1.f, 0.f, "Reverse", on_off_labels);
		configInput(IN_STEP_2_INPUT, "Trigger B");
		configInput(LENGTH_2_INPUT, "CV Steps");
		configInput(REVERSE_2_INPUT, "CV Reverse");
		
		configSwitch(STEP_B1_PARAM, 0.f, 1.f, 0.f, "1", on_off_labels);
		configSwitch(STEP_B2_PARAM, 0.f, 1.f, 0.f, "2", on_off_labels);
		configSwitch(STEP_B3_PARAM, 0.f, 1.f, 0.f, "3", on_off_labels);
		configSwitch(STEP_B4_PARAM, 0.f, 1.f, 0.f, "4", on_off_labels);
		configSwitch(STEP_B5_PARAM, 0.f, 1.f, 0.f, "5", on_off_labels);
		configSwitch(STEP_B6_PARAM, 0.f, 1.f, 0.f, "6", on_off_labels);
		configSwitch(STEP_B7_PARAM, 0.f, 1.f, 0.f, "7", on_off_labels);
		configSwitch(STEP_B8_PARAM, 0.f, 1.f, 0.f, "8", on_off_labels);
		configSwitch(STEP_B9_PARAM, 0.f, 1.f, 0.f, "9", on_off_labels);
		configSwitch(STEP_B10_PARAM, 0.f, 1.f, 0.f, "10", on_off_labels);
		configSwitch(STEP_B11_PARAM, 0.f, 1.f, 0.f, "11", on_off_labels);
		configSwitch(STEP_B12_PARAM, 0.f, 1.f, 0.f, "12", on_off_labels);
		configSwitch(STEP_B13_PARAM, 0.f, 1.f, 0.f, "13", on_off_labels);
		configSwitch(STEP_B14_PARAM, 0.f, 1.f, 0.f, "14", on_off_labels);
		configSwitch(STEP_B15_PARAM, 0.f, 1.f, 0.f, "15", on_off_labels);
		configSwitch(STEP_B16_PARAM, 0.f, 1.f, 0.f, "16", on_off_labels);
			
		configOutput(OUT_2_OUTPUT, "Seq B");

		// Expander: assign message buffers for this module.
		leftExpander.producerMessage  = &leftBuf[0];
		leftExpander.consumerMessage  = &leftBuf[1];
		rightExpander.producerMessage = &rightBuf[0];
		rightExpander.consumerMessage = &rightBuf[1];
	}

// --------------------   Helpers (LEDs / inputs / indicators)  ------------------
	// Write latched step LEDs for both sequencers (A: 8, B: 16).
	void setStepsLeds(Module* module, const bool latch_a[], int len_a, const bool latch_b[], int len_b) {
		const int stepLEDs[] = {
			STEP_A1_LED, STEP_A2_LED, STEP_A3_LED, STEP_A4_LED, STEP_A5_LED, STEP_A6_LED, STEP_A7_LED, STEP_A8_LED,
			STEP_B1_LED, STEP_B2_LED, STEP_B3_LED, STEP_B4_LED, STEP_B5_LED, STEP_B6_LED, STEP_B7_LED, STEP_B8_LED,
			STEP_B9_LED, STEP_B10_LED, STEP_B11_LED, STEP_B12_LED, STEP_B13_LED, STEP_B14_LED, STEP_B15_LED, STEP_B16_LED,
		};

		// A latch LEDs (8)
		for (int i = 0; i < len_a; ++i) {
			module->lights[stepLEDs[i]].setBrightness(latch_a[i] ? 1.0f : 0.0f);
		}
		// B latch LEDs (16)
		for (int i = 0; i < len_b; ++i) {
			module->lights[stepLEDs[len_a + i]].setBrightness(latch_b[i] ? 1.0f : 0.0f);
		}
	}
	
	// Read user controls and CVs, update derived states (length/reverse toggles, clocks).
	void updateAllInputStates() {
		// A steps (8 toggles)
		ParamId stepsParamsA[] = {
			STEP_A1_PARAM, STEP_A2_PARAM, STEP_A3_PARAM, STEP_A4_PARAM,
			STEP_A5_PARAM, STEP_A6_PARAM, STEP_A7_PARAM, STEP_A8_PARAM
		};

		for (int i = 0; i < 8; i++) {
			latch_a[i] = params[stepsParamsA[i]].getValue() == 1.0f;
		}

		input_a   = inputs[IN_STEP_1_INPUT].getVoltage() >= 1.0f;
		
		// A length (CV edge toggles switch)
		length_a  = params[LENGTH_1_PARAM].getValue() == 1.0f;
		cv_len_a  = inputs[LENGTH_1_INPUT].getVoltage() >= 1.0f;
		if (length_a_state != length_a) {
			length_a_state = length_a;
		}
		if (length_a_cv_trigger.process(inputs[LENGTH_1_INPUT].getVoltage())) {
			length_a_state = !length_a_state;
		}
		
		// Reflect current logical state in the visual switch.
		params[LENGTH_1_PARAM].setValue(length_a_state ? 1.0f : 0.0f);
		lengthA = length_a_state;


		// A reverse (CV edge toggles switch)
		reverse_a = params[REVERSE_1_PARAM].getValue() == 1.0f;
		cv_rev_a  = inputs[REVERSE_1_INPUT].getVoltage() >= 1.0f;
		if (reverse_a_state != reverse_a) {
			reverse_a_state = reverse_a;
		}
		if (reverse_a_cv_trigger.process(inputs[REVERSE_1_INPUT].getVoltage())) {
			reverse_a_state = !reverse_a_state;
		}
		
		// Reflect current logical state in the visual switch.
		params[REVERSE_1_PARAM].setValue(reverse_a_state ? 1.0f : 0.0f);
		reverseA = reverse_a_state;

		// B steps (16 toggles)
		ParamId stepsParamsB[] = {
			STEP_B1_PARAM, STEP_B2_PARAM, STEP_B3_PARAM, STEP_B4_PARAM,
			STEP_B5_PARAM, STEP_B6_PARAM, STEP_B7_PARAM, STEP_B8_PARAM,
			STEP_B9_PARAM, STEP_B10_PARAM, STEP_B11_PARAM, STEP_B12_PARAM,
			STEP_B13_PARAM, STEP_B14_PARAM, STEP_B15_PARAM, STEP_B16_PARAM
		};

		for (int i = 0; i < 16; i++) {
			latch_b[i] = params[stepsParamsB[i]].getValue() == 1.0f;
		}

		input_b   = inputs[IN_STEP_2_INPUT].getVoltage() >= 1.0f;

		// B length (CV edge toggles switch)
		length_b  = params[LENGTH_2_PARAM].getValue() == 1.0f;
		cv_len_b  = inputs[LENGTH_2_INPUT].getVoltage() >= 1.0f;
		if (length_b_state != length_b) {
			length_b_state = length_b;
		}
		if (length_b_cv_trigger.process(inputs[LENGTH_2_INPUT].getVoltage())) {
			length_b_state = !length_b_state;
		}
		
		// Reflect current logical state in the visual switch.
		params[LENGTH_2_PARAM].setValue(length_b_state ? 1.0f : 0.0f);
		lengthB = length_b_state;

		// B reverse (CV edge toggles switch)
		reverse_b = params[REVERSE_2_PARAM].getValue() == 1.0f;
		cv_rev_b  = inputs[REVERSE_2_INPUT].getVoltage() >= 1.0f;
		if (reverse_b_state != reverse_b) {
			reverse_b_state = reverse_b;
		}
		if (reverse_b_cv_trigger.process(inputs[REVERSE_2_INPUT].getVoltage())) {
			reverse_b_state = !reverse_b_state;
		}
		params[REVERSE_2_PARAM].setValue(reverse_b_state ? 1.0f : 0.0f);
		reverseB = reverse_b_state;

	}

	// Update both step indicator rings (current step highlight).
	void updateLedRings(const int currentSteps[2]) {
		const int ledLeds[2][16] = {
			{
				LED_A1_LIGHT, LED_A2_LIGHT, LED_A3_LIGHT, LED_A4_LIGHT,
				LED_A5_LIGHT, LED_A6_LIGHT, LED_A7_LIGHT, LED_A8_LIGHT
			},
			{
				LED_B1_LIGHT, LED_B2_LIGHT, LED_B3_LIGHT, LED_B4_LIGHT,
				LED_B5_LIGHT, LED_B6_LIGHT, LED_B7_LIGHT, LED_B8_LIGHT,
				LED_B9_LIGHT, LED_B10_LIGHT, LED_B11_LIGHT, LED_B12_LIGHT,
				LED_B13_LIGHT, LED_B14_LIGHT, LED_B15_LIGHT, LED_B16_LIGHT
			}
		};

		const int miniLeds[2][16] = {
			{
				MINILED_A1_LIGHT, MINILED_A2_LIGHT, MINILED_A3_LIGHT, MINILED_A4_LIGHT,
				MINILED_A5_LIGHT, MINILED_A6_LIGHT, MINILED_A7_LIGHT, MINILED_A8_LIGHT
			},
			{
				MINILED_B1_LIGHT, MINILED_B2_LIGHT, MINILED_B3_LIGHT, MINILED_B4_LIGHT,
				MINILED_B5_LIGHT, MINILED_B6_LIGHT, MINILED_B7_LIGHT, MINILED_B8_LIGHT,
				MINILED_B9_LIGHT, MINILED_B10_LIGHT, MINILED_B11_LIGHT, MINILED_B12_LIGHT,
				MINILED_B13_LIGHT, MINILED_B14_LIGHT, MINILED_B15_LIGHT, MINILED_B16_LIGHT
			}
		};

		const int totalSteps[2] = {8, 16};

		for (int seq = 0; seq < 2; ++seq) {
			for (int i = 0; i < totalSteps[seq]; ++i) {
				bool active = (i == currentSteps[seq]);
				lights[ledLeds[seq][i]].setBrightness(active ? 1.f : 0.f);
				lights[miniLeds[seq][i]].setBrightness(active ? 1.f : 0.f);
			}
		}
	}

// --------------------   Audio/logic process loop  ------------------------------
	void process(const ProcessArgs& args) override {

		updateAllInputStates();                 // Pull inputs & update internal states
		setStepsLeds(this, latch_a, 8, latch_b, 16);  // Write step latch LEDs
		readExpanderResets();                   // Handle expander reset pulses

		// --- Sequencer A ---
		totalStepsA = lengthA ? 8 : 4;
		if (resetAPulse) {
			gatePulseA.reset();
			currentStepA = reverseA ? (totalStepsA - 1) : 0;
		}
		
		if (clockTriggerA.process(input_a)) {
			if (reverseA) {
				currentStepA = (currentStepA - 1 + totalStepsA) % totalStepsA;
			} else {
				currentStepA = (currentStepA + 1) % totalStepsA;
			}
			
			if (latch_a[currentStepA]) {
				gatePulseA.trigger(1e-3f); // 1 ms pulse
			}
		}
		
		// A gate out
		float gateOutA = gatePulseA.process(args.sampleTime) ? 10.f : 0.f;
		outputs[OUT_1_OUTPUT].setVoltage(gateOutA);

		
		// --- Sequencer B ---
		totalStepsB = lengthB ? 16 : 8;
		if (resetBPulse) {
			gatePulseB.reset();
			currentStepB = reverseB ? (totalStepsB - 1) : 0;
		}
		
		if (clockTriggerB.process(input_b)) {
			if (reverseB) {
				currentStepB = (currentStepB - 1 + totalStepsB) % totalStepsB;
			} else {
				currentStepB = (currentStepB + 1) % totalStepsB;
			}
			
			if (latch_b[currentStepB]) {
				gatePulseB.trigger(1e-3f); // 1 ms pulse
			}
		}
		
		// B gate out
		float gateOutB = gatePulseB.process(args.sampleTime) ? 10.f : 0.f;
		outputs[OUT_2_OUTPUT].setVoltage(gateOutB);

		// LED rings: highlight current steps (A & B)
		int steps[2] = {currentStepA, currentStepB};
		updateLedRings(steps);
	}
};


// --------------------   Widget / UI layout  ------------------------------------
struct TL_Seq4Widget : ModuleWidget {
	TL_Seq4Widget(TL_Seq4* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/TL_Seq4.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Sequencer A -----------------------------------------------------------

		// Params.
		addParam(createParamCentered<NKK>(mm2px(Vec(22.535, 33.906)), module, TL_Seq4::LENGTH_1_PARAM));
		addParam(createParamCentered<NKK>(mm2px(Vec(68.936, 33.827)), module, TL_Seq4::REVERSE_1_PARAM));
		// Steps.
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(17.159, 52.133)), module, TL_Seq4::STEP_A1_PARAM, TL_Seq4::STEP_A1_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(25.368, 51.069)), module, TL_Seq4::STEP_A2_PARAM, TL_Seq4::STEP_A2_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(33.518, 50.005)), module, TL_Seq4::STEP_A3_PARAM, TL_Seq4::STEP_A3_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(41.679, 48.988)), module, TL_Seq4::STEP_A4_PARAM, TL_Seq4::STEP_A4_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(49.906, 48.953)), module, TL_Seq4::STEP_A5_PARAM, TL_Seq4::STEP_A5_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(58.045, 50.028)), module, TL_Seq4::STEP_A6_PARAM, TL_Seq4::STEP_A6_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(66.218, 51.092)), module, TL_Seq4::STEP_A7_PARAM, TL_Seq4::STEP_A7_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(74.391, 52.121)), module, TL_Seq4::STEP_A8_PARAM, TL_Seq4::STEP_A8_LED));
		// Inputs.
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.371, 18.447)), module, TL_Seq4::IN_STEP_1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.973, 34.265)), module, TL_Seq4::LENGTH_1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(83.588, 34.314)), module, TL_Seq4::REVERSE_1_INPUT));
		// Output.
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(75.937, 18.496)), module, TL_Seq4::OUT_1_OUTPUT));
		// LEDs (ring).
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(45.788, 14.285)), module, TL_Seq4::LED_A1_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(54.965, 18.117)), module, TL_Seq4::LED_A2_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(58.743, 27.276)), module, TL_Seq4::LED_A3_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(54.946, 36.447)), module, TL_Seq4::LED_A4_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(45.777, 40.251)), module, TL_Seq4::LED_A5_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(36.612, 36.437)), module, TL_Seq4::LED_A6_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(32.79, 27.284)), module, TL_Seq4::LED_A7_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(36.612, 18.091)), module, TL_Seq4::LED_A8_LIGHT));
		// Mini-LEDs (per step).
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(17.228, 47.927)), module, TL_Seq4::MINILED_A1_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(25.414, 46.848)), module, TL_Seq4::MINILED_A2_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(33.55, 45.891)), module, TL_Seq4::MINILED_A3_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(41.716, 44.754)), module, TL_Seq4::MINILED_A4_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(49.953, 44.78)), module, TL_Seq4::MINILED_A5_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(58.081, 45.834)), module, TL_Seq4::MINILED_A6_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(66.242, 46.905)), module, TL_Seq4::MINILED_A7_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(74.403, 47.96)), module, TL_Seq4::MINILED_A8_LIGHT));


		// Sequencer B -----------------------------------------------------------

		// Params.
		addParam(createParamCentered<NKK>(mm2px(Vec(22.496, 84.135)), module, TL_Seq4::LENGTH_2_PARAM));
		addParam(createParamCentered<NKK>(mm2px(Vec(68.893, 84.16)), module, TL_Seq4::REVERSE_2_PARAM));
		// Steps.
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(13.984, 104.521)), module, TL_Seq4::STEP_B1_PARAM, TL_Seq4::STEP_B1_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(23.251, 102.927)), module, TL_Seq4::STEP_B2_PARAM, TL_Seq4::STEP_B2_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(32.459, 101.863)), module, TL_Seq4::STEP_B3_PARAM, TL_Seq4::STEP_B3_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(41.15, 100.846)), module, TL_Seq4::STEP_B4_PARAM, TL_Seq4::STEP_B4_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(50.435, 100.811)), module, TL_Seq4::STEP_B5_PARAM, TL_Seq4::STEP_B5_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(59.103, 101.887)), module, TL_Seq4::STEP_B6_PARAM, TL_Seq4::STEP_B6_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(68.335, 102.951)), module, TL_Seq4::STEP_B7_PARAM, TL_Seq4::STEP_B7_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(77.566, 104.509)), module, TL_Seq4::STEP_B8_PARAM, TL_Seq4::STEP_B8_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(13.984, 114.046)), module, TL_Seq4::STEP_B9_PARAM, TL_Seq4::STEP_B9_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(23.251, 112.452)), module, TL_Seq4::STEP_B10_PARAM, TL_Seq4::STEP_B10_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(32.459, 111.388)), module, TL_Seq4::STEP_B11_PARAM, TL_Seq4::STEP_B11_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(41.15, 110.371)), module, TL_Seq4::STEP_B12_PARAM, TL_Seq4::STEP_B12_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(50.435, 110.336)), module, TL_Seq4::STEP_B13_PARAM, TL_Seq4::STEP_B13_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(59.103, 111.412)), module, TL_Seq4::STEP_B14_PARAM, TL_Seq4::STEP_B14_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(68.335, 112.476)), module, TL_Seq4::STEP_B15_PARAM, TL_Seq4::STEP_B15_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(77.566, 114.034)), module, TL_Seq4::STEP_B16_PARAM, TL_Seq4::STEP_B16_LED));

		// Inputs.
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.412, 68.624)), module, TL_Seq4::IN_STEP_2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.973, 84.537)), module, TL_Seq4::LENGTH_2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(83.575, 84.548)), module, TL_Seq4::REVERSE_2_INPUT));
		// Output.
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(75.869, 68.816)), module, TL_Seq4::OUT_2_OUTPUT));
		// LEDs (ring).
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(45.689, 56.925)), module, TL_Seq4::LED_B1_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(52.132, 58.218)), module, TL_Seq4::LED_B2_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(57.672, 61.911)), module, TL_Seq4::LED_B3_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(61.318, 67.392)), module, TL_Seq4::LED_B4_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(62.593, 73.891)), module, TL_Seq4::LED_B5_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(61.311, 80.329)), module, TL_Seq4::LED_B6_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(57.603, 85.805)), module, TL_Seq4::LED_B7_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(52.157, 89.486)), module, TL_Seq4::LED_B8_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(45.663, 90.796)), module, TL_Seq4::LED_B9_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(39.226, 89.504)), module, TL_Seq4::LED_B10_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(33.738, 85.81)), module, TL_Seq4::LED_B11_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(30.043, 80.349)), module, TL_Seq4::LED_B12_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(28.746, 73.871)), module, TL_Seq4::LED_B13_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(30.059, 67.37)), module, TL_Seq4::LED_B14_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(33.744, 61.878)), module, TL_Seq4::LED_B15_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(39.215, 58.251)), module, TL_Seq4::LED_B16_LIGHT));
		// Mini-LEDs (per step).
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(14.053, 100.315)), module, TL_Seq4::MINILED_B1_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(23.297, 98.707)), module, TL_Seq4::MINILED_B2_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(32.492, 97.749)), module, TL_Seq4::MINILED_B3_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(41.186, 96.613)), module, TL_Seq4::MINILED_B4_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(50.482, 96.638)), module, TL_Seq4::MINILED_B5_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(59.139, 97.693)), module, TL_Seq4::MINILED_B6_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(68.359, 98.763)), module, TL_Seq4::MINILED_B7_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(77.578, 100.347)), module, TL_Seq4::MINILED_B8_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(14.053, 109.84)), module, TL_Seq4::MINILED_B9_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(23.297, 108.232)), module, TL_Seq4::MINILED_B10_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(32.492, 107.274)), module, TL_Seq4::MINILED_B11_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(41.186, 106.138)), module, TL_Seq4::MINILED_B12_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(50.482, 106.163)), module, TL_Seq4::MINILED_B13_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(59.139, 107.218)), module, TL_Seq4::MINILED_B14_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(68.359, 108.288)), module, TL_Seq4::MINILED_B15_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(77.578, 109.872)), module, TL_Seq4::MINILED_B16_LIGHT));


	}
};


Model* modelTL_Seq4 = createModel<TL_Seq4, TL_Seq4Widget>("TL_Seq4");
extern Model* modelTL_Reseter;
