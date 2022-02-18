#include "Core.h"
#include "Profiler.h"
#include "Res.h"

namespace ars {
void init_core() {
    IRes::register_type();
    init_profiler();
}

void destroy_core() {
    destroy_profiler();
}
} // namespace ars