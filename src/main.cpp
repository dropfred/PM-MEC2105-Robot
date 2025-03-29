#include <Arduino.h>

#include <string>

namespace
{
    std::string const DEVICE_NAME {"MEC2105"};
}

#ifdef TEST_XBLUE

#include <xblue.h>

void setup()
{
    Serial.begin(115200);

    XBlue::on_button("b0", [] (bool b)
    {
        Serial.print("button "); Serial.println(b ? "pressed" : "released");
    });

    XBlue::on_slider("sl0", [] (float f)
    {
        Serial.printf("slider: %.2f\n", f);
    });

    XBlue::on_toggle("sw0", [] (bool b)
    {
        Serial.printf("toggle %s\n", b ? "on" : "off");
    });

    XBlue::on_text("t0", [] (std::string const & s)
    {
        Serial.println(("text: " + s).c_str());
    });

    XBlue::on_pad("d0", [] (float x, float y)
    {
        Serial.printf("pad: (%.2f, %.2f)\n", x, y);
    });

    XBlue::on_pad("d1", [] (float x, float y)
    {
        Serial.printf("joystick: (%.2f, %.2f)\n", x, y);
    });

    XBlue::start(DEVICE_NAME);
}

void loop()
{
}

#else

#include <hm10.h>

#include <cctype>

void setup()
{
    Serial.begin(115200);

    HM10::onConnect([] (bool connected)
    {
        Serial.println((DEVICE_NAME + ": " + (connected ? "connected" : "disconnected")).c_str());    
    });

    HM10::onWrite([] (uint8_t const * data, size_t size)
    {
        Serial.println((DEVICE_NAME + ':').c_str());
        bool is_text = true;
        for (size_t i = 0; i < size; ++i)
        {
            uint8_t b = data[i];
            is_text = is_text && (std::isprint(b) != 0);
            Serial.printf("%s%02X", (i == 0) ? "  " : "-", b);
        }
        Serial.println("");
        if (is_text)
        {
            printf("  reads as '%s'\n", std::string(reinterpret_cast<char const *>(data), size).c_str());
        }
    });

    HM10::start(DEVICE_NAME);
}

void loop()
{
    std::string s = {};
    while (true)
    {
        if (Serial.available())
        {
            int c = Serial.read();
            if (c == '\n')
            {
                Serial.println((DEVICE_NAME + ": " + s).c_str());
                HM10::set(s);
                break;
            }
            else if (c != '\r')
            {
                s+= c;
            }
        }
    }
}

#endif