#pragma once

#include "inc/robin_hood.h"
#include "typedefs3D.h"

#include "Material.h"
#include "Texture.h"
#include "Sampler.h"
#include "RenderTarget.h"
#include "RenderState.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "MeshBuffer.h"
#include "TextureManager.h"
#include "RenderFrame.h"
#include "RenderPass.h"

#ifdef ENABLE_VR
#include <openvr.h>
#endif

#ifndef ENABLE_SDL
#define CHECKNVAPI(s) { NvAPI_Status hr = (s); if (hr != NVAPI_OK) { NvAPI_ShortString ss; NvAPI_GetErrorMessage(hr,ss); g_pvp->MessageBox(ss, "NVAPI", MB_OK | MB_ICONEXCLAMATION); } }
#endif

void ReportFatalError(const HRESULT hr, const char *file, const int line);
void ReportError(const char *errorText, const HRESULT hr, const char *file, const int line);

#if 1//def _DEBUG
#ifdef ENABLE_SDL
//void checkGLErrors(const char *file, const int line);
#define CHECKD3D(s) { s; } //checkGLErrors(__FILE__, __LINE__); } // by now the callback is used instead
#else //ENABLE_SDL
#define CHECKD3D(s) { const HRESULT hrTmp = (s); if (FAILED(hrTmp)) ReportFatalError(hrTmp, __FILE__, __LINE__); }
#endif
#else //_DEBUG
#define CHECKD3D(s) { s; }
#endif

bool IsWindows10_1803orAbove();

struct VideoMode
{
   int width;
   int height;
   int depth;
   int refreshrate;
};

struct DisplayConfig
{
   int display;
   int adapter;
   int top;
   int left;
   int width;
   int height;
   bool isPrimary;
   char DeviceName[CCHDEVICENAME];
   char GPU_Name[MAX_DEVICE_IDENTIFIER_STRING];
};

int getNumberOfDisplays();
void EnumerateDisplayModes(const int display, vector<VideoMode>& modes);
bool getDisplaySetupByID(const int display, int &x, int &y, int &width, int &height);
int getDisplayList(vector<DisplayConfig>& displays);
int getPrimaryDisplay();

class Shader;
class ModelViewProj;

class RenderDevice final
{
public:
#ifdef ENABLE_SDL
   enum PrimitiveTypes
   {
      TRIANGLEFAN = GL_TRIANGLE_FAN,
      TRIANGLESTRIP = GL_TRIANGLE_STRIP,
      TRIANGLELIST = GL_TRIANGLES,
      POINTLIST = GL_POINTS,
      LINELIST = GL_LINES,
      LINESTRIP = GL_LINE_STRIP
   };

   SDL_Window* m_sdl_playfieldHwnd;
   SDL_GLContext m_sdl_context;
#else
   enum PrimitiveTypes
   {
      TRIANGLEFAN = D3DPT_TRIANGLEFAN,
      TRIANGLESTRIP = D3DPT_TRIANGLESTRIP,
      TRIANGLELIST = D3DPT_TRIANGLELIST,
      POINTLIST = D3DPT_POINTLIST,
      LINELIST = D3DPT_LINELIST,
      LINESTRIP = D3DPT_LINESTRIP
   };
#endif

   enum TransformStateType {
      TRANSFORMSTATE_WORLD,
      TRANSFORMSTATE_VIEW,
      TRANSFORMSTATE_PROJECTION
   };

   RenderDevice(const HWND hwnd, const int width, const int height, const bool fullscreen, const int colordepth, int VSync, const float AAfactor, const StereoMode stereo3D, const unsigned int FXAA, const bool sharpen, const bool ss_refl, const bool useNvidiaApi, const bool disable_dwm, const int BWrendering);
   ~RenderDevice();
   void CreateDevice(int &refreshrate, UINT adapterIndex = D3DADAPTER_DEFAULT);
   bool LoadShaders();

   void SetRenderTarget(const string& passName, RenderTarget* rt, bool ignoreStereo = false);
   void AddRenderTargetDependency(RenderTarget* rt, const bool needDepth = false);
   void Clear(const DWORD flags, const D3DCOLOR color, const D3DVALUE z, const DWORD stencil);
   void BlitRenderTarget(RenderTarget* source, RenderTarget* destination, const bool copyColor = true, const bool copyDepth = true,  
      const int x1 = -1, const int y1 = -1, const int w1 = -1, const int h1 = -1,
      const int x2 = -1, const int y2 = -1, const int w2 = -1, const int h2 = -1);
   void SubmitVR(RenderTarget* source);
   void RenderLiveUI();
   void DrawMesh(Shader* shader, const bool isTransparent, const Vertex3Ds& center, const float depthBias, MeshBuffer* mb, const PrimitiveTypes type, const DWORD startIndice, const DWORD indexCount);
   void DrawTexturedQuad(Shader* shader, const Vertex3D_TexelOnly* vertices);
   void DrawTexturedQuad(Shader* shader, const Vertex3D_NoTex2* vertices);
   void DrawFullscreenTexturedQuad(Shader* shader);
   void DrawGaussianBlur(RenderTarget* source, RenderTarget* tmp, RenderTarget* dest, float kernel_size);
   void LogNextFrame() { m_logNextFrame = true; }
   bool IsLogNextFrame() const { return m_logNextFrame; }
   void FlushRenderFrame();
   void Flip(const bool vsync);

   bool SetMaximumPreRenderedFrames(const DWORD frames);

   RenderTarget* GetMSAABackBufferTexture() const { return m_pOffscreenMSAABackBufferTexture; } // Main render target, may be MSAA enabled and not suited for sampling, also may have stereo output (2 viewports)
   void ResolveMSAA(); // Resolve MSAA back buffer texture to be sample  from back buffer texture
   RenderTarget* GetBackBufferTexture() const { return m_pOffscreenBackBufferTexture; } // Main render target, with MSAA resolved if any, also may have stereo output (2 viewports)
   RenderTarget* GetPostProcessRenderTarget1();
   RenderTarget* GetPostProcessRenderTarget2();
   RenderTarget* GetPostProcessRenderTarget(RenderTarget* renderedRT);
   RenderTarget* GetOffscreenVR(int eye) const { return eye == 0 ? m_pOffscreenVRLeft : m_pOffscreenVRRight; }
   RenderTarget* GetReflectionBufferTexture() const { return m_pReflectionBufferTexture; }
   RenderTarget* GetBloomBufferTexture() const { return m_pBloomBufferTexture; }
   RenderTarget* GetBloomTmpBufferTexture() const { return m_pBloomTmpBufferTexture; }
   RenderTarget* GetOutputBackBuffer() const { return m_pBackBuffer; } // The screen render target
   RenderTarget* GetAORenderTarget(int idx);
   void SwapAORenderTargets();
   void ReleaseAORenderTargets() { delete m_pAORenderTarget1; m_pAORenderTarget1 = nullptr; delete m_pAORenderTarget2; m_pAORenderTarget2 = nullptr; }

   // VR/Stereo Stuff
#ifdef ENABLE_VR
   void InitVR();
   bool IsVRReady() const { return m_pHMD != nullptr; }
   void UpdateVRPosition(ModelViewProj& mvp);
   void tableUp();
   void tableDown();
   void recenterTable();
   void updateTableMatrix();
   static bool isVRinstalled();
   static bool isVRturnedOn();
   static void turnVROff();

   float m_scale = 1.0f;

private:
   static vr::IVRSystem* m_pHMD;
   float m_slope, m_orientation, m_tablex, m_tabley, m_tablez;
   vr::TrackedDevicePose_t m_hmdPosition;
   Matrix3D m_vrMatProj[2];
   Matrix3D m_tableWorld;
   vr::TrackedDevicePose_t* m_rTrackedDevicePose;

public:
#endif

   bool DepthBufferReadBackAvailable();

   RenderState& GetActiveRenderState() { return m_current_renderstate; }
   void SetClipPlane(const vec4 &plane);
   void SetRenderState(const RenderState::RenderStates p1, const RenderState::RenderStateValue p2);
   void SetRenderStateCulling(RenderState::RenderStateValue cull);
   void SetRenderStateDepthBias(float bias);
   void CopyRenderStates(const bool copyTo, RenderState& state);
   void ApplyRenderStates();

private:
   RenderState m_current_renderstate, m_renderstate;
   bool m_logNextFrame = false; // Output a log of next frame to main application log

public:
   void SetViewport(const ViewPort*);
   void GetViewport(ViewPort*);

   void SetMainTextureDefaultFiltering(const SamplerFilter filter);
   void CompressTextures(const bool enable) { m_compress_textures = enable; }

   // performance counters
   unsigned int Perf_GetNumDrawCalls() const        { return m_frameDrawCalls; }
   unsigned int Perf_GetNumStateChanges() const     { return m_frameStateChanges; }
   unsigned int Perf_GetNumTextureChanges() const   { return m_frameTextureChanges; }
   unsigned int Perf_GetNumParameterChanges() const { return m_frameParameterChanges; }
   unsigned int Perf_GetNumTechniqueChanges() const { return m_frameTechniqueChanges; }
   unsigned int Perf_GetNumTextureUploads() const   { return m_frameTextureUpdates; }
   unsigned int Perf_GetNumLockCalls() const        { return m_frameLockCalls; }

   void FreeShader();

#ifdef ENABLE_SDL
   int getGLVersion() const { return m_GLversion; }
#else
   IDirect3DDevice9* GetCoreDevice() const { return m_pD3DDevice; }
#endif

   HWND getHwnd() const { return m_windowHwnd; }

   HWND         m_windowHwnd;
   int          m_width; // Width of the render buffer (not the window width, for example for stereo the render width is doubled, or for VR, the size depends on the headset)
   int          m_height; // Height of the render buffer
   bool         m_fullscreen;
   int          m_colorDepth;
   int          m_vsync;
   StereoMode   m_stereo3D;
   float        m_AAfactor;
   bool         m_ssRefl;
   bool         m_disableDwm;
   bool         m_sharpen;
   unsigned int m_FXAA;
   int          m_BWrendering;

private:
   void UploadAndSetSMAATextures();

public:
   Sampler* m_SMAAsearchTexture = nullptr;
   Sampler* m_SMAAareaTexture = nullptr;
   Sampler* m_nullTexture = nullptr;

private:
#ifndef ENABLE_SDL
#ifdef USE_D3D9EX
   IDirect3D9Ex* m_pD3DEx;
   IDirect3DDevice9Ex* m_pD3DDeviceEx;
#endif
   IDirect3D9* m_pD3D;
   IDirect3DDevice9* m_pD3DDevice;
#endif

   RenderTarget* m_pBackBuffer = nullptr;

   RenderTarget* m_pOffscreenMSAABackBufferTexture = nullptr;
   RenderTarget* m_pOffscreenBackBufferTexture = nullptr;
   RenderTarget* m_pPostProcessRenderTarget1 = nullptr;
   RenderTarget* m_pPostProcessRenderTarget2 = nullptr;
   RenderTarget* m_pOffscreenVRLeft = nullptr;
   RenderTarget* m_pOffscreenVRRight = nullptr;
   RenderTarget* m_pBloomBufferTexture = nullptr;
   RenderTarget* m_pBloomTmpBufferTexture = nullptr;
   RenderTarget* m_pReflectionBufferTexture = nullptr;
   RenderTarget* m_pAORenderTarget1 = nullptr;
   RenderTarget* m_pAORenderTarget2 = nullptr;

   unsigned int m_adapter; // index of the display adapter to use

public:
   void SetSamplerState(int unit, SamplerFilter filter, SamplerAddressMode clamp_u, SamplerAddressMode clamp_v);

private:
#ifdef ENABLE_SDL
   GLfloat m_maxaniso;
   int m_GLversion;
   static GLuint m_samplerStateCache[3 * 3 * 5];
#else
   DWORD m_maxaniso;
   bool m_mag_aniso;
   static constexpr DWORD TEXTURESET_STATE_CACHE_SIZE = 32;
   SamplerFilter m_bound_filter[TEXTURESET_STATE_CACHE_SIZE];
   SamplerAddressMode m_bound_clampu[TEXTURESET_STATE_CACHE_SIZE];
   SamplerAddressMode m_bound_clampv[TEXTURESET_STATE_CACHE_SIZE];
#endif

public:
   bool m_autogen_mipmap;
   bool m_compress_textures;

private:
   bool m_dwm_was_enabled;
   bool m_dwm_enabled;

public:
   MeshBuffer* m_quadMeshBuffer = nullptr; // internal vb for rendering quads
   MeshBuffer* m_quadPNTDynMeshBuffer = nullptr; // internal vb for rendering dynamic quads (position/normal/texture)
   MeshBuffer* m_quadPTDynMeshBuffer = nullptr; // internal vb for rendering dynamic quads (position/texture)

public:
#ifndef ENABLE_SDL
   bool m_useNvidiaApi;
   bool m_INTZ_support;
   bool NVAPIinit;
#endif

   // performance counters
   unsigned int m_curDrawCalls = 0, m_frameDrawCalls = 0;
   unsigned int m_curStateChanges = 0, m_frameStateChanges = 0;
   unsigned int m_curTextureChanges = 0, m_frameTextureChanges = 0;
   unsigned int m_curParameterChanges = 0, m_frameParameterChanges = 0;
   unsigned int m_curTechniqueChanges = 0, m_frameTechniqueChanges = 0;
   unsigned int m_curTextureUpdates = 0, m_frameTextureUpdates = 0;
   unsigned int m_curLockCalls = 0, m_frameLockCalls = 0;

   Shader *basicShader = nullptr;
   Shader *DMDShader = nullptr;
   Shader *FBShader = nullptr;
   Shader *flasherShader = nullptr;
   Shader *lightShader = nullptr;
   Shader *StereoShader = nullptr;
   #define classicLightShader basicShader

   //Shader* m_curShader; // for caching

   TextureManager m_texMan;


   RenderTarget* GetCurrentRenderTarget() { return m_currentPass == nullptr ? nullptr : m_currentPass->m_rt; }

private :
   RenderFrame m_renderFrame;
   RenderPass* m_currentPass = nullptr;

public:
#ifdef ENABLE_SDL
   std::vector<SamplerBinding*> m_samplerBindings;
#endif

   unsigned int m_curDrawnTriangles, m_frameDrawnTriangles;

#if defined(ENABLE_SDL) // OpenGL
   GLuint m_curVAO = 0;
   
#else // DirectX9
   IDirect3DVertexBuffer9* m_curVertexBuffer = nullptr;
   IDirect3DIndexBuffer9* m_curIndexBuffer = nullptr;
   IDirect3DVertexDeclaration9* m_currentVertexDeclaration = nullptr;
   IDirect3DVertexDeclaration9* m_pVertexTexelDeclaration = nullptr;
   IDirect3DVertexDeclaration9* m_pVertexNormalTexelDeclaration = nullptr;
#endif
};
