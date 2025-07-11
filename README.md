# wyzeRGBW

CHECK OUT THE NEW "tunableRGBW" BRANCH for an update to this that's more versatile! (Updated July 2025)


Old version readme below:

Wyze Bulb Color (RGBW) Tunable Custom Component for use with ESPHome

I've used these bulbs with Tasmota for a while, but prefer ESPHome and the customization options that seem more intuitive with ESPHome. 

Here's a link to the Tasmota template and bulb reference: 
https://templates.blakadder.com/wyze_WLPA19C.html

Instructions for using this component:

- Make a folder for "custom_components" alongside your ESPHome files
  - https://esphome.io/custom/custom_component.html (these are being deprecated and I'll have to adjust in the future for an "external component")
  - In my case, running within Docker, I have a "custom_components" folder that's in the same directory as the "bulb_wyze_color" directory
- Copy "__init__.py", "light.py", and "wyzeRGBW.h" into that directory
- Include the text from the "wyzeRGBW.yaml" in your ESPHome config for the light
- Add any other custom components and light effects in your YAML
- Flash the light
  - Keep in mind, the esp-idf framework that makes BLE tracking more efficient requires a non-OTA flash to reformat the partition
- Enjoy a tunable white feature running ESPHome on a cool ESP32 enabled LED bulb!

  
From what I can tell, these projects helped inform the way I created this code (along with some sanity checking from ChatGPT):

https://gist.github.com/madjam002/31cc88640efa370630fed6914fa4eb7f

https://gist.github.com/triphoppingman/76153ddf58072b10e229e9147b2bdf72

https://github.com/triphoppingman/CustomRGBWLight

Thanks for checking this out and I hope you find it useful!


Support me, my wife, and 5 kiddos here:
https://www.buymeacoffee.com/benpowers
