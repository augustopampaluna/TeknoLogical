// helpers/widgets/sliders.hpp
#pragma once
#include <app/SvgSlider.hpp>
#include <plugin.hpp>

struct SmallHSlider : rack::app::SvgSlider {
  SmallHSlider(float margin = 2.f) {
    auto bg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/custom_components/MasterHSlider-bg.svg"));
    auto hd = APP->window->loadSvg(asset::plugin(pluginInstance, "res/custom_components/MasterHSlider-handle.svg"));
    setBackgroundSvg(bg);
    setHandleSvg(hd);

    box.size = background->box.size; // 88x16 de ese SVG
    if (box.size.isZero()) box.size = rack::math::Vec(70.4f, 12.8f);

    const float W = box.size.x, H = box.size.y;
    const float hw = handle ? handle->box.size.x : H * 0.6f;
    setHandlePosCentered(
      rack::math::Vec(margin + hw * 0.1f, H * 0.5f),
      rack::math::Vec(W - margin - hw * 0.1f, H * 0.5f)
    );
    horizontal = true;

    // Borde rojo: quítalo cuando confirmes que se ve
    struct DebugBorder : rack::widget::Widget {
      void draw(const DrawArgs& args) override {
        nvgBeginPath(args.vg);
        nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
        nvgStrokeWidth(args.vg, 1.f);
        nvgStrokeColor(args.vg, nvgRGBA(255, 0, 0, 200));
        nvgStroke(args.vg);
      }
    };
    addChild(new DebugBorder);
  }
};
