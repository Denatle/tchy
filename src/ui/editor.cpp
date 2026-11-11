#include "editor.hpp"
#include <SDL3/SDL_opengl.h>
namespace touch { Graph demo_graph(const Registry&); }
namespace touch::ui {
Editor::Editor(Dependencies deps) : services(deps) {
    if (services.preferences) {
        auto p = services.preferences->load();
        grid = p.grid;
        inspector_visible = p.inspector;
        muted = p.muted;
        volume_db = p.volume_db;
        background = p.background;
        services.runtime.volume(volume_db);
        services.runtime.mute(muted);
    }
    document.graph=demo_graph(services.registry);
    saved_graph=document.graph;
    services.runtime.update(document.graph);
}
Editor::~Editor() {
    if (services.preferences)
        try {
            services.preferences->save({grid, inspector_visible, muted, volume_db, background});
        } catch (const std::exception& e) {
            SDL_Log("Could not save preferences: %s", e.what());
        }
    if (texture)
        glDeleteTextures(1, &texture);
}
void Editor::changed() {
    dirty = document.graph!=saved_graph;
    services.runtime.update(document.graph);
}
void Editor::add_node(const Descriptor& d) {
    try{Id id=0;document.edit([&](Graph& g){id=add(g,d,add_position.x,add_position.y);if(pending_wire){auto signal=port_signal(g,services.registry,pending_wire->port,pending_wire->output);auto index=compatible_port(d,signal,!pending_wire->output);if(!index)throw std::runtime_error("Choose a compatible socket");Edge edge=pending_wire->output?Edge{pending_wire->port,{id,*index}}:Edge{{id,*index},pending_wire->port};connect(g,edge,services.registry);}});document.selected={id};active_node=id;cancel_wire();status.clear();changed();}catch(const std::exception& e){status=e.what();document.prune_selection();}
}
void Editor::conversion_popup() {
    if (!conversion)
        return;
    if (!ImGui::IsPopupOpen("Convert connection"))
        ImGui::OpenPopup("Convert connection");
    if (ImGui::BeginPopup("Convert connection")) {
        try {
            auto edge = *conversion;
            auto from = port_signal(document.graph, services.registry, edge.from, true),
                 to = port_signal(document.graph, services.registry, edge.to, false);
            auto options = converters(services.registry, from, to);
            if (options.empty())
                ImGui::TextUnformatted("No compatible converter installed");
            for (auto d : options)
                if (ImGui::Selectable(d->label.c_str())) {
                    document.edit([&](Graph& g) {
                        auto id = add(g, *d, add_position.x, add_position.y);
                        connect(g, {edge.from, {id, 0}}, services.registry);
                        auto output=std::find(d->outputs.begin(),d->outputs.end(),to)-d->outputs.begin();
                        connect(g,{{id,size_t(output)},edge.to},services.registry);
                        if(wire_original&&wire_original->to!=edge.to)std::erase(g.edges,*wire_original);
                    });
                    changed();
                    cancel_wire();
                    ImGui::CloseCurrentPopup();
                }
            if (ImGui::Button("Cancel")) {
                cancel_wire();
                ImGui::CloseCurrentPopup();
            }
        } catch (const std::exception& e) {
            status = e.what();
            conversion.reset();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
void Editor::drop(const char* file) {
    if (!file)
        return;
    auto p = std::filesystem::path(file);
    if (p.extension() == ".json") {
        load(file);
        return;
    }
    auto ext = p.extension().string();
    std::string kind = ext == ".png" || ext == ".jpg" || ext == ".jpeg"   ? "builtin.Image"
                       : ext == ".wav" || ext == ".mp3" || ext == ".flac" ? "builtin.AudioFile"
                                                                          : "builtin.Movie";
    document.edit([&](Graph& g) {
        auto id = add(g, *services.registry.find(kind), add_position.x, add_position.y);
        g.find(id)->file = file;
        document.selected = {id};
    });
    changed();
}
} // namespace touch::ui
