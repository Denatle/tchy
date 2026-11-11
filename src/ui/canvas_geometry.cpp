#include "canvas_geometry.hpp"
#include <algorithm>
#include <cmath>
namespace touch::ui {
bool CanvasRect::contains(ImVec2 p)const{return p.x>=min.x&&p.x<=max.x&&p.y>=min.y&&p.y<=max.y;}
bool CanvasRect::overlaps(CanvasRect b)const{return min.x<=b.max.x&&max.x>=b.min.x&&min.y<=b.max.y&&max.y>=b.min.y;}
bool reroute(const Node& n){return n.type.starts_with("builtin.Reroute");}
ImVec2 node_size(const Node& n,const Descriptor* d){if(reroute(n))return {24,14};size_t rows=d?std::max({d->inputs.size()+d->params.size(),d->outputs.size(),size_t(1)}):1;return {164,theme::header+theme::row*rows+4};}
std::vector<ImVec2> cable_points(ImVec2 a,ImVec2 b){if(b.x>=a.x+24){float mid=std::round((a.x+b.x)*.5f);return {a,{mid,a.y},{mid,b.y},b};}float bend=std::abs(a.y-b.y)<24?a.y-16:std::round((a.y+b.y)*.5f);return {a,{a.x+13,a.y},{a.x+13,bend},{b.x-13,bend},{b.x-13,b.y},b};}
float segment_distance(ImVec2 p,ImVec2 a,ImVec2 b){float dx=b.x-a.x,dy=b.y-a.y,len=dx*dx+dy*dy;float t=len?std::clamp(((p.x-a.x)*dx+(p.y-a.y)*dy)/len,0.f,1.f):0;return std::hypot(p.x-a.x-t*dx,p.y-a.y-t*dy);}
std::optional<ImVec2> intersection(ImVec2 a,ImVec2 b,ImVec2 c,ImVec2 d){auto cross=[](ImVec2 x,ImVec2 y){return x.x*y.y-x.y*y.x;};ImVec2 r{b.x-a.x,b.y-a.y},s{d.x-c.x,d.y-c.y},offset{c.x-a.x,c.y-a.y};float denominator=cross(r,s);if(std::abs(denominator)<1e-6f)return {};float t=cross(offset,s)/denominator,u=cross(offset,r)/denominator;if(t<0||t>1||u<0||u>1)return {};return ImVec2{a.x+t*r.x,a.y+t*r.y};}
std::optional<size_t> compatible_port(const Descriptor& d,Signal signal,bool output){auto& ports=output?d.outputs:d.inputs;auto it=std::find(ports.begin(),ports.end(),signal);if(it!=ports.end())return it-ports.begin();if(!output&&signal==Signal::Control&&!d.params.empty())return ports.size();return {};}
std::string port_label(const Descriptor& d,bool output,size_t i){std::vector<std::string> names;auto k=d.legacy;if((k=="SeparateRgba"&&output)||(k=="CombineRgba"&&!output))names={"Red","Green","Blue","Alpha"};else if(!output){if(k=="Over")names={"Foreground","Background"};if(k=="MaskedMix")names={"Original","Effect","Mask"};if(k=="SetAlpha")names={"Image","Mask"};if(k=="MultiplyChannels"||k=="Wavetable")names={"Channels","Weights"};if(k=="ScanOnce")names={"Channels","Trigger"};}if(i<names.size())return names[i];if(!output&&i>=d.inputs.size())return d.params.at(i-d.inputs.size()).name;auto& ports=output?d.outputs:d.inputs;if(i>=ports.size())return "?";auto label=theme::signal_label(ports[i]);if(!output&&d.inputs.size()>1)label+=i==0?" A":" B";return label;}
}
