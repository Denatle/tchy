#include "editor.hpp"
namespace touch::ui {
void Editor::palette() {
    if (!ImGui::BeginPopup("Operators"))
        return;
    if (ImGui::IsWindowAppearing())
        ImGui::SetKeyboardFocusHere();
    if (ImGui::InputText("Search", search, sizeof(search)))
        palette_cursor = 0;
    if (ImGui::IsKeyPressed(ImGuiKey_Tab)) {
        palette_tab = (palette_tab + 1) % 5;
        palette_cursor = 0;
    }
    const char* labels[] = {"All", "TOP", "CHOP", "DAT", "Convert"};
    for (int i = 0; i < 5; ++i) {
        if (i)
            ImGui::SameLine();
        if (ImGui::Selectable(labels[i], palette_tab == i, 0, ImVec2(i == 4 ? 65 : 40, 0))) {
            palette_tab = i;
            palette_cursor = 0;
        }
    }
    std::vector<const Descriptor*> results;
    for (auto& [id, d] : services.registry.all()) {
        if(pending_wire){try{auto signal=port_signal(document.graph,services.registry,pending_wire->port,pending_wire->output);if(!compatible_port(d,signal,!pending_wire->output))continue;}catch(...){continue;}}
        std::string name = d.label, query = search;
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        std::transform(query.begin(), query.end(), query.begin(), ::tolower);
        if (name.find(query) == std::string::npos)
            continue;
        auto has = [&](Signal s) {
            return std::find(d.inputs.begin(), d.inputs.end(), s) != d.inputs.end() ||
                   std::find(d.outputs.begin(), d.outputs.end(), s) != d.outputs.end();
        };
        if (palette_tab == 1 && !has(Signal::Video) && !has(Signal::Mask))
            continue;
        if (palette_tab == 2 && !has(Signal::Audio) && !has(Signal::Control) &&
            !has(Signal::Channels))
            continue;
        if (palette_tab == 3 && !has(Signal::Table))
            continue;
        if (palette_tab == 4 &&
            (d.inputs.empty() || d.outputs.empty() || d.inputs[0] == d.outputs[0]))
            continue;
        results.push_back(&d);
    }
    std::sort(results.begin(), results.end(), [](auto a, auto b) { return a->label < b->label; });
    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
        ++palette_cursor;
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
        --palette_cursor;
    palette_cursor = std::clamp(palette_cursor, 0, std::max(0, int(results.size()) - 1));
    ImGui::BeginChild("catalog", ImVec2(310, 360));
    for (size_t i = 0; i < results.size(); ++i) {
        bool enter = int(i) == palette_cursor && ImGui::IsKeyPressed(ImGuiKey_Enter);
        if (ImGui::Selectable(results[i]->label.c_str(), int(i) == palette_cursor) || enter) {
            add_node(*results[i]);
            search[0] = 0;
            ImGui::CloseCurrentPopup();
        }
        if (int(i) == palette_cursor &&
            (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || ImGui::IsKeyPressed(ImGuiKey_UpArrow)))
            ImGui::SetScrollHereY();
    }
    ImGui::EndChild();
    ImGui::EndPopup();
}
} // namespace touch::ui
