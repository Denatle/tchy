#include "theme.hpp"
#include <array>
#include <cctype>
#include <cmath>
namespace touch::ui::theme {
namespace {
std::array<unsigned char,7> glyph(char c){switch(std::toupper(static_cast<unsigned char>(c))){
#include "glyphs.inc"
default:return {14,17,1,2,4,0,4};}}
ImVec4 color(ImU32 c){return ImGui::ColorConvertU32ToFloat4(c);}
}
ImU32 signal_color(Signal s){constexpr unsigned colors[]={0,0xb4a0d5,0x81b4df,0x91c4a5,0xd4b879,0xda9fb9,0x74c7c9};return rgb(colors[int(s)]);}
std::string signal_label(Signal s){const char* labels[]={"","TOP","Audio","CHOP","DAT","Mask","Bus"};return labels[int(s)];}
void pixel_text(ImDrawList* draw,ImVec2 at,const std::string& text,ImU32 tint,float width){int count=std::max(0,int(width/6));for(size_t i=0;i<text.size()&&int(i)<count;++i){auto rows=glyph(text[i]);for(int y=0;y<7;++y)for(int x=0;x<5;++x)if(rows[y]&(16>>x)){float px=std::round(at.x)+i*6+x,py=std::round(at.y)+y;draw->AddRectFilled({px,py},{px+1,py+1},tint);}}}
void configure_font(){auto& atlas=*ImGui::GetIO().Fonts;static const ImWchar range[]={0x100,0x101,0};ImFontConfig cfg;cfg.SizePixels=7;cfg.PixelSnapH=true;cfg.GlyphRanges=range;auto font=atlas.AddFontDefault(&cfg);std::array<int,95> rects;for(int c=32;c<127;++c)rects[c-32]=atlas.AddCustomRectFontGlyph(font,ImWchar(c),5,7,6);unsigned char* pixels;int width,height;atlas.GetTexDataAsRGBA32(&pixels,&width,&height);for(int c=32;c<127;++c){auto r=atlas.GetCustomRectByIndex(rects[c-32]);auto rows=glyph(char(c));for(int y=0;y<7;++y)for(int x=0;x<5;++x){auto p=pixels+((r->Y+y)*width+r->X+x)*4;p[0]=p[1]=p[2]=255;p[3]=(rows[y]&(16>>x))?255:0;}}}
void configure(){auto& s=ImGui::GetStyle();s.WindowPadding={0,0};s.ChildRounding=s.FrameRounding=s.PopupRounding=s.ScrollbarRounding=s.GrabRounding=s.TabRounding=0;s.WindowBorderSize=0;s.ChildBorderSize=0;s.PopupBorderSize=s.FrameBorderSize=1;s.FramePadding={6,7.5};s.ItemSpacing={4,4};s.ItemInnerSpacing={4,4};s.ScrollbarSize=10;s.AntiAliasedLines=false;s.AntiAliasedFill=false;s.WindowMinSize={20,20};s.Colors[ImGuiCol_WindowBg]=color(panel);s.Colors[ImGuiCol_ChildBg]=color(panel);s.Colors[ImGuiCol_PopupBg]=color(panel);s.Colors[ImGuiCol_Border]=color(border);s.Colors[ImGuiCol_Text]=color(text);s.Colors[ImGuiCol_TextDisabled]=color(muted);s.Colors[ImGuiCol_FrameBg]=s.Colors[ImGuiCol_Button]=color(control);s.Colors[ImGuiCol_ButtonHovered]=s.Colors[ImGuiCol_FrameBgHovered]=color(rgb(0x515151));s.Colors[ImGuiCol_ButtonActive]=s.Colors[ImGuiCol_FrameBgActive]=color(rgb(0x606060));s.Colors[ImGuiCol_CheckMark]=s.Colors[ImGuiCol_SliderGrab]=color(select);s.Colors[ImGuiCol_Header]=color(control);s.Colors[ImGuiCol_HeaderHovered]=color(rgb(0x515151));s.Colors[ImGuiCol_HeaderActive]=color(rgb(0x606060));}
}
