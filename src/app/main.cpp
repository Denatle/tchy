#include "../ui/editor.hpp"
#include <SDL3/SDL_opengl.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>
#include <iostream>
int main(int argc, char** argv) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        std::cerr << SDL_GetError() << '\n';
        return 1;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
#ifdef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif
#ifdef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif
    auto window =
        SDL_CreateWindow("Touch", 1200, 760,
                         SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!window) {
        std::cerr << SDL_GetError() << '\n';
        SDL_Quit();
        return 1;
    }
    SDL_SetWindowMinimumSize(window, 640, 380);
    auto context = SDL_GL_CreateContext(window);
    if (!context) {
        std::cerr << SDL_GetError() << '\n';
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_GL_MakeCurrent(window, context);
    SDL_GL_SetSwapInterval(1);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    touch::ui::theme::configure();
    touch::ui::theme::configure_font();
    ImGui_ImplSDL3_InitForOpenGL(window, context);
#ifdef __APPLE__
    ImGui_ImplOpenGL3_Init("#version 150");
#else
    ImGui_ImplOpenGL3_Init("#version 330 core");
#endif
    {
        touch::FFmpegMedia media;
        touch::Registry registry;
        touch::register_builtins(registry, media);
        touch::PluginManager plugins(registry);
        std::vector<std::filesystem::path> paths;
        auto base = SDL_GetBasePath();
        if (base)
            paths.emplace_back(std::filesystem::path(base) / "plugins");
        if (auto user = SDL_GetPrefPath("Touch", "Touch")) {
            paths.emplace_back(std::filesystem::path(user) / "plugins");
            SDL_free(user);
        }
        for (auto path : paths)
            if (std::filesystem::exists(path))
                for (auto entry : std::filesystem::directory_iterator(path)) {
                    auto ext = entry.path().extension();
                    if (ext == ".so" || ext == ".dylib" || ext == ".dll")
                        try {
                            plugins.load(entry.path());
                        } catch (const std::exception& e) {
                            std::cerr << e.what() << '\n';
                        }
                }
        touch::JsonProjects projects;
        std::filesystem::path preferences_path;
        if (auto user = SDL_GetPrefPath("Touch", "Touch")) {
            preferences_path = std::filesystem::path(user) / "settings.json";
            SDL_free(user);
        }
        touch::JsonPreferences preferences(preferences_path);
        touch::SdlAudioOutputs audio;
        touch::SteadyClock clock;
        touch::Runtime runtime(registry, audio, clock);
        touch::ui::Editor editor({registry, runtime, plugins, projects, window, &preferences});
        bool smoke = argc > 1 && std::string(argv[1]) == "--smoke";
        if (argc > 1 && !smoke)
            editor.drop(argv[1]);
        int frames = 0;
        bool done = false;
        while (!done) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL3_ProcessEvent(&event);
                if (event.type == SDL_EVENT_QUIT)
                    editor.request_close();
                if (event.type == SDL_EVENT_DROP_FILE) {
                    editor.drop(event.drop.data);
                }
            }
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();
            editor.draw();
            if(editor.wants_close())done=true;
            ImGui::Render();
            int w, h;
            SDL_GetWindowSizeInPixels(window, &w, &h);
            glViewport(0, 0, w, h);
            glClearColor(.1f, .1f, .1f, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            SDL_GL_SwapWindow(window);
            if (smoke && ++frames >= 5)
                done = true;
        }
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
