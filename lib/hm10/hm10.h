#ifndef HM10_H
#define HM10_H

#include <string>
#include <functional>

#include <cstdint>
#include <cstddef>

namespace HM10
{
    using ConnectCb = std::function<void (bool)>;
    using WriteCb = std::function<void (uint8_t const *, size_t)>;
    
    bool start(std::string const & name);

    void stop();

    bool set(uint8_t const * data, size_t size);

    bool set(std::string const & data);

    template <typename T>
    bool set(T data)
    {
        return set(reinterpret_cast<uint8_t const *>(&data), sizeof data);
    }

    void onConnect(ConnectCb cb);
    void onWrite(WriteCb cb);
}

#endif
