#pragma once
#include <SDL3/SDL.h>
#include <imgui.h>
#include "canvas_geometry.hpp"
#include <touch/core.hpp>
#include <touch/graph/routing.hpp>
#include <touch/settings/types.hpp>
namespace touch::ui {
struct Dependencies {
    Registry& registry;
    Runtime& runtime;
    PluginManager& plugins;
    ProjectStore& projects;
    SDL_Window* window;
    PreferencesStore* preferences = nullptr;
};
struct Endpoint {
    Port port;
    bool output = false;
};
struct PendingDialog {
    std::mutex mutex;
    int action = 0;
    std::string path, error;
};
struct CanvasHit {
    std::optional<Port> input, output;
    std::optional<Id> node;
    std::optional<Edge> edge;
};
class Editor {
    Dependencies services;
    Document document;
    Id active_node=0;
    Id primary()const{return document.selected.contains(active_node)?active_node:document.selected.empty()?0:*document.selected.begin();}
    std::string path, status;
    bool muted = true, inspector_visible = true, grid = true, dirty = false;
    float volume_db = 0, zoom = 1;
    ImVec2 pan{0, 0}, origin{}, canvas_size{}, add_position{};
    std::optional<Endpoint> wire;
    std::optional<Edge> wire_original;
    std::optional<Graph> wire_before;
    std::optional<Edge> conversion;
    std::optional<Edge> context_edge;
    std::optional<Id> context_node;
    ImVec2 wire_start{};
    void cancel_wire(){wire.reset();wire_original.reset();wire_before.reset();pending_wire.reset();conversion.reset();}
    void duplicate_selection();
    void insert_reroute(Edge,ImVec2);
    void conversion_popup();
    std::optional<Endpoint> pending_wire;
    std::optional<Graph> gesture_before, stroke_before;
    std::vector<Edge> stroke_edges;
    std::vector<std::pair<Edge,ImVec2>> stroke_hits;
    std::vector<ImVec2> stroke_points;
    bool stroke_reroute=false;
    bool modal_transform = false;
    void wire_stroke(const CanvasHit&);
    std::map<Id, Layout> gesture_layout;
    ImVec2 gesture_start{};
    int transform = 0, axis = 0;
    bool box_select = false,box_dragging=false,box_extend=false;
    ImVec2 box_start{};
    std::shared_ptr<PendingDialog> dialog = std::make_shared<PendingDialog>();
    char search[128] = {};
    int palette_tab = 0, palette_cursor = 0, background = 0;
    unsigned texture = 0;
    Value uploaded;
    Graph saved_graph;
    bool panel_resize=false;
    int pending_project=0;
    bool close_requested=false;
    void project_action(int);
    void apply_project_action(int);
    void discard_popup();
    Graph clipboard;
    Monitor monitor;
    int inspected_channel=0;
    std::optional<Graph> parameter_before;
    void draw_grid(ImDrawList*);
    CanvasHit draw_nodes(ImDrawList*, bool);
    void canvas_input(const CanvasHit&, bool);
    void canvas_menu(const CanvasHit&, bool);
    void update_transform(std::optional<Edge>);
    void draw_selection(ImDrawList*);
    void toolbar();
    void footer();
    void settings_panel();
    void panel();
    bool settings_open=false;
    float inspector_width=280,preview_height=0;
    void canvas();
    void inspector();
    void preview();
    void palette();
    void shortcuts();
    void file_dialog(int action);
    void process_dialog();
    void load(const std::string&);
    void save(bool as = false);
    void changed();
    void fit(bool selected = false);
    void begin_transform(int, bool modal = false);
    void finish_transform(bool cancel);
    void copy_selection();
    void paste();
    ImVec2 screen(float x, float y) const;
    ImVec2 world(ImVec2) const;
    ImVec2 port_position(Port, bool) const;
    CanvasRect node_bounds(Id)const;
    void connect_endpoints(Endpoint, Endpoint);
    void add_node(const Descriptor&);

  public:
    explicit Editor(Dependencies);
    ~Editor();
    void draw();
    void drop(const char* path);
    void request_close(){project_action(3);}
    bool wants_close()const{return close_requested;}
    friend struct EditorTestAccess;
};
} // namespace touch::ui
