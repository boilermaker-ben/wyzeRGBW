#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/light/light_output.h"
#include "esphome/components/ledc/ledc_output.h"

namespace esphome {
namespace tunableRGBW {

static const char *TAG = "tunableRGBW";

enum LightMode {
    RGB_MODE,
    COLOR_TEMP_MODE
};

struct ColorMix {
    float red = 1.0f;
    float green = 1.0f;
    float blue = 1.0f;
    float white = 1.0f;
};

class tunableRGBW : public light::LightOutput, public Component {
  public:
    void set_output_r(output::FloatOutput *output_r) { output_r_ = output_r; }
    void set_output_g(output::FloatOutput *output_g) { output_g_ = output_g; }
    void set_output_b(output::FloatOutput *output_b) { output_b_ = output_b; }
    void set_output_w(output::FloatOutput *output_w) { output_w_ = output_w; }
    void set_temperature_cw(float temperature_cw) { temperature_cw_ = temperature_cw; }
    void set_temperature_ww(float temperature_ww) { temperature_ww_ = temperature_ww; }
    
    void set_warm_white_mix(float r, float g, float b, float w) {
        warm_white_mix_ = {r, g, b, w};
    }
    
    void set_cool_white_mix(float r, float g, float b, float w) {
        cool_white_mix_ = {r, g, b, w};
    }
    
    void set_white_brightness(float brightness) {
        white_brightness_ = brightness;
    }

    void set_white_brightness_number(number::Number *brightness_number) {
        white_brightness_number_ = brightness_number;
    }

    void set_warm_mix_numbers(number::Number *r, number::Number *g, number::Number *b, number::Number *w) {
        warm_mix_red_ = r;
        warm_mix_green_ = g;
        warm_mix_blue_ = b;
        warm_mix_white_ = w;
    }

    void set_cool_mix_numbers(number::Number *r, number::Number *g, number::Number *b, number::Number *w) {
        cool_mix_red_ = r;
        cool_mix_green_ = g;
        cool_mix_blue_ = b;
        cool_mix_white_ = w;
    }

    void set_gamma_correction(float gamma) {
        gamma_correction_ = gamma;
    }

    void set_minimum_color_brightness(float min_color_brightness) {
        minimum_color_brightness_ = min_color_brightness;
    }

    void set_minimum_tunable_brightness(float min_tunable_brightness) {
        minimum_tunable_brightness_ = min_tunable_brightness;
    }

    void set_color_temperature_boost(float boost) {
        color_temperature_boost_ = boost;
    }

    void set_brightness_range_expansion(bool expand) {
        brightness_range_expansion_ = expand;
    }
    
    light::LightTraits get_traits() override {
        auto traits = light::LightTraits();
        traits.set_supported_color_modes({light::ColorMode::RGB, light::ColorMode::COLOR_TEMPERATURE});
        traits.set_min_mireds(this->temperature_cw_);
        traits.set_max_mireds(this->temperature_ww_);
        return traits;
    }
    
    void write_state(light::LightState *state) override {
        float red, green, blue, cold_white, warm_white;
        float brightness = state->current_values.get_brightness();
        bool is_on = state->current_values.is_on();
        
        if (!is_on) {
            // Turn off all outputs
            set_all_outputs(0.0f, 0.0f, 0.0f, 0.0f);
            return;
        }
        
        // Get current color mode
        auto color_mode = state->current_values.get_color_mode();
        
        if (color_mode == light::ColorMode::RGB) {
            // RGB mode - use RGB values directly, turn off white
            state->current_values_as_rgb(&red, &green, &blue);
            
            // Apply RGB-specific brightness mapping and gamma correction
            float final_brightness = calculate_rgb_brightness(brightness);
            red *= final_brightness;
            green *= final_brightness;
            blue *= final_brightness;
            
            set_all_outputs(red, green, blue, 0.0f);
            
            ESP_LOGD(TAG, "RGB Mode - Input B:%.1f%% Final B:%.1f%% R:%.3f G:%.3f B:%.3f W:0.000", 
                     brightness * 100.0f, final_brightness * 100.0f, red, green, blue);
            
        } else if (color_mode == light::ColorMode::COLOR_TEMPERATURE) {
            // Color temperature mode - simulate using RGB mix
            float temperature = state->current_values.get_color_temperature();
            auto mixed_color = calculate_color_temperature_mix(temperature, brightness);
            
            set_all_outputs(mixed_color.red, mixed_color.green, mixed_color.blue, mixed_color.white);
            
            ESP_LOGD(TAG, "Color Temp Mode - T:%.0fK B:%.1f%% R:%.3f G:%.3f B:%.3f W:%.3f", 
                     1000000.0f / temperature, brightness * 100.0f, 
                     mixed_color.red, mixed_color.green, mixed_color.blue, mixed_color.white);
        }
    }
    
  private:
    output::FloatOutput *output_r_;
    output::FloatOutput *output_g_;
    output::FloatOutput *output_b_;
    output::FloatOutput *output_w_;
    float temperature_cw_;
    float temperature_ww_;

    number::Number *warm_mix_red_;
    number::Number *warm_mix_green_;
    number::Number *warm_mix_blue_;
    number::Number *warm_mix_white_;
    number::Number *cool_mix_red_;
    number::Number *cool_mix_green_;
    number::Number *cool_mix_blue_;
    number::Number *cool_mix_white_;
    number::Number *white_brightness_number_ = nullptr;

    ColorMix warm_white_mix_ = {1.0f, 0.4f, 0.0f, 1.0f};  // Default warm white mix
    ColorMix cool_white_mix_ = {0.8f, 1.0f, 1.0f, 1.0f};  // Default cool white mix
    float white_brightness_ = 0.8f;
    
    // Configurable parameters
    float gamma_correction_ = 2.2f;  // Standard gamma correction
    float minimum_color_brightness_ = 0.2f;  // 20% minimum for RGB mode
    float minimum_tunable_brightness_ = 0.05f;  // 5% minimum for color temp mode to preserve color ratios
    float color_temperature_boost_ = 1.5f;  // Boost color temperature effect at low brightness
    bool brightness_range_expansion_ = true;  // Expand the usable brightness range
    
    void set_all_outputs(float r, float g, float b, float w) {
        output_r_->set_level(r);
        output_g_->set_level(g);
        output_b_->set_level(b);
        output_w_->set_level(w);
    }
    
    float apply_gamma_correction(float brightness) {
        // Apply gamma correction directly - no minimum brightness floor needed
        // as we handle minimums in the brightness mapping functions
        return std::pow(brightness, gamma_correction_);
    }
    
    float calculate_rgb_brightness(float input_brightness) {
        // Special handling for RGB mode to ensure minimum brightness is respected
        if (input_brightness <= 0.0f) {
            return 0.0f;
        }
        
        // Map 0-100% input to minimum_color_brightness_-100% output BEFORE gamma correction
        // This ensures the minimum is maintained even after gamma correction
        float mapped_brightness = minimum_color_brightness_ + (input_brightness * (1.0f - minimum_color_brightness_));
        
        // Apply gamma correction to the mapped brightness
        float gamma_corrected = apply_gamma_correction(mapped_brightness);
        
        // Ensure we never go below the minimum after gamma correction
        // This is the key fix - we enforce the minimum AFTER gamma correction
        return std::max(gamma_corrected, minimum_color_brightness_);
    }
    
    float map_brightness_range(float brightness) {
        // Map the full 0-100% input range to a more usable output range for color temperature
        // This ensures we use the full dimming capability without losing color at low levels
        
        if (brightness <= 0.0f) {
            return 0.0f;
        }
        
        if (!brightness_range_expansion_) {
            return brightness;  // Use original brightness if expansion is disabled
        }
        
        // Map brightness to avoid the "dead zone" at low levels
        // Scale the range so that the bottom 5% maps to the configured minimum
        float mapped_brightness;
        
        if (brightness <= 0.05f) {
            // For the bottom 5%, map to the minimum brightness to preserve color ratios
            mapped_brightness = minimum_tunable_brightness_ + (brightness / 0.05f) * (0.15f - minimum_tunable_brightness_);
        } else {
            // For 5-100%, map to 15-95% output range to avoid problematic extremes
            float normalized = (brightness - 0.05f) / 0.95f;  // 0-1 range for 5-100%
            mapped_brightness = 0.15f + (normalized * 0.8f);  // Map to 15-95%
        }
        
        return std::min(mapped_brightness, 1.0f);
    }
    
    ColorMix calculate_color_temperature_mix(float temperature_mireds, float brightness) {
        // Calculate interpolation factor (0.0 = cool, 1.0 = warm)
        float temp_factor = (temperature_mireds - temperature_cw_) / (temperature_ww_ - temperature_cw_);
        temp_factor = std::min(std::max(temp_factor, 0.0f), 1.0f);
        
        // Fetch live values or use defaults
        ColorMix warm = {
            warm_mix_red_ ? warm_mix_red_->state / 100.0f : warm_white_mix_.red,
            warm_mix_green_ ? warm_mix_green_->state / 100.0f : warm_white_mix_.green,
            warm_mix_blue_ ? warm_mix_blue_->state / 100.0f : warm_white_mix_.blue,
            warm_mix_white_ ? warm_mix_white_->state / 100.0f : warm_white_mix_.white
        };

        ColorMix cool = {
            cool_mix_red_ ? cool_mix_red_->state / 100.0f : cool_white_mix_.red,
            cool_mix_green_ ? cool_mix_green_->state / 100.0f : cool_white_mix_.green,
            cool_mix_blue_ ? cool_mix_blue_->state / 100.0f : cool_white_mix_.blue,
            cool_mix_white_ ? cool_mix_white_->state / 100.0f : cool_white_mix_.white
        };
        
        float effective_white_brightness = white_brightness_;
        if (white_brightness_number_ != nullptr) {
            effective_white_brightness = white_brightness_number_->state / 100.0f;
        }

        // Map brightness to avoid dead zones and preserve color ratios
        float mapped_brightness = map_brightness_range(brightness);
        
        // Calculate color temperature boost for low brightness
        // This enhances the warm/cool effect when dimmed
        float temp_boost = 1.0f;
        if (brightness < 0.3f) {
            // Gradually increase color temperature bias as brightness decreases
            float boost_factor = (0.3f - brightness) / 0.3f;  // 0 to 1 as brightness goes from 30% to 0%
            temp_boost = 1.0f + (color_temperature_boost_ - 1.0f) * boost_factor;
        }
        
        // Apply boosted temperature factor
        float boosted_temp_factor = temp_factor;
        if (temp_boost > 1.0f) {
            if (temp_factor > 0.5f) {
                // Enhance warm bias - make it more pronounced at low brightness
                boosted_temp_factor = 0.5f + (temp_factor - 0.5f) * temp_boost;
            } else {
                // Enhance cool bias  
                boosted_temp_factor = temp_factor / temp_boost;
            }
            boosted_temp_factor = std::min(std::max(boosted_temp_factor, 0.0f), 1.0f);
        }
        
        // Interpolate between cool and warm white mixes using the raw color ratios
        ColorMix base_mix;
        base_mix.red = interpolate(cool.red, warm.red, boosted_temp_factor);
        base_mix.green = interpolate(cool.green, warm.green, boosted_temp_factor);
        base_mix.blue = interpolate(cool.blue, warm.blue, boosted_temp_factor);
        base_mix.white = interpolate(cool.white, warm.white, boosted_temp_factor);
        
        // Apply gamma correction to the mapped brightness
        float gamma_brightness = apply_gamma_correction(mapped_brightness);
        
        // Scale all channels proportionally to maintain color ratios
        ColorMix result;
        result.red = base_mix.red * gamma_brightness;
        result.green = base_mix.green * gamma_brightness;
        result.blue = base_mix.blue * gamma_brightness;
        result.white = base_mix.white * gamma_brightness * effective_white_brightness;

        // For very low brightness, apply a minimum floor to preserve color ratios
        // This is critical for maintaining warm white appearance at very low levels
        if (brightness < 0.1f && brightness > 0.0f) {
            float min_channel = minimum_tunable_brightness_ * 0.1f;  // 10% of minimum brightness as floor
            
            // Apply minimum only to non-zero channels to preserve color ratios
            if (result.red > 0.0f && result.red < min_channel) result.red = min_channel;
            if (result.green > 0.0f && result.green < min_channel) result.green = min_channel;
            if (result.blue > 0.0f && result.blue < min_channel) result.blue = min_channel;
            
            // For warm white at very low brightness, ensure red channel maintains prominence
            if (temp_factor > 0.7f && result.red > 0.0f) {
                // Boost red channel slightly to maintain warmth at very low levels
                result.red = std::max(result.red, min_channel * 1.5f);
            }
        }

        // Ensure values are within bounds
        result.red = std::min(std::max(result.red, 0.0f), 1.0f);
        result.green = std::min(std::max(result.green, 0.0f), 1.0f);
        result.blue = std::min(std::max(result.blue, 0.0f), 1.0f);
        result.white = std::min(std::max(result.white, 0.0f), 1.0f);
        
        return result;
    }
    
    float interpolate(float min_val, float max_val, float factor) {
        return min_val + (max_val - min_val) * factor;
    }
};

}  // namespace tunableRGBW
}  // namespace esphome
