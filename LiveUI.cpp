#include "stdafx.h"

#include "LiveUI.h"

#include "Shader.h"

#include "inc/fonts/DroidSans.h"
#include "inc/fonts/IconsForkAwesome.h"
#include "inc/fonts/ForkAwesome.h"

#include "imgui/imgui.h"
#ifdef ENABLE_SDL
#include "imgui/imgui_impl_opengl3.h"
#else
#include "imgui/imgui_impl_dx9.h"
#endif
#include "imgui/imgui_impl_win32.h"
#include "imgui/implot/implot.h"
#include "imgui/imgui_stdlib.h"
#include "imgui/ImGuizmo.h"

#if __cplusplus >= 202002L && !defined(__clang__)
#define stable_sort std::ranges::stable_sort
#define sort std::ranges::sort
#else
#define stable_sort std::stable_sort
#define sort std::sort
#endif

#include "BAM/BAMView.h"

// Titles (used as Ids) of modal dialogs
#define ID_MODAL_SPLASH "In Game UI"
#define ID_VIDEO_SETTINGS "Video Options"
#define ID_AUDIO_SETTINGS "Audio Options"
#define ID_RENDERER_INSPECTION "Renderer Inspection"
#define ID_BAM_SETTINGS "Headtracking Settings"

#define PROP_WIDTH (125.f * m_dpi)
#define PROP_TIMER(is_live, startup_obj, live_obj)                                                                                                                                                                  \
   if (ImGui::CollapsingHeader("Timer", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE) \
   { \
      PropCheckbox("Enabled", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_tdr.m_TimerEnabled) : nullptr, live_obj ? &(live_obj->m_d.m_tdr.m_TimerEnabled) : nullptr); \
      PropInt("Interval (ms)", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_tdr.m_TimerInterval) : nullptr, live_obj ? &(live_obj->m_d.m_tdr.m_TimerInterval) : nullptr); \
      ImGui::EndTable(); \
   }
#define BEGIN_PROP_TABLE ImGui::BeginTable("props", 2, ImGuiTableFlags_Borders)
#define PROP_TABLE_SETUP \
   if (ImGui::TableGetRowIndex() == -1) \
   { \
      ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthStretch); \
      ImGui::TableSetupColumn("Sync", ImGuiTableColumnFlags_WidthFixed); \
   }


#define ICON_SAVE ICON_FK_FLOPPY_O

void Frustum(float left, float right, float bottom, float top, float znear, float zfar, float *m16)
{
   float temp, temp2, temp3, temp4;
   temp = 2.0f * znear;
   temp2 = right - left;
   temp3 = top - bottom;
   temp4 = zfar - znear;
   m16[0] = temp / temp2;
   m16[1] = 0.0;
   m16[2] = 0.0;
   m16[3] = 0.0;
   m16[4] = 0.0;
   m16[5] = temp / temp3;
   m16[6] = 0.0;
   m16[7] = 0.0;
   m16[8] = (right + left) / temp2;
   m16[9] = (top + bottom) / temp3;
   m16[10] = (-zfar - znear) / temp4;
   m16[11] = -1.0f;
   m16[12] = 0.0;
   m16[13] = 0.0;
   m16[14] = (-temp * zfar) / temp4;
   m16[15] = 0.0;
}

void Perspective(float fovyInDegrees, float aspectRatio, float znear, float zfar, float *m16)
{
   float ymax, xmax;
   ymax = znear * tanf(fovyInDegrees * 3.141592f / 180.0f);
   xmax = ymax * aspectRatio;
   Frustum(-xmax, xmax, -ymax, ymax, znear, zfar, m16);
}

void Cross(const float *a, const float *b, float *r)
{
   r[0] = a[1] * b[2] - a[2] * b[1];
   r[1] = a[2] * b[0] - a[0] * b[2];
   r[2] = a[0] * b[1] - a[1] * b[0];
}

float Dot(const float *a, const float *b) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }

void Normalize(const float *a, float *r)
{
   float il = 1.f / (sqrtf(Dot(a, a)) + FLT_EPSILON);
   r[0] = a[0] * il;
   r[1] = a[1] * il;
   r[2] = a[2] * il;
}

void LookAt(const float *eye, const float *at, const float *up, float *m16)
{
   float X[3], Y[3], Z[3], tmp[3];

   tmp[0] = eye[0] - at[0];
   tmp[1] = eye[1] - at[1];
   tmp[2] = eye[2] - at[2];
   Normalize(tmp, Z);
   Normalize(up, Y);

   Cross(Y, Z, tmp);
   Normalize(tmp, X);

   Cross(Z, X, tmp);
   Normalize(tmp, Y);

   m16[0] = X[0];
   m16[1] = Y[0];
   m16[2] = Z[0];
   m16[3] = 0.0f;
   m16[4] = X[1];
   m16[5] = Y[1];
   m16[6] = Z[1];
   m16[7] = 0.0f;
   m16[8] = X[2];
   m16[9] = Y[2];
   m16[10] = Z[2];
   m16[11] = 0.0f;
   m16[12] = -Dot(X, eye);
   m16[13] = -Dot(Y, eye);
   m16[14] = -Dot(Z, eye);
   m16[15] = 1.0f;
}

void OrthoGraphic(const float l, float r, float b, const float t, float zn, const float zf, float *m16)
{
   m16[0] = 2 / (r - l);
   m16[1] = 0.0f;
   m16[2] = 0.0f;
   m16[3] = 0.0f;
   m16[4] = 0.0f;
   m16[5] = 2 / (t - b);
   m16[6] = 0.0f;
   m16[7] = 0.0f;
   m16[8] = 0.0f;
   m16[9] = 0.0f;
   m16[10] = 1.0f / (zf - zn);
   m16[11] = 0.0f;
   m16[12] = (l + r) / (l - r);
   m16[13] = (t + b) / (b - t);
   m16[14] = zn / (zn - zf);
   m16[15] = 1.0f;
}

inline void rotationY(const float angle, float *m16)
{
   float c = cosf(angle);
   float s = sinf(angle);

   m16[0] = c;
   m16[1] = 0.0f;
   m16[2] = -s;
   m16[3] = 0.0f;
   m16[4] = 0.0f;
   m16[5] = 1.f;
   m16[6] = 0.0f;
   m16[7] = 0.0f;
   m16[8] = s;
   m16[9] = 0.0f;
   m16[10] = c;
   m16[11] = 0.0f;
   m16[12] = 0.f;
   m16[13] = 0.f;
   m16[14] = 0.f;
   m16[15] = 1.0f;
}

// utility structure for realtime plot //!! cleanup
class ScrollingData
{
private:
   int MaxSize;

public:
   int Offset;
   ImVector<ImVec2> Data;
   ScrollingData()
   {
      MaxSize = 500;
      Offset = 0;
      Data.reserve(MaxSize);
   }
   void AddPoint(const float x, const float y)
   {
      if (Data.size() < MaxSize)
         Data.push_back(ImVec2(x, y));
      else
      {
         Data[Offset] = ImVec2(x, y);
         Offset++;
         if (Offset == MaxSize)
            Offset = 0;
      }
   }
   void Erase()
   {
      if (!Data.empty())
      {
         Data.shrink(0);
         Offset = 0;
      }
   }
   ImVec2 GetLast()
   {
      if (Data.empty())
         return ImVec2(0.f, 0.f);
      else if (Data.size() < MaxSize || Offset == 0)
         return Data.back();
      else
         return Data[Offset - 1];
   }
};

// utility structure for realtime plot
/*class RollingData {
    float Span;
    ImVector<ImVec2> Data;
    RollingData() {
        Span = 10.0f;
        Data.reserve(500);
    }
    void AddPoint(const float x, const float y) {
        const float xmod = fmodf(x, Span);
        if (!Data.empty() && xmod < Data.back().x)
            Data.shrink(0);
        Data.push_back(ImVec2(xmod, y));
    }
};*/

// Maps directinput keycode used by VPX to ImGui keycode enum
// FIXME This is not ok and will fail for all non QWERTY keyboards, since VPX (DirectInput Keyboard) is keyboard layout neutral while ImGui is layout aware
// For the time being, this is used to translate global shortcuts for:
// - Debugger (default is D which should be ok on all keyboards), 
// - FPS display (default is F11 which should be ok)
// - Exit (default is Escape which should be ok)
static constexpr ImGuiKey dikToImGuiKeys[] = {
   ImGuiKey_None,
   ImGuiKey_Escape,    //DIK_ESCAPE          0x01
   ImGuiKey_1,         //DIK_1               0x02
   ImGuiKey_2,         //DIK_2               0x03
   ImGuiKey_3,         //DIK_3               0x04
   ImGuiKey_4,         //DIK_4               0x05
   ImGuiKey_5,         //DIK_5               0x06
   ImGuiKey_6,         //DIK_6               0x07
   ImGuiKey_7,         //DIK_7               0x08
   ImGuiKey_8,         //DIK_8               0x09
   ImGuiKey_9,         //DIK_9               0x0A
   ImGuiKey_0,         //DIK_0               0x0B
   ImGuiKey_Minus,     //DIK_MINUS           0x0C    /* - on main keyboard */
   ImGuiKey_Equal,     //DIK_EQUALS          0x0D
   ImGuiKey_Backspace, //DIK_BACK            0x0E    /* backspace */
   ImGuiKey_Tab,       //DIK_TAB             0x0F
   ImGuiKey_Q,         //DIK_Q               0x10
   ImGuiKey_W,         //DIK_W               0x11
   ImGuiKey_E,         //DIK_E               0x12
   ImGuiKey_R,         //DIK_R               0x13
   ImGuiKey_T,         //DIK_T               0x14
   ImGuiKey_Y,         //DIK_Y               0x15
   ImGuiKey_U,         //DIK_U               0x16
   ImGuiKey_I,         //DIK_I               0x17
   ImGuiKey_O,         //DIK_O               0x18
   ImGuiKey_P,         //DIK_P               0x19
   ImGuiKey_LeftBracket, //DIK_LBRACKET        0x1A
   ImGuiKey_RightBracket, //DIK_RBRACKET        0x1B
   ImGuiKey_Enter,     //DIK_RETURN          0x1C    /* Enter on main keyboard */
   ImGuiKey_LeftCtrl,  //DIK_LCONTROL        0x1D
   ImGuiKey_A,         //DIK_A               0x1E
   ImGuiKey_S,         //DIK_S               0x1F
   ImGuiKey_D,         //DIK_D               0x20
   ImGuiKey_F,         //DIK_F               0x21
   ImGuiKey_G,         //DIK_G               0x22
   ImGuiKey_H,         //DIK_H               0x23
   ImGuiKey_J,         //DIK_J               0x24
   ImGuiKey_K,         //DIK_K               0x25
   ImGuiKey_L,         //DIK_L               0x26
   ImGuiKey_Semicolon, //DIK_SEMICOLON       0x27
   ImGuiKey_Apostrophe,//DIK_APOSTROPHE      0x28
   ImGuiKey_GraveAccent, //DIK_GRAVE           0x29    /* accent grave */
   ImGuiKey_LeftShift, //DIK_LSHIFT          0x2A
   ImGuiKey_Backslash, //DIK_BACKSLASH       0x2B
   ImGuiKey_Z,         //DIK_Z               0x2C
   ImGuiKey_X,         //DIK_X               0x2D
   ImGuiKey_C,         //DIK_C               0x2E
   ImGuiKey_V,         //DIK_V               0x2F
   ImGuiKey_B,         //DIK_B               0x30
   ImGuiKey_N,         //DIK_N               0x31
   ImGuiKey_M,         //DIK_M               0x32
   ImGuiKey_Comma,     //DIK_COMMA           0x33
   ImGuiKey_Period,    //DIK_PERIOD          0x34    /* . on main keyboard */
   ImGuiKey_Slash,     //DIK_SLASH           0x35    /* / on main keyboard */
   ImGuiKey_RightShift,//DIK_RSHIFT          0x36
   ImGuiKey_KeypadMultiply, //DIK_MULTIPLY        0x37    /* * on numeric keypad */
   ImGuiKey_Menu,      //DIK_LMENU           0x38    /* left Alt */
   ImGuiKey_Space,     //DIK_SPACE           0x39
   ImGuiKey_CapsLock,  //DIK_CAPITAL         0x3A
   ImGuiKey_F1,        //DIK_F1              0x3B
   ImGuiKey_F2,        //DIK_F2              0x3C
   ImGuiKey_F3,        //DIK_F3              0x3D
   ImGuiKey_F4,        //DIK_F4              0x3E
   ImGuiKey_F5,        //DIK_F5              0x3F
   ImGuiKey_F6,        //DIK_F6              0x40
   ImGuiKey_F7,        //DIK_F7              0x41
   ImGuiKey_F8,        //DIK_F8              0x42
   ImGuiKey_F9,        //DIK_F9              0x43
   ImGuiKey_F10,       //DIK_F10             0x44
   ImGuiKey_NumLock,   //DIK_NUMLOCK         0x45
   ImGuiKey_ScrollLock,//DIK_SCROLL          0x46    /* Scroll Lock */
   ImGuiKey_Keypad7,   //DIK_NUMPAD7         0x47
   ImGuiKey_Keypad8,   //DIK_NUMPAD8         0x48
   ImGuiKey_Keypad9,   //DIK_NUMPAD9         0x49
   ImGuiKey_KeypadSubtract, //DIK_SUBTRACT        0x4A    /* - on numeric keypad */
   ImGuiKey_Keypad4,   //DIK_NUMPAD4         0x4B
   ImGuiKey_Keypad5,   //DIK_NUMPAD5         0x4C
   ImGuiKey_Keypad6,   //DIK_NUMPAD6         0x4D
   ImGuiKey_KeypadAdd, //DIK_ADD             0x4E    /* + on numeric keypad */
   ImGuiKey_Keypad1,   //DIK_NUMPAD1         0x4F
   ImGuiKey_Keypad2,   //DIK_NUMPAD2         0x50
   ImGuiKey_Keypad3,   //DIK_NUMPAD3         0x51
   ImGuiKey_Keypad0,   //DIK_NUMPAD0         0x52
   ImGuiKey_KeypadDecimal, //DIK_DECIMAL         0x53    /* . on numeric keypad */
   ImGuiKey_None,      //0x54
   ImGuiKey_None,      //0x55
   ImGuiKey_None,      //DIK_OEM_102         0x56    /* < > | on UK/Germany keyboards */
   ImGuiKey_F11,       //DIK_F11             0x57
   ImGuiKey_F12,       //DIK_F12             0x58
   ImGuiKey_None,      //0x59
   ImGuiKey_None,      //0x5A
   ImGuiKey_None,      //0x5B
   ImGuiKey_None,      //0x5C
   ImGuiKey_None,      //0x5D
   ImGuiKey_None,      //0x5E
   ImGuiKey_None,      //0x5F
   ImGuiKey_None,      //0x60
   ImGuiKey_None,      //0x61
   ImGuiKey_None,      //0x62
   ImGuiKey_None,      //0x63
   ImGuiKey_None,      //DIK_F13             0x64    /*                     (NEC PC98) */
   ImGuiKey_None,      //DIK_F14             0x65    /*                     (NEC PC98) */
   ImGuiKey_None,      //DIK_F15             0x66    /*                     (NEC PC98) */
   ImGuiKey_None,      //0x67
   ImGuiKey_None,      //0x68
   ImGuiKey_None,      //0x69
   ImGuiKey_None,      //0x6A
   ImGuiKey_None,      //0x6B
   ImGuiKey_None,      //0x6C
   ImGuiKey_None,      //0x6D
   ImGuiKey_None,      //0x6E
   ImGuiKey_None,      //0x6F

   ImGuiKey_None, //0x70
   ImGuiKey_None, //0x71
   ImGuiKey_None, //0x72
   ImGuiKey_None, //0x73
   ImGuiKey_None, //0x74
   ImGuiKey_None, //0x75
   ImGuiKey_None, //0x76
   ImGuiKey_None, //0x77
   ImGuiKey_None, //0x78
   ImGuiKey_None, //0x79
   ImGuiKey_None, //0x7A
   ImGuiKey_None, //0x7B
   ImGuiKey_None, //0x7C
   ImGuiKey_None, //0x7D
   ImGuiKey_None, //0x7E
   ImGuiKey_None, //0x7F

   ImGuiKey_None, //0x80
   ImGuiKey_None, //0x81
   ImGuiKey_None, //0x82
   ImGuiKey_None, //0x83
   ImGuiKey_None, //0x84
   ImGuiKey_None, //0x85
   ImGuiKey_None, //0x86
   ImGuiKey_None, //0x87
   ImGuiKey_None, //0x88
   ImGuiKey_None, //0x89
   ImGuiKey_None, //0x8A
   ImGuiKey_None, //0x8B
   ImGuiKey_None, //0x8C
   ImGuiKey_None, //0x8D
   ImGuiKey_None, //0x8E
   ImGuiKey_None, //0x8F

   ImGuiKey_None, //0x90
   ImGuiKey_None, //0x91
   ImGuiKey_None, //0x92
   ImGuiKey_None, //0x93
   ImGuiKey_None, //0x94
   ImGuiKey_None, //0x95
   ImGuiKey_None, //0x96
   ImGuiKey_None, //0x97
   ImGuiKey_None, //0x98
   ImGuiKey_None, //0x99
   ImGuiKey_None, //0x9A
   ImGuiKey_None, //0x9B

   ImGuiKey_KeypadEnter, //DIK_NUMPADENTER     0x9C    /* Enter on numeric keypad */
   ImGuiKey_RightCtrl,   //DIK_RCONTROL        0x9D

   ImGuiKey_None, //0x9E
   ImGuiKey_None, //0x9F

   ImGuiKey_None, //0xA0
   ImGuiKey_None, //0xA1
   ImGuiKey_None, //0xA2
   ImGuiKey_None, //0xA3
   ImGuiKey_None, //0xA4
   ImGuiKey_None, //0xA5
   ImGuiKey_None, //0xA6
   ImGuiKey_None, //0xA7
   ImGuiKey_None, //0xA8
   ImGuiKey_None, //0xA9
   ImGuiKey_None, //0xAA
   ImGuiKey_None, //0xAB
   ImGuiKey_None, //0xAC
   ImGuiKey_None, //0xAD
   ImGuiKey_None, //0xAE
   ImGuiKey_None, //0xAF

   ImGuiKey_None, //0xB0
   ImGuiKey_None, //0xB1
   ImGuiKey_None, //0xB2
   ImGuiKey_None, //0xB3
   ImGuiKey_None, //0xB4

   ImGuiKey_KeypadDivide, //DIK_DIVIDE          0xB5    /* / on numeric keypad */
   ImGuiKey_None,         //0xB6
   ImGuiKey_None,         //DIK_SYSRQ           0xB7
   ImGuiKey_RightAlt,     //DIK_RMENU           0xB8    /* right Alt */

   ImGuiKey_None, //0xB9
   ImGuiKey_None, //0xBA
   ImGuiKey_None, //0xBB
   ImGuiKey_None, //0xBC
   ImGuiKey_None, //0xBD
   ImGuiKey_None, //0xBE
   ImGuiKey_None, //0xBF

   ImGuiKey_None, //0xC0
   ImGuiKey_None, //0xC1
   ImGuiKey_None, //0xC2
   ImGuiKey_None, //0xC3
   ImGuiKey_None, //0xC4
   ImGuiKey_None, //0xC5
   ImGuiKey_None, //0xC6

   ImGuiKey_Home,       //DIK_HOME            0xC7    /* Home on arrow keypad */
   ImGuiKey_UpArrow,    //DIK_UP              0xC8    /* UpArrow on arrow keypad */
   ImGuiKey_PageUp,     //DIK_PRIOR           0xC9    /* PgUp on arrow keypad */
   ImGuiKey_None,       //0xCA
   ImGuiKey_LeftArrow,  //DIK_LEFT            0xCB    /* LeftArrow on arrow keypad */
   ImGuiKey_None,       //0xCC
   ImGuiKey_RightArrow, //DIK_RIGHT           0xCD    /* RightArrow on arrow keypad */
   ImGuiKey_None,       //0xCE
   ImGuiKey_End,        //DIK_END             0xCF    /* End on arrow keypad */
   ImGuiKey_DownArrow,  //DIK_DOWN            0xD0    /* DownArrow on arrow keypad */
   ImGuiKey_PageDown,   //DIK_NEXT            0xD1    /* PgDn on arrow keypad */
   ImGuiKey_Insert,     //DIK_INSERT          0xD2    /* Insert on arrow keypad */
   ImGuiKey_Delete,     //DIK_DELETE          0xD3    /* Delete on arrow keypad */

   ImGuiKey_None, //0xD4
   ImGuiKey_None, //0xD5
   ImGuiKey_None, //0xD6
   ImGuiKey_None, //0xD7
   ImGuiKey_None, //0xD8
   ImGuiKey_None, //0xD9
   ImGuiKey_None, //0xDA

   ImGuiKey_LeftSuper,  //DIK_LWIN            0xDB    /* Left Windows key */
   ImGuiKey_RightSuper, //DIK_RWIN            0xDC    /* Right Windows key */
   ImGuiKey_None,       //DIK_APPS            0xDD    /* AppMenu key */
};

static void SetupImGuiStyle(const float overall_alpha)
{
   // Rounded Visual Studio style by RedNicStone from ImThemes
   ImGuiStyle &style = ImGui::GetStyle();

   style.Alpha = 1.0f;
   style.DisabledAlpha = 0.6000000238418579f;
   style.WindowPadding = ImVec2(8.0f, 8.0f);
   style.WindowRounding = 4.0f;
   style.WindowBorderSize = 0.0f;
   style.WindowMinSize = ImVec2(32.0f, 32.0f);
   style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
   style.WindowMenuButtonPosition = ImGuiDir_Left;
   style.ChildRounding = 0.0f;
   style.ChildBorderSize = 1.0f;
   style.PopupRounding = 4.0f;
   style.PopupBorderSize = 1.0f;
   style.FramePadding = ImVec2(4.0f, 3.0f);
   style.FrameRounding = 2.5f;
   style.FrameBorderSize = 0.0f;
   style.ItemSpacing = ImVec2(8.0f, 4.0f);
   style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
   style.CellPadding = ImVec2(4.0f, 2.0f);
   style.IndentSpacing = 21.0f;
   style.ColumnsMinSpacing = 6.0f;
   style.ScrollbarSize = 11.0f;
   style.ScrollbarRounding = 2.5f;
   style.GrabMinSize = 10.0f;
   style.GrabRounding = 2.0f;
   style.TabRounding = 3.5f;
   style.TabBorderSize = 0.0f;
   style.TabMinWidthForCloseButton = 0.0f;
   style.ColorButtonPosition = ImGuiDir_Right;
   style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
   style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

   style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
   style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.5921568870544434f, 0.5921568870544434f, 0.5921568870544434f, overall_alpha);
   style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, overall_alpha);
   style.Colors[ImGuiCol_ChildBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, overall_alpha);
   style.Colors[ImGuiCol_PopupBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, overall_alpha);
   style.Colors[ImGuiCol_Border] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, overall_alpha);
   style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, overall_alpha);
   style.Colors[ImGuiCol_FrameBg] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, overall_alpha);
   style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, overall_alpha);
   style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, overall_alpha);
   style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, overall_alpha);
   style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, overall_alpha);
   style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, overall_alpha);
   style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, overall_alpha);
   style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, overall_alpha);
   style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.321568638086319f, 0.321568638086319f, 0.3333333432674408f, overall_alpha);
   style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.3529411852359772f, 0.3529411852359772f, 0.3725490272045135f, overall_alpha);
   style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.3529411852359772f, 0.3529411852359772f, 0.3725490272045135f, overall_alpha);
   style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, overall_alpha);
   style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, overall_alpha);
   style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, overall_alpha);
   style.Colors[ImGuiCol_Button] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, overall_alpha);
   style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, overall_alpha);
   style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, overall_alpha);
   style.Colors[ImGuiCol_Header] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, overall_alpha);
   style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, overall_alpha);
   style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, overall_alpha);
   style.Colors[ImGuiCol_Separator] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, overall_alpha);
   style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, overall_alpha);
   style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, overall_alpha);
   style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, overall_alpha);
   style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, overall_alpha);
   style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.321568638086319f, 0.321568638086319f, 0.3333333432674408f, overall_alpha);
   style.Colors[ImGuiCol_Tab] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, overall_alpha);
   style.Colors[ImGuiCol_TabHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, overall_alpha);
   style.Colors[ImGuiCol_TabActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, overall_alpha);
   style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, overall_alpha);
   style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, overall_alpha);
   style.Colors[ImGuiCol_PlotLines] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, overall_alpha);
   style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, overall_alpha);
   style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, overall_alpha);
   style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, overall_alpha);
   style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, overall_alpha);
   style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, overall_alpha);
   style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, overall_alpha);
   style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f * overall_alpha);
   style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f * overall_alpha);
   style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, overall_alpha);
   style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, overall_alpha);
   style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, overall_alpha);
   style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f * overall_alpha);
   style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f * overall_alpha);
   style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, overall_alpha);
}

// Helper to display a little (?) mark which shows a tooltip when hovered.
static void HelpMarker(const char *desc)
{
   ImGui::TextDisabled("(?)");
   if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
   {
      ImGui::BeginTooltip();
      ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
      ImGui::TextUnformatted(desc);
      ImGui::PopTextWrapPos();
      ImGui::EndTooltip();
   }
}

static void HelpTextCentered(const std::string& text)
{
   const ImVec2 win_size = ImGui::GetWindowSize();
   const ImVec2 text_size = ImGui::CalcTextSize(text.c_str());

   // calculate the indentation that centers the text on one line, relative
   // to window left, regardless of the `ImGuiStyleVar_WindowPadding` value
   float text_indentation = (win_size.x - text_size.x) * 0.5f;

   // if text is too long to be drawn on one line, `text_indentation` can
   // become too small or even negative, so we check a minimum indentation
   constexpr float min_indentation = 20.0f;
   if (text_indentation <= min_indentation)
      text_indentation = min_indentation;

   ImGui::SameLine(text_indentation);
   ImGui::PushTextWrapPos(win_size.x - text_indentation);
   ImGui::TextWrapped(text.c_str());
   ImGui::PopTextWrapPos();
}

static void HelpSplash(const std::string& text, int rotation)
{
   ImVec2 win_size = ImGui::GetIO().DisplaySize;
   const ImVec2 text_size = ImGui::CalcTextSize(text.c_str());
   if (rotation == 1 || rotation == 3)
   {
      const float tmp = win_size.x;
      win_size.x = win_size.y;
      win_size.y = tmp;
   }
   constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
   ImGui::SetNextWindowBgAlpha(0.35f);
   ImGui::SetNextWindowPos(ImVec2((win_size.x - text_size.x - 10) / 2, (win_size.y - text_size.y - 10) / 2));
   ImGui::SetNextWindowSize(ImVec2(text_size.x + 20, text_size.y + 20));
   ImGui::Begin("ToolTip", nullptr, window_flags);
   ImGui::Text(text.c_str());
   ImGui::End();
}

static void HelpEditableHeader(bool is_live, IEditable *editable, IEditable *live_editable)
{
   string title;
   IEditable *notnull_editable = editable ? editable : live_editable;
   IEditable *select_editable = is_live ? live_editable : editable;
   if (notnull_editable == nullptr)
      return;
   switch (notnull_editable->GetItemType())
   {
   // Missing: eItemLightCenter, eItemDragPoint, eItemCollection
   case eItemBumper: title = "Bumper"s; break;
   case eItemDecal: title = "Decal"s; break;
   case eItemDispReel: title = "Reel"s; break;
   case eItemGate: title = "Gate"s; break;
   case eItemFlasher: title = "Flasher"s; break;
   case eItemFlipper: title = "Flipper"s; break;
   case eItemHitTarget: title = "Target"s; break;
   case eItemKicker: title = "Kicker"s; break;
   case eItemLight: title = "Light"s; break;
   case eItemLightSeq: title = "Light Sequencer"s; break;
   case eItemPlunger: title = "Plunger"s; break;
   case eItemPrimitive: title = ((Primitive *)notnull_editable)->IsPlayfield() ? "Playfield"s : "Primitive"s; break;
   case eItemRamp: title = "Ramp"s; break;
   case eItemRubber: title = "Rubber"s; break;
   case eItemSpinner: title = "Spinner"s; break;
   case eItemSurface: title = "Surface"s; break;
   case eItemTable: title = "Table"s; break;
   case eItemTextbox: title = "TextBox"s; break;
   case eItemTimer: title = "Timer"s; break;
   case eItemTrigger: title = "Trigger"s; break;
   }
   HelpTextCentered(title);
   ImGui::BeginDisabled(is_live); // Do not edit name of live objects, it would likely break the script
   string name = select_editable ? select_editable->GetName() : ""s;
   if (ImGui::InputText("Name", &name))
   {
      editable->SetName(name);
   }
   ImGui::EndDisabled();
   ImGui::Separator();
}

LiveUI::LiveUI(RenderDevice *const rd)
   : m_rd(rd)
{
   m_StartTime_usec = usec();
   m_app = g_pvp;
   m_player = g_pplayer;
   m_table = g_pplayer->m_pEditorTable;
   m_live_table = g_pplayer->m_ptable;
   m_pininput = &(g_pplayer->m_pininput);
   m_pin3d = &(g_pplayer->m_pin3d);
   m_disable_esc = LoadValueBoolWithDefault(regKey[RegName::Player], "DisableESC"s, m_disable_esc);
   m_old_player_dynamic_mode = m_player->m_dynamicMode;
   m_old_player_camera_mode = m_player->m_cameraMode;

   m_selection.type = Selection::SelectionType::S_NONE;
   m_useEditorCam = false;

   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImPlot::CreateContext();
   ImGuiIO &io = ImGui::GetIO();
   io.IniFilename = nullptr; //don't use an ini file for configuration

   // Editor camera position. We use a right handed system for easy ImGuizmo integration while VPX renderer is left handed, so reverse X axis
   m_orthoCam = true;
   m_camDistance = m_live_table->m_bottom * 0.7f;
   bool isPerspective = false;
   float viewWidth = 10.f; // for orthographic
   float camYAngle = 165.f / 180.f * 3.14159f;
   float camXAngle = 32.f / 180.f * 3.14159f;
   float eye[] = { m_live_table->m_right * 0.5f, m_live_table->m_bottom * 0.5f, -m_camDistance };
   float at[] = { m_live_table->m_right * 0.5f, m_live_table->m_bottom * 0.5f, 0.f };
   float up[] = { 0.f, -1.f, 0.f };
   LookAt(eye, at, up, (float *)(m_camView.m));
   ImGuizmo::AllowAxisFlip(false);

   ImGui_ImplWin32_Init(rd->getHwnd());

   SetupImGuiStyle(1.0f);

   ImGui_ImplWin32_EnableDpiAwareness();
   m_dpi = ImGui_ImplWin32_GetDpiScaleForHwnd(rd->getHwnd());
   ImGui::GetStyle().ScaleAllSizes(m_dpi);

   float overlaySize = min(32.f * m_dpi, min(m_player->m_wnd_width, m_player->m_wnd_height) / (26.f * 2.0f)); // Fit 26 lines of text on screen
   m_overlayFont = io.Fonts->AddFontFromMemoryCompressedTTF(droidsans_compressed_data, droidsans_compressed_size, overlaySize);

   m_baseFont = io.Fonts->AddFontFromMemoryCompressedTTF(droidsans_compressed_data, droidsans_compressed_size, 13.0f * m_dpi);
   ImFontConfig icons_config;
   icons_config.MergeMode = true;
   icons_config.PixelSnapH = true;
   icons_config.GlyphMinAdvanceX = 13.0f * m_dpi;
   static constexpr ImWchar icons_ranges[] = { ICON_MIN_FK, ICON_MAX_16_FK, 0 };
   io.Fonts->AddFontFromMemoryCompressedTTF(fork_awesome_compressed_data, fork_awesome_compressed_size, 13.0f * m_dpi, &icons_config, icons_ranges);

#ifdef ENABLE_SDL
   ImGui_ImplOpenGL3_Init();
#else
   ImGui_ImplDX9_Init(rd->GetCoreDevice());
#endif
}

LiveUI::~LiveUI()
{
   if (ImGui::GetCurrentContext())
   {
#ifdef ENABLE_SDL
      ImGui_ImplOpenGL3_Shutdown();
#else
      ImGui_ImplDX9_Shutdown();
#endif
      ImGui_ImplWin32_Shutdown();
      ImPlot::DestroyContext();
      ImGui::DestroyContext();
   }
}

bool LiveUI::HasKeyboardCapture() const
{
   return ImGui::GetIO().WantCaptureKeyboard;
}

bool LiveUI::HasMouseCapture() const
{
   return ImGui::GetIO().WantCaptureKeyboard;
}

void LiveUI::Render()
{
   // For the time being, the UI is only available inside a running player
   if (g_pplayer == nullptr || g_pplayer->m_closing != Player::CS_PLAYING)
      return;
   if (m_rotate != 0)
   {
      // We hack into ImGui renderer for the simple tooltips that must be displayed facing the user
      ImGui::GetBackgroundDrawList()->AddCallback(
         [](const ImDrawList *parent_list, const ImDrawCmd *cmd)
         {
            LiveUI *const lui = (LiveUI *)cmd->UserCallbackData;
            Matrix3D matRotate, matTranslate;
            matRotate.RotateZMatrix((float)(lui->m_rotate * (M_PI / 2.0)));
            switch (lui->m_rotate)
            {
            case 1: matTranslate.SetTranslation(ImGui::GetIO().DisplaySize.y, 0, 0); break;
            case 2: matTranslate.SetTranslation(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y, 0); break;
            case 3: matTranslate.SetTranslation(0, ImGui::GetIO().DisplaySize.x, 0); break;
            }
            matTranslate.Multiply(matRotate, matTranslate);
#ifdef ENABLE_SDL
            const float L = 0, R = (lui->m_rotate == 1 || lui->m_rotate == 3) ? ImGui::GetIO().DisplaySize.y : ImGui::GetIO().DisplaySize.x;
            const float T = 0, B = (lui->m_rotate == 1 || lui->m_rotate == 3) ? ImGui::GetIO().DisplaySize.x : ImGui::GetIO().DisplaySize.y;
            Matrix3D matProj(
               2.0f / (R - L), 0.0f, 0.0f, 0.0f, 
               0.0f, 2.0f / (T - B), 0.0f, 0.0f, 
               0.0f, 0.0f, -1.0f, 0.0f, 
               (R + L) / (L - R), (T + B) / (B - T), 0.0f, 1.0f);
            matProj.Multiply(matTranslate, matProj);
            GLint shaderHandle;
            glGetIntegerv(GL_CURRENT_PROGRAM, &shaderHandle);
            GLuint attribLocationProjMtx = glGetUniformLocation(shaderHandle, "ProjMtx");
            glUniformMatrix4fv(attribLocationProjMtx, 1, GL_FALSE, (float*)&(matProj.m[0]));
            glDisable(GL_SCISSOR_TEST);
#else
            lui->m_rd->GetCoreDevice()->SetTransform(D3DTS_WORLD, (const D3DXMATRIX *)&matTranslate);
            lui->m_rd->GetCoreDevice()->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
#endif

         }, this);
   }
   ImGui::Render();
   ImDrawData *draw_data = ImGui::GetDrawData();
   if (m_rotate == 1 || m_rotate == 3)
   {
      float tmp = draw_data->DisplaySize.x;
      draw_data->DisplaySize.x = draw_data->DisplaySize.y;
      draw_data->DisplaySize.y = tmp;
   }
#ifdef ENABLE_SDL
   if (GLAD_GL_VERSION_4_3)
      glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "ImGui");
   ImGui_ImplOpenGL3_RenderDrawData(draw_data);
   if (GLAD_GL_VERSION_4_3)
      glPopDebugGroup();
#else
   ImGui_ImplDX9_RenderDrawData(draw_data);
#endif
}

void LiveUI::OpenMainUI()
{
   // Opens the main UI. This will only open the main splash modal which allows to go further in the live UI.
   if (!m_ShowUI && !m_ShowSplashModal)
   {
      while (ShowCursor(FALSE)>=0) ;
      while (ShowCursor(TRUE)<0) ;
      m_ShowUI = false;
      m_ShowSplashModal = true;
      m_OpenUITime = msec();
      PausePlayer(true);
   }
}

void LiveUI::ToggleFPS()
{
   m_show_fps = (m_show_fps + 1) % 3;
   if (m_show_fps == 0)
      m_rd->LogNextFrame();
   if (m_show_fps == 1)
      g_pplayer->InitFPS();
}

void LiveUI::Update()
{
   // For the time being, the UI is only available inside a running player
   if (m_player == nullptr || m_player->m_closing != Player::CS_PLAYING)
      return;
   
#ifdef ENABLE_SDL
   ImGui_ImplOpenGL3_NewFrame();
#else
   ImGui_ImplDX9_NewFrame();
#endif
   ImGui_ImplWin32_NewFrame();
   if (m_ShowUI || m_ShowSplashModal)
      m_rotate = 0;
   else
   {
      // If we are showing overlays, apply main camera rotation
      m_rotate = ((int)(g_pplayer->m_ptable->m_BG_rotation[g_pplayer->m_ptable->m_BG_current_set] / 90.0f)) & 3;
      if (m_rotate == 1 || m_rotate == 3)
      {
         ImGuiIO &io = ImGui::GetIO();
         float tmp = io.DisplaySize.x;
         io.DisplaySize.x = io.DisplaySize.y;
         io.DisplaySize.y = tmp;
      }
   }
   ImGui::NewFrame();
   ImGuizmo::SetOrthographic(m_orthoCam);
   ImGuizmo::BeginFrame();
   ImGuiIO &io = ImGui::GetIO();
   ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
   ImGui::PushFont(m_baseFont);

   if (m_ShowUI || m_ShowSplashModal)
   {
      // Main UI
      UpdateMainUI();
   }
   else
   {
      // Info overlays: this is not a normal UI aligned to the monitor orientation but an overlay used when playing, 
      // therefore it is rotated like the playfield to face the user and only displays for right angles
      ImGui::PushFont(m_overlayFont);
      if ((float)m_rotate * 90.0f == g_pplayer->m_ptable->m_BG_rotation[g_pplayer->m_ptable->m_BG_current_set])
      {
         if (g_pplayer->m_cameraMode)
            // Camera mode info text
            UpdateCameraModeUI();
         else
         {
            // Info tooltips
            if (g_pplayer->m_closing == Player::CS_PLAYING
               && (g_pplayer->m_stereo3D != STEREO_OFF && g_pplayer->m_stereo3D != STEREO_VR && !g_pplayer->m_stereo3Denabled
                  && (usec() < m_StartTime_usec + (U64)4e+6))) // show for max. 4 seconds
               HelpSplash("3D Stereo is enabled but currently toggled off, press F10 to toggle 3D Stereo on", m_rotate);
            //!! visualize with real buttons or at least the areas?? Add extra buttons?
            if (g_pplayer->m_closing == Player::CS_PLAYING && g_pplayer->m_supportsTouch && g_pplayer->m_showTouchMessage
               && (usec() < m_StartTime_usec + (U64)12e+6)) // show for max. 12 seconds
               HelpSplash("You can use Touch controls on this display: bottom left area to Start Game, bottom right area to use the Plunger\n"
                           "lower left/right for Flippers, upper left/right for Magna buttons, top left for Credits and (hold) top right to Exit",
                  m_rotate);
         }
      }
      ImGui::PopFont();
   }

   // FPS Overlays
   if (m_show_fps > 0)
   {
      // Display simple FPS window
      constexpr ImGuiWindowFlags window_flags
         = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
      ImGui::SetNextWindowBgAlpha(0.75f);
      ImGui::SetNextWindowPos(ImVec2(10, 10 + m_menubar_height + m_toolbar_height));
      ImGui::Begin("FPS", nullptr, window_flags);
      const float fpsAvg = (m_player->m_avgFrameDuration == 0) ? 0.0f : ((float)((double)1000000.0 * m_player->m_avgFrameCount / m_player->m_avgFrameDuration));
      ImGui::Text("FPS: %.1f (%.1f %5.1fms avg)", m_player->m_fps + 0.01f, fpsAvg + 0.01f, 1000.0f / (fpsAvg + 0.01f));
      ImGui::End();
   }
   if (m_show_fps == 2)
   {
      // Display FPS window with plots
      constexpr ImGuiWindowFlags window_flags
         = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
      ImGui::SetNextWindowSize(ImVec2(530, 500));
      ImGui::SetNextWindowPos(ImVec2((float)(m_player->m_wnd_width - 530 - 10), 10 + m_menubar_height + m_toolbar_height));
      ImGui::Begin("Plots", nullptr, window_flags);
      //!! This example assumes 60 FPS. Higher FPS requires larger buffer size.
      static ScrollingData sdata1, sdata2, sdata3, sdata4, sdata5, sdata6;
      //static RollingData   rdata1, rdata2;
      static double t = 0.;
      t += ImGui::GetIO().DeltaTime;

      sdata6.AddPoint((float)t, float(1e-3 * m_player->m_script_period) * 1.f);
      sdata5.AddPoint((float)t, sdata5.GetLast().y * 0.95f + sdata6.GetLast().y * 0.05f);

      sdata4.AddPoint((float)t, float(1e-3 * (m_player->m_phys_period - m_player->m_script_period)) * 5.f);
      sdata3.AddPoint((float)t, sdata3.GetLast().y * 0.95f + sdata4.GetLast().y * 0.05f);

      sdata2.AddPoint((float)t, m_player->m_fps * 0.003f);
      //rdata2.AddPoint((float)t, m_fps * 0.003f);
      sdata1.AddPoint((float)t, sdata1.GetLast().y * 0.95f + sdata2.GetLast().y * 0.05f);
      //rdata1.AddPoint((float)t, sdata1.GetLast().y*0.95f + sdata2.GetLast().y*0.05f);

      // Do not show controls since mouse events are not translated with regards to screen rotation
      static float history = 2.5f;
      //ImGui::SliderFloat("History", &history, 1, 10, "%.1f s");
      //rdata1.Span = history;
      //rdata2.Span = history;
      constexpr int rt_axis = ImPlotAxisFlags_NoTickLabels;
      if (ImPlot::BeginPlot("##ScrollingFPS", ImVec2(-1, 150), ImPlotFlags_None))
      {
         ImPlot::SetupAxis(ImAxis_X1, nullptr, rt_axis);
         ImPlot::SetupAxis(ImAxis_Y1, nullptr, rt_axis | ImPlotAxisFlags_LockMin);
         ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always);
         ImPlot::PlotLine("FPS", &sdata2.Data[0].x, &sdata2.Data[0].y, sdata2.Data.size(), 0, sdata2.Offset, 2 * sizeof(float));
         ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(1, 0, 0, 0.25f));
         ImPlot::PlotLine("Smoothed FPS", &sdata1.Data[0].x, &sdata1.Data[0].y, sdata1.Data.size(), 0, sdata1.Offset, 2 * sizeof(float));
         ImPlot::PopStyleColor();
         ImPlot::EndPlot();
      }
      /*
      if (ImPlot::BeginPlot("##RollingFPS", ImVec2(-1, 150), ImPlotFlags_Default)) {
         ImPlot::SetupAxis(ImAxis_X1, nullptr, rt_axis);
         ImPlot::SetupAxis(ImAxis_Y1, nullptr, rt_axis);
         ImPlot::SetupAxis(ImAxis_X1, 0, history, ImGuiCond_Always);
         ImPlot::PlotLine("Average FPS", &rdata1.Data[0].x, &rdata1.Data[0].y, rdata1.Data.size(), 0, 0, 2 * sizeof(float));
         ImPlot::PlotLine("FPS", &rdata2.Data[0].x, &rdata2.Data[0].y, rdata2.Data.size(), 0, 0, 2 * sizeof(float));
         ImPlot::EndPlot();
      }*/
      if (ImPlot::BeginPlot("##ScrollingPhysics", ImVec2(-1, 150), ImPlotFlags_None))
      {
         ImPlot::SetupAxis(ImAxis_X1, nullptr, rt_axis);
         ImPlot::SetupAxis(ImAxis_Y1, nullptr, rt_axis | ImPlotAxisFlags_LockMin);
         ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always);
         ImPlot::PlotLine("ms Physics", &sdata4.Data[0].x, &sdata4.Data[0].y, sdata4.Data.size(), 0, sdata4.Offset, 2 * sizeof(float));
         ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(1, 0, 0, 0.25f));
         ImPlot::PlotLine("Smoothed ms Physics", &sdata3.Data[0].x, &sdata3.Data[0].y, sdata3.Data.size(), 0, sdata3.Offset, 2 * sizeof(float));
         ImPlot::PopStyleColor();
         ImPlot::EndPlot();
      }
      if (ImPlot::BeginPlot("##ScrollingScript", ImVec2(-1, 150), ImPlotFlags_None))
      {
         ImPlot::SetupAxis(ImAxis_X1, nullptr, rt_axis);
         ImPlot::SetupAxis(ImAxis_Y1, nullptr, rt_axis | ImPlotAxisFlags_LockMin);
         ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always);
         ImPlot::PlotLine("ms Script", &sdata6.Data[0].x, &sdata6.Data[0].y, sdata6.Data.size(), 0, sdata6.Offset, 2 * sizeof(float));
         ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(1, 0, 0, 0.25f));
         ImPlot::PlotLine("Smoothed ms Script", &sdata5.Data[0].x, &sdata5.Data[0].y, sdata5.Data.size(), 0, sdata5.Offset, 2 * sizeof(float));
         ImPlot::PopStyleColor();
         ImPlot::EndPlot();
      }
      ImGui::End();
   }

   ImGui::PopFont();
   ImGui::EndFrame();
}

void LiveUI::UpdateCameraModeUI()
{
   static float last_frame_height = -100.f; // Hide first frame below display bounds
   PinTable* table = m_live_table;
   constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
   ImGui::SetNextWindowBgAlpha(0.35f);
   ImGui::SetNextWindowPos(ImVec2(10, ImGui::GetIO().DisplaySize.y - 10 - last_frame_height));
   ImGui::Begin("CameraMode", nullptr, window_flags);
   for (int i = 0; i < 14; i++)
   {
      if (m_player->m_cameraMode && (i == m_player->m_backdropSettingActive || (m_player->m_backdropSettingActive == 3 && (i == 4 || i == 5))))
         ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
      switch (i)
      {
      case 0: ImGui::Text("Inclination: %.3f", table->m_BG_inclination[table->m_BG_current_set]); break;
      case 1: ImGui::Text("Field Of View: %.3f", table->m_BG_FOV[table->m_BG_current_set]); break;
      case 2: ImGui::Text("Layback: %.3f", table->m_BG_layback[table->m_BG_current_set]); ImGui::NewLine(); break;
      case 4: ImGui::Text("X Scale: %.3f", table->m_BG_scalex[table->m_BG_current_set]); break;
      case 5: ImGui::Text("Y Scale: %.3f", table->m_BG_scaley[table->m_BG_current_set]); break;
      case 6: ImGui::Text("Z Scale: %.3f", table->m_BG_scalez[table->m_BG_current_set]); ImGui::NewLine(); break;
      case 7: ImGui::Text("X Offset: %.0f", table->m_BG_xlatex[table->m_BG_current_set]); break;
      case 8: ImGui::Text("Y Offset: %.0f", table->m_BG_xlatey[table->m_BG_current_set]); break;
      case 9: ImGui::Text("Z Offset: %.0f", table->m_BG_xlatez[table->m_BG_current_set]); ImGui::NewLine(); break;
      case 10: ImGui::Text("Light Emission Scale: %.0f", table->m_lightEmissionScale); break;
      case 11: ImGui::Text("Light Range: %.0f", table->m_lightRange); break;
      case 12: ImGui::Text("Light Height: %.0f", table->m_lightHeight); ImGui::NewLine(); break;
      case 13: ImGui::Text("Environment Emission: %.3f", table->m_envEmissionScale); break;
      }
      if (m_player->m_cameraMode && (i == m_player->m_backdropSettingActive || (m_player->m_backdropSettingActive == 3 && (i == 4 || i == 5))))
         ImGui::PopStyleColor();
   }

   ImGui::NewLine();

   ImGui::Text("Camera at X: %.2f Y: %.2f Z: %.2f,  Rotation: %.2f", 
      -m_pin3d->GetMVP().GetView()._41,
      (table->m_BG_current_set == 0 || table->m_BG_current_set == 2) ? m_pin3d->GetMVP().GetView()._42 : -m_pin3d->GetMVP().GetView()._42, 
      m_pin3d->GetMVP().GetView()._43,
      table->m_BG_rotation[table->m_BG_current_set]);
   
   if (m_player->m_cameraMode)
   {
      ImGui::NewLine();
      ImGui::Text("Left / Right flipper key = decrease / increase value highlighted in green");
      ImGui::Text("Left / Right magna save key = previous / next option");
      ImGui::NewLine();
      ImGui::Text("Left / Right nudge key = rotate table orientation (if enabled in the Key settings)");
      ImGui::Text("Navigate around with the Arrow Keys and Left Alt Key (if enabled in the Key settings)");
      if (m_app->m_povEdit)
         ImGui::Text("Start Key: export POV file and quit, or Credit Key: quit without export");
      else
         ImGui::Text("Start Key: reset POV to old values");
   }

   last_frame_height = ImGui::GetWindowHeight();
   ImGui::End();
}

void LiveUI::PausePlayer(bool pause)
{
   g_pplayer->m_debugWindowActive = pause;
   g_pplayer->RecomputePauseState();
   g_pplayer->RecomputePseudoPauseState();
}

void LiveUI::EnterEditMode()
{
   m_player->EnableStaticPrePass(false);
   m_player->m_cameraMode = false;
}

void LiveUI::ExitEditMode()
{
   m_player->EnableStaticPrePass(!m_old_player_dynamic_mode);
   m_player->m_cameraMode = m_old_player_camera_mode;
   SetupImGuiStyle(1.0f);
   m_useEditorCam = false;
   m_pin3d->InitLayout(m_live_table->m_BG_enable_FSS, m_live_table->GetMaxSeparation());
}

void LiveUI::HideUI()
{ 
   m_ShowSplashModal = false;
   m_ShowUI = false;
   PausePlayer(false);
   while (ShowCursor(TRUE)<0) ;
   while (ShowCursor(FALSE)>=0) ;
}

void LiveUI::UpdateMainUI()
{
   m_menubar_height = 0.0f;
   m_toolbar_height = 0.0f;

   // Gives some transparency when positioning camera to better view camera view bounds
   // TODO for some reasons, this breaks the modal background behavior
   //SetupImGuiStyle(m_selection.type == LiveUI::Selection::SelectionType::S_CAMERA ? 0.3f : 1.0f);

   bool popup_video_settings = false;
   bool popup_audio_settings = false;
   bool popup_renderer_inspection = false;
   bool popup_headtracking = false;

   // Main menubar
   if (!m_RendererInspection)
   {
      m_player->EnableStaticPrePass(false);
      if (ImGui::BeginMainMenuBar())
      {
         if (ImGui::BeginMenu("Preferences"))
         {
            //if (ImGui::MenuItem("Audio Options"))
            //   popup_audio_settings = true;
            if (ImGui::MenuItem("Video Options"))
               popup_video_settings = true;
            if (ImGui::MenuItem("BAM Headtracking"))
               popup_headtracking = true;
            if (ImGui::MenuItem("Renderer Inspection"))
               m_RendererInspection = true;
            ImGui::EndMenu();
         }
         m_menubar_height = ImGui::GetWindowSize().y;
         ImGui::EndMainMenuBar();
      }

      // Main toolbar
      m_toolbar_height = 20.f * m_dpi;
      const ImGuiViewport *const viewport = ImGui::GetMainViewport();
      ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + m_menubar_height));
      ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, m_toolbar_height));
      constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
      ImGui::Begin("TOOLBAR", nullptr, window_flags);
      ImGui::PopStyleVar();
      if (ImGui::Button(g_pplayer->m_debugWindowActive ? ICON_FK_PLAY : ICON_FK_PAUSE))
      {
         PausePlayer(!g_pplayer->m_debugWindowActive);
      }
      ImGui::SameLine();
      if (ImGui::Button(ICON_FK_STOP))
      {
         ImGui::CloseCurrentPopup();
         HideUI();
         m_table->QuitPlayer(Player::CS_STOP_PLAY);
      }
      /* if (ImGui::Button("Resume Game"))
      {
         ExitEditMode();
         HideUI();
      }*/
      ImGui::End();

      // Side panels
      UpdateOutlinerUI();
      UpdatePropertyUI();
   }

   // View gizmo
   if (m_useEditorCam)
   {
      ImGuiIO &io = ImGui::GetIO();
      if (m_orthoCam)
      {
         float viewHeight = m_camDistance;
         float viewWidth = viewHeight * io.DisplaySize.x / io.DisplaySize.y;
         OrthoGraphic(-viewWidth, viewWidth, -viewHeight, viewHeight, 10.f, -10000.f, (float *)(m_camProj.m));
      }
      else
      {
         Perspective(39.6f, io.DisplaySize.x / io.DisplaySize.y, 10.0f, 10000.0f, (float *)(m_camProj.m));
      }
      float* cameraView = (float *)(m_camView.m);
      float *cameraProjection = (float *)(m_camProj.m); 

      /* Matrix3D gridMatrix;
      static const float identityMatrix[16] = { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f };
      gridMatrix.RotateXMatrix(M_PI * 0.5f);
      gridMatrix.Scale(10.0f, 1.0f, 10.0f);
      ImGuizmo::DrawGrid((const float *)(m_camView.m), (const float *)(m_camProj.m), (const float *)(gridMatrix.m), 100.f); */
      //ImGuizmo::DrawGrid(cameraView, cameraProjection, identityMatrix, 100.f);

      Matrix3D transform;
      if (GetSelectionTransform(transform))
      {
         float camViewLH[16];
         memcpy(camViewLH, m_camView.m, sizeof(float) * 4 * 4);
         for (int i = 8; i < 12; i++)
            camViewLH[i] = -camViewLH[i];
         Matrix3D prevTransform(transform);
         ImGuizmo::Manipulate(camViewLH, cameraProjection, m_gizmoOperation, m_gizmoMode, (float *)(transform.m));
         if (memcmp(transform.m, prevTransform.m, 16 * sizeof(float)) != 0)
            SetSelectionTransform(transform);
      }

      // Camera orbit manipulator
      Matrix3D prevView(m_camView);
      float viewManipulateRight = ImGui::GetIO().DisplaySize.x - m_properties_width - 16;
      float viewManipulateTop = m_toolbar_height + m_menubar_height + 16;
      ImGuizmo::ViewManipulate(cameraView, cameraProjection, m_gizmoOperation, m_gizmoMode, cameraView, m_camDistance,
         ImVec2(viewManipulateRight - 128, viewManipulateTop + 16), ImVec2(128, 128), 0x10101010);
      if (memcmp(cameraView, prevView.m, 16 * sizeof(float)) != 0)
         m_orthoCam = false; // switch to perspective when user orbit the view
   }

   // Popups & Modal dialogs
   if (popup_video_settings)
      ImGui::OpenPopup(ID_VIDEO_SETTINGS);
   if (ImGui::IsPopupOpen(ID_VIDEO_SETTINGS))
      UpdateVideoOptionsModal();

   if (popup_audio_settings)
      ImGui::OpenPopup(ID_AUDIO_SETTINGS);
   if (ImGui::IsPopupOpen(ID_AUDIO_SETTINGS))
      UpdateAudioOptionsModal();

   if (m_RendererInspection)
      UpdateRendererInspectionModal();

   if (popup_headtracking)
      ImGui::OpenPopup(ID_BAM_SETTINGS);
   if (ImGui::IsPopupOpen(ID_BAM_SETTINGS))
      UpdateHeadTrackingModal();

   if (m_ShowSplashModal && !ImGui::IsPopupOpen(ID_MODAL_SPLASH))
      ImGui::OpenPopup(ID_MODAL_SPLASH);
   if (m_ShowSplashModal)
      UpdateMainSplashModal();

   // Handle uncaught mouse & keyboard interaction
   if (!ImGui::GetIO().WantCaptureMouse)
   {
      // Zoom in/out with mouse wheel
      if (m_useEditorCam && ImGui::GetIO().MouseWheel != 0)
      {
         Matrix3D view(m_camView);
         view.Invert();
         vec3 up(view._21, view._22, view._23);
         vec3 dir(view._31, view._32, view._33);
         vec3 pos(view._41, view._42, view._43);
         vec3 camTarget = pos - dir * m_camDistance;
         m_camDistance *= (float) pow(1.1, -ImGui::GetIO().MouseWheel);
         vec3 newEye = camTarget + dir * m_camDistance;
         LookAt(&newEye.x, &camTarget.x, &up.x, (float *)(m_camView.m));
      }

      // Pan mouse
      if (m_useEditorCam && ImGui::IsMouseDown(ImGuiMouseButton_Middle))
      {
         ImVec2 drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle);
         ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);
         m_useEditorCam = true;
         Matrix3D view(m_camView);
         view.Invert();
         vec3 right(view._11, view._12, view._13);
         vec3 up(view._21, view._22, view._23);
         vec3 dir(view._31, view._32, view._33);
         vec3 pos(view._41, view._42, view._43);
         vec3 camTarget = pos - dir * m_camDistance;
         if (ImGui::GetIO().KeyShift)
         {
            pos = pos - right * drag.x + up * drag.y;
            camTarget = camTarget - right * drag.x + up * drag.y;
            LookAt(&pos.x, &camTarget.x, &up.x, (float *)(m_camView.m));
         }
      }
   }
   if (!ImGui::GetIO().WantCaptureKeyboard)
   {
      if (!m_ShowSplashModal)
      {
         if (ImGui::IsKeyPressed(ImGuiKey_Escape) && m_gizmoOperation != ImGuizmo::NONE)
         {
            // Cancel current operation
            m_gizmoOperation = ImGuizmo::NONE;
         }
         else if (ImGui::IsKeyPressed(dikToImGuiKeys[m_player->m_rgKeys[eEscape]]) && !m_disable_esc)
         {
            // Open Main modal dialog
            m_ShowSplashModal = true;
         }
         else if (m_useEditorCam && ImGui::IsKeyPressed(ImGuiKey_G))
         {
            // Grab (translate)
            if (ImGui::GetIO().KeyAlt)
            {
               Matrix3D transform;
               if (GetSelectionTransform(transform))
                  SetSelectionTransform(transform, true, false, false);
            }
            else
            {
               m_gizmoOperation = ImGuizmo::TRANSLATE;
               m_gizmoMode = m_gizmoOperation == ImGuizmo::TRANSLATE ? (m_gizmoMode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL) : ImGuizmo::WORLD;
            }
         }
         else if (m_useEditorCam && ImGui::IsKeyPressed(ImGuiKey_S))
         {
            // Scale
            if (ImGui::GetIO().KeyAlt)
            {
               Matrix3D transform;
               if (GetSelectionTransform(transform))
                  SetSelectionTransform(transform, false, true, false);
            }
            else
            {
               m_gizmoOperation = ImGuizmo::SCALE;
               m_gizmoMode = m_gizmoOperation == ImGuizmo::SCALE ? (m_gizmoMode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL) : ImGuizmo::WORLD;
            }
         }
         else if (m_useEditorCam && ImGui::IsKeyPressed(ImGuiKey_R))
         {
            // Rotate
            if (ImGui::GetIO().KeyAlt)
            {
               Matrix3D transform;
               if (GetSelectionTransform(transform))
                  SetSelectionTransform(transform, false, false, true);
            }
            else
            {
               m_gizmoOperation = ImGuizmo::ROTATE;
               m_gizmoMode = m_gizmoOperation == ImGuizmo::ROTATE ? (m_gizmoMode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL) : ImGuizmo::WORLD;
            }
         }
         else if (ImGui::IsKeyPressed(ImGuiKey_Keypad5))
         {
            // Editor toggle orthographic / perspective
            m_useEditorCam = true;
            m_orthoCam = !m_orthoCam;
         }
         else if (ImGui::IsKeyPressed(ImGuiKey_KeypadDecimal))
         {
            // Editor Camera center on selection
            m_useEditorCam = true;
            Matrix3D view(m_camView);
            view.Invert();
            vec3 up(view._21, view._22, view._23);
            vec3 dir(view._31, view._32, view._33);
            vec3 pos(view._41, view._42, view._43);
            vec3 camTarget = pos - dir * m_camDistance;
            vec3 newTarget = camTarget;
            Matrix3D transform;
            if (GetSelectionTransform(transform))
               newTarget = vec3(transform._41, transform._42, transform._43);
            vec3 newEye = newTarget + dir * m_camDistance;
            LookAt(&newEye.x, &newTarget.x, &up.x, (float *)(m_camView.m));
         }
         else if (ImGui::IsKeyPressed(ImGuiKey_Keypad7))
         {
            // Editor Camera to Top / Bottom
            m_useEditorCam = true;
            m_orthoCam = true;
            Matrix3D view(m_camView);
            view.Invert();
            vec3 up(view._21, view._22, view._23);
            vec3 dir(view._31, view._32, view._33);
            vec3 pos(view._41, view._42, view._43);
            vec3 camTarget = pos - dir * m_camDistance;
            vec3 newUp(0.f, -1.f, 0.f);
            vec3 newDir(0.f, 0.f, ImGui::GetIO().KeyCtrl ? 1.f : -1.f);
            vec3 newEye = camTarget + newDir * m_camDistance;
            LookAt(&newEye.x, &camTarget.x, &newUp.x, (float *)(m_camView.m));
         }
         else if (ImGui::IsKeyPressed(ImGuiKey_Keypad1))
         {
            // Editor Camera to Front / Back
            m_useEditorCam = true;
            m_orthoCam = true;
            Matrix3D view(m_camView);
            view.Invert();
            vec3 up(view._21, view._22, view._23);
            vec3 dir(view._31, view._32, view._33);
            vec3 pos(view._41, view._42, view._43);
            vec3 camTarget = pos - dir * m_camDistance;
            vec3 newUp(0.f, 0.f, -1.f);
            vec3 newDir(0.f, ImGui::GetIO().KeyCtrl ? -1.f : 1.f, 0.f);
            vec3 newEye = camTarget + newDir * m_camDistance;
            LookAt(&newEye.x, &camTarget.x, &newUp.x, (float *)(m_camView.m));
         }
         else if (ImGui::IsKeyPressed(ImGuiKey_Keypad3))
         {
            // Editor Camera to Right / Left
            m_useEditorCam = true;
            m_orthoCam = true;
            Matrix3D view(m_camView);
            view.Invert();
            vec3 up(view._21, view._22, view._23);
            vec3 dir(view._31, view._32, view._33);
            vec3 pos(view._41, view._42, view._43);
            vec3 camTarget = pos - dir * m_camDistance;
            vec3 newUp(0.f, 0.f, 1.f);
            vec3 newDir(ImGui::GetIO().KeyCtrl ? 1.f : -1.f, 0.f, 0.f);
            vec3 newEye = camTarget + newDir * m_camDistance;
            LookAt(&newEye.x, &camTarget.x, &newUp.x, (float *)(m_camView.m));
         }
      }
   }

   if (m_useEditorCam)
   {
      // Apply editor camera to renderer (move view from right handed to left handed)
      Matrix3D mat(m_camView); // Convert from right handed (ImGuizmo view manipulate is right handed) to VPX's left handed coordinate system
      mat._31 = -mat._31;
      mat._32 = -mat._32;
      mat._33 = -mat._33;
      mat._34 = -mat._34;
      m_pin3d->GetMVP().SetView(mat);
      m_pin3d->GetMVP().SetProj(0, m_camProj);
   }
   else
   {
      m_pin3d->InitLayout(m_live_table->m_BG_enable_FSS, m_live_table->GetMaxSeparation());
   }
}

bool LiveUI::GetSelectionTransform(Matrix3D& transform)
{
   if (m_selection.type == LiveUI::Selection::SelectionType::S_EDITABLE && m_selection.editable->GetItemType() == eItemPrimitive)
   {
      Primitive *p = (Primitive *)m_selection.editable;
      Matrix3D Smatrix;
      Smatrix.SetScaling(p->m_d.m_vSize.x, p->m_d.m_vSize.y, p->m_d.m_vSize.z);
      Matrix3D Tmatrix;
      Tmatrix.SetTranslation(p->m_d.m_vPosition.x, p->m_d.m_vPosition.y, p->m_d.m_vPosition.z + m_live_table->m_tableheight);
      Matrix3D Rmatrix;
      Matrix3D tempMatrix;
      Rmatrix.RotateZMatrix(ANGTORAD(p->m_d.m_aRotAndTra[2]));
      tempMatrix.RotateYMatrix(ANGTORAD(p->m_d.m_aRotAndTra[1]));
      tempMatrix.Multiply(Rmatrix, Rmatrix);
      tempMatrix.RotateXMatrix(ANGTORAD(p->m_d.m_aRotAndTra[0]));
      tempMatrix.Multiply(Rmatrix, Rmatrix);
      transform = Smatrix;
      Rmatrix.Multiply(transform, transform);
      Tmatrix.Multiply(transform, transform); // fullMatrix = Scale * Rotate * Translate
      return true;
   }

   if (m_selection.type == LiveUI::Selection::SelectionType::S_EDITABLE && m_selection.editable->GetItemType() == eItemLight)
   {
      Light *l = (Light *)m_selection.editable;
      const float height = (m_selection.is_live ? m_live_table : m_table)->GetSurfaceHeight(l->m_d.m_szSurface, l->m_d.m_vCenter.x, l->m_d.m_vCenter.y);
      transform.SetTranslation(l->m_d.m_vCenter.x, l->m_d.m_vCenter.y, height + l->m_d.m_height);
      return true;
   }

   return false;
}

void LiveUI::SetSelectionTransform(Matrix3D &newTransform, bool clearPosition, bool clearScale, bool clearRotation)
{
   Matrix3D transform = newTransform;
   Vertex3Ds right(transform._11, transform._12, transform._13);
   Vertex3Ds up(transform._21, transform._22, transform._23);
   Vertex3Ds dir(transform._31, transform._32, transform._33);
   float xscale = right.Length();
   float yscale = up.Length();
   float zscale = dir.Length();
   transform._11 /= xscale; // Normalize transform to evaluate rotation
   transform._12 /= xscale;
   transform._13 /= xscale;
   transform._21 /= yscale;
   transform._22 /= yscale;
   transform._23 /= yscale;
   transform._31 /= zscale;
   transform._32 /= zscale;
   transform._33 /= zscale;
   if (clearScale)
      xscale = yscale = zscale = 1.f;

   float posX = transform._41;
   float posY = transform._42;
   float posZ = transform._43;
   if (clearPosition)
      posX = posY = posZ = 0.f;

   // Derived from https://learnopencv.com/rotation-matrix-to-euler-angles/
   float rotX, rotY, rotZ;
   float sy = sqrtf(transform._11 * transform._11 + transform._21 * transform._21);
   if (sy > 1e-6f)
   {
      rotX = -RADTOANG(atan2f(transform._32, transform._33));
      rotY = -RADTOANG(atan2f(-transform._31, sy));
      rotZ = -RADTOANG(atan2f(transform._21, transform._11));
   }
   else
   {
      rotX = -RADTOANG(atan2f(transform._23, transform._22));
      rotY = -RADTOANG(atan2f(-transform._22, sy));
      rotZ = 0.f;
   }
   if (clearRotation)
      rotX = rotY = rotZ = 0.f;

   if (m_selection.type == LiveUI::Selection::SelectionType::S_EDITABLE && m_selection.editable->GetItemType() == eItemPrimitive)
   {
      Primitive *p = (Primitive *)m_selection.editable;
      p->m_d.m_vPosition.x = posX;
      p->m_d.m_vPosition.y = posY;
      p->m_d.m_vPosition.z = posZ;
      p->m_d.m_aRotAndTra[0] = rotX;
      p->m_d.m_aRotAndTra[1] = rotY;
      p->m_d.m_aRotAndTra[2] = rotZ;
      p->m_d.m_vSize.x = xscale;
      p->m_d.m_vSize.y = yscale;
      p->m_d.m_vSize.z = zscale;
   }
}

void LiveUI::UpdateOutlinerUI()
{
   const ImGuiViewport * const viewport = ImGui::GetMainViewport();
   const float pane_width = 200.f * m_dpi;
   ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + m_menubar_height + m_toolbar_height));
   ImGui::SetNextWindowSize(ImVec2(pane_width, viewport->Size.y - m_menubar_height - m_toolbar_height));
   constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
      | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f * m_dpi, 4.0f * m_dpi));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
   ImGui::Begin("OUTLINER", nullptr, window_flags);

   if (ImGui::BeginTabBar("Startup/Live", ImGuiTabBarFlags_AutoSelectNewTabs))
   {
      for (int tab = 0; tab < 2; tab++)
      {
         const bool is_live = (tab == 1);
         PinTable *table = is_live ? m_live_table : m_table;
         if (ImGui::BeginTabItem(is_live ? "Live" : "Startup", nullptr))
         {
            if (ImGui::TreeNodeEx("View Setups"))
            {
               if (ImGui::Selectable("Live Editor Camera"))
               {
                  m_selection.type = Selection::SelectionType::S_NONE;
                  m_useEditorCam = true;
               }
               Selection cam0(Selection::SelectionType::S_CAMERA, is_live, 0);
               if (ImGui::Selectable("Desktop", m_selection == cam0))
               {
                  m_selection = cam0;
                  m_useEditorCam = false;
                  table->m_BG_current_set = 0;
               }
               Selection cam1(Selection::SelectionType::S_CAMERA, is_live, 1);
               if (ImGui::Selectable("Cabinet", m_selection == cam1))
               {
                  m_selection = cam1;
                  m_useEditorCam = false;
                  table->m_BG_current_set = 1;
               }
               Selection cam2(Selection::SelectionType::S_CAMERA, is_live, 2);
               if (ImGui::Selectable("Full Single Screen", m_selection == cam2))
               {
                  m_selection = cam2;
                  m_useEditorCam = false;
                  table->m_BG_current_set = 2;
               }
               ImGui::TreePop();
            }
            if (ImGui::TreeNode("Materials"))
            {
               for (size_t t = 0; t < table->m_materials.size(); t++)
               {
                  Material *material = table->m_materials[t];
                  Selection sel(is_live, material);
                  if (ImGui::Selectable(material->m_szName.c_str(), m_selection == sel))
                     m_selection = sel;
               }
               ImGui::TreePop();
            }
            if (is_live && ImGui::TreeNode("Live Objects"))
            {
               for (size_t t = 0; t < table->m_vedit.size(); t++)
               {
                  ISelect *const psel = table->m_vedit[t]->GetISelect();
                  if (psel != nullptr && psel->m_layerName.empty())
                  {
                     Selection sel(is_live, table->m_vedit[t]);
                     if (ImGui::Selectable(table->m_vedit[t]->GetName(), m_selection == sel))
                        m_selection = sel;
                  }
               }
               for (size_t t = 0; t < m_player->m_vball.size(); t++)
               {
                  Selection sel(Selection::SelectionType::S_BALL, is_live, (int)t);
                  if (ImGui::Selectable("Ball #"s.append(std::to_string(m_player->m_vball[t]->m_id)).c_str(), m_selection == sel))
                     m_selection = sel;
               }
               ImGui::TreePop();
            }
            if (ImGui::TreeNode("Render Probes"))
            {
               for (RenderProbe *probe : table->m_vrenderprobe)
               {
                  Selection sel(is_live, probe);
                  if (ImGui::Selectable(probe->GetName().c_str(), m_selection == sel))
                     m_selection = sel;
               }
               ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("Layers", ImGuiTreeNodeFlags_DefaultOpen))
            {
               // Very very unefficient...
               robin_hood::unordered_map<std::string, vector<IEditable *>> layers;
               for (size_t t = 0; t < table->m_vedit.size(); t++)
               {
                  ISelect *const psel = table->m_vedit[t]->GetISelect();
                  if (psel != nullptr)
                  {
                     auto iter = layers.find(psel->m_layerName);
                     if (iter != layers.end())
                     {
                        iter->second.push_back(table->m_vedit[t]);
                     }
                     else
                     {
                        vector<IEditable *> list;
                        list.push_back(table->m_vedit[t]);
                        layers[psel->m_layerName] = list;
                     }
                  }
               }
               std::vector<std::string> keys;
               keys.reserve(layers.size());
               for (auto &it : layers)
                  keys.push_back(it.first);
               sort(keys.begin(), keys.end());
               for (auto &it : keys)
               {
                  if (it == ""s) // Skip editables without a layer (like live implicit playfield,...)
                     continue;
                  if (ImGui::TreeNode(it.c_str()))
                  {
                     auto list = layers[it];
                     for (size_t t = 0; t < list.size(); t++)
                     {
                        Selection sel(is_live, list[t]);
                        if (ImGui::Selectable(list[t]->GetName(), m_selection == sel))
                           m_selection = sel;
                     }
                     ImGui::TreePop();
                  }
               }
               ImGui::TreePop();
            }
            ImGui::EndTabItem();
         }
      }
      ImGui::EndTabBar();
   }
   m_outliner_width = ImGui::GetWindowWidth();
   ImGui::End();
   ImGui::PopStyleVar(3);
}

void LiveUI::UpdatePropertyUI()
{
   const ImGuiViewport *const viewport = ImGui::GetMainViewport();
   const float pane_width = 250.f * m_dpi;
   ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + viewport->Size.x - pane_width, viewport->Pos.y + m_menubar_height + m_toolbar_height));
   ImGui::SetNextWindowSize(ImVec2(pane_width, viewport->Size.y - m_menubar_height - m_toolbar_height));
   constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
      | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f * m_dpi, 4.0f * m_dpi));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
   ImGui::Begin("PROPERTIES", nullptr, window_flags);
   ImGui::PushItemWidth(PROP_WIDTH);

   if (ImGui::BeginTabBar("Startup/Live", ImGuiTabBarFlags_AutoSelectNewTabs))
   {
      for (int tab = 0; tab < 2; tab++)
      {
         const bool is_live = (tab == 1);
         if (ImGui::BeginTabItem(is_live ? "Live" : "Startup", nullptr))
         {
            ImGui::NewLine();
            switch (m_selection.type)
            {
            case Selection::SelectionType::S_NONE: TableProperties(is_live); break; // Use header tab for live since table is displayed when there si no selection
            case Selection::SelectionType::S_CAMERA: CameraProperties(is_live); break;
            case Selection::SelectionType::S_MATERIAL: MaterialProperties(is_live); break;
            case Selection::SelectionType::S_BALL: BallProperties(is_live); break;
            case Selection::SelectionType::S_RENDERPROBE: RenderProbeProperties(is_live); break;
            case Selection::SelectionType::S_EDITABLE:
            {
               const bool is_live_selected = m_selection.is_live;
               IEditable *live_obj = (IEditable *)(is_live_selected ? m_selection.editable : m_live_table->m_startupToLive[m_selection.editable]);
               IEditable *startup_obj = (IEditable *)(is_live_selected ? m_live_table->m_liveToStartup[m_selection.editable] : m_selection.editable);
               assert(live_obj == nullptr || std::find(m_live_table->m_vedit.begin(), m_live_table->m_vedit.end(), live_obj) != m_live_table->m_vedit.end());
               assert(startup_obj == nullptr || std::find(m_table->m_vedit.begin(), m_table->m_vedit.end(), startup_obj) != m_table->m_vedit.end());
               if ((is_live && live_obj == nullptr) || (!is_live && startup_obj == nullptr))
               {
                  HelpTextCentered("No Object");
               }
               else
               {
                  HelpEditableHeader(is_live, startup_obj, live_obj);
                  switch (m_selection.editable->GetItemType())
                  {
                  // eItemFlipper, eItemTimer, eItemPlunger, eItemTextbox, eItemBumper, eItemTrigger, eItemKicker, eItemDecal, eItemGate, eItemSpinner, eItemTable,
                  // eItemLightCenter, eItemDragPoint, eItemCollection, eItemDispReel, eItemLightSeq, eItemHitTarget,
                  case eItemFlasher: FlasherProperties(is_live, (Flasher *) startup_obj, (Flasher *)live_obj); break;
                  case eItemLight: LightProperties(is_live, (Light *)startup_obj, (Light *)live_obj); break;
                  case eItemPrimitive: PrimitiveProperties(is_live, (Primitive *)startup_obj, (Primitive *)live_obj); break;
                  case eItemSurface: SurfaceProperties(is_live, (Surface *)startup_obj, (Surface *)live_obj); break;
                  case eItemRamp: RampProperties(is_live, (Ramp *)startup_obj, (Ramp *)live_obj); break;
                  case eItemRubber: RubberProperties(is_live, (Rubber *)startup_obj, (Rubber *)live_obj); break;
                  }
               }
               break;
            }
            }
            ImGui::EndTabItem();
         }
      }
      ImGui::EndTabBar();
   }
   ImGui::PopItemWidth();
   m_properties_width = ImGui::GetWindowWidth();
   ImGui::End();
   ImGui::PopStyleVar(3);
}

void LiveUI::UpdateAudioOptionsModal()
{
   bool p_open = true;
   if (ImGui::BeginPopupModal(ID_AUDIO_SETTINGS, &p_open, ImGuiWindowFlags_AlwaysAutoResize))
   {
      ImGui::EndPopup();
   }
}

void LiveUI::UpdateVideoOptionsModal()
{
   bool p_open = true;
   if (ImGui::BeginPopupModal(ID_VIDEO_SETTINGS, &p_open, ImGuiWindowFlags_AlwaysAutoResize))
   {
      ImGui::Text("Global settings");
      ImGui::NewLine();
      if (ImGui::Checkbox("Force Bloom filter off", &m_player->m_bloomOff))
         SaveValueBool(regKey[m_player->m_stereo3D == STEREO_VR ? RegName::PlayerVR : RegName::Player], "ForceBloomOff"s, m_player->m_bloomOff);
      if (m_table->m_useFXAA == -1)
      {
         const char *postprocessed_aa_items[] = { "Disabled", "Fast FXAA", "Standard FXAA", "Quality FXAA", "Fast NFAA", "Standard DLAA", "Quality SMAA" };
         if (ImGui::Combo("Postprocessed AA", &m_player->m_FXAA, postprocessed_aa_items, IM_ARRAYSIZE(postprocessed_aa_items)))
            SaveValueInt(regKey[m_player->m_stereo3D == STEREO_VR ? RegName::PlayerVR : RegName::Player], "FXAA"s, m_player->m_FXAA);
      }
      const char *sharpen_items[] = { "Disabled", "CAS", "Bilateral CAS" };
      if (ImGui::Combo("Sharpen", &m_player->m_sharpen, sharpen_items, IM_ARRAYSIZE(sharpen_items)))
         SaveValueInt(regKey[m_player->m_stereo3D == STEREO_VR ? RegName::PlayerVR : RegName::Player], "Sharpen"s, m_player->m_sharpen);
      if (ImGui::Checkbox("Enable stereo rendering", &m_player->m_stereo3Denabled))
         SaveValueBool(regKey[RegName::Player], "Stereo3DEnabled"s, m_player->m_stereo3Denabled);
      ImGui::EndPopup();
   }
}

void LiveUI::UpdateHeadTrackingModal()
{
   BAMView::drawMenu();
}

void LiveUI::UpdateRendererInspectionModal()
{
   m_player->EnableStaticPrePass(!m_old_player_dynamic_mode);
   m_player->m_cameraMode = m_old_player_camera_mode;
   m_useEditorCam = false;
   m_pin3d->InitLayout(m_live_table->m_BG_enable_FSS, m_live_table->GetMaxSeparation());

   ImGui::SetNextWindowSize(ImVec2(550.f * m_dpi, 0));
   if (ImGui::Begin(ID_RENDERER_INSPECTION, &m_RendererInspection))
   {
      ImGui::Text("Display single render pass:");
      static int pass_selection = IF_FPS;
      ImGui::RadioButton("Disabled", &pass_selection, IF_FPS);
      #ifndef ENABLE_SDL // No GPU profiler for OpenGL
      ImGui::RadioButton("Profiler", &pass_selection, IF_PROFILING);
      ImGui::RadioButton("Profiler (Split rendering)", &pass_selection, IF_PROFILING_SPLIT_RENDERING);
      #endif
      ImGui::RadioButton("Static prerender pass", &pass_selection, IF_STATIC_ONLY);
      ImGui::RadioButton("Dynamic render pass", &pass_selection, IF_DYNAMIC_ONLY);
      ImGui::RadioButton("Transmitted light pass", &pass_selection, IF_LIGHT_BUFFER_ONLY);
      if (m_player->GetAOMode() != 0)
         ImGui::RadioButton("Ambient Occlusion pass", &pass_selection, IF_AO_ONLY);
      for (size_t i = 0; i < 2 * m_table->m_vrenderprobe.size(); i++)
      {
         string name = m_table->m_vrenderprobe[i >> 1]->GetName() + ((i & 1) == 0 ? " - Static pass" : " - Dynamic pass");
         ImGui::RadioButton(name.c_str(), &pass_selection, 100 + (int) i);
      }
      if (pass_selection < 100)
         m_player->m_infoMode = (InfoMode)pass_selection;
      else
      {
         m_player->m_infoMode = IF_RENDER_PROBES;
         m_player->m_infoProbeIndex = pass_selection - 100;
      }
      ImGui::NewLine();
      // Main frame timing table
      if (ImGui::BeginTable("Timings", 6, ImGuiTableFlags_Borders))
      {
         const U32 period = m_player->m_lastFrameDuration;
         ImGui::TableSetupColumn("##Cat", ImGuiTableColumnFlags_WidthFixed);
         ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed);
         ImGui::TableSetupColumn("Ratio", ImGuiTableColumnFlags_WidthFixed);
         ImGui::TableSetupColumn("Avg Time", ImGuiTableColumnFlags_WidthFixed);
         ImGui::TableSetupColumn("Avg Ratio", ImGuiTableColumnFlags_WidthFixed);
         ImGui::TableSetupColumn("Additional informations", ImGuiTableColumnFlags_WidthStretch);
         ImGui::TableHeadersRow();

         // Overall frame timing
         const float fpsAvg = m_player->m_avgFrameDuration == 0 ? 0.0f : ((float)((double)1000000.0 * m_player->m_avgFrameCount / m_player->m_avgFrameDuration));
         // info << "Overall: " << (float(1e-3 * m_max)) << "ms max (last second), " << (float(1e-3 * m_max_total)) << "ms max\n";
         ImGui::TableNextColumn();
         ImGui::Text("Frame");
         ImGui::TableNextColumn();
         ImGui::Text("%4.1fms", 1000.0f / (m_player->m_fps + 0.01f));
         ImGui::TableNextColumn();
         ImGui::Text("100.0%%");
         ImGui::TableNextColumn();
         ImGui::Text("%4.1fms", 1000.0f / (fpsAvg + 0.01f));
         ImGui::TableNextColumn();
         ImGui::Text("100.0%%");
         ImGui::TableNextColumn();
         ImGui::Text("FPS: %4.1f (%4.1f average)", m_player->m_fps, fpsAvg);
         ImGui::TableNextRow();
         // Renderer command collect timing
         ImGui::TableNextColumn();
         ImGui::Text("> Collect");
         ImGui::TableNextColumn();
         ImGui::Text("%4.1fms", 1e-3 * m_player->m_frame_collect);
         ImGui::TableNextColumn();
         ImGui::Text("%4.1f%%", m_player->m_frame_collect * 100.0 / period);
         ImGui::TableNextColumn();
         ImGui::Text("%4.1fms", 1e-3 * (double)m_player->m_frame_collect_total / (double)m_player->m_count);
         ImGui::TableNextColumn();
         ImGui::Text("%4.1f%%", (double)m_player->m_frame_collect_total * 100.0 / (double)m_player->m_total);
         ImGui::TableNextRow();
         // Renderer command submit timing
         ImGui::TableNextColumn();
         ImGui::Text("> Submit");
         ImGui::TableNextColumn();
         ImGui::Text("%4.1fms", 1e-3 * m_player->m_frame_submit);
         ImGui::TableNextColumn();
         ImGui::Text("%4.1f%%", m_player->m_frame_submit * 100.0 / period);
         ImGui::TableNextColumn();
         ImGui::Text("%4.1fms", 1e-3 * (double)m_player->m_frame_submit_total / (double)m_player->m_count);
         ImGui::TableNextColumn();
         ImGui::Text("%4.1f%%", (double)m_player->m_frame_submit_total * 100.0 / (double)m_player->m_total);
         ImGui::TableNextRow();
         // Renderer frame flip timing
         ImGui::TableNextColumn();
         ImGui::Text("> Flip");
         ImGui::TableNextColumn();
         ImGui::Text("%4.1fms", 1e-3 * m_player->m_frame_flip);
         ImGui::TableNextColumn();
         ImGui::Text("%4.1f%%", m_player->m_frame_flip * 100.0 / period);
         ImGui::TableNextColumn();
         ImGui::Text("%4.1fms", 1e-3 * (double)m_player->m_frame_flip_total / (double)m_player->m_count);
         ImGui::TableNextColumn();
         ImGui::Text("%4.1f%%", (double)m_player->m_frame_flip_total * 100.0 / (double)m_player->m_total);
         ImGui::TableNextRow();
         // Physics timing
         U32 frame_phys = m_player->m_phys_period - m_player->m_script_period;
         ImGui::TableNextColumn();
         ImGui::Text("> Physics");
         ImGui::TableNextColumn();
         ImGui::Text("%4.1fms", 1e-3 * frame_phys);
         ImGui::TableNextColumn();
         ImGui::Text("%4.1f%%", frame_phys * 100.0 / period);
         ImGui::TableNextColumn();
         ImGui::Text("%4.1fms", 1e-3 * (double) m_player->m_phys_total / (double) m_player->m_count);
         ImGui::TableNextColumn();
         ImGui::Text("%4.1f%%", (double)m_player->m_phys_total * 100.0 / (double)m_player->m_total);
         ImGui::TableNextColumn();
         ImGui::Text("Max: %4.1fms (over last second), %4.1fms", 1e-3 * m_player->m_phys_max, 1e-3 * m_player->m_phys_max_total);
         ImGui::TableNextRow();
         // Script timing
         ImGui::TableNextColumn();
         ImGui::Text("> Timers");
         ImGui::TableNextColumn();
         ImGui::Text("%4.1fms", 1e-3 * m_player->m_script_period);
         ImGui::TableNextColumn();
         ImGui::Text("%4.1f%%", m_player->m_script_period * 100.0 / period);
         ImGui::TableNextColumn();
         ImGui::Text("%4.1fms", 1e-3 * (double)m_player->m_script_total / (double)m_player->m_count);
         ImGui::TableNextColumn();
         ImGui::Text("%4.1f%%", (double)m_player->m_script_total * 100.0 / (double)m_player->m_total);
         ImGui::TableNextColumn();
         ImGui::Text("Max: %4.1fms (over last second), %4.1fms", 1e-3 * m_player->m_script_max, 1e-3 * m_player->m_script_max_total);
         ImGui::TableNextRow();

         ImGui::EndTable();

         ImGui::Text("Press F11 to reset average timings");
         if (ImGui::IsKeyPressed(dikToImGuiKeys[m_player->m_rgKeys[eFrameCount]]))
            m_player->InitFPS();
         ImGui::NewLine();
      }
      // Other detailled informations
      ImGui::Text(m_player->GetPerfInfo().c_str());
      
      ImGui::End();
   }
}

void LiveUI::UpdateMainSplashModal()
{
   const bool enableKeyboardShortcuts = (msec() - m_OpenUITime) > 250;

   ImGuiStyle &style = ImGui::GetStyle();
   // FIXME push style
   style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0, 0, 0, 0);

   // Display table name,author,version and blurb and description
   {
      std::ostringstream info;
      if (m_ShowUI) // Move below menu & toolbar
         info << "\n\n\n\n";
      if (!m_table->m_szTableName.empty())
         info << m_table->m_szTableName;
      else
         info << "Table";
      if (!m_table->m_szAuthor.empty())
         info << " by " << m_table->m_szAuthor;
      if (!m_table->m_szVersion.empty())
         info << " (" << m_table->m_szVersion << ")";
      //info << std::format(" ({:s} Revision {:d})\n", !m_table->m_szDateSaved.empty() ? m_table->m_szDateSaved : "N.A.", m_table->m_numTimesSaved);
      info << " (" << (!m_table->m_szDateSaved.empty() ? m_table->m_szDateSaved : "N.A.") << " Revision " << m_table->m_numTimesSaved << ")\n";
      const size_t line_length = info.str().size();
      info << std::string(line_length, '=') << "\n";
      if (!m_table->m_szBlurb.empty())
         info << m_table->m_szBlurb << std::string(line_length, '=') << "\n";
      if (!m_table->m_szDescription.empty())
         info << m_table->m_szDescription;
      constexpr ImGuiWindowFlags window_flags
         = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
      ImGui::SetNextWindowBgAlpha(0.5f);
      ImGui::SetNextWindowPos(ImVec2(0, 0));
      ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
      ImGui::Begin("Table Info", nullptr, window_flags);
      HelpTextCentered(info.str());
      ImGui::End();
   }

   constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
   if (ImGui::BeginPopupModal(ID_MODAL_SPLASH, nullptr, window_flags))
   {
      const ImVec2 size(100.f * m_dpi, 0);
      // Resume: click on the button, or press escape key
      if (ImGui::Button("Resume Game", size) || (enableKeyboardShortcuts && ((ImGui::IsKeyPressed(dikToImGuiKeys[m_player->m_rgKeys[eEscape]]) && !m_disable_esc))))
      {
         ExitEditMode();
         ImGui::CloseCurrentPopup();
         HideUI();
      }
      if (ImGui::Button("Live Editor", size))
      {
         m_ShowUI = true;
         m_ShowSplashModal = false;
         m_useEditorCam = true;
         ImGui::CloseCurrentPopup();
         EnterEditMode();
      }
      if (ImGui::Button("Debugger", size) || (enableKeyboardShortcuts && ImGui::IsKeyPressed(dikToImGuiKeys[m_player->m_rgKeys[eDebugger]])))
      {
         ExitEditMode();
         ImGui::CloseCurrentPopup();
         HideUI();
         m_player->m_showDebugger = true;
      }
      // Quit: click on the button, or press exit button
      if (ImGui::Button("Quit to editor", size) || (enableKeyboardShortcuts && ImGui::IsKeyPressed(dikToImGuiKeys[m_player->m_rgKeys[eExitGame]])))
      {
         ImGui::CloseCurrentPopup();
         HideUI();
         m_table->QuitPlayer(Player::CS_STOP_PLAY);
      }
      ImVec2 pos = ImGui::GetWindowPos();
      ImVec2 max = ImGui::GetWindowSize();
      bool hovered = ImGui::IsWindowHovered();
      ImGui::EndPopup();

      // Handle dragging mouse to allow dragging the main application window
      max.x += pos.x;
      max.y += pos.y;
      static ImVec2 initial_pos;
      if (m_player && !m_player->m_fullScreen && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
      {
         if (!hovered  && !(pos.x <= initial_pos.x && initial_pos.x <= max.x && pos.y <= initial_pos.y && initial_pos.y <= max.y))
         {

            ImVec2 drag = ImGui::GetMouseDragDelta();
            int x, y;
#ifdef ENABLE_SDL
            SDL_GetWindowPosition(m_player->m_sdl_playfieldHwnd, &x, &y);
            x += (int)drag.x;
            y += (int)drag.y;
            SDL_SetWindowPosition(m_player->m_sdl_playfieldHwnd, x, y);
#else
            auto rect = m_player->GetWindowRect();
            x = rect.left + (int)drag.x;
            y = rect.top + (int)drag.y;
            m_player->SetWindowPos(nullptr, x, y, m_player->m_wnd_width, m_player->m_wnd_height, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
#endif
            SaveValueInt((m_player->m_stereo3D == STEREO_VR) ? regKey[RegName::PlayerVR] : regKey[RegName::Player], "WindowPosX"s, x);
            SaveValueInt((m_player->m_stereo3D == STEREO_VR) ? regKey[RegName::PlayerVR] : regKey[RegName::Player], "WindowPosY"s, y);
         }
      }
      else
      {
         initial_pos = ImGui::GetMousePos();
      }
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Property panes
//

void LiveUI::TableProperties(bool is_live)
{
   HelpEditableHeader(is_live, m_table, m_live_table);
   if (ImGui::CollapsingHeader("User", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("Lights", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropRGB("Ambient Color", m_table, is_live, &(m_table->m_lightAmbient), m_live_table ? &(m_live_table->m_lightAmbient) : nullptr);
      PropSeparator();
      PropRGB("Light Em. Color", m_table, is_live, &(m_table->m_Light[0].emission), m_live_table ? &(m_live_table->m_Light[0].emission) : nullptr);
      auto reinit_lights = [this](bool is_live, float prev, float v) { m_pin3d->InitLights(); }; // Needed to update shaders with new light settings 
      PropFloat("Light Em. Scale", m_table, is_live, &(m_table->m_lightEmissionScale), m_live_table ? &(m_live_table->m_lightEmissionScale) : nullptr, 20000.0f, 100000.0f, "%.0f", ImGuiInputTextFlags_CharsDecimal, reinit_lights);
      PropFloat("Light Height", m_table, is_live, &(m_table->m_lightHeight), m_live_table ? &(m_live_table->m_lightHeight) : nullptr, 20.0f, 100.0f, "%.0f");
      PropFloat("Light Range", m_table, is_live, &(m_table->m_lightRange), m_live_table ? &(m_live_table->m_lightRange) : nullptr, 200.0f, 1000.0f, "%.0f");
      PropSeparator();
      // TODO Missing: environment texture combo
      auto upd_env_em_scale = [this](bool is_live, float prev, float v)
      {
         const vec4 st(v * m_player->m_globalEmissionScale,
            m_pin3d->m_envTexture ? (float)m_pin3d->m_envTexture->m_height /*+m_pin3d->m_envTexture->m_width)*0.5f*/
                                      : (float)m_pin3d->m_builtinEnvTexture.m_height /*+m_pin3d->m_builtinEnvTexture.m_width)*0.5f*/,
            0.f, 0.f);
         m_rd->basicShader->SetVector(SHADER_fenvEmissionScale_TexWidth, &st);
      };
      PropFloat("Environment Em. Scale", m_table, is_live, &(m_table->m_envEmissionScale), m_live_table ? &(m_live_table->m_envEmissionScale) : nullptr, 0.1f, 0.5f, "%.3f", ImGuiInputTextFlags_CharsDecimal, upd_env_em_scale);
      PropFloat("Ambient Occlusion Scale", m_table, is_live, &(m_table->m_AOScale), m_live_table ? &(m_live_table->m_AOScale) : nullptr, 0.1f, 1.0f);
      PropFloat("Bloom Strength", m_table, is_live, &(m_table->m_bloom_strength), m_live_table ? &(m_live_table->m_bloom_strength) : nullptr, 0.1f, 1.0f);
      PropFloat("Screen Space Reflection Scale", m_table, is_live, &(m_table->m_SSRScale), m_live_table ? &(m_live_table->m_SSRScale) : nullptr, 0.1f, 1.0f);
      ImGui::EndTable();
   }
}

void LiveUI::CameraProperties(bool is_live)
{
   switch (m_selection.camera)
   {
   case 0: ImGui::Text("Camera: Desktop"); break;
   case 1: ImGui::Text("Camera: Full Single Screen"); break;
   case 2: ImGui::Text("Camera: Cabinet"); break;
   default: return; // unsupported
   }
   ImGui::Separator();

   // FIXME buttons need to be updated between startup/live table
   if (ImGui::Button("Reset"))
   {
      const bool old_camera_mode = m_player->m_cameraMode;
      m_player->m_cameraMode = true;
      m_pininput->FireKeyEvent(DISPID_GameEvents_KeyDown, m_player->m_rgKeys[eStartGameKey]);
      m_player->m_cameraMode = old_camera_mode;
      m_pin3d->InitLights(); // Needed to update shaders with new light settings
      const vec4 st(m_table->m_envEmissionScale * m_player->m_globalEmissionScale,
         m_pin3d->m_envTexture ? (float)m_pin3d->m_envTexture->m_height /*+m_pin3d->m_envTexture->m_width)*0.5f*/
                               : (float)m_pin3d->m_builtinEnvTexture.m_height /*+m_pin3d->m_builtinEnvTexture.m_width)*0.5f*/,
         0.f, 0.f);
      m_rd->basicShader->SetVector(SHADER_fenvEmissionScale_TexWidth, &st);
   }
   ImGui::SameLine();
   if (ImGui::Button("Import"))
   {
      m_table->ImportBackdropPOV(string());
      m_pin3d->InitLights(); // Needed to update shaders with new light settings
      const vec4 st(m_table->m_envEmissionScale * m_player->m_globalEmissionScale,
         m_pin3d->m_envTexture ? (float)m_pin3d->m_envTexture->m_height /*+m_pin3d->m_envTexture->m_width)*0.5f*/
                               : (float)m_pin3d->m_builtinEnvTexture.m_height /*+m_pin3d->m_builtinEnvTexture.m_width)*0.5f*/,
         0.f, 0.f);
      m_rd->basicShader->SetVector(SHADER_fenvEmissionScale_TexWidth, &st);
   }
   ImGui::SameLine();
   if (ImGui::Button("Export"))
      m_table->ExportBackdropPOV(string());
   ImGui::NewLine();
   ImGui::Checkbox("Interactive camera mode", &m_player->m_cameraMode);
   ImGui::NewLine();
   if (BEGIN_PROP_TABLE)
   {
      for (int i = 0; i < 14; i++)
      {
         if (m_player->m_cameraMode && (i == m_player->m_backdropSettingActive || (m_player->m_backdropSettingActive == 3 && (i == 4 || i == 5))))
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
         switch (i)
         {
         case 0:
            PropFloat("Inclination", m_table, is_live, &(m_table->m_BG_inclination[m_selection.camera]), m_live_table ? &(m_live_table->m_BG_inclination[m_selection.camera]) : nullptr, 0.2f, 1.0f);
            break;
         case 1:
            PropFloat("Field Of View", m_table, is_live, &(m_table->m_BG_FOV[m_selection.camera]), m_live_table ? &(m_live_table->m_BG_FOV[m_selection.camera]) : nullptr, 0.2f, 1.0f);
            break;
         case 2:
            PropFloat("Layback", m_table, is_live, &(m_table->m_BG_layback[m_selection.camera]), m_live_table ? &(m_live_table->m_BG_layback[m_selection.camera]) : nullptr, 0.2f, 1.0f);
            break;
         case 4:
            PropFloat("X Scale", m_table, is_live, &(m_table->m_BG_scalex[m_selection.camera]), m_live_table ? &(m_live_table->m_BG_scalex[m_selection.camera]) : nullptr, 0.002f, 0.01f);
            break;
         case 5:
            PropFloat("Y Scale", m_table, is_live, &(m_table->m_BG_scaley[m_selection.camera]), m_live_table ? &(m_live_table->m_BG_scaley[m_selection.camera]) : nullptr, 0.002f, 0.01f);
            break;
         case 6:
            PropFloat("Z Scale", m_table, is_live, &(m_table->m_BG_scalez[m_selection.camera]), m_live_table ? &(m_live_table->m_BG_scalez[m_selection.camera]) : nullptr, 0.002f, 0.01f);
            break;
         case 7:
            PropFloat("X Offset", m_table, is_live, &(m_table->m_BG_xlatex[m_selection.camera]), m_live_table ? &(m_live_table->m_BG_xlatex[m_selection.camera]) : nullptr, 10.0f, 50.0f, "%.0f");
            break;
         case 8:
            PropFloat("Y Offset", m_table, is_live, &(m_table->m_BG_xlatey[m_selection.camera]), m_live_table ? &(m_live_table->m_BG_xlatey[m_selection.camera]) : nullptr, 10.0f, 50.0f, "%.0f");
            break;
         case 9:
            PropFloat("Z Offset", m_table, is_live, &(m_table->m_BG_xlatez[m_selection.camera]), m_live_table ? &(m_live_table->m_BG_xlatez[m_selection.camera]) : nullptr, 10.0f, 50.0f, "%.0f");
            break;
         }
         if (m_player->m_cameraMode && (i == m_player->m_backdropSettingActive || (m_player->m_backdropSettingActive == 3 && (i == 4 || i == 5))))
            ImGui::PopStyleColor();
      }
      PropFloat("Rotation", m_table, true, &(m_table->m_BG_rotation[m_selection.camera]), m_live_table ? &(m_live_table->m_BG_rotation[m_selection.camera]) : nullptr, 90.f, 90.0f, "%.0f");
      ImGui::EndTable();
   }
   ImGui::Separator();
   ImGui::Text("Absolute position:\nX: %.2f  Y: %.2f  Z: %.2f", -m_pin3d->GetMVP().GetView()._41,
      (m_selection.camera == 0 || m_selection.camera == 2) ? m_pin3d->GetMVP().GetView()._42 : -m_pin3d->GetMVP().GetView()._42, 
      m_pin3d->GetMVP().GetView()._43);
}

void LiveUI::RenderProbeProperties(bool is_live)
{
   RenderProbe *live_probe = (RenderProbe *)(m_selection.is_live ? m_selection.renderprobe : m_live_table->m_startupToLive[m_selection.renderprobe]);
   RenderProbe *startup_probe = (RenderProbe *)(m_selection.is_live ? m_live_table->m_liveToStartup[m_selection.renderprobe] : m_selection.renderprobe);
   HelpTextCentered("Render Probe");
   string name = ((RenderProbe *)m_selection.renderprobe)->GetName();
   ImGui::BeginDisabled(is_live); // Editing the name of a live item can break the script
   if (ImGui::InputText("Name", &name))
   {
      // FIXME add undo
      if (startup_probe)
         startup_probe->SetName(name);
   }
   ImGui::EndDisabled();
   ImGui::Separator();
   if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      const string types[] = { "Reflection"s, "Refraction"s };

      auto upd_normal = [startup_probe, live_probe](bool is_live, vec3& prev, vec3& v)
      {
         RenderProbe *probe = (is_live ? live_probe : startup_probe);
         if (probe)
         {
            vec4 plane;
            probe->GetReflectionPlane(plane);
            plane.x = v.x;
            plane.y = v.y;
            plane.z = v.z;
            probe->SetReflectionPlane(plane);
         }
      };
      vec4 startup_plane, live_plane;
      Vertex3Ds startup_normal, live_normal;
      if (startup_probe)
      {
         startup_probe->GetReflectionPlane(startup_plane);
         startup_normal = Vertex3Ds(startup_plane.x, startup_plane.y, startup_plane.z);
      }
      if (live_probe)
      {
         live_probe->GetReflectionPlane(live_plane);
         live_normal = Vertex3Ds(live_plane.x, live_plane.y, live_plane.z);
      }
      PropVec3("Normal", nullptr, is_live, startup_probe ? &startup_normal : nullptr, live_probe ? &live_normal : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal, upd_normal);

      auto upd_distance = [startup_probe, live_probe](bool is_live, float prev, float v)
      {
         RenderProbe *probe = (is_live ? live_probe : startup_probe);
         if (probe)
         {
            vec4 plane;
            probe->GetReflectionPlane(plane);
            plane.w = v;
            probe->SetReflectionPlane(plane);
         }
      };
      PropFloat("Distance", nullptr, is_live, startup_probe ? &startup_plane.w : nullptr, live_probe ? &live_plane.w : nullptr, 1.f, 10.f, "%.0f", ImGuiInputTextFlags_CharsDecimal, upd_distance);

      ImGui::EndTable();
   }
}

void LiveUI::BallProperties(bool is_live)
{
   if (!is_live)
      return;
   Ball *ball = m_player->m_vball[m_selection.ball_index];
   HelpTextCentered("Ball #"s.append(std::to_string(ball->m_id)));
   ImGui::Separator();
   if (ImGui::CollapsingHeader("Visual", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      auto upd_ball_tex = [this, ball](bool is_live, string prev, string v) { ball->m_pinballEnv = m_live_table->GetImage(ball->m_image); };
      auto upd_ball_decal = [this, ball](bool is_live, string prev, string v) { ball->m_pinballDecal = m_live_table->GetImage(ball->m_imageDecal); };
      PropCheckbox("Visible", nullptr, is_live, nullptr, ball ? &(ball->m_visible) : nullptr);
      PropRGB("Color", nullptr, is_live, nullptr, ball ? &(ball->m_color) : nullptr);
      PropImageCombo("Image", nullptr, is_live, nullptr, ball ? &(ball->m_image) : nullptr, m_live_table, upd_ball_tex);
      PropImageCombo("Decal", nullptr, is_live, nullptr, ball ? &(ball->m_imageDecal) : nullptr, m_live_table, upd_ball_decal);
      PropCheckbox("Decal mode", nullptr, is_live, nullptr, ball ? &(ball->m_decalMode) : nullptr);
      PropFloat("PF Reflection Strength", nullptr, is_live, nullptr, ball ? &(ball->m_playfieldReflectionStrength) : nullptr, 0.02f, 0.1f);
      PropFloat("Bulb Intensity Scale", nullptr, is_live, nullptr, ball ? &(ball->m_bulb_intensity_scale) : nullptr, 0.02f, 0.1f);
      PropCheckbox("Reflection enabled", nullptr, is_live, nullptr, ball ? &(ball->m_reflectionEnabled) : nullptr);
      PropCheckbox("Reflection forced", nullptr, is_live, nullptr, ball ? &(ball->m_forceReflection) : nullptr);
      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropVec3("Position", nullptr, is_live, nullptr, ball ? &(ball->m_d.m_pos) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);
      PropFloat("Radius", nullptr, is_live, nullptr, ball ? &(ball->m_d.m_radius) : nullptr, 0.02f, 0.1f);
      PropFloat("Mass", nullptr, is_live, nullptr, ball ? &(ball->m_d.m_mass) : nullptr, 0.02f, 0.1f);
      PropVec3("Velocity", nullptr, is_live, nullptr, ball ? &(ball->m_d.m_vel) : nullptr, "%.3f", ImGuiInputTextFlags_CharsDecimal);
      PropVec3("Angular Momentum", nullptr, is_live, nullptr, ball ? &(ball->m_angularmomentum) : nullptr, "%.3f", ImGuiInputTextFlags_CharsDecimal);
      ImGui::EndTable();
   }
}

void LiveUI::MaterialProperties(bool is_live)
{
   Material *live_material = (Material *)(m_selection.is_live ? m_selection.editable : m_live_table->m_startupToLive[m_selection.editable]);
   Material *startup_material = (Material *)(m_selection.is_live ? m_live_table->m_liveToStartup[m_selection.editable] : m_selection.editable);
   HelpTextCentered("Material");
   string name = ((Material *)m_selection.editable)->m_szName;
   ImGui::BeginDisabled(is_live); // Editing the name of a live item can break the script
   if (ImGui::InputText("Name", &name))
   {
      // FIXME add undo
      if (startup_material)
         startup_material->m_szName = name;
   }
   ImGui::EndDisabled();
   ImGui::Separator();
   if (ImGui::CollapsingHeader("Visual", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      // FIXME add MaterialType m_type;
      PropRGB("Base Color", m_table, is_live, startup_material ? &(startup_material->m_cBase) : nullptr, live_material ? &(live_material->m_cBase) : nullptr);
      PropFloat("Wrap Lighting", m_table, is_live, startup_material ? &(startup_material->m_fWrapLighting) : nullptr, live_material ? &(live_material->m_fWrapLighting) : nullptr, 0.02f, 0.1f);
      PropRGB("Glossy Color", m_table, is_live, startup_material ? &(startup_material->m_cGlossy) : nullptr, live_material ? &(live_material->m_cGlossy) : nullptr);
      PropFloat("Glossy Image Lerp", m_table, is_live, startup_material ? &(startup_material->m_fGlossyImageLerp) : nullptr, live_material ? &(live_material->m_fGlossyImageLerp) : nullptr, 0.02f, 0.1f);
      PropFloat("Shininess", m_table, is_live, startup_material ? &(startup_material->m_fRoughness) : nullptr, live_material ? &(live_material->m_fRoughness) : nullptr, 0.02f, 0.1f);
      PropRGB("Clearcoat Color", m_table, is_live, startup_material ? &(startup_material->m_cClearcoat) : nullptr, live_material ? &(live_material->m_cClearcoat) : nullptr);
      PropFloat("Edge Brightness", m_table, is_live, startup_material ? &(startup_material->m_fEdge) : nullptr, live_material ? &(live_material->m_fEdge) : nullptr, 0.02f, 0.1f);
      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("Transparency", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropCheckbox("Enable Transparency", m_table, is_live, startup_material ? &(startup_material->m_bOpacityActive) : nullptr, live_material ? &(live_material->m_bOpacityActive) : nullptr);
      PropFloat("Opacity", m_table, is_live, startup_material ? &(startup_material->m_fOpacity) : nullptr, live_material ? &(live_material->m_fOpacity) : nullptr, 0.02f, 0.1f);
      PropFloat("Edge Opacity", m_table, is_live, startup_material ? &(startup_material->m_fEdgeAlpha) : nullptr, live_material ? &(live_material->m_fEdgeAlpha) : nullptr, 0.02f, 0.1f);
      PropFloat("Thickness", m_table, is_live, startup_material ? &(startup_material->m_fThickness) : nullptr, live_material ? &(live_material->m_fThickness) : nullptr, 0.02f, 0.1f);
      PropRGB("Refraction Tint", m_table, is_live, startup_material ? &(startup_material->m_cRefractionTint) : nullptr, live_material ? &(live_material->m_cRefractionTint) : nullptr);
      ImGui::EndTable();
   }
}

void LiveUI::FlasherProperties(bool is_live, Flasher *startup_obj, Flasher *live_obj) {

}

void LiveUI::LightProperties(bool is_live, Light *startup_light, Light *live_light)
{
   if (ImGui::CollapsingHeader("Visual", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      auto upd_intensity = [startup_light, live_light](bool is_live, float prev, float v)
      {
         Light *light = (is_live ? live_light : startup_light);
         if (prev > 0.1f && v > 0.1f)
         {
            const float fade_up_ms = prev / light->m_d.m_fadeSpeedUp;
            light->m_d.m_fadeSpeedUp = fade_up_ms < 0.1f ? 100000.0f : v / fade_up_ms;
            const float fade_down_ms = prev / light->m_d.m_fadeSpeedDown;
            light->m_d.m_fadeSpeedDown = fade_down_ms < 0.1f ? 100000.0f : v / fade_down_ms;
         }
         startup_light->m_currentIntensity = startup_light->m_d.m_intensity * startup_light->m_d.m_intensity_scale * startup_light->m_inPlayState;
         live_light->m_currentIntensity = live_light->m_d.m_intensity * live_light->m_d.m_intensity_scale * live_light->m_inPlayState;
      };
      float startup_fadeup = startup_light ? (startup_light->m_d.m_intensity / startup_light->m_d.m_fadeSpeedUp) : 0.f;
      float live_fadeup = live_light ? (live_light->m_d.m_intensity / live_light->m_d.m_fadeSpeedUp) : 0.f;
      auto upd_fade_up = [startup_light, live_light](bool is_live, float prev, float v)
      { 
         Light *light = (is_live ? live_light : startup_light);
         light->m_d.m_fadeSpeedUp = v < 0.1f ? 100000.0f : light->m_d.m_intensity / v; 
      };
      float startup_fadedown = startup_light ? (startup_light->m_d.m_intensity / startup_light->m_d.m_fadeSpeedDown) : 0.f;
      float live_fadedown = live_light ? (live_light->m_d.m_intensity / live_light->m_d.m_fadeSpeedDown) : 0.f;
      auto upd_fade_down = [startup_light, live_light](bool is_live, float prev, float v)
      {
         Light *light = (is_live ? live_light : startup_light);
         light->m_d.m_fadeSpeedDown = v < 0.1f ? 100000.0f : light->m_d.m_intensity / v;
      };
      bool startup_shadow = startup_light ? (startup_light->m_d.m_shadows == ShadowMode::RAYTRACED_BALL_SHADOWS) : ShadowMode::NONE;
      bool live_shadow = live_light ? (live_light->m_d.m_shadows == ShadowMode::RAYTRACED_BALL_SHADOWS) : ShadowMode::NONE;
      auto upd_shadow = [startup_light, live_light](bool is_live, bool prev, bool v)
      {
         Light *light = (is_live ? live_light : startup_light);
         light->m_d.m_shadows = v ? ShadowMode::RAYTRACED_BALL_SHADOWS : ShadowMode::NONE;
      };
      PropSeparator("Light Settings");
      PropFloat("Intensity", startup_light, is_live, startup_light ? &(startup_light->m_d.m_intensity) : nullptr, live_light ? &(live_light->m_d.m_intensity) : nullptr, 0.1f, 1.0f, "%.1f", ImGuiInputTextFlags_CharsDecimal, upd_intensity);
      const string faders[] = { "None"s, "Linear"s, "Incandescent"s };
      PropCombo("Fader", startup_light, is_live, startup_light ? (int *)&(startup_light->m_d.m_fader) : nullptr, live_light ? (int *)&(live_light->m_d.m_fader) : nullptr, 3, faders);
      PropFloat("Fade Up (ms)", startup_light, is_live, startup_light ? &startup_fadeup : nullptr, live_light ? &live_fadeup : nullptr, 10.0f, 50.0f, "%.0f", ImGuiInputTextFlags_CharsDecimal, upd_fade_up);
      PropFloat("Fade Down (ms)", startup_light, is_live, startup_light ? &startup_fadedown : nullptr, live_light ? &live_fadedown : nullptr, 10.0f, 50.0f, "%.0f", ImGuiInputTextFlags_CharsDecimal, upd_fade_down);
      PropRGB("Light Color", startup_light, is_live, startup_light ? &(startup_light->m_d.m_color) : nullptr, live_light ? &(live_light->m_d.m_color) : nullptr);
      PropRGB("Center Burst", startup_light, is_live, startup_light ? &(startup_light->m_d.m_color2) : nullptr, live_light ? &(live_light->m_d.m_color2) : nullptr);
      PropFloat("Falloff Range", startup_light, is_live, startup_light ? &(startup_light->m_d.m_falloff) : nullptr, live_light ? &(live_light->m_d.m_falloff) : nullptr, 10.f, 100.f, "%.0f");
      PropFloat("Falloff Power", startup_light, is_live, startup_light ? &(startup_light->m_d.m_falloff_power) : nullptr, live_light ? &(live_light->m_d.m_falloff_power) : nullptr, 0.1f, 0.5f, "%.2f");
      PropSeparator("Render Mode");
      // Missing render mode properties
      PropSeparator("Bulb");
      PropCheckbox("Render bulb", startup_light, is_live, startup_light ? &(startup_light->m_d.m_showBulbMesh) : nullptr, live_light ? &(live_light->m_d.m_showBulbMesh) : nullptr);
      PropCheckbox("Static rendering", startup_light, is_live, startup_light ? &(startup_light->m_d.m_staticBulbMesh) : nullptr, live_light ? &(live_light->m_d.m_staticBulbMesh) : nullptr);
      PropFloat("Bulb Size", startup_light, is_live, startup_light ? &(startup_light->m_d.m_meshRadius) : nullptr, live_light ? &(live_light->m_d.m_meshRadius) : nullptr, 1.0f, 5.0f, "%.0f");
      PropSeparator("Ball reflections & Shadows");
      PropCheckbox("Show Reflection on Balls", startup_light, is_live, startup_light ? &(startup_light->m_d.m_showReflectionOnBall) : nullptr, live_light ? &(live_light->m_d.m_showReflectionOnBall) : nullptr);
      PropCheckbox("Raytraced ball shadows", startup_light, is_live, startup_light ? &startup_shadow : nullptr, live_light ? &live_shadow : nullptr, upd_shadow);
      PropSeparator("Position");
      PropFloat("X", startup_light, is_live, startup_light ? &(startup_light->m_d.m_vCenter.x) : nullptr, live_light ? &(live_light->m_d.m_vCenter.x) : nullptr, 0.1f, 0.5f, "%.1f");
      PropFloat("Y", startup_light, is_live, startup_light ? &(startup_light->m_d.m_vCenter.y) : nullptr, live_light ? &(live_light->m_d.m_vCenter.y) : nullptr, 0.1f, 0.5f, "%.1f");
      PropFloat("Z", startup_light, is_live, startup_light ? &(startup_light->m_d.m_height) : nullptr, live_light ? &(live_light->m_d.m_height) : nullptr, 0.1f, 0.5f, "%.1f");

      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("States", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      auto upd_inplaystate = [startup_light, live_light](bool is_live, float prev, float v)
      {
         Light *light = (is_live ? live_light : startup_light);
         light->setInPlayState(v > 1.f ? (float)LightStateBlinking : v);
      };
      PropFloat("State", startup_light, is_live, startup_light ? &(startup_light->m_d.m_state) : nullptr, live_light ? &(live_light->m_d.m_state) : nullptr, 0.1f, 0.5f, "%.1f", ImGuiInputTextFlags_CharsDecimal, upd_inplaystate);
      // Missing blink pattern
      PropInt("Blink interval", startup_light, is_live, startup_light ? &(startup_light->m_d.m_blinkinterval) : nullptr, live_light ? &(live_light->m_d.m_blinkinterval) : nullptr);
      ImGui::EndTable();
   }
   PROP_TIMER(is_live, startup_light, live_light)
   if (is_live && ImGui::CollapsingHeader("Live state", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PROP_TABLE_SETUP
      ImGui::BeginDisabled();
      ImGui::TableNextColumn();
      ImGui::InputFloat("Intensity", &live_light->m_currentIntensity);
      ImGui::TableNextColumn();
      ImGui::Button(ICON_SAVE "##t2");
      if (live_light->m_d.m_fader == FADER_INCANDESCENT)
      {
         ImGui::TableNextColumn();
         float temperature = (float)live_light->m_currentFilamentTemperature;
         ImGui::InputFloat("Filament Temperature", &temperature);
         ImGui::TableNextColumn();
         ImGui::Button(ICON_SAVE "##t1");
      }
      ImGui::EndDisabled();
      ImGui::EndTable();
   }
}

void LiveUI::PrimitiveProperties(bool is_live, Primitive *startup_obj, Primitive *live_obj)
{
   if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropSeparator("Render Options");
      PropCheckbox("Static Rendering", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_staticRendering) : nullptr, live_obj ? &(live_obj->m_d.m_staticRendering) : nullptr);
      PropCheckbox("Visible", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_visible) : nullptr, live_obj ? &(live_obj->m_d.m_visible) : nullptr);
      PropCheckbox("Reflection Enabled", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_reflectionEnabled) : nullptr, live_obj ? &(live_obj->m_d.m_reflectionEnabled) : nullptr);
      PropFloat("Depth Bias", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_depthBias) : nullptr, live_obj ? &(live_obj->m_d.m_depthBias) : nullptr, 10.f, 50.f, "%.0f");
      PropCheckbox("Render Backfaces", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_backfacesEnabled) : nullptr, live_obj ? &(live_obj->m_d.m_backfacesEnabled) : nullptr);
      PropCheckbox("Additive Blend", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_addBlend) : nullptr, live_obj ? &(live_obj->m_d.m_addBlend) : nullptr);
      PropSeparator("Material");
      PropFloat("Disable Light", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_disableLightingTop) : nullptr, live_obj ? &(live_obj->m_d.m_disableLightingTop) : nullptr, 0.01f, 0.05f, "%.3f");
      PropFloat("Disable Light from below", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_disableLightingBelow) : nullptr, live_obj ? &(live_obj->m_d.m_disableLightingBelow) : nullptr, 0.01f, 0.05f, "%.3f");
      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("Position", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropSeparator("Position, Rotation & Size");
      PropVec3("Position", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_vPosition) : nullptr, live_obj ? &(live_obj->m_d.m_vPosition) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);
      PropVec3("Orientation", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_aRotAndTra[0]) : nullptr, live_obj ? &(live_obj->m_d.m_aRotAndTra[0]) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);
      PropVec3("Scale", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_vSize) : nullptr, live_obj ? &(live_obj->m_d.m_vSize) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);
      PropSeparator("Additional Transform");
      PropVec3("Translation", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_aRotAndTra[3]) : nullptr, live_obj ? &(live_obj->m_d.m_aRotAndTra[3]) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);
      PropVec3("Rotation", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_aRotAndTra[6]) : nullptr, live_obj ? &(live_obj->m_d.m_aRotAndTra[6]) : nullptr, "%.0f", ImGuiInputTextFlags_CharsDecimal);
      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen))
   {
   }
}

void LiveUI::RampProperties(bool is_live, Ramp *startup_obj, Ramp *live_obj)
{
}

void LiveUI::RubberProperties(bool is_live, Rubber *startup_obj, Rubber *live_obj)
{
   if (ImGui::CollapsingHeader("Visuals", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      PropCheckbox("Static Rendering", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_staticRendering) : nullptr, live_obj ? &(live_obj->m_d.m_staticRendering) : nullptr);
      PropCheckbox("Visible", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_visible) : nullptr, live_obj ? &(live_obj->m_d.m_visible) : nullptr);
      PropCheckbox("Reflection Enabled", startup_obj, is_live, startup_obj ? &(startup_obj->m_d.m_reflectionEnabled) : nullptr, live_obj ? &(live_obj->m_d.m_reflectionEnabled) : nullptr);
      ImGui::EndTable();
   }
   if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen) && BEGIN_PROP_TABLE)
   {
      ImGui::EndTable();
   }
   PROP_TIMER(is_live, startup_obj, live_obj)
}

void LiveUI::SurfaceProperties(bool is_live, Surface *startup_obj, Surface *live_obj)
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Property field helpers
//

#define PROP_HELPER_BEGIN(type)                                                                                                                                                              \
   PROP_TABLE_SETUP                                                                                                                                                                          \
   type *v = is_live ? live_v : startup_v;                                                                                                                                                   \
   type *ov = is_live ? startup_v : live_v;                                                                                                                                                  \
   ImGui::TableNextColumn();                                                                                                                                                                 \
   if (v == nullptr)                                                                                                                                                                         \
   {                                                                                                                                                                                         \
      /* Missing value just skip */                                                                                                                                                          \
      ImGui::TableNextColumn();                                                                                                                                                              \
      return;                                                                                                                                                                                \
   }                                                                                                                                                                                         \
   ImGui::PushID(label); \
   type prev_v = *v;

#define PROP_HELPER_SYNC(type)                                                                                                                                                               \
   /* Sync button(also show if there are difference between live and startup through the enable state) */                                                                                    \
   ImGui::TableNextColumn();                                                                                                                                                                 \
   if (ov != nullptr)                                                                                                                                                                        \
   {                                                                                                                                                                                         \
      const bool synced = ((*ov) == (*v));                                                                                                                                                   \
      if (synced)                                                                                                                                                                            \
         ImGui::BeginDisabled(); \
      type prev_ov = *ov; \
      if (ImGui::Button(ICON_SAVE)) \
      { \
         *ov = *v; \

/* 
TODO update undo stack instead of SetNonUndoableDirty
psel->GetIEditable()->BeginUndo();
psel->GetIEditable()->MarkForUndo();
// Change value
psel->GetIEditable()->EndUndo();
psel->GetIEditable()->SetDirtyDraw();
*/

#define PROP_HELPER_END                                                                                                                                                                      \
      if (is_live) \
         m_table->SetNonUndoableDirty(eSaveDirty); \
   }                                                                                                                                                                                         \
   if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))                                                                                                                                   \
   {                                                                                                                                                                                         \
      ImGui::BeginTooltip();                                                                                                                                                                 \
      ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);                                                                                                                                  \
      ImGui::Text("Copy this value to the %s version", is_live ? "startup" : "live");                                                                                                        \
      ImGui::PopTextWrapPos();                                                                                                                                                               \
      ImGui::EndTooltip();                                                                                                                                                                   \
   }                                                                                                                                                                                         \
   if (synced)                                                                                                                                                                               \
      ImGui::EndDisabled();                                                                                                                                                                  \
   }                                                                                                                                                                                         \
   ImGui::PopID();

void LiveUI::PropSeparator(const char *label)
{
   PROP_TABLE_SETUP
   ImGui::TableNextColumn();
   if (label)
      ImGui::Text(label);
   ImGui::TableNextColumn();
}

void LiveUI::PropCheckbox(const char *label, IEditable *undo_obj, bool is_live, bool *startup_v, bool *live_v, OnBoolPropChange chg_callback)
{
   PROP_HELPER_BEGIN(bool)
   if (ImGui::Checkbox(label, v))
   {
      if (chg_callback)
         chg_callback(is_live, prev_v, *v);
      if (!is_live)
         m_table->SetNonUndoableDirty(eSaveDirty);
   }
   PROP_HELPER_SYNC(bool)
   if (chg_callback)
      chg_callback(!is_live, prev_ov, *ov);
   PROP_HELPER_END
}

void LiveUI::PropFloat(const char *label, IEditable* undo_obj, bool is_live, float *startup_v, float *live_v, float step, float step_fast, const char *format, ImGuiInputTextFlags flags, OnFloatPropChange chg_callback)
{
   PROP_HELPER_BEGIN(float)
   if (ImGui::InputFloat(label, v, step, step_fast, format, flags))
   {
      if (chg_callback)
         chg_callback(is_live, prev_v, *v);
      if (!is_live)
         m_table->SetNonUndoableDirty(eSaveDirty);
   }
   PROP_HELPER_SYNC(float)
   if (chg_callback)
      chg_callback(!is_live, prev_ov, *ov);
   PROP_HELPER_END
}

void LiveUI::PropInt(const char *label, IEditable *undo_obj, bool is_live, int *startup_v, int *live_v)
{
   PROP_HELPER_BEGIN(int)
   if (ImGui::InputInt(label, v))
   {
      if (!is_live)
         m_table->SetNonUndoableDirty(eSaveDirty);
   }
   PROP_HELPER_SYNC(int)
   PROP_HELPER_END
}

void LiveUI::PropRGB(const char *label, IEditable *undo_obj, bool is_live, COLORREF *startup_v, COLORREF *live_v, ImGuiColorEditFlags flags)
{
   PROP_HELPER_BEGIN(COLORREF)
   float col[3];
   col[0] = (float)((*v) & 255) * (float)(1.0 / 255.0);
   col[1] = (float)((*v) & 65280) * (float)(1.0 / 65280.0);
   col[2] = (float)((*v) & 16711680) * (float)(1.0 / 16711680.0);
   if (ImGui::ColorEdit3(label, col, flags))
   {
      const int r = clamp((int)(col[0] * 255.f + 0.5f), 0, 255);
      const int g = clamp((int)(col[1] * 255.f + 0.5f), 0, 255);
      const int b = clamp((int)(col[2] * 255.f + 0.5f), 0, 255);
      *v = RGB(r, g, b);
      if (!is_live)
         m_table->SetNonUndoableDirty(eSaveDirty);
   }
   PROP_HELPER_SYNC(COLORREF)
   PROP_HELPER_END
}

void LiveUI::PropVec3(const char *label, IEditable *undo_obj, bool is_live, float *startup_v, float *live_v, const char *format, ImGuiInputTextFlags flags, OnVec3PropChange chg_callback)
{
   PROP_HELPER_BEGIN(float)
   vec3 v1(v[0], v[1], v[2]);
   if (ImGui::InputFloat3(label, v, format, flags))
   {
      if (chg_callback)
      {
         vec3 v2(v[0], v[1], v[2]);
         chg_callback(is_live, v1, v2);
      }
      if (!is_live)
         m_table->SetNonUndoableDirty(eSaveDirty);
   }
   PROP_HELPER_SYNC(float)
   PROP_HELPER_END
}

void LiveUI::PropVec3(const char *label, IEditable *undo_obj, bool is_live, Vertex3Ds *startup_v, Vertex3Ds *live_v, const char *format, ImGuiInputTextFlags flags, OnVec3PropChange chg_callback)
{
   PROP_HELPER_BEGIN(Vertex3Ds)
   float col[3] = { v->x, v->y, v->z };
   if (ImGui::InputFloat3(label, col, format, flags))
   {
      v->x = col[0];
      v->y = col[1];
      v->z = col[2];
      if (chg_callback)
      {
         vec3 v1(prev_v.x, prev_v.y, prev_v.z), v2(v->x, v->y, v->z);
         chg_callback(is_live, v1, v2);
      }
      if (!is_live)
         m_table->SetNonUndoableDirty(eSaveDirty);
   }
   PROP_HELPER_SYNC(Vertex3Ds)
   PROP_HELPER_END
}

void LiveUI::PropCombo(const char* label, IEditable* undo_obj, bool is_live, int* startup_v, int* live_v, int n_values, const string labels[])
{
   PROP_HELPER_BEGIN(int)
   const char *preview_value = labels[*v].c_str();
   if (ImGui::BeginCombo(label, preview_value))
   {
      for (int i = 0; i < n_values; i++)
      {
         if (ImGui::Selectable(labels[i].c_str()))
         {
            *v = i;
            if (!is_live)
               m_table->SetNonUndoableDirty(eSaveDirty);
         }
      }
      ImGui::EndCombo();
   }
   PROP_HELPER_SYNC(int)
   PROP_HELPER_END
}

void LiveUI::PropImageCombo(const char *label, IEditable *undo_obj, bool is_live, string *startup_v, string *live_v, PinTable *table, OnStringPropChange chg_callback)
{
   PROP_HELPER_BEGIN(string)
   const char *preview_value = (*v).c_str();
   if (ImGui::BeginCombo(label, preview_value))
   {
      for (size_t i = 0; i < table->m_vimage.size(); i++)
      {
         if (ImGui::Selectable(table->m_vimage[i]->m_szName.c_str()))
         {
            *v = table->m_vimage[i]->m_szName;
            if (chg_callback)
               chg_callback(is_live, prev_v, *v);
            if (!is_live)
               m_table->SetNonUndoableDirty(eSaveDirty);
         }
      }
      ImGui::EndCombo();
   }
   PROP_HELPER_SYNC(string)
   if (chg_callback)
      chg_callback(!is_live, prev_ov, *ov);
   PROP_HELPER_END
}

#undef PROP_HELPER_BEGIN
#undef PROP_HELPER_SYNC
#undef PROP_HELPER_END
