#include "plugin.hpp"
#include "../helpers/widgets/switches.hpp"
#include "../helpers/messages.hpp"

extern Model* modelTL_Seq4;  // external model declaration for the neighbor

// Resetter/expander module: generates reset pulses for adjacent TL_Seq4 instances.
struct TL_Reseter : Module {
// --------------------   Parameter / I/O / Light indices  ----------------------
	enum ParamId {
		PUSH_A_PARAM,
		SIDE_A_PARAM,
		PUSH_B_PARAM,
		SIDE_B_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		IN_A_INPUT,
		IN_B_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUTPUTS_LEN
	};
	enum LightId {
		PUSH_A_LED,
		PUSH_B_LED,
		LIGHTS_LEN
	};

// --------------------   Runtime state (buttons, fades, edges)  -----------------
    float aLightIntensity = 0.f;
    float bLightIntensity = 0.f;
    bool aPressed = false;
    bool bPressed = false;
	bool lastAPressed = false;
	bool lastBPressed = false;

	// Expander message buffers (left/right)
	ReseterMessage leftBuf[2];
	ReseterMessage rightBuf[2];

// --------------------   Constructor / configuration  ---------------------------
	TL_Reseter() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configButton(PUSH_A_PARAM, "Push A");
		configButton(PUSH_B_PARAM, "Push B");
		configSwitch(SIDE_A_PARAM, 0.f, 1.f, 0.f, "Side A", {"Left", "Right"});
		configSwitch(SIDE_B_PARAM, 0.f, 1.f, 0.f, "Side B", {"Left", "Right"});
		configInput(IN_A_INPUT, "Gate A");
		configInput(IN_B_INPUT, "Gate B");

		// Expander: bind producer/consumer buffers for message flips
		leftExpander.producerMessage  = &leftBuf[0];
		leftExpander.consumerMessage  = &leftBuf[1];
		rightExpander.producerMessage = &rightBuf[0];
		rightExpander.consumerMessage = &rightBuf[1];
	}

// --------------------   Helpers: UI feedback & expander I/O  -------------------
    void updateLightsAndTriggers(float deltaTime) {
        // Read momentary buttons or gate inputs (>= 1 V considered active)
        aPressed = (params[PUSH_A_PARAM].getValue() > 0.f) 
                || (inputs[IN_A_INPUT].isConnected() && std::fabs(inputs[IN_A_INPUT].getVoltage()) > 1.f);
        bPressed = (params[PUSH_B_PARAM].getValue() > 0.f) 
                || (inputs[IN_B_INPUT].isConnected() && std::fabs(inputs[IN_B_INPUT].getVoltage()) > 1.f);

        // Simple one-pole rise/fall for LED intensities
        aLightIntensity += (aPressed ? (1.f - aLightIntensity) : -5.f * deltaTime);
        bLightIntensity += (bPressed ? (1.f - bLightIntensity) : -5.f * deltaTime);

        // Clamp to [0..1]
        aLightIntensity = clamp(aLightIntensity, 0.f, 1.f);
        bLightIntensity = clamp(bLightIntensity, 0.f, 1.f);

        // Apply to LEDs
        lights[PUSH_A_LED].setBrightness(aLightIntensity);
        lights[PUSH_B_LED].setBrightness(bLightIntensity);
    }

	void sendToExpander() {
		// Rising-edge detection on A/B to generate one-frame pulses
		bool sendA = (!lastAPressed && aPressed);
		bool sendB = (!lastBPressed && bPressed);
		lastAPressed = aPressed;
		lastBPressed = bPressed;

		// Route pulses to left or right neighbor depending on switch state
		bool aToLeft  = params[SIDE_A_PARAM].getValue() == 0 && sendA;
		bool aToRight = params[SIDE_A_PARAM].getValue() == 1 && sendA;
		bool bToLeft  = params[SIDE_B_PARAM].getValue() == 0 && sendB;
		bool bToRight = params[SIDE_B_PARAM].getValue() == 1 && sendB;

		auto* l = (ReseterMessage*) leftExpander.producerMessage;
		auto* r = (ReseterMessage*) rightExpander.producerMessage;

		// Send to left (only if the neighbor is TL_Seq4), otherwise clear
		if (leftExpander.module && leftExpander.module->model == modelTL_Seq4) {
			l->aGate = aToLeft;
			l->bGate = bToLeft;
			leftExpander.requestMessageFlip();
		} else {
			l->aGate = false;
			l->bGate = false;
		}

		// Send to right (only if the neighbor is TL_Seq4), otherwise clear
		if (rightExpander.module && rightExpander.module->model == modelTL_Seq4) {
			r->aGate = aToRight;
			r->bGate = bToRight;
			rightExpander.requestMessageFlip();
		} else {
			r->aGate = false;
			r->bGate = false;
		}
	}

// --------------------   Process: per-sample UI + expander messaging ------------
	void process(const ProcessArgs& args) override {
        updateLightsAndTriggers(args.sampleTime);
		sendToExpander();
    }
};


// --------------------   Widget / panel layout  ---------------------------------
struct TL_ReseterWidget : ModuleWidget {
	TL_ReseterWidget(TL_Reseter* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/TL_Reseter.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Push buttons (with integrated lights).
		addParam(createLightParamCentered<VCVLightButton<LargeSimpleLight<WhiteLight>>>(mm2px(Vec(7.629, 42.405)), module, TL_Reseter::PUSH_A_PARAM, TL_Reseter::PUSH_A_LED));
		addParam(createLightParamCentered<VCVLightButton<LargeSimpleLight<WhiteLight>>>(mm2px(Vec(7.664, 90.533)), module, TL_Reseter::PUSH_B_PARAM, TL_Reseter::PUSH_B_LED));
		
		// Left/Right routing switches.
		addParam(createParamCentered<Switch2Horiz>(mm2px(Vec(7.673, 55.553)), module, TL_Reseter::SIDE_A_PARAM));
		addParam(createParamCentered<Switch2Horiz>(mm2px(Vec(7.676, 103.721)), module, TL_Reseter::SIDE_B_PARAM));
		
		// Gate inputs.
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.724, 28.584)), module, TL_Reseter::IN_A_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.7, 76.748)), module, TL_Reseter::IN_B_INPUT));

	}
};


Model* modelTL_Reseter = createModel<TL_Reseter, TL_ReseterWidget>("TL_Reseter");
