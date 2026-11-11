#include "editor.hpp"
namespace touch::ui {
namespace {
struct Request {
    std::shared_ptr<PendingDialog> result;
    int action;
};
void SDLCALL completed(void* userdata, const char* const* files, int) {
    std::unique_ptr<Request> request(static_cast<Request*>(userdata));
    std::lock_guard lock(request->result->mutex);
    if (!files) {
        request->result->error = SDL_GetError();
        return;
    }
    if (!files[0])
        return;
    request->result->action = request->action;
    request->result->path = files[0];
}
} // namespace
void Editor::file_dialog(int action) {
    auto request = new Request{dialog, action};
    if (action == 2)
        SDL_ShowSaveFileDialog(completed, request, services.window, nullptr, 0,
                               path.empty() ? nullptr : path.c_str());
    else
        SDL_ShowOpenFileDialog(completed, request, services.window, nullptr, 0, nullptr, false);
}
void Editor::process_dialog() {
    std::lock_guard lock(dialog->mutex);
    if (!dialog->error.empty()) {
        status = dialog->error;
        dialog->error.clear();
    }
    if (!dialog->action)
        return;
    try {
        switch (dialog->action) {
        case 1:
            load(dialog->path);
            break;
        case 2:
            path = dialog->path;
            save();
            break;
        case 3:
            services.plugins.load(dialog->path);
            status = "Plugin loaded";
            break;
        case 4:
            if (!document.selected.empty()) {
                document.edit([&](Graph& g) {
                    if (auto n = g.find(*document.selected.begin()))
                        n->file = dialog->path;
                });
                changed();
            }
            break;
        }
    } catch (const std::exception& e) {
        status = e.what();
    }
    dialog->action = 0;
}
void Editor::load(const std::string& file) {
    try {
        auto g = services.projects.load(file, services.registry);
        services.runtime.run(false);
        document.graph = std::move(g);
        document.selected.clear();
        document.undo_stack.clear();
        document.redo_stack.clear();
        path = file;
        saved_graph=document.graph;
        dirty = false;
        services.runtime.update(document.graph);
        fit();
        status.clear();
    } catch (const std::exception& e) {
        status = e.what();
    }
}
void Editor::save(bool as) {
    if (as || path.empty()) {
        file_dialog(2);
        return;
    }
    try {
        services.projects.save(path, document.graph);
        saved_graph=document.graph;
        dirty = false;
        status.clear();
    } catch (const std::exception& e) {
        status = e.what();
    }
}
} // namespace touch::ui

namespace touch::ui {
void Editor::project_action(int action){if(dirty){pending_project=action;return;}apply_project_action(action);}
void Editor::apply_project_action(int action){if(action==1){services.runtime.run(false);document={};saved_graph=document.graph;path.clear();dirty=false;wire.reset();pending_wire.reset();gesture_before.reset();transform=0;services.runtime.update(document.graph);}if(action==2)file_dialog(1);if(action==3)close_requested=true;}
void Editor::discard_popup(){if(!pending_project)return;if(!ImGui::IsPopupOpen("Discard unsaved changes?"))ImGui::OpenPopup("Discard unsaved changes?");if(ImGui::BeginPopupModal("Discard unsaved changes?",nullptr,ImGuiWindowFlags_AlwaysAutoResize)){ImGui::TextUnformatted("Save the project first if you want to keep these edits.");if(ImGui::Button("Discard")){auto action=pending_project;pending_project=0;ImGui::CloseCurrentPopup();apply_project_action(action);}ImGui::SameLine();if(ImGui::Button("Cancel")){pending_project=0;ImGui::CloseCurrentPopup();}ImGui::EndPopup();}}
}
