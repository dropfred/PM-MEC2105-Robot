#ifndef XBLUE_H
#define XBLUE_H

#include <string>
#include <functional>

namespace XBlue
{
    bool start(std::string const & name);
    void stop();

    using ButtonCb = std::function<void (bool)>;
    using SharedButtonCb = std::function<void (std::string const &, bool)>;

    using ToggleCb = std::function<void (bool)>;
    using SharedToggleCb = std::function<void (std::string const &, bool)>;

    using SliderCb = std::function<void (float)>;
    using SharedSliderCb = std::function<void (std::string const &, float)>;

    using PadCb = std::function<void (float, float)>;
    using SharedPadCb = std::function<void (std::string const &, float, float)>;

    using TextCb = std::function<void (std::string const &)>;
    using SharedTextCb = std::function<void (std::string const &, std::string const &)>;

    void on_button(std::string const & name, ButtonCb cb);
    void on_button(std::string const & name, SharedButtonCb cb);

    void on_toggle(std::string const & name, ToggleCb cb);
    void on_toggle(std::string const & name, SharedToggleCb cb);

    void on_slider(std::string const & name, SliderCb cb);
    void on_slider(std::string const & name, SharedSliderCb cb);

    void on_pad(std::string const & name, PadCb cb);
    void on_pad(std::string const & name, SharedPadCb cb);

    void on_text(std::string const & name, TextCb cb);
    void on_text(std::string const & name, SharedTextCb cb);
}

#endif
