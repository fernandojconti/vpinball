#pragma once

#include "stdafx.h"

#include "imgui/imgui.h"
#include "imgui/ImGuizmo.h"

class LiveUI
{
public:
   LiveUI(RenderDevice* const rd);
   ~LiveUI();
   void Update();
   void Render();
   bool HasKeyboardCapture() const;
   bool HasMouseCapture() const;

   bool IsOpened() const { return m_ShowUI || m_ShowSplashModal; }
   void OpenMainUI();
   void ToggleFPS();

private:
   // Interactive Camera Mode
   void UpdateCameraModeUI();

   // Main UI frame & panels
   void UpdateMainUI();
   void UpdateOutlinerUI();
   void UpdatePropertyUI();

   // Pop ups & Modals
   void UpdateMainSplashModal();
   void UpdateAudioOptionsModal();
   void UpdateVideoOptionsModal();
   void UpdateRendererInspectionModal();
   void UpdateHeadTrackingModal();

   // UI Selection & properties
   void RenderProbeProperties(bool is_live);
   void BallProperties(bool is_live);
   void CameraProperties(bool is_live);
   void MaterialProperties(bool is_live);
   void TableProperties(bool is_live);
   void FlasherProperties(bool is_live, Flasher *startup_obj, Flasher *live_obj);
   void LightProperties(bool is_live, Light *startup_obj, Light *live_obj);
   void PrimitiveProperties(bool is_live, Primitive *startup_obj, Primitive *live_obj);
   void RampProperties(bool is_live, Ramp *startup_obj, Ramp *live_obj);
   void RubberProperties(bool is_live, Rubber *startup_obj, Rubber *live_obj);
   void SurfaceProperties(bool is_live, Surface *startup_obj, Surface *live_obj);

   // Helpers for property edition
   typedef std::function<void(bool is_live, float prev, float v)> OnFloatPropChange;
   typedef std::function<void(bool is_live, int prev, int v)> OnIntPropChange;
   typedef std::function<void(bool is_live, bool prev, bool v)> OnBoolPropChange;
   typedef std::function<void(bool is_live, string prev, string v)> OnStringPropChange;
   typedef std::function<void(bool is_live, vec3& prev, vec3& v)> OnVec3PropChange;
   void PropSeparator(const char *label = nullptr);
   void PropCheckbox(const char *label, IEditable *undo_obj, bool is_live, bool *startup_v, bool *live_v, OnBoolPropChange chg_callback = nullptr);
   void PropFloat(const char *label, IEditable* undo_obj, bool is_live, float *startup_v, float *live_v, float step, float step_fast, const char *format = "%.3f", ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsDecimal, OnFloatPropChange chg_callback = nullptr);
   void PropInt(const char *label, IEditable *undo_obj, bool is_live, int *startup_v, int *live_v);
   void PropRGB(const char *label, IEditable *undo_obj, bool is_live, COLORREF *startup_v, COLORREF *live_v, ImGuiColorEditFlags flags = 0);
   void PropVec3(const char *label, IEditable* undo_obj, bool is_live, Vertex3Ds *startup_v, Vertex3Ds *live_v, const char *format = "%.3f", ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsDecimal, OnVec3PropChange chg_callback = nullptr);
   void PropVec3(const char *label, IEditable *undo_obj, bool is_live, float *startup_v, float *live_v, const char *format = "%.3f", ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsDecimal, OnVec3PropChange chg_callback = nullptr);
   void PropCombo(const char *label, IEditable *undo_obj, bool is_live, int *startup_v, int *live_v, int n_values, const string labels[]);
   void PropImageCombo(const char *label, IEditable *undo_obj, bool is_live, string *startup_v, string *live_v, PinTable *table, OnStringPropChange chg_callback = nullptr);

   // Enter/Exit edit mode (manage table backup, dynamic mode,...)
   void HideUI();
   void PausePlayer(bool pause);
   void EnterEditMode();
   void ExitEditMode();

   // UI Context
   VPinball *m_app;
   Player *m_player;
   PinTable *m_table; // The edited table
   PinTable *m_live_table; // The live copy of the edited table being played by the player (all properties can be changed at any time by the script)
   PinInput *m_pininput;
   Pin3D *m_pin3d;
   struct Selection
   {
      enum SelectionType { S_NONE, S_CAMERA, S_MATERIAL, S_BALL, S_EDITABLE, S_RENDERPROBE } type = S_NONE;
      bool is_live;
      union
      {
         int camera;
         IEditable* editable;
         Material *material;
         RenderProbe *renderprobe;
         int ball_index;
      };
      Selection() {};
      Selection(SelectionType t, bool live, int ball) { type = t; is_live = live; ball_index = ball; };
      Selection(bool live, IEditable *data) { type = S_EDITABLE; is_live = live; editable = data; };
      Selection(bool live, Material *data) { type = S_MATERIAL; is_live = live; material = data; };
      Selection(bool live, RenderProbe *data) { type = S_RENDERPROBE; is_live = live; renderprobe = data; };
      bool operator==(Selection s)
      {
         if (type != s.type || is_live != s.is_live)
            return false;
         switch (type)
         {
         case S_NONE: return true;
         case S_CAMERA: return camera == s.camera;
         case S_MATERIAL: return material == s.material;
         case S_BALL: return ball_index == s.ball_index;
         case S_EDITABLE: return editable == s.editable;
         case S_RENDERPROBE: return renderprobe == s.renderprobe;
         }
         assert(false);
         return false;
      };
   } m_selection;

   // Rendering
   RenderDevice* const m_rd;
   int m_rotate = 0;
   float m_dpi = 1.0f;
   ImFont *m_baseFont = nullptr;
   ImFont *m_overlayFont = nullptr;
   float m_menubar_height = 0.0f;
   float m_toolbar_height = 0.0f;
   float m_outliner_width = 0.0f;
   float m_properties_width = 0.0f;
   bool m_old_player_dynamic_mode;
   bool m_old_player_camera_mode;

   // UI state
   bool m_ShowUI = false;
   bool m_ShowSplashModal = false;
   bool m_RendererInspection = false;
   bool m_disable_esc = false; // Option for keyboard shortcuts
   U32 m_OpenUITime = 0; // Used to delay keyboard shortcut
   U64 m_StartTime_usec = 0; // Used for timed splash overlays
   int m_show_fps = 0; // 0=disabled / 1=FPS / 2=FPS+dynamic plot

   // 3D editor
   ImGuizmo::OPERATION m_gizmoOperation = ImGuizmo::NONE;
   ImGuizmo::MODE m_gizmoMode = ImGuizmo::WORLD;
   bool GetSelectionTransform(Matrix3D& transform);
   void SetSelectionTransform(Matrix3D& transform, bool clearPosition = false, bool clearScale = false, bool clearRotation = false);

   // Editor camera
   bool m_useEditorCam = false;
   bool m_orthoCam = true;
   Matrix3D m_camView, m_camProj;
   float m_camDistance;
};
