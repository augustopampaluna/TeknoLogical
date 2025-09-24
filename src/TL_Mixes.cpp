#include "plugin.hpp"
#include "../helpers/widgets/sliders.hpp"
#include "rack.hpp"
#include "dsp/digital.hpp"
#include <array>
#include <algorithm>
#include <cmath>
#include "../helpers/dsp_utils.hpp"

using namespace rack;

struct TL_Mixes : Module {
	enum ParamId {
		CUT_1_PARAM, CUT_2_PARAM, CUT_3_PARAM, CUT_4_PARAM, CUT_5_PARAM, CUT_6_PARAM, CUT_7_PARAM,
		PAN_1_PARAM, PAN_2_PARAM, PAN_3_PARAM, PAN_4_PARAM, PAN_5_PARAM, PAN_6_PARAM, PAN_7_PARAM,
		VOL_1_PARAM, VOL_2_PARAM, VOL_3_PARAM, VOL_4_PARAM, VOL_5_PARAM, VOL_6_PARAM, VOL_7_PARAM,
		MUTE_1_PARAM, MUTE_2_PARAM, MUTE_3_PARAM, MUTE_4_PARAM, MUTE_5_PARAM, MUTE_6_PARAM, MUTE_7_PARAM,
		SOLO_1_PARAM, SOLO_2_PARAM, SOLO_3_PARAM, SOLO_4_PARAM, SOLO_5_PARAM, SOLO_6_PARAM, SOLO_7_PARAM,
		MASTER_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		L_IN_1_INPUT, L_IN_2_INPUT, L_IN_3_INPUT, L_IN_4_INPUT, L_IN_5_INPUT, L_IN_6_INPUT, L_IN_7_INPUT,
		R_IN_1_INPUT, R_IN_2_INPUT, R_IN_3_INPUT, R_IN_4_INPUT, R_IN_5_INPUT, R_IN_6_INPUT, R_IN_7_INPUT,
		VOL_IN_1_INPUT, VOL_IN_2_INPUT, VOL_IN_3_INPUT, VOL_IN_4_INPUT, VOL_IN_5_INPUT, VOL_IN_6_INPUT, VOL_IN_7_INPUT,
		PAN_IN_1_INPUT, PAN_IN_2_INPUT, PAN_IN_3_INPUT, PAN_IN_4_INPUT, PAN_IN_5_INPUT, PAN_IN_6_INPUT, PAN_IN_7_INPUT,
		INPUTS_LEN
	};
	enum OutputId { OUT_L_OUTPUT, OUT_R_OUTPUT, OUTPUTS_LEN };
	enum LightId {
		LED_1_LIGHT, LED_2_LIGHT, LED_3_LIGHT, LED_4_LIGHT, LED_5_LIGHT, LED_6_LIGHT, LED_7_LIGHT,
		L_VU_1_LIGHT, L_VU_2_LIGHT, L_VU_3_LIGHT, L_VU_4_LIGHT, L_VU_5_LIGHT,
		R_VU_1_LIGHT, R_VU_2_LIGHT, R_VU_3_LIGHT, R_VU_4_LIGHT, R_VU_5_LIGHT,
		MUTE_1_LED, MUTE_2_LED, MUTE_3_LED, MUTE_4_LED, MUTE_5_LED, MUTE_6_LED, MUTE_7_LED,
		SOLO_1_LED, SOLO_2_LED, SOLO_3_LED, SOLO_4_LED, SOLO_5_LED, SOLO_6_LED, SOLO_7_LED,
		LIGHTS_LEN
	};

	// ===== Runtime state =====
	static constexpr int CH = 7;
	
	// One HP per side per channel
	DSPUtils::HP1 hpL[CH];
	DSPUtils::HP1 hpR[CH];
	
	// master meter smoothing
	float vuL = 0.f, vuR = 0.f;
	float sampleRate = 44100.f;

	// fixed high-pass cutoff for CUT.
	float cutHz = 180.f;

	TL_Mixes() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		static const std::vector<std::string> onoff_labels = {"Off", "On"};
		configSwitch(CUT_1_PARAM, 0.f, 1.f, 0.f, "Cutoff", onoff_labels);
		configSwitch(CUT_2_PARAM, 0.f, 1.f, 0.f, "Cutoff", onoff_labels);
		configSwitch(CUT_3_PARAM, 0.f, 1.f, 0.f, "Cutoff", onoff_labels);
		configSwitch(CUT_4_PARAM, 0.f, 1.f, 0.f, "Cutoff", onoff_labels);
		configSwitch(CUT_5_PARAM, 0.f, 1.f, 0.f, "Cutoff", onoff_labels);
		configSwitch(CUT_6_PARAM, 0.f, 1.f, 0.f, "Cutoff", onoff_labels);
		configSwitch(CUT_7_PARAM, 0.f, 1.f, 0.f, "Cutoff", onoff_labels);

		configParam(PAN_1_PARAM, -1.f, 1.f, 0.f, "Pan");
		configParam(PAN_2_PARAM, -1.f, 1.f, 0.f, "Pan");
		configParam(PAN_3_PARAM, -1.f, 1.f, 0.f, "Pan");
		configParam(PAN_4_PARAM, -1.f, 1.f, 0.f, "Pan");
		configParam(PAN_5_PARAM, -1.f, 1.f, 0.f, "Pan");
		configParam(PAN_6_PARAM, -1.f, 1.f, 0.f, "Pan");
		configParam(PAN_7_PARAM, -1.f, 1.f, 0.f, "Pan");

		configParam(VOL_1_PARAM, 0.f, 10.f, 0.f, "Vol");
		configParam(VOL_2_PARAM, 0.f, 10.f, 0.f, "Vol");
		configParam(VOL_3_PARAM, 0.f, 10.f, 0.f, "Vol");
		configParam(VOL_4_PARAM, 0.f, 10.f, 0.f, "Vol");
		configParam(VOL_5_PARAM, 0.f, 10.f, 0.f, "Vol");
		configParam(VOL_6_PARAM, 0.f, 10.f, 0.f, "Vol");
		configParam(VOL_7_PARAM, 0.f, 10.f, 0.f, "Vol");

		configSwitch(MUTE_1_PARAM, 0.f, 1.f, 0.f, "Mute", onoff_labels);
		configSwitch(MUTE_2_PARAM, 0.f, 1.f, 0.f, "Mute", onoff_labels);
		configSwitch(MUTE_3_PARAM, 0.f, 1.f, 0.f, "Mute", onoff_labels);
		configSwitch(MUTE_4_PARAM, 0.f, 1.f, 0.f, "Mute", onoff_labels);
		configSwitch(MUTE_5_PARAM, 0.f, 1.f, 0.f, "Mute", onoff_labels);
		configSwitch(MUTE_6_PARAM, 0.f, 1.f, 0.f, "Mute", onoff_labels);
		configSwitch(MUTE_7_PARAM, 0.f, 1.f, 0.f, "Mute", onoff_labels);

		configSwitch(SOLO_1_PARAM, 0.f, 1.f, 0.f, "Solo", onoff_labels);
		configSwitch(SOLO_2_PARAM, 0.f, 1.f, 0.f, "Solo", onoff_labels);
		configSwitch(SOLO_3_PARAM, 0.f, 1.f, 0.f, "Solo", onoff_labels);
		configSwitch(SOLO_4_PARAM, 0.f, 1.f, 0.f, "Solo", onoff_labels);
		configSwitch(SOLO_5_PARAM, 0.f, 1.f, 0.f, "Solo", onoff_labels);
		configSwitch(SOLO_6_PARAM, 0.f, 1.f, 0.f, "Solo", onoff_labels);
		configSwitch(SOLO_7_PARAM, 0.f, 1.f, 0.f, "Solo", onoff_labels);

		configParam(MASTER_PARAM, 0.f, 100.f, 0.f, "Master");

		for (int i = 0; i < CH; ++i) {
			configInput(L_IN_1_INPUT + i, "L audio");
			configInput(R_IN_1_INPUT + i, "R audio");
			configInput(VOL_IN_1_INPUT + i, "Vol CV");
			configInput(PAN_IN_1_INPUT + i, "Pan CV");
		}

		configOutput(OUT_L_OUTPUT, "Left");
		configOutput(OUT_R_OUTPUT, "Right");
	}

	void onSampleRateChange(const SampleRateChangeEvent& e) override {
		Module::onSampleRateChange(e);
		sampleRate = e.sampleRate;
		for (int i = 0; i < CH; ++i) { hpL[i].setCutoff(cutHz, sampleRate); hpR[i].setCutoff(cutHz, sampleRate); }
	}

	void onReset(const ResetEvent& e) override {
		Module::onReset(e);
		vuL = vuR = 0.f;
		for (int i = 0; i < CH; ++i) { hpL[i].reset(); hpR[i].reset(); }
	}

	void process(const ProcessArgs& args) override {
		// ensure HP is configured
		if (args.sampleRate != sampleRate) {
			sampleRate = args.sampleRate;
			for (int i = 0; i < CH; ++i) { hpL[i].setCutoff(cutHz, sampleRate); hpR[i].setCutoff(cutHz, sampleRate); }
		}

		float mixL = 0.f;
		float mixR = 0.f;

		// Any SOLO active?
		bool anySolo = false;
		for (int c = 0; c < CH; ++c) anySolo |= (params[SOLO_1_PARAM + c].getValue() > 0.5f);

		for (int c = 0; c < CH; ++c) {
			// Status
			bool cut = params[CUT_1_PARAM + c].getValue() > 0.5f;
			bool mute = params[MUTE_1_PARAM + c].getValue() > 0.5f;
			bool solo = params[SOLO_1_PARAM + c].getValue() > 0.5f;

			// LEDs
			lights[LED_1_LIGHT + c].setBrightness(cut ? 1.f : 0.f);
			lights[MUTE_1_LED + c].setBrightness(mute ? 1.f : 0.f);
			lights[SOLO_1_LED + c].setBrightness(solo ? 1.f : 0.f);

			if (mute || (anySolo && !solo)) continue; // skip this channel in mix

			// Inputs
			bool lConn = inputs[L_IN_1_INPUT + c].isConnected();
			bool rConn = inputs[R_IN_1_INPUT + c].isConnected();
			float inL = lConn ? inputs[L_IN_1_INPUT + c].getVoltage() : 0.f;
			float inR = rConn ? inputs[R_IN_1_INPUT + c].getVoltage() : 0.f;

			// Mono/stereo behavior
			if (!(lConn && rConn)) {
				float mono = lConn ? inL : (rConn ? inR : 0.f);
				inL = mono;
				inR = mono;
			}

			// CUT (HP) per side when switch active
			if (cut) {
				inL = hpL[c].process(inL);
				inR = hpR[c].process(inR);
			}

			// Volume — knob sets maximum, CV overrides absolute within 0..max
			float vol = DSPUtils::resolveVolume01(
			    params[VOL_1_PARAM + c].getValue(),
			    inputs[VOL_IN_1_INPUT + c].isConnected(),
			    inputs[VOL_IN_1_INPUT + c].getVoltage()
			);

			// Pan — CV fully overrides knob
			float pan = DSPUtils::resolvePanMinus1to1(
			    params[PAN_1_PARAM + c].getValue(),
			    inputs[PAN_IN_1_INPUT + c].isConnected(),
			    inputs[PAN_IN_1_INPUT + c].getVoltage()
			);

			// Apply per-channel gain
			inL *= vol;
			inR *= vol;

			// Apply panning only when the channel behaves as mono; if both inputs present, treat as balance
			if (inputs[L_IN_1_INPUT + c].isConnected() && inputs[R_IN_1_INPUT + c].isConnected()) {
				float l = inL, r = inR;
				DSPUtils::balanceStereoEqualPower(l, r, pan);
				mixL += l;
				mixR += r;
			}
			else {
				// mono -> stereo with equal-power pan
				float mono = 0.5f * (inL + inR);
				DSPUtils::panMonoEqualPower(mono, pan, mixL, mixR);
			}
		}

		// Master gain (0..1) applied pre-limiter
		float master = clamp(params[MASTER_PARAM].getValue() / 100.f, 0.f, 1.f);
		mixL *= master;
		mixR *= master;

		// Master limiter
		float outL = DSPUtils::softLimit5V(mixL);
		float outR = DSPUtils::softLimit5V(mixR);

		// VU meters (post-limiter): fast-ish attack / slow release
		float absL = std::fabs(outL);
		float absR = std::fabs(outR);
		float rel = 0.02f;
		vuL = std::max(absL, vuL * (1.f - rel) + absL * rel);
		vuR = std::max(absR, vuR * (1.f - rel) + absR * rel);

		auto setVU = [&](float v, int baseLight) {
			// thresholds relative to 5V FS
			float fs = 5.f;
			float t1 = 0.05f * fs; // lowest
			float t2 = 0.12f * fs;
			float t3 = 0.25f * fs;
			float t4 = 0.50f * fs;
			float t5 = 0.90f * fs; // near clip only
			lights[baseLight + 0].setBrightness(v >= t1 ? 1.f : 0.f);
			lights[baseLight + 1].setBrightness(v >= t2 ? 1.f : 0.f);
			lights[baseLight + 2].setBrightness(v >= t3 ? 1.f : 0.f);
			lights[baseLight + 3].setBrightness(v >= t4 ? 1.f : 0.f);
			lights[baseLight + 4].setBrightness(v >= t5 ? 1.f : 0.f);
		};
		setVU(vuL, L_VU_1_LIGHT);
		setVU(vuR, R_VU_1_LIGHT);

		// Outputs (post-limiter)
		outputs[OUT_L_OUTPUT].setVoltage(outL);
		outputs[OUT_R_OUTPUT].setVoltage(outR);
	}
};

struct TL_MixesWidget : ModuleWidget {
	TL_MixesWidget(TL_Mixes* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/TL_Mixes.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<NKK>(mm2px(Vec(8.697, 64.656)), module, TL_Mixes::CUT_1_PARAM));
		addParam(createParamCentered<NKK>(mm2px(Vec(25.869, 62.004)), module, TL_Mixes::CUT_2_PARAM));
		addParam(createParamCentered<NKK>(mm2px(Vec(43.946, 59.346)), module, TL_Mixes::CUT_3_PARAM));
		addParam(createParamCentered<NKK>(mm2px(Vec(61.087, 57.221)), module, TL_Mixes::CUT_4_PARAM));
		addParam(createParamCentered<NKK>(mm2px(Vec(78.701, 59.284)), module, TL_Mixes::CUT_5_PARAM));
		addParam(createParamCentered<NKK>(mm2px(Vec(96.012, 61.92)), module, TL_Mixes::CUT_6_PARAM));
		addParam(createParamCentered<NKK>(mm2px(Vec(113.655, 64.528)), module, TL_Mixes::CUT_7_PARAM));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(8.566, 78.383)), module, TL_Mixes::PAN_1_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(25.738, 75.831)), module, TL_Mixes::PAN_2_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(43.815, 73.173)), module, TL_Mixes::PAN_3_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(60.956, 71.048)), module, TL_Mixes::PAN_4_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(78.57, 73.111)), module, TL_Mixes::PAN_5_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(95.881, 75.847)), module, TL_Mixes::PAN_6_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(113.524, 78.456)), module, TL_Mixes::PAN_7_PARAM));

		addParam(createParamCentered<Rogan1PWhite>(mm2px(Vec(8.635, 91.752)), module, TL_Mixes::VOL_1_PARAM));
		addParam(createParamCentered<Rogan1PWhite>(mm2px(Vec(25.807, 89.1)), module, TL_Mixes::VOL_2_PARAM));
		addParam(createParamCentered<Rogan1PWhite>(mm2px(Vec(43.884, 86.442)), module, TL_Mixes::VOL_3_PARAM));
		addParam(createParamCentered<Rogan1PWhite>(mm2px(Vec(61.025, 84.317)), module, TL_Mixes::VOL_4_PARAM));
		addParam(createParamCentered<Rogan1PWhite>(mm2px(Vec(78.639, 86.38)), module, TL_Mixes::VOL_5_PARAM));
		addParam(createParamCentered<Rogan1PWhite>(mm2px(Vec(95.95, 89.016)), module, TL_Mixes::VOL_6_PARAM));
		addParam(createParamCentered<Rogan1PWhite>(mm2px(Vec(113.593, 91.624)), module, TL_Mixes::VOL_7_PARAM));
		
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(5.015, 104.696)), module, TL_Mixes::MUTE_1_PARAM, TL_Mixes::MUTE_1_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(22.529, 102.446)), module, TL_Mixes::MUTE_2_PARAM, TL_Mixes::MUTE_2_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(39.939, 99.866)), module, TL_Mixes::MUTE_3_PARAM, TL_Mixes::MUTE_3_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(57.31, 97.269)), module, TL_Mixes::MUTE_4_PARAM, TL_Mixes::MUTE_4_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(74.869, 98.905)), module, TL_Mixes::MUTE_5_PARAM, TL_Mixes::MUTE_5_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(92.377, 101.544)), module, TL_Mixes::MUTE_6_PARAM, TL_Mixes::MUTE_6_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(109.274, 104.159)), module, TL_Mixes::MUTE_7_PARAM, TL_Mixes::MUTE_7_LED));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(12.935, 103.665)), module, TL_Mixes::SOLO_1_PARAM, TL_Mixes::SOLO_1_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(30.449, 101.415)), module, TL_Mixes::SOLO_2_PARAM, TL_Mixes::SOLO_2_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(47.936, 98.866)), module, TL_Mixes::SOLO_3_PARAM, TL_Mixes::SOLO_3_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(65.281, 97.254)), module, TL_Mixes::SOLO_4_PARAM, TL_Mixes::SOLO_4_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(82.822, 99.89)), module, TL_Mixes::SOLO_5_PARAM, TL_Mixes::SOLO_5_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(100.297, 102.544)), module, TL_Mixes::SOLO_6_PARAM, TL_Mixes::SOLO_6_LED));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<BlueLight>>>(mm2px(Vec(117.257, 105.27)), module, TL_Mixes::SOLO_7_PARAM, TL_Mixes::SOLO_7_LED));

		addParam(createParamCentered<SmallHSlider>(mm2px(rack::math::Vec(61.085, 107.963)), module, TL_Mixes::MASTER_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.55, 22.44)), module, TL_Mixes::L_IN_1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.74, 19.788)), module, TL_Mixes::L_IN_2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(43.7, 17.13)), module, TL_Mixes::L_IN_3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(60.95, 15.005)), module, TL_Mixes::L_IN_4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(78.6, 17.068)), module, TL_Mixes::L_IN_5_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(95.88, 19.704)), module, TL_Mixes::L_IN_6_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(113.2, 22.312)), module, TL_Mixes::L_IN_7_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.55, 31.987)), module, TL_Mixes::R_IN_1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.74, 29.335)), module, TL_Mixes::R_IN_2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(43.7, 26.677)), module, TL_Mixes::R_IN_3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(60.95, 24.552)), module, TL_Mixes::R_IN_4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(78.6, 26.615)), module, TL_Mixes::R_IN_5_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(95.88, 29.251)), module, TL_Mixes::R_IN_6_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(113.2, 31.859)), module, TL_Mixes::R_IN_7_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.55, 43.585)), module, TL_Mixes::VOL_IN_1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.74, 40.932)), module, TL_Mixes::VOL_IN_2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(43.7, 38.275)), module, TL_Mixes::VOL_IN_3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(60.95, 36.149)), module, TL_Mixes::VOL_IN_4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(78.6, 38.213)), module, TL_Mixes::VOL_IN_5_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(95.88, 40.848)), module, TL_Mixes::VOL_IN_6_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(113.2, 43.457)), module, TL_Mixes::VOL_IN_7_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.55, 53.11)), module, TL_Mixes::PAN_IN_1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.74, 50.457)), module, TL_Mixes::PAN_IN_2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(43.7, 47.8)), module, TL_Mixes::PAN_IN_3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(60.95, 45.674)), module, TL_Mixes::PAN_IN_4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(78.6, 47.738)), module, TL_Mixes::PAN_IN_5_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(95.88, 50.373)), module, TL_Mixes::PAN_IN_6_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(113.2, 52.982)), module, TL_Mixes::PAN_IN_7_INPUT));

		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(55.805, 116.52)), module, TL_Mixes::OUT_L_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(66.731, 116.551)), module, TL_Mixes::OUT_R_OUTPUT));

		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(3.48, 61.582)), module, TL_Mixes::LED_1_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(20.652, 58.929)), module, TL_Mixes::LED_2_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(38.729, 56.271)), module, TL_Mixes::LED_3_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(55.87, 54.146)), module, TL_Mixes::LED_4_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(73.483, 56.209)), module, TL_Mixes::LED_5_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(90.795, 58.845)), module, TL_Mixes::LED_6_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(108.438, 61.454)), module, TL_Mixes::LED_7_LIGHT));

		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(44.653, 121.596)), module, TL_Mixes::L_VU_1_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(44.666, 119.364)), module, TL_Mixes::L_VU_2_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(44.659, 116.826)), module, TL_Mixes::L_VU_3_LIGHT));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(44.674, 113.645)), module, TL_Mixes::L_VU_4_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(44.674, 109.922)), module, TL_Mixes::L_VU_5_LIGHT));
		
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(78.233, 121.58)), module, TL_Mixes::R_VU_1_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(78.246, 119.349)), module, TL_Mixes::R_VU_2_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(78.239, 116.81)), module, TL_Mixes::R_VU_3_LIGHT));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(78.254, 113.63)), module, TL_Mixes::R_VU_4_LIGHT));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(78.254, 109.907)), module, TL_Mixes::R_VU_5_LIGHT));
	}
};


Model* modelTL_Mixes = createModel<TL_Mixes, TL_MixesWidget>("TL_Mixes");
