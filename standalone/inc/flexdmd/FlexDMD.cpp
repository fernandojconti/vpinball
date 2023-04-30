#include "stdafx.h"
#include "FlexDMD.h"
#include "FlexDMDGroupActor.h"
#include "FlexDMDImageActor.h"
#include "FlexDMDFrameActor.h"
#include "FlexDMDVideoActor.h"
#include "FlexDMDLabelActor.h"
#include "FlexDMDFont.h"
#include "FlexDMDGIFImage.h"

#include <algorithm>

#if !((defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__))
#include "../zedmd/ZeDMD.h"
#endif

FlexDMD::FlexDMD()
{
   m_frameRate = 60;

   m_run = false;
   m_runtimeVersion = 1008;

   m_clear = false;
   m_renderLockCount = 0;

   m_pixels = NULL;
   m_pixelsSize = 0;

   m_colorPixels = NULL;
   m_colorPixelsSize = 0;

   m_renderMode = RenderMode::RenderMode_DMD_GRAY_4;

   m_dmdColor = RGB(255, 88, 32);

   m_pStage = FlexDMDGroupActor::Create(this, "Stage");
}

FlexDMD::~FlexDMD()
{
   m_run = false;

   if (m_pThread) {
      m_pThread->join();
      delete m_pThread;
   }

   if (m_pixels)
      free(m_pixels);

   if (m_colorPixels)
      free(m_colorPixels);

   m_pStage->Release();
}

STDMETHODIMP FlexDMD::get_Version(LONG *pRetVal)
{
   *pRetVal = 9999;

   return S_OK;
}

STDMETHODIMP FlexDMD::get_RuntimeVersion(LONG *pRetVal)
{
   *pRetVal = m_runtimeVersion;

   return S_OK;
}

STDMETHODIMP FlexDMD::put_RuntimeVersion(LONG pRetVal)
{
   m_runtimeVersion = pRetVal;

   return S_OK;
}

STDMETHODIMP FlexDMD::get_Run(VARIANT_BOOL *pRetVal)
{
   *pRetVal = m_run;

   return S_OK;
}

STDMETHODIMP FlexDMD::put_Run(VARIANT_BOOL pRetVal)
{
   if (!m_run && pRetVal == VARIANT_FALSE)
      return S_OK;

   if (m_run && pRetVal == VARIANT_TRUE)
      return S_OK;

   m_run = (pRetVal == VARIANT_TRUE);

   if (m_run) {
      PLOGI.printf("Starting render thread");

      m_pThread = new std::thread([this](){
         RenderLoop();
      });
   }
   else {
      PLOGI.printf("Stopping render thread");

      m_pThread->join();
      delete m_pThread;

      m_pThread = NULL;
   }

   return S_OK;
}

STDMETHODIMP FlexDMD::get_Show(VARIANT_BOOL *pRetVal)
{
   *pRetVal = VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP FlexDMD::put_Show(VARIANT_BOOL pRetVal)
{
   return S_OK;
}

STDMETHODIMP FlexDMD::get_GameName(BSTR *pRetVal)
{
   CComBSTR Val(m_szGameName.c_str());
   *pRetVal = Val.Detach();

   return S_OK;
}

STDMETHODIMP FlexDMD::put_GameName(BSTR pRetVal)
{
   m_szGameName = MakeString(pRetVal);

   PLOGI.printf("Game name set to %s", m_szGameName.c_str());

   return S_OK;
}

STDMETHODIMP FlexDMD::get_Width(unsigned short *pRetVal)
{
   *pRetVal = m_width;

   return S_OK;
}

STDMETHODIMP FlexDMD::put_Width(unsigned short pRetVal)
{
   m_width = pRetVal;

   return S_OK;
}

STDMETHODIMP FlexDMD::get_Height(unsigned short *pRetVal)
{
   *pRetVal = m_height;

   return S_OK;
}

STDMETHODIMP FlexDMD::put_Height(unsigned short pRetVal)
{
   m_height = pRetVal;

   return S_OK;
}

STDMETHODIMP FlexDMD::get_Color(OLE_COLOR *pRetVal)
{
   *pRetVal = m_dmdColor;

   return S_OK;
}

STDMETHODIMP FlexDMD::put_Color(OLE_COLOR pRetVal)
{
   m_dmdColor = pRetVal;

   return S_OK;
}

STDMETHODIMP FlexDMD::get_RenderMode(RenderMode *pRetVal)
{
   *pRetVal = m_renderMode;

   return S_OK;
}

STDMETHODIMP FlexDMD::put_RenderMode(RenderMode pRetVal)
{
   m_renderMode = pRetVal;

   return S_OK;
}

STDMETHODIMP FlexDMD::get_ProjectFolder(BSTR *pRetVal)
{
   CComBSTR Val(m_szProjectFolder.c_str());
   *pRetVal = Val.Detach();

   return S_OK;
}

STDMETHODIMP FlexDMD::put_ProjectFolder(BSTR pRetVal)
{
   m_szProjectFolder = MakeString(pRetVal);

   PLOGI.printf("Project folder set to %s", m_szProjectFolder.c_str());

   return S_OK;
}

STDMETHODIMP FlexDMD::get_TableFile(BSTR *pRetVal)
{
   CComBSTR Val(m_szTableFile.c_str());
   *pRetVal = Val.Detach();

   return S_OK;
}

STDMETHODIMP FlexDMD::put_TableFile(BSTR pRetVal)
{
   m_szTableFile = MakeString(pRetVal);

   PLOGI.printf("Table file set to %s", m_szTableFile.c_str());

   return S_OK;
}

STDMETHODIMP FlexDMD::get_Clear(VARIANT_BOOL *pRetVal)
{
   *pRetVal = m_clear ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP FlexDMD::put_Clear(VARIANT_BOOL pRetVal)
{
   m_clear = (pRetVal == VARIANT_TRUE);

   return S_OK;
}

STDMETHODIMP FlexDMD::get_DmdColoredPixels(VARIANT *pRetVal)
{
   const UINT32 end = m_width * m_height;

   if (!m_colorPixels || m_colorPixelsSize != end) {
      if (m_colorPixels)
         free(m_colorPixels);

      m_colorPixels = (UINT32*)malloc(end * sizeof(UINT32));
      m_colorPixelsSize = end;
      return S_FALSE;
   }

   SAFEARRAY *psa = SafeArrayCreateVector(VT_VARIANT, 0, end);

   VARIANT *pData;
   SafeArrayAccessData(psa, (void **)&pData);

   for (UINT32 i = 0; i < end; ++i) {
      V_VT(&pData[i]) = VT_UI4;
      V_UI4(&pData[i]) = m_colorPixels[i];
   }
   SafeArrayUnaccessData(psa);

   V_VT(pRetVal) = VT_ARRAY | VT_VARIANT;
   V_ARRAY(pRetVal) = psa;

   return S_OK;
}

STDMETHODIMP FlexDMD::get_DmdPixels(VARIANT *pRetVal)
{
   const UINT32 end = m_width * m_height;

   if (!m_pixels || m_pixelsSize != end) {
      if (m_pixels)
         free(m_pixels);

      m_pixels = (uint8_t*)malloc(end);
      m_pixelsSize = end;

      return S_FALSE;
   }

   SAFEARRAY *psa = SafeArrayCreateVector(VT_VARIANT, 0, end);

   VARIANT *pData;
   SafeArrayAccessData(psa, (void **)&pData);

   for (UINT32 i = 0; i < end; ++i) {
      V_VT(&pData[i]) = VT_UI1;
      V_UI1(&pData[i]) = m_pixels[i];
   }
   SafeArrayUnaccessData(psa);

   V_VT(pRetVal) = VT_ARRAY | VT_VARIANT;
   V_ARRAY(pRetVal) = psa;

   return S_OK;
}

STDMETHODIMP FlexDMD::putref_Segments(VARIANT rhs) { return E_NOTIMPL; }

STDMETHODIMP FlexDMD::get_Stage(IGroupActor **pRetVal)
{
   return m_pStage->QueryInterface(IID_IGroupActor, (void**)pRetVal);
}

STDMETHODIMP FlexDMD::LockRenderThread()
{
   m_renderLockCount++;

   if (m_renderLockCount == 1)
      m_renderMutex.lock();

   return S_OK;
}

STDMETHODIMP FlexDMD::UnlockRenderThread()
{
   m_renderLockCount--;
   if (m_renderLockCount == 0)
      m_renderMutex.unlock();

   return S_OK;
}

STDMETHODIMP FlexDMD::NewGroup(BSTR Name, IGroupActor **pRetVal)
{
   CComObject<FlexDMDGroupActor>* obj = FlexDMDGroupActor::Create(this, MakeString(Name));
   return obj->QueryInterface(IID_IGroupActor, (void**)pRetVal);
}

STDMETHODIMP FlexDMD::NewFrame(BSTR Name, IFrameActor **pRetVal)
{
   CComObject<FlexDMDFrameActor>* obj = FlexDMDFrameActor::Create(this, MakeString(Name));
   return obj->QueryInterface(IID_IFrameActor, (void**)pRetVal); 
}

STDMETHODIMP FlexDMD::NewLabel(BSTR Name, IUnknown *Font,BSTR Text, ILabelActor **pRetVal)
{
   CComObject<FlexDMDLabelActor>* obj = FlexDMDLabelActor::Create(this, MakeString(Name), (FlexDMDFont*)Font, MakeString(Text));
   return obj->QueryInterface(IID_ILabelActor, (void**)pRetVal);
}

STDMETHODIMP FlexDMD::NewVideo(BSTR Name, BSTR video, IVideoActor **pRetVal)
{
   string szVideo = MakeString(video);

   if (szVideo.find("|") != std::string::npos) {
   }
   else {
      string ext = "";
      ext = szVideo.substr(szVideo.length() - 4);
      std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return std::tolower(c); });

      if (ext == ".gif") {
          CComObject<FlexDMDGIFImage>* obj = FlexDMDGIFImage::Create(this, MakeString(Name), szVideo);
          return obj->QueryInterface(IID_IVideoActor, (void**)pRetVal);
      }
   }

   PLOGE.printf("Unable to load %s", szVideo.c_str());

   return E_FAIL;
}

STDMETHODIMP FlexDMD::NewImage(BSTR Name, BSTR image, IImageActor **pRetVal)
{
   string szImage = MakeString(image);

   if (szImage.find("|") != std::string::npos) {
   }
   else {
      string ext = "";
      ext = szImage.substr(szImage.length() - 4);
      std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return std::tolower(c); });

      if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp") {
         CComObject<FlexDMDImageActor>* obj = FlexDMDImageActor::Create(this, MakeString(Name), szImage);
         return obj->QueryInterface(IID_IImageActor, (void**)pRetVal);
      }
   }

   PLOGE.printf("Unable to load %s", szImage.c_str());

   return E_FAIL;
}

STDMETHODIMP FlexDMD::NewFont(BSTR Font, OLE_COLOR tint, OLE_COLOR borderTint, LONG borderSize, IUnknown **pRetVal)
{
   string szFont = MakeString(Font);

   if (szFont.find("|") != std::string::npos) {
   }
   else {
      string ext = "";
      ext = szFont.substr(szFont.length() - 4);
      std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return std::tolower(c); });

      if (ext == ".fnt") {
         FlexDMDFont* obj = FlexDMDFont::Create(this, szFont, tint, borderTint, borderSize);
         return obj->QueryInterface(IID_FlexDMDFontInterface, (void**)pRetVal);
      }
   }

   PLOGE.printf("Unable to load %s", szFont.c_str());

   return E_FAIL;
}

STDMETHODIMP FlexDMD::NewUltraDMD(IUltraDMD **pRetVal) { return E_NOTIMPL; }

void FlexDMD::RenderLoop()
{
   SDL_Surface* graphics = SDL_CreateRGBSurface(0, m_width, m_height, 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);

   m_pStage->SetSize(m_width, m_height);
   m_pStage->SetOnStage(true);

   UINT8* bpFrame = (UINT8*)malloc(m_width * m_height);

   double elapsedMs = 0.0;

#if !((defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__))
   ZeDMD zedmd;
   zedmd.Open(m_width, m_height);
   zedmd.SetColor(m_dmdColor);
#endif

   while (m_run) {
      Uint64 startTime = SDL_GetTicks64();

      if (m_clear)
         SDL_FillRect(graphics, NULL, SDL_MapRGB(graphics->format, 0, 0, 0));

      m_renderMutex.lock();

      m_pStage->Update((float)(elapsedMs / 1000.0));
      m_pStage->Draw(graphics);

      m_renderMutex.unlock();

      switch (m_renderMode) {
         case RenderMode::RenderMode_DMD_GRAY_2:
         {
            UINT8* dst = bpFrame;
            UINT8* ptr = (UINT8*)graphics->pixels;
            int pos = 0;
            for (int y = 0; y < m_height; y++) {
               for (int x = 0; x < m_width; x++) {
                  int r = *ptr++;
                  int g = *ptr++;
                  int b = *ptr++;
                  int v = (int)(0.2126f * r + 0.7152f * g + 0.0722f * b);
                  if (v > 255) v = 255;
                  dst[pos] = v >> 6;
                  if (m_pixels) m_pixels[pos] = v;
                  pos++;
               }
            }
#if !((defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__))
            zedmd.RenderGray2(dst, m_width, m_height);
#endif
         }
         break;

         case RenderMode::RenderMode_DMD_GRAY_4:
         {
            UINT8* dst = bpFrame;
            UINT8* ptr = (UINT8*)graphics->pixels;
            int pos = 0;
            for (int y = 0; y < m_height; y++) {
               for (int x = 0; x < m_width; x++) {
                  int r = *ptr++;
                  int g = *ptr++;
                  int b = *ptr++;
                  int v = (int)(0.2126f * r + 0.7152f * g + 0.0722f * b);
                  if (v > 255) v = 255;
                  dst[pos] = v >> 4;
                  if (m_pixels) m_pixels[pos] = v;
                  pos++;
               }
            }
#if !((defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__))
            zedmd.RenderGray4(dst, m_width, m_height);
#endif
         }
         break;

         case RenderMode::RenderMode_DMD_RGB:
         {
            if (m_pixels) {
               UINT8* ptr = (UINT8*)graphics->pixels;
               int pos = 0;
               for (int y = 0; y < m_height; y++) {
                  for (int x = 0; x < m_width; x++) {
                     int r = *ptr++;
                     int g = *ptr++;
                     int b = *ptr++;
                     int v = (int)(0.2126f * r + 0.7152f * g + 0.0722f * b);
                     if (v > 255) v = 255;
                     m_pixels[pos] = v;
                     pos++;
                  }
               }
            }
#if !((defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__))
            zedmd.RenderRgb24((UINT8*)graphics->pixels, m_width, m_height);
#endif
         }
         break;

         default:
         break;
      }

      if (m_colorPixels) {
         int pos = 0;
         UINT8* ptr = (UINT8*)graphics->pixels;
         for (int y = 0; y < m_height; y++)
            for (int x = 0; x < m_width; x++) {
               int r = *ptr++;
               int g = *ptr++;
               int b = *ptr++;
               m_colorPixels[pos++] = ((b << 16) + (g << 8) + r);
            }
      }

      double renderingDuration = SDL_GetTicks64() - startTime;

      int sleepMs = (1000 / m_frameRate) - (int)renderingDuration;

      if (sleepMs > 1)
         SDL_Delay(sleepMs);

      elapsedMs = SDL_GetTicks64() - startTime;

      if (elapsedMs > 4000 / m_frameRate) {
         PLOGI.printf("Abnormally long elapsed time between frames of %fs (rendering lasted %fms, sleeping was %dms), limiting to %dms", 
            elapsedMs / 1000.0, renderingDuration, std::max(0, sleepMs), 4000 / m_frameRate);

         elapsedMs = 4000 / m_frameRate;
      }
   }

   m_pStage->SetOnStage(false);

   free(bpFrame);

   SDL_FreeSurface(graphics);
}