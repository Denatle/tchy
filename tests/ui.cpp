#include "../src/ui/editor.hpp"
#include <iostream>
int main() {
    try {
        ImGui::CreateContext();
        touch::ui::theme::configure();
        touch::ui::theme::configure_font();
        auto& io = ImGui::GetIO();
        io.DisplaySize = {1200, 760};
        io.DeltaTime = 1.f / 60;
        unsigned char* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        {
            touch::FFmpegMedia media;
            touch::Registry registry;
            touch::register_builtins(registry, media);
            touch::PluginManager plugins(registry);
            touch::JsonProjects projects;
            touch::SdlAudioOutputs audio;
            touch::SteadyClock clock;
            touch::Runtime runtime(registry, audio, clock);
            touch::ui::Editor editor({registry, runtime, plugins, projects, nullptr});
            for (int i = 0; i < 5; ++i) {
                ImGui::NewFrame();
                editor.draw();
                ImGui::Render();
                if (!ImGui::GetDrawData()->Valid)
                    throw std::runtime_error("Invalid UI draw data");
            }
        }
        ImGui::DestroyContext();
        std::cout << "ImGui layout smoke check passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
