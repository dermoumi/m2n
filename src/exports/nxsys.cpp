#include "../config.hpp"
#include <thread>
#include <chrono>

extern "C"
{
    NX_EXPORT void nxSysSleep(double s)
    {
        auto time = std::chrono::duration<double>(s);
        std::this_thread::sleep_for(
            std::chrono::duration_cast<std::chrono::milliseconds>(time)
        );
    }

    NX_EXPORT double nxSysGetTime()
    {
        auto t = std::chrono::high_resolution_clock::now().time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(t).count() / 1000.0;
    }
}