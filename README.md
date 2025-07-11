# tunableRGBW
A newly-developed generic implementation of color temperature ability added to any bulb that is RGBW for integration with ESPHome. This code allows for the fine tuning of red and blue mixing with the white output to emulate a colore temperature bulb.

Formerly, this was developed exclusively for the older Wyze Bulb Color (RGBW, WLPA19C). Here's a link to the Tasmota template and bulb reference: https://templates.blakadder.com/wyze_WLPA19C.html

Instructions for using this component:

- Make a folder for "custom_components" alongside your ESPHome files
  - https://esphome.io/custom/custom_component.html (I know there's a push to go to "external_components")
  - In my case, running within Docker, I have a "custom_components" folder nested alongside all my applicable device folders
- Copy "__init__.py", "light.py", and "tunableRGBW.h" into that directory
- Include the text from the "tunableRGBW.yaml" in your ESPHome config for the light
- Add any other custom components and light effects in your YAML
- Flash the light
- Enjoy a tunable white feature running ESPHome on a cool ESP32 enabled LED bulb!

Key improvements over old "wyzeRGBW" implementation:

- Allows for dynamic adjustment of color mix via sliders exposed to HA (R, G, B, W for both warm and cool)
- Ensures that the brightness of the color output doesn't drop so low as to be non-visible (adjustable with "minimum_color_brightness" in YAML)
- Ensures that the brightness of the tunable white output doesn't drop so low as to be non-visible (adjustable with "minimum_tunable_brightness" in YAML)
- Allows for a color_temperature_boost feature to ensure that as the light dims, the red and blue aren't lost in color mixing
- Attempts to smooth dimming in a way that keeps the light looking warmer or cooler as the brightness diminishes


From what I can tell, these projects helped inform the way I created this code (along with some sanity checking from ChatGPT):

https://gist.github.com/madjam002/31cc88640efa370630fed6914fa4eb7f

https://gist.github.com/triphoppingman/76153ddf58072b10e229e9147b2bdf72

https://github.com/triphoppingman/CustomRGBWLight

Thanks for checking this out and I hope you find it useful!


Support me, my wife, and 5 kiddos here:
https://www.buymeacoffee.com/benpowers
