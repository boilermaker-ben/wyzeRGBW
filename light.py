import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light, output, number
from esphome.const import (
    CONF_OUTPUT_ID, CONF_COLOR_TEMPERATURE, CONF_BRIGHTNESS,
    CONF_RED, CONF_GREEN, CONF_BLUE, CONF_WHITE,
    CONF_COLD_WHITE_COLOR_TEMPERATURE, CONF_WARM_WHITE_COLOR_TEMPERATURE,
    CONF_GAMMA_CORRECT,
)

tunableRGBW_ns = cg.esphome_ns.namespace('tunableRGBW')
tunableRGBW = tunableRGBW_ns.class_('tunableRGBW', light.LightOutput)

# Config keys
CONF_WARM_WHITE_MIX = 'warm_white_mix'
CONF_COOL_WHITE_MIX = 'cool_white_mix'
CONF_WHITE_BRIGHTNESS = 'white_brightness'
CONF_MINIMUM_COLOR_BRIGHTNESS = 'minimum_color_brightness'
CONF_MINIMUM_TUNABLE_BRIGHTNESS = 'minimum_tunable_brightness'
CONF_COLOR_TEMPERATURE_BOOST = 'color_temperature_boost'
CONF_BRIGHTNESS_RANGE_EXPANSION = 'brightness_range_expansion'

COLOR_MIX_KEYS = ['red', 'green', 'blue', 'white']

# Allow either static float or a number ID
MIX_VALUE = cv.Any(cv.percentage, cv.use_id(number.Number))

COLOR_MIX_SCHEMA = cv.Schema({
    cv.Optional('red', default=1.0): MIX_VALUE,
    cv.Optional('green', default=1.0): MIX_VALUE,
    cv.Optional('blue', default=1.0): MIX_VALUE,
    cv.Optional('white', default=1.0): MIX_VALUE,
})

DEFAULT_WARM_MIX = {'red': 1.0, 'green': 0.4, 'blue': 0.0, 'white': 1.0}
DEFAULT_COOL_MIX = {'red': 0.8, 'green': 1.0, 'blue': 1.0, 'white': 1.0}

CONFIG_SCHEMA = light.RGB_LIGHT_SCHEMA.extend({
    cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(tunableRGBW),
    cv.Required(CONF_RED): cv.use_id(output.FloatOutput),
    cv.Required(CONF_GREEN): cv.use_id(output.FloatOutput),
    cv.Required(CONF_BLUE): cv.use_id(output.FloatOutput),
    cv.Required(CONF_WHITE): cv.use_id(output.FloatOutput),
    cv.Required(CONF_COLD_WHITE_COLOR_TEMPERATURE): cv.color_temperature,
    cv.Required(CONF_WARM_WHITE_COLOR_TEMPERATURE): cv.color_temperature,
    cv.Optional(CONF_WARM_WHITE_MIX, default=DEFAULT_WARM_MIX): COLOR_MIX_SCHEMA,
    cv.Optional(CONF_COOL_WHITE_MIX, default=DEFAULT_COOL_MIX): COLOR_MIX_SCHEMA,
    cv.Optional(CONF_WHITE_BRIGHTNESS, default=0.8): cv.Any(cv.percentage, cv.use_id(number.Number)),
    cv.Optional(CONF_GAMMA_CORRECT, default=2.2): cv.float_range(min=0.1, max=5.0),
    cv.Optional(CONF_MINIMUM_COLOR_BRIGHTNESS, default=0.2): cv.float_range(min=0.0, max=1.0),
    cv.Optional(CONF_MINIMUM_TUNABLE_BRIGHTNESS, default=0.05): cv.float_range(min=0.0, max=0.5),
    cv.Optional(CONF_COLOR_TEMPERATURE_BOOST, default=1.5): cv.float_range(min=1.0, max=5.0),
    cv.Optional(CONF_BRIGHTNESS_RANGE_EXPANSION, default=True): cv.boolean,
})

def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    yield light.register_light(var, config)

    # Set outputs
    cg.add(var.set_output_r((yield cg.get_variable(config[CONF_RED]))))
    cg.add(var.set_output_g((yield cg.get_variable(config[CONF_GREEN]))))
    cg.add(var.set_output_b((yield cg.get_variable(config[CONF_BLUE]))))
    cg.add(var.set_output_w((yield cg.get_variable(config[CONF_WHITE]))))

    cg.add(var.set_temperature_cw(config[CONF_COLD_WHITE_COLOR_TEMPERATURE]))
    cg.add(var.set_temperature_ww(config[CONF_WARM_WHITE_COLOR_TEMPERATURE]))
    
    # Set configurable parameters
    cg.add(var.set_gamma_correction(config[CONF_GAMMA_CORRECT]))
    cg.add(var.set_minimum_color_brightness(config[CONF_MINIMUM_COLOR_BRIGHTNESS]))
    cg.add(var.set_minimum_tunable_brightness(config[CONF_MINIMUM_TUNABLE_BRIGHTNESS]))
    cg.add(var.set_color_temperature_boost(config[CONF_COLOR_TEMPERATURE_BOOST]))
    cg.add(var.set_brightness_range_expansion(config[CONF_BRIGHTNESS_RANGE_EXPANSION]))
    
    # Handle white brightness
    white_brightness = config[CONF_WHITE_BRIGHTNESS]
    if isinstance(white_brightness, float):
        cg.add(var.set_white_brightness(white_brightness))
    else:
        brightness_number = yield cg.get_variable(white_brightness)
        cg.add(var.set_white_brightness_number(brightness_number))

    # Handle warm white mix
    warm_mix = config[CONF_WARM_WHITE_MIX]
    warm_mix_numbers = []
    warm_mix_has_numbers = False
    
    for color in COLOR_MIX_KEYS:
        value = warm_mix.get(color, 1.0)
        if isinstance(value, float):
            warm_mix_numbers.append(None)
        else:
            warm_mix_numbers.append((yield cg.get_variable(value)))
            warm_mix_has_numbers = True
    
    if warm_mix_has_numbers:
        # Convert None to nullptr for C++
        warm_mix_args = []
        for i, num in enumerate(warm_mix_numbers):
            if num is None:
                warm_mix_args.append(cg.RawExpression('nullptr'))
            else:
                warm_mix_args.append(num)
        cg.add(var.set_warm_mix_numbers(*warm_mix_args))
    else:
        # All values are static floats
        cg.add(var.set_warm_white_mix(
            warm_mix['red'], warm_mix['green'], warm_mix['blue'], warm_mix['white']
        ))

    # Handle cool white mix
    cool_mix = config[CONF_COOL_WHITE_MIX]
    cool_mix_numbers = []
    cool_mix_has_numbers = False
    
    for color in COLOR_MIX_KEYS:
        value = cool_mix.get(color, 1.0)
        if isinstance(value, float):
            cool_mix_numbers.append(None)
        else:
            cool_mix_numbers.append((yield cg.get_variable(value)))
            cool_mix_has_numbers = True
    
    if cool_mix_has_numbers:
        # Convert None to nullptr for C++
        cool_mix_args = []
        for i, num in enumerate(cool_mix_numbers):
            if num is None:
                cool_mix_args.append(cg.RawExpression('nullptr'))
            else:
                cool_mix_args.append(num)
        cg.add(var.set_cool_mix_numbers(*cool_mix_args))
    else:
        # All values are static floats
        cg.add(var.set_cool_white_mix(
            cool_mix['red'], cool_mix['green'], cool_mix['blue'], cool_mix['white']
        ))
