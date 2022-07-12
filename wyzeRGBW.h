#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/light/light_output.h"
#include "esphome/components/ledc/ledc_output.h"
enum mode {rgb, tunable};

namespace esphome {
namespace wyzeRGBW {

class wyzeRGBW : public light::LightOutput, public Component {
 public:
  //void set_type(wyzeRGBW type) { type_ = type; }
  void set_output_r(output::FloatOutput *output_r) { output_r_ = output_r; }
  void set_output_g(output::FloatOutput *output_g) { output_g_ = output_g; }
  void set_output_b(output::FloatOutput *output_b) { output_b_ = output_b; }
  void set_output_w(output::FloatOutput *output_w) { output_w_ = output_w; }
  void set_temperature_cw(float temperature_cw) { temperature_cw_ = temperature_cw; }
  void set_temperature_ww(float temperature_ww) { temperature_ww_ = temperature_ww; }


  light::LightTraits get_traits() override {
    auto traits = light::LightTraits();
    traits.set_supported_color_modes({light::ColorMode::BRIGHTNESS, light::ColorMode::RGB, light::ColorMode::COLOR_TEMPERATURE}); // BRIGHTNESS true
    traits.set_min_mireds(this->temperature_cw_); // MIN_MIREDS
    traits.set_max_mireds(this->temperature_ww_); // MAX_MIREDS
    return traits;
  }

  // const float maxMired = 400; // 250 K
  // const float minMired = 150; // 6666.67 K
  
  // // Default Scaling - ADJUST THESE TO CHANGE WARM/COOL OUTPUT
  float scalar_r = 1; // red scaling
  float offset_r = 0; // red offset
  float scalar_g = -0.6; // green scaling
  float offset_g = 1; // green offset
  float scalar_b = -0.75; // blue scaling
  float offset_b = 0.75; // blue offset
  float scalar_w = 0; // white scaling
  float offset_w = 1; // white offset

  // Write the state to this light
  void write_state(light::LightState * state) override {
      float temperature = state->current_values.get_color_temperature();
      if (temperature > 0){
        ESP_LOGD("uart_log", "Temp Old: %f, Temp New: %f", temperature_, temperature);
      }
      float brightness;
      state->current_values_as_brightness(&brightness);
      float xaxis, r, g, b, w, cw, ww;
      state->current_values_as_rgbww(&r, &g, &b, &cw, &ww);
      
      xaxis = (temperature - temperature_cw_) / (temperature_ww_ - temperature_cw_);
      
      // Switch modes if rgb values have been sent or if color temp value has been sent
      if (temperature != temperature_) {
        mode_ = tunable;
        ESP_LOGD("uart_log", "Tunable - 1");
      } else if (r != previous_r || g != previous_g || b != previous_b) {
        mode_ = rgb;
        ESP_LOGD("uart_log", "RGB - 2");
      } else if (brightness != brightness_ && ((r == previous_r && g == previous_g && b == previous_b) || (temperature == temperature_))) {
        mode_ = oldMode_;
        ESP_LOGD("uart_log", "Existing - 3");
      } else if (brightness != brightness_) {
        mode_ = oldMode_;
        ESP_LOGD("uart_log", "Existing - 4");
      } else {
        mode_ = tunable;
        ESP_LOGD("uart_log", "Tunable - 5");
      }
      
      if (mode_ == tunable) {

          // Place the rgb values on three lines
          r = std::min(std::max((scalar_r * xaxis + offset_r) * brightness, 0.0f), 1.0f);
          g = std::min(std::max((scalar_g * xaxis + offset_g) * brightness, 0.0f), 1.0f);
          b = std::min(std::max((scalar_b * xaxis + offset_b) * brightness, 0.0f), 1.0f);
          w = std::min(std::max((scalar_w * xaxis + offset_w) * brightness, 0.0f), 1.0f);
  
          // Set WW/CW color output
          this->output_r_->set_level(r);
          this->output_g_->set_level(g);
          this->output_b_->set_level(b);
          this->output_w_->set_level(w);
  

      } else {
          
          // Set color output
          this->output_r_->set_level(r);
          this->output_g_->set_level(g);
          this->output_b_->set_level(b);
          this->output_w_->set_level(0.0f);
      }
    
      this->previous_r = r;
      this->previous_g = g;
      this->previous_b = b;
      this->oldMode_ = mode_;
      
      // Store this
      temperature_ = temperature;    
      brightness_ = brightness;
  }

protected:
  output::FloatOutput *output_r_;
  output::FloatOutput *output_g_;
  output::FloatOutput *output_b_;
  output::FloatOutput *output_w_;
  float temperature_cw_;
  float temperature_ww_;
  
  float previous_r;
  float previous_g;
  float previous_b;
  float previous_w;
  float temperature_;
  float brightness_;
  
  mode mode_;
  mode oldMode_;

};

}  // namespace wyzeRGBW
}  // namespace esphome
