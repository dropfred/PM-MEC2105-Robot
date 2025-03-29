#include <xblue.h>
#include <hm10.h>

#include <esp_log.h>

#include <vector>
#include <algorithm>

namespace
{
    template <class C>
    struct Control
    {
        using Cb = C;
        std::string name;
        Cb cb;
    };

    using Button = Control<std::function<void (std::string const &, bool)>>;
    using Toggle = Control<std::function<void (std::string const &, bool)>>;
    using Slider = Control<std::function<void (std::string const &, float)>>;
    using Pad    = Control<std::function<void (std::string const &, float, float)>>;
    using Text   = Control<std::function<void (std::string const &, std::string const &)>>;

    std::vector<Button> buttons {};
    std::vector<Toggle> toggles {};
    std::vector<Slider> sliders {};
    std::vector<Pad>    pads    {};
    std::vector<Text>   texts   {};

    template <class C>
    typename C::Cb get_control(std::vector<C> const & cs, std::string const & name)
    {
        for (auto const & c : cs)
        {
            if (c.name == name) return c.cb;
        }
        return {};
    }

    template <class C>
    void on_control(std::vector<C> & cs, std::string const & name, typename C::Cb cb)
    {
        auto c = std::find_if(cs.begin(), cs.end(), [& name] (C const & c) {return c.name == name;});
        if (c != cs.end())
        {
            if (cb)
            {
                c->cb = cb;
            }
            else
            {
                cs.erase(c);
            }
        }
        else if (cb)
        {
            cs.push_back({name, cb});
        }
    }

    void hm10_callback(uint8_t const * data, size_t size)
    {
        char const * d  = (char const *)(data);
        char const * de = d + size;
        std::string name {};
        std::string value {};

        if ((d == de) || (*d++ != 1))
        {
            // log invalid event
            return;
        }
        while ((d != de) && (*d != 2))
        {
            name += *d++;
        }
        if (d == de)
        {
            // log invalid event
            return;
        }
        ++d;
        while ((d != de) && (*d != 3))
        {
            value += *d++;
        }
        if (d == de)
        {
            // log invalid event
            return;
        }

        if (auto cb = get_control(pads, name); cb)
        {
            auto c = value.find_first_of(',');
            if (c == std::string::npos)
            {
                // log invalid event
                return;
            }
            auto sx = value.substr(0, c);
            auto sy = value.substr(c + 1);
            if (sx.empty() || sy.empty())
            {
                // log invalid event
                return;
            }
            // -512 <= xy < 512
            int x = std::stoi(sx) - 512;
            int y = std::stoi(sy) - 512;
#ifdef XBLUE_NORMALIZE_POSITIVE_PAD
            cb(name, x / ((x > 0) ? 511.0f : 512.0f), y / ((y > 0) ? 511.0f : 512.0f));
#else
            cb(name, x / 512.0f, y / 512.0f);
#endif            
        }
        else if (auto cb = get_control(buttons, name); cb)
        {
            cb(name, value == "1");
        }
        else if (auto cb = get_control(sliders, name); cb)
        {
            cb(name, std::stoul(value) / 100.0f);
        }
        else if (auto cb = get_control(toggles, name); cb)
        {   
            cb(name, value == "1");
        }
        else if (auto cb = get_control(texts, name); cb)
        {   
            cb(name, value);
        }
    }
}

namespace XBlue
{
    bool start(std::string const & name)
    {
        HM10::onWrite(hm10_callback);
        return HM10::start(name);
    }

    void stop()
    {
        HM10::stop();
    }

    void on_pad(std::string const & name, std::function<void (std::string const &, float, float)> cb)
    {
        on_control(pads, name, cb);
    }

    void on_pad(std::string const & name, std::function<void (float, float)> cb)
    {
        on_pad(name, [cb] (std::string const & n, float x, float y) {cb(x, y);});
    }

    void on_button(std::string const & name, std::function<void (std::string const &, bool)> cb)
    {
        on_control(buttons, name, cb);
    }

    void on_button(std::string const & name, std::function<void (bool)> cb)
    {
        on_button(name, [cb] (std::string const & n, bool v) {cb(v);});
    }

    void on_slider(std::string const & name, std::function<void (std::string const &, float)> cb)
    {
        on_control(sliders, name, cb);
    }

    void on_slider(std::string const & name, std::function<void (float)> cb)
    {
        on_slider(name, [cb] (std::string const & n, float v) {cb(v);});
    }

    void on_toggle(std::string const & name, std::function<void (std::string const &, bool)> cb)
    {
        on_control(toggles, name, cb);
    }

    void on_toggle(std::string const & name, std::function<void (bool)> cb)
    {
        on_toggle(name, [cb] (std::string const & n, bool v) {cb(v);});
    }

    void on_text(std::string const & name, std::function<void (std::string const &, std::string const &)> cb)
    {
        on_control(texts, name, cb);
    }

    void on_text(std::string const & name, std::function<void (std::string const &)> cb)
    {
        on_text(name, [cb] (std::string const & n, std::string const & msg) {cb(msg);});
    }
}
