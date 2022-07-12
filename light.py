import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light, output
from esphome.const import CONF_OUTPUT_ID, CONF_COLOR_TEMPERATURE, CONF_BRIGHTNESS, \
  CONF_RED, CONF_GREEN, CONF_BLUE, CONF_WHITE, \
  CONF_COLD_WHITE_COLOR_TEMPERATURE, CONF_WARM_WHITE_COLOR_TEMPERATURE

wyzeRGBW_ns = cg.esphome_ns.namespace('wyzeRGBW')
wyzeRGBW = wyzeRGBW_ns.class_('wyzeRGBW', light.LightOutput)

CONFIG_SCHEMA = light.RGB_LIGHT_SCHEMA.extend({
    cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(wyzeRGBW),
    cv.Required(CONF_RED): cv.use_id(output.FloatOutput),
    cv.Required(CONF_GREEN): cv.use_id(output.FloatOutput),
    cv.Required(CONF_BLUE): cv.use_id(output.FloatOutput),
    cv.Required(CONF_WHITE): cv.use_id(output.FloatOutput),
    cv.Required(CONF_COLD_WHITE_COLOR_TEMPERATURE): cv.color_temperature,
    cv.Required(CONF_WARM_WHITE_COLOR_TEMPERATURE): cv.color_temperature,
})


def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    yield light.register_light(var, config)
    red = yield cg.get_variable(config[CONF_RED])
    cg.add(var.set_output_r(red))
    green = yield cg.get_variable(config[CONF_GREEN])
    cg.add(var.set_output_g(green))
    blue = yield cg.get_variable(config[CONF_BLUE])
    cg.add(var.set_output_b(blue))
    white = yield cg.get_variable(config[CONF_WHITE])
    cg.add(var.set_output_w(white))
    cg.add(var.set_temperature_cw(config[CONF_COLD_WHITE_COLOR_TEMPERATURE]))
    cg.add(var.set_temperature_ww(config[CONF_WARM_WHITE_COLOR_TEMPERATURE]))
