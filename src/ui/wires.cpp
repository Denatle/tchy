#include "editor.hpp"
namespace touch::ui {
namespace {std::string reroute_type(Signal signal){return std::string("builtin.Reroute")+(signal==Signal::Video?"Video":signal==Signal::Audio?"Audio":signal==Signal::Control?"Control":signal==Signal::Table?"Table":signal==Signal::Mask?"Mask":"Channels");}}
void Editor::insert_reroute(Edge edge,ImVec2 position){try{Id id=0;document.edit([&](Graph& g){id=add(g,*services.registry.find(reroute_type(port_signal(g,services.registry,edge.from,true))),position.x-12,position.y-7);insert_on_edge(g,edge,id,services.registry);});document.selected={id};active_node=id;changed();}catch(const std::exception& e){status=e.what();}}
void Editor::wire_stroke(const CanvasHit&){auto& io=ImGui::GetIO();auto mouse=io.MousePos;
if(!stroke_before&&ImGui::IsMouseClicked(1)&&(io.KeyCtrl||io.KeyShift)&&CanvasRect{origin,{origin.x+canvas_size.x,origin.y+canvas_size.y}}.contains(mouse)){stroke_before=document.graph;stroke_reroute=io.KeyShift;stroke_points={mouse};stroke_hits.clear();}
if(!stroke_before)return;
auto previous=stroke_points.back();if(stroke_points.size()<2048)stroke_points.push_back(mouse);
for(auto edge:document.graph.edges){if(std::any_of(stroke_hits.begin(),stroke_hits.end(),[&](auto hit){return hit.first==edge;}))continue;auto path=cable_points(port_position(edge.from,true),port_position(edge.to,false));for(size_t i=1;i<path.size();++i)if(auto at=intersection(previous,mouse,path[i-1],path[i])){stroke_hits.push_back({edge,*at});break;}}
if(ImGui::IsMouseDown(1))return;
try{auto before=*stroke_before;auto candidate=document.graph;std::set<Id> created;for(auto [edge,at]:stroke_hits){if(stroke_reroute){auto pos=world(at);auto d=services.registry.find(reroute_type(port_signal(candidate,services.registry,edge.from,true)));auto id=add(candidate,*d,pos.x-12,pos.y-7);insert_on_edge(candidate,edge,id,services.registry);created.insert(id);}else std::erase(candidate.edges,edge);}document.graph=std::move(candidate);document.commit(std::move(before));if(!created.empty()){document.selected=created;active_node=*created.rbegin();}if(!stroke_hits.empty())changed();}catch(const std::exception& e){document.graph=*stroke_before;status=e.what();}stroke_before.reset();stroke_hits.clear();stroke_points.clear();}
}
