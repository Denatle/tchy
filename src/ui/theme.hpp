#pragma once
#include <imgui.h>
#include <touch/graph/types.hpp>
namespace touch::ui::theme {
inline ImU32 rgb(unsigned c){return IM_COL32((c>>16)&255,(c>>8)&255,c&255,255);}
inline const ImU32 background=rgb(0x181818),panel=rgb(0x252525),border=rgb(0x4a4a4a),control=rgb(0x393939),text=rgb(0xe0e0e0),muted=rgb(0xababab),select=rgb(0xffbd67),error=rgb(0xf18d80);
inline constexpr float bar=32,footer=24,header=21,row=16,grid=24;
ImU32 signal_color(Signal);
std::string signal_label(Signal);
void configure();
void pixel_text(ImDrawList*,ImVec2,const std::string&,ImU32,float width=10000);
void configure_font();
}
