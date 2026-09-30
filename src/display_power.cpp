#include "display_power.h"

#include <cstdio>
#include <cstdlib>

namespace fotoframe {

void set_display_power(bool on) {
    const char* command = on ? "wlopm --on '*'" : "wlopm --off '*'";
    const int status = std::system(command);
    std::printf("%s -> %d\n", command, status);
}

}  // namespace fotoframe
