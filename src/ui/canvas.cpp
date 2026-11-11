#include "editor.hpp"
namespace touch::ui {
void Editor::canvas() {
    ImGui::PushStyleColor(ImGuiCol_ChildBg,theme::background);
    ImGui::BeginChild("canvas", ImVec2(0, 0), false,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    origin = ImGui::GetCursorScreenPos();
    canvas_size = ImGui::GetContentRegionAvail();
    auto& io = ImGui::GetIO();
    ImGui::InvisibleButton("canvas_surface", canvas_size,
                           ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight |
                               ImGuiButtonFlags_MouseButtonMiddle);
    bool hovered = ImGui::IsItemHovered();
    auto mouse = io.MousePos;
    auto draw = ImGui::GetWindowDrawList();
    draw->PushClipRect(origin, ImVec2(origin.x + canvas_size.x, origin.y + canvas_size.y), true);
    if (hovered) {
        add_position = world(mouse);
        if (io.MouseWheel != 0 && !transform && !wire && !box_select) {
            if(io.KeyShift){pan.x+=io.MouseWheel*22; }else{
            auto before = world(mouse);
            zoom = std::clamp(zoom * std::exp(io.MouseWheel*22*.002f), .25f, 2.5f);
            pan = {mouse.x - origin.x - before.x * zoom, mouse.y - origin.y - before.y * zoom};
            }
        }
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle) ||
            (ImGui::IsKeyDown(ImGuiKey_Space) && ImGui::IsMouseDragging(ImGuiMouseButton_Left))) {
            pan.x += io.MouseDelta.x;
            pan.y += io.MouseDelta.y;
        }
    }
    draw_grid(draw);
    auto hit = draw_nodes(draw, hovered);
    canvas_input(hit, hovered);
    update_transform(hit.edge);
    draw_selection(draw);
    canvas_menu(hit, hovered);
    if(stroke_points.size()>1)draw->AddPolyline(stroke_points.data(),int(stroke_points.size()),stroke_reroute?theme::select:theme::error,0,1);
    draw->PopClipRect();
    ImGui::EndChild();
    ImGui::PopStyleColor();
}
} // namespace touch::ui
