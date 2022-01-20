#include <ars/runtime/core/Profiler.h>
#include <ars/runtime/engine/Engine.h>
#include <ars/runtime/render/IWindow.h>
#include <thread>

using namespace ars;

class ProfilerApplication : public engine::IApplication {
  public:
    void start() override {
        init_profiler();
        profiler_enable_group(PROFILER_GROUP_CPU_MAIN_THREAD, true);

        {
            using namespace std::chrono_literals;
            ARS_PROFILE_SAMPLE("Sample Top", 0xFFFFFFFF);
            {
                ARS_PROFILE_SAMPLE("Update", 0xFA123A4F);
                std::this_thread::sleep_for(3ms);
            }
            {
                ARS_PROFILE_SAMPLE("Render", 0x0A177A4F);
                std::this_thread::sleep_for(2ms);
            }
        }
    }

    void update(double dt) override {
        window()->present(nullptr);
    }

    void on_imgui() override {
        ProfilerGuiState state{};
        profiler_on_gui("Profiler", state);
    }

    void destroy() override {
        destroy_profiler();
    }
};

int main() {
    engine::start_engine(std::make_unique<ProfilerApplication>());
}