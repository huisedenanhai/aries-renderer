#include <ars/runtime/core/Profiler.h>
#include <ars/runtime/engine/Engine.h>
#include <ars/runtime/render/IWindow.h>
#include <thread>

using namespace ars;

class ProfilerApplication : public engine::IApplication {
  public:
    void update(double dt) override {
        {
            using namespace std::chrono_literals;
            ARS_PROFILER_SAMPLE("Sample Top", 0xFFFFFFFF);
            {
                ARS_PROFILER_SAMPLE("Update", 0xFA123A4F);
                std::this_thread::sleep_for(3ms);
            }
            {
                ARS_PROFILER_SAMPLE("Render", 0x0A177A4F);
                std::this_thread::sleep_for(2ms);
            }
        }

        window()->present(nullptr);
    }

    void on_imgui() override {
        profiler_on_gui("Profiler", _profiler_state);
    }

    void destroy() override {
        destroy_profiler();
    }

  private:
    ProfilerGuiState _profiler_state{};
};

int main() {
    engine::start_engine(std::make_unique<ProfilerApplication>());
}