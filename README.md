# Display Tibber price on Arduino E-Paper

Print out the Tibber electric price on a [Lilygo TTGO e-paper display 2.13 inch](http://www.lilygo.cn/prod_view.aspx?TypeId=50061&Id=1393&FId=t3:50061:3) ESP32

![TTGo E-Paper](https://0.rc.xiniu.com/g4/M00/06/1E/CgAG0mEFGAaAIcXpAAI_Z3TUE-A398.jpg)

## Device

Used Arduino IDE with ESP32 Dev Module board settings.

## Development

In the loop the time (from NTP) and electric price (from Tibber API) will be printed once every hour.

### Library

- WiFi to connect to SSID to access internet
- HTTPClient to talk to the Tibber API (Token needed)
- ArduinoJson to parse the response from Tibber API
- GxEPD2 to setup and print to the E-Paper
- Adafruit GFX [Fonts](https://github.com/adafruit/Adafruit-GFX-Library/tree/master/Fonts) 
- `time.h` to ask for current time from NTP
