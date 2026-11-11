#include <touch/operators/module.hpp>
#include <touch/graph/graph.hpp>
namespace touch {
Graph demo_graph(const Registry& r){Graph g;std::vector<Id> ids;for(auto entry:std::vector<std::tuple<const char*,float,float>>{{"Pattern",40,55},{"Grain",250,55},{"VideoOut",460,55},{"Tone",40,245},{"Gain",250,245},{"AudioOut",460,245},{"Lfo",40,415},{"Rms",460,415}}){auto [type,x,y]=entry;ids.push_back(add(g,*r.find(std::string("builtin.")+type),x,y));}for(auto edge:std::vector<std::array<int,3>>{{0,1,0},{1,2,0},{3,4,0},{4,5,0},{6,3,1},{4,7,0},{7,1,1}})connect(g,{{ids[edge[0]],0},{ids[edge[1]],size_t(edge[2])}},r);return g;}
}
