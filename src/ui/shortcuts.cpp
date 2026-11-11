#include "editor.hpp"
namespace touch::ui {
void Editor::begin_transform(int mode, bool modal) {
    modal_transform = modal;
    if (document.selected.empty())
        return;
    if(!gesture_before)gesture_before = document.graph;
    gesture_layout.clear();
    for (auto id : document.selected)
        if (document.graph.layout.contains(id))
            gesture_layout[id] = document.graph.layout.at(id);
    gesture_start = ImGui::GetIO().MousePos;
    transform = mode;
    axis = 0;
}
void Editor::finish_transform(bool cancel) {
    if (!gesture_before)
        return;
    if(cancel){document.graph=*gesture_before;document.prune_selection();changed();}else{document.commit(*gesture_before);changed();}
    gesture_before.reset();
    gesture_layout.clear();
    transform = 0;
}
void Editor::fit(bool selected) {
    std::optional<CanvasRect> extent;
    for(auto& n:document.graph.nodes){if(selected&&!document.selected.contains(n.id))continue;auto l=document.graph.layout.at(n.id);auto size=node_size(n,services.registry.find(n.type));if(!reroute(n))size.x=l.width;CanvasRect r{{l.x,l.y},{l.x+size.x,l.y+size.y}};if(!extent)extent=r;else{extent->min.x=std::min(extent->min.x,r.min.x);extent->min.y=std::min(extent->min.y,r.min.y);extent->max.x=std::max(extent->max.x,r.max.x);extent->max.y=std::max(extent->max.y,r.max.y);}}
    if(!extent){if(!selected){pan={0,0};zoom=1;}return;}zoom=std::clamp(std::min((canvas_size.x-64)/(extent->max.x-extent->min.x),(canvas_size.y-64)/(extent->max.y-extent->min.y)),.25f,1.6f);pan={canvas_size.x*.5f-(extent->min.x+extent->max.x)*.5f*zoom,canvas_size.y*.5f-(extent->min.y+extent->max.y)*.5f*zoom};
}
void Editor::duplicate_selection(){if(document.selected.empty())return;auto before=document.graph;document.duplicate();if(document.graph==before)return;document.undo_stack.pop_back();for(auto id:document.selected){auto& l=document.graph.layout.at(id);l.x-=32;l.y-=32;}gesture_before=before;active_node=*document.selected.rbegin();begin_transform(1,true);}
void Editor::copy_selection() {
    clipboard = document.graph;
    std::erase_if(clipboard.nodes, [&](auto n) { return !document.selected.contains(n.id); });
    std::erase_if(clipboard.edges, [&](auto e) {
        return !document.selected.contains(e.from.node) || !document.selected.contains(e.to.node);
    });
    clipboard.preview = clipboard.video = clipboard.audio = 0;
    try {
        SDL_SetClipboardText(
            encode_project(clipboard, std::filesystem::current_path() / "clipboard.json")
                .dump()
                .c_str());
    } catch (const std::exception& e) {
        status = e.what();
    }
}
void Editor::paste() {
    if (auto text = SDL_GetClipboardText()) {
        try {
            clipboard = decode_project(Json::parse(text),
                                       std::filesystem::current_path() / "clipboard.json",
                                       services.registry);
        } catch (...) {
        }
        SDL_free(text);
    }
    if (clipboard.nodes.empty())
        return;
    document.edit([&](Graph& g) {
        std::map<Id, Id> ids;
        document.selected.clear();
        for (auto n : clipboard.nodes) {
            Id old = n.id;
            n.id = ++g.next;
            ids[old] = n.id;
            g.nodes.push_back(n);
            auto l = clipboard.layout.at(old);
            l.x += 40;
            l.y += 40;
            g.layout[n.id] = l;
            document.selected.insert(n.id);
        }
        for (auto e : clipboard.edges)
            g.edges.push_back(
                {{ids.at(e.from.node), e.from.index}, {ids.at(e.to.node), e.to.index}});
    });
    changed();
}
void Editor::shortcuts() {
    auto& io = ImGui::GetIO();
    if(ImGui::IsKeyPressed(ImGuiKey_Escape)){if(gesture_before)finish_transform(true);cancel_wire();box_select=box_dragging=false;if(stroke_before){document.graph=*stroke_before;stroke_before.reset();stroke_edges.clear();changed();}settings_open=false;return;}
    if(transform&&modal_transform){if(ImGui::IsKeyPressed(ImGuiKey_X))axis=axis==1?0:1;if(ImGui::IsKeyPressed(ImGuiKey_Y))axis=axis==2?0:2;return;}
    if (io.WantTextInput || ImGui::IsPopupOpen("Operators")||ImGui::IsPopupOpen("Convert connection"))return;
#ifdef __APPLE__
    bool command = io.KeySuper;
#else
    bool command = io.KeyCtrl;
#endif
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_X) && !command) {
        try {
            document.edit(
                [&](Graph& g) { erase_and_reconnect(g, document.selected, services.registry); });
            document.selected.clear();
            changed();
        } catch (const std::exception& e) {
            status = e.what();
        }
        return;
    }
    if (command) {
        if (ImGui::IsKeyPressed(ImGuiKey_Z)) {
            if (io.KeyShift)
                document.redo();
            else
                document.undo();
            changed();
        }
        if (ImGui::IsKeyPressed(ImGuiKey_C))
            copy_selection();
        if (ImGui::IsKeyPressed(ImGuiKey_X)) {
            copy_selection();
            document.erase();
            changed();
        }
        if (ImGui::IsKeyPressed(ImGuiKey_V))
            paste();
        if (ImGui::IsKeyPressed(ImGuiKey_O))project_action(2);
        if (ImGui::IsKeyPressed(ImGuiKey_S))
            save(io.KeyShift);
        if (ImGui::IsKeyPressed(ImGuiKey_N))project_action(1);
        return;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Tab) || ImGui::IsKeyPressed(ImGuiKey_F3) ||
        (io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_A)))
        ImGui::OpenPopup("Operators");
    if (ImGui::IsKeyPressed(ImGuiKey_B)) {
        box_select = true;
        box_start = io.MousePos;box_extend=io.KeyShift;box_dragging=false;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_A) && !io.KeyShift) {
        document.selected.clear();
        if (!io.KeyAlt)
            for (auto n : document.graph.nodes)
                document.selected.insert(n.id);
    }
    if (io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_D)) {
        duplicate_selection();
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Delete) || (ImGui::IsKeyPressed(ImGuiKey_X) && !transform)) {
        document.erase();
        changed();
    }
    if (ImGui::IsKeyPressed(ImGuiKey_G))
        begin_transform(1, true);
    if (ImGui::IsKeyPressed(ImGuiKey_S))
        begin_transform(2, true);
    if (ImGui::IsKeyPressed(ImGuiKey_R)){if(document.selected.empty()){CanvasHit hit;auto mouse=io.MousePos;for(auto e:document.graph.edges){auto points=cable_points(port_position(e.from,true),port_position(e.to,false));for(size_t i=1;i<points.size();++i)if(segment_distance(mouse,points[i-1],points[i])<=6)hit.edge=e;}if(hit.edge)insert_reroute(*hit.edge,world(mouse));}else begin_transform(3,true);}
    if (transform && ImGui::IsKeyPressed(ImGuiKey_X))
        axis = 1;
    if (transform && ImGui::IsKeyPressed(ImGuiKey_Y))
        axis = 2;
    if (ImGui::IsKeyPressed(ImGuiKey_F) || ImGui::IsKeyPressed(ImGuiKey_Home))
        fit();
    if (ImGui::IsKeyPressed(ImGuiKey_Period))
        fit(true);
    if (ImGui::IsKeyPressed(ImGuiKey_N))
        inspector_visible = !inspector_visible;
}
} // namespace touch::ui
