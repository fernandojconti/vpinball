#include "stdafx.h"
#include "VPinMAMEController.h"
#include "VPinMAMEGames.h"

#include "inc/mINI/ini.h"
#include "serum-decode.h"

void CALLBACK VPinMAMEController::GetGameCallback(PinmameGame* pPinmameGame, const void* pUserData)
{
   VPinMAMEController* pController = (VPinMAMEController*)pUserData;

   if (pController->m_pPinmameGame)
      delete pController->m_pPinmameGame;

   pController->m_pPinmameGame = new PinmameGame();
   memcpy(pController->m_pPinmameGame, pPinmameGame, sizeof(PinmameGame));
}

void CALLBACK VPinMAMEController::OnStateUpdated(int state, const void* pUserData) {
   PLOGI.printf("state=%d", state);
}

void CALLBACK VPinMAMEController::OnDisplayAvailable(int index, int displayCount, PinmameDisplayLayout *p_displayLayout, const void* pUserData)
{
   VPinMAMEController* pController = (VPinMAMEController*)pUserData;

   PLOGI.printf("index=%d, displayCount=%d, type=%d, top=%d, left=%d, width=%d, height=%d, depth=%d, length=%d", 
      index,
      displayCount,
      p_displayLayout->type, 
      p_displayLayout->top,
      p_displayLayout->left, 
      p_displayLayout->width, 
      p_displayLayout->height, 
      p_displayLayout->depth, 
      p_displayLayout->length);

   if ((p_displayLayout->type & DMD) == DMD && index == 0) {
      pController->m_dmdHeight = p_displayLayout->height;
      pController->m_dmdWidth = p_displayLayout->width;
      pController->m_dmdDepth = p_displayLayout->depth;

      pController->m_pLevels = (UINT8*)((p_displayLayout->depth == 2) ? LEVELS_WPC :
         (PinmameGetHardwareGen() & PINMAME_HARDWARE_GEN::SAM | PINMAME_HARDWARE_GEN::SPA) ? LEVELS_SAM : LEVELS_GTS3);

      pController->m_pDmdData = (UINT8*)malloc(pController->m_dmdHeight * pController->m_dmdWidth);
      memset(pController->m_pDmdData, 0, pController->m_dmdHeight * pController->m_dmdWidth);

      int colors = (p_displayLayout->depth == 2) ? 4 : 16;

      UINT8 r = pController->m_dmdColor & 0xFF;
      UINT8 g = (pController->m_dmdColor >> 8) & 0xFF;
      UINT8 b = (pController->m_dmdColor >> 16) & 0xFF;

      int pos = 0;
      for (int i = 0; i < colors; i++) {
         float perc = calc_brightness(i / (float)(colors-1));
         pController->m_dmdPalette[pos++] = (UINT8)r * perc;
         pController->m_dmdPalette[pos++] = (UINT8)g * perc;
         pController->m_dmdPalette[pos++] = (UINT8)b * perc;
      }

#if !((defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__))
      pController->m_zedmd.Open(p_displayLayout->width, p_displayLayout->height);
      pController->m_zedmd.SetColor(pController->m_dmdColor);
#endif

      /*string altcolorPath = pController->m_szPath + "altcolor";
      int width;
      int height;

      if (Serum_Load(altcolorPath.c_str(), pController->m_pPinmameGame->name, &width, &height, &colors, NULL)) {
         if (width == pController->m_dmdWidth && height == pController->m_dmdHeight) {
            PLOGI.printf("Serum loaded: width=%d, height=%d, colors=%d", width, height, colors);
            pController->m_hasSerum = true;
         }
         else {
            PLOGI.printf("Serum not loading due to mismatch width or height: width=%d, height=%d, colors=%d", width, height, colors);
            Serum_Dispose();
         }
      }*/
   }
}

void CALLBACK VPinMAMEController::OnDisplayUpdated(int index, void* p_displayData, PinmameDisplayLayout* p_displayLayout, const void* pUserData)
{
   VPinMAMEController* pController = (VPinMAMEController*)pUserData;

   std::unique_lock<std::mutex> lock(pController->m_dmdMutex);

   if ((p_displayLayout->type & DMD) == DMD && index == 0) {
      memcpy(pController->m_pDmdData, p_displayData, p_displayLayout->width * p_displayLayout->height);

#if !((defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__))
      if (p_displayLayout->depth == 2)
         pController->m_zedmd.RenderGray2((UINT8*)p_displayData, p_displayLayout->width, p_displayLayout->height);
      else if (p_displayLayout->depth == 4)
         pController->m_zedmd.RenderGray4((UINT8*)p_displayData, p_displayLayout->width, p_displayLayout->height);
#endif


      pController->m_hasDMDUpdates = true;
   }

   lock.unlock();
}

int CALLBACK VPinMAMEController::OnAudioAvailable(PinmameAudioInfo* p_audioInfo, const void* pUserData)
{
   VPinMAMEController* pController = (VPinMAMEController*)pUserData;

   PLOGI.printf("format=%d, channels=%d, sampleRate=%.2f, framesPerSecond=%.2f, samplesPerFrame=%d, bufferSize=%d", 
      p_audioInfo->format,
      p_audioInfo->channels,
      p_audioInfo->sampleRate,
      p_audioInfo->framesPerSecond,
      p_audioInfo->samplesPerFrame,
      p_audioInfo->bufferSize);

   pController->m_audio = new AudioPlayer();
   pController->m_audio->StreamInit(p_audioInfo->sampleRate, p_audioInfo->channels, 1.);

   pController->m_audioChannels = p_audioInfo->channels;

   return p_audioInfo->samplesPerFrame;
}

int CALLBACK VPinMAMEController::OnAudioUpdated(void* p_buffer, int samples, const void* pUserData)
{
   VPinMAMEController* pController = (VPinMAMEController*)pUserData;

   pController->m_audio->StreamUpdate(p_buffer, samples * pController->m_audioChannels * sizeof(int16_t));

   return samples;
}

void CALLBACK VPinMAMEController::OnMechAvailable(int mechNo, PinmameMechInfo* p_mechInfo, const void* pUserData)
{
   PLOGI.printf("mechNo=%d, type=%d, length=%d, steps=%d, pos=%d, speed=%d", 
      mechNo, 
      p_mechInfo->type, 
      p_mechInfo->length, 
      p_mechInfo->steps,
      p_mechInfo->pos, 
      p_mechInfo->speed);
}

void CALLBACK VPinMAMEController::OnMechUpdated(int mechNo, PinmameMechInfo* p_mechInfo, const void* pUserData)
{
   VPinMAMEController* pController = (VPinMAMEController*)pUserData;

   PLOGD.printf("mechNo=%d, type=%d, length=%d, steps=%d, pos=%d, speed=%d",
      mechNo, 
      p_mechInfo->type, 
      p_mechInfo->length, 
      p_mechInfo->steps, 
      p_mechInfo->pos,
      p_mechInfo->speed);

   pController->m_mechData[mechNo] = p_mechInfo->pos;
}

void CALLBACK VPinMAMEController::OnConsoleDataUpdated(void* p_data, int size, const void* pUserData)
{
   PLOGI.printf("size=%d", size);
}

void CALLBACK VPinMAMEController::OnLogMessage(const char* format, va_list args, const void* pUserData)
{
   char buffer[1024];
   vsnprintf(buffer, sizeof(buffer), format, args);

   PLOGI.printf("%s", buffer);
}

VPinMAMEController::VPinMAMEController()
{
   PinmameConfig config = { 
      AUDIO_FORMAT_INT16, 
      44100, 
      "", 
      &VPinMAMEController::OnStateUpdated,
      &VPinMAMEController::OnDisplayAvailable,
      &VPinMAMEController::OnDisplayUpdated,
      &VPinMAMEController::OnAudioAvailable,
      &VPinMAMEController::OnAudioUpdated,
      &VPinMAMEController::OnMechAvailable,
      &VPinMAMEController::OnMechUpdated,
      NULL,
      &VPinMAMEController::OnConsoleDataUpdated,
      NULL,
      &VPinMAMEController::OnLogMessage,
   };

#if (defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__)
   m_szPath = g_pvp->m_szMyPath + "pinmame" + PATH_SEPARATOR_CHAR;
#else
   m_szPath = string(getenv("HOME")) + PATH_SEPARATOR_CHAR + ".pinmame" + PATH_SEPARATOR_CHAR;
#endif
   strncpy_s((char*)config.vpmPath, m_szPath.c_str(), PINMAME_MAX_PATH - 1);

   PinmameSetConfig(&config);
   PinmameSetUserData((void*)this);
   PinmameSetDmdMode(PINMAME_DMD_MODE::RAW);

   PinmameSetHandleKeyboard(0);
   PinmameSetHandleMechanics(0xFF);

   m_pSolenoidBuffer = new PinmameSolenoidState[PinmameGetMaxSolenoids()];
   m_pLampBuffer = new PinmameLampState[PinmameGetMaxLamps()];
   m_pGIBuffer = new PinmameGIState[PinmameGetMaxGIs()];
   m_pLEDBuffer = new PinmameLEDState[PinmameGetMaxLEDs()];

   m_pPinmameGame = NULL;
   m_pPinmameMechConfig = NULL;

   m_pDmdData = NULL;

   m_dmdColor = RGB(255, 88, 32);

   m_hasRGBs = false;

   m_hasSerum = false;

   CComObject<VPinMAMEGames>::CreateInstance(&m_pGames);
   m_pGames->AddRef();
}

VPinMAMEController::~VPinMAMEController()
{
   if (PinmameIsRunning())
      PinmameStop();

   if (m_pDmdData)
      free(m_pDmdData);

   if (m_pSolenoidBuffer)
      delete m_pSolenoidBuffer;

   if (m_pLampBuffer)
      delete m_pLampBuffer;

   if (m_pGIBuffer)
      delete m_pGIBuffer;

   if (m_pLEDBuffer)
      delete m_pLEDBuffer;

   if (m_pPinmameGame)
      delete m_pPinmameGame;

   if (m_pPinmameMechConfig)
      delete m_pPinmameMechConfig;

   m_pGames->Release();
}

STDMETHODIMP VPinMAMEController::Run(/*[in]*/ LONG_PTR hParentWnd, /*[in,defaultvalue(100)]*/ int nMinVersion)
{
   if (m_pPinmameGame) {

      mINI::INIStructure ini;

      mINI::INIFile file(m_szPath + "vpinball" + PATH_SEPARATOR_CHAR + m_pPinmameGame->name + ".ini");
      if (file.read(ini))
         if (ini.has("dmd") && ini["dmd"].has("red") && ini["dmd"].has("green") && ini["dmd"].has("blue"))
            m_dmdColor = RGB(atoll(ini["dmd"]["red"].c_str()), atoll(ini["dmd"]["green"].c_str()), atoll(ini["dmd"]["blue"].c_str()));

      UINT8 r = m_dmdColor & 0xFF;
      UINT8 g = (m_dmdColor >> 8) & 0xFF;
      UINT8 b = (m_dmdColor >> 16) & 0xFF;

      ini["dmd"]["red"] = std::to_string(r);
      ini["dmd"]["green"] = std::to_string(g);
      ini["dmd"]["blue"] = std::to_string(b);

      file.write(ini);

      PINMAME_STATUS status = PinmameRun(m_pPinmameGame->name);

      if (status == PINMAME_STATUS::OK) {
         int timeout = 0;

         while (!PinmameIsRunning() && timeout < 20) {
            SDL_Delay(75);
            timeout++;
         }

         m_hasRGBs = (PinmameGetHardwareGen() == PINMAME_HARDWARE_GEN::SAM);

         return S_OK;
      }

      if (status == PINMAME_STATUS::GAME_ALREADY_RUNNING) {
         PLOGI.printf("Game already running.");
      }
   }

   return S_FALSE;
}

STDMETHODIMP VPinMAMEController::Stop()
{
   if (PinmameIsRunning())
      PinmameStop();

   m_hasRGBs = false;

   if (m_pDmdData) {
      free(m_pDmdData);
      m_pDmdData = NULL;
   }

   if (m_hasSerum) {
      Serum_Dispose();

      m_hasSerum = false;
   }

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_Lamp(int nLamp, VARIANT_BOOL *pVal)
{
   *pVal = PinmameGetLamp(nLamp) ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_Solenoid(int nSolenoid, VARIANT_BOOL *pVal)
{
   *pVal = PinmameGetSolenoid(nSolenoid) ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_Switch(int nSwitchNo, VARIANT_BOOL *pVal)
{
   *pVal = PinmameGetSwitch(nSwitchNo) ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_Switch(int nSwitchNo, VARIANT_BOOL newVal)
{
   PinmameSetSwitch(nSwitchNo, (newVal == VARIANT_TRUE) ? 1 : 0);

   PLOGD.printf("switchNo=%d, newVal=%d", nSwitchNo, newVal == VARIANT_TRUE);

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_Pause(VARIANT_BOOL *pVal)
{
   *pVal = PinmameIsPaused() ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_Pause(VARIANT_BOOL newVal)
{
   PinmamePause((newVal == VARIANT_TRUE) ? 1 : 0);

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_WPCNumbering(/*[out, retval]*/ VARIANT_BOOL *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_Lamps(VARIANT *pVal) { return E_NOTIMPL; }

STDMETHODIMP VPinMAMEController::get_RawDmdWidth(int *pVal)
{
   *pVal = m_dmdWidth;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_RawDmdHeight(int *pVal)
{
   *pVal = m_dmdHeight;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_NVRAM(VARIANT *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_ChangedNVRAM(VARIANT *pVal) { return E_NOTIMPL; }

STDMETHODIMP VPinMAMEController::get_RawDmdPixels(VARIANT *pVal)
{
   std::unique_lock<std::mutex> lock(m_dmdMutex);

   if (m_hasDMDUpdates) {
      m_hasDMDUpdates = false;

      SAFEARRAY *psa = SafeArrayCreateVector(VT_VARIANT, 0, m_dmdWidth * m_dmdHeight);

      VARIANT *pData;
      SafeArrayAccessData(psa, (void **)&pData);
      const UINT32 end = m_dmdWidth * m_dmdHeight;
      for (UINT32 i = 0; i < end; ++i)
      {
         V_VT(&pData[i]) = VT_UI1;
         V_UI1(&pData[i]) = m_pLevels[m_pDmdData[i]];
      }
      SafeArrayUnaccessData(psa);

      V_VT(pVal) = VT_ARRAY | VT_VARIANT;
      V_ARRAY(pVal) = psa;

      lock.unlock();
      return S_OK;
   }

   lock.unlock();
   return S_FALSE;
}

STDMETHODIMP VPinMAMEController::get_RawDmdColoredPixels(VARIANT *pVal)
{
   std::unique_lock<std::mutex> lock(m_dmdMutex);

   if (m_hasDMDUpdates) {
      m_hasDMDUpdates = false;

      if (m_hasSerum && !Serum_ColorizeOrApplyRotations(m_pDmdData, m_dmdWidth, m_dmdHeight, m_dmdPalette, NULL)) {
         lock.unlock();
         return S_FALSE;
      }
   }
   else if (m_hasSerum && !Serum_ColorizeOrApplyRotations(m_pDmdData, m_dmdWidth, m_dmdHeight, m_dmdPalette, NULL)) {
      lock.unlock();
      return S_FALSE;
   }
   else {
      lock.unlock();
      return S_FALSE;
   }

   SAFEARRAY *psa = SafeArrayCreateVector(VT_VARIANT, 0, m_dmdWidth * m_dmdHeight);

   VARIANT *pData;
   SafeArrayAccessData(psa, (void **)&pData);
   const UINT32 end = m_dmdWidth * m_dmdHeight;
   for (UINT32 i = 0; i < end; ++i)
   {
      int r = m_dmdPalette[(m_pDmdData[i] * 3)];
      int g = m_dmdPalette[(m_pDmdData[i] * 3) + 1];
      int b = m_dmdPalette[(m_pDmdData[i] * 3) + 2];

      V_VT(&pData[i]) = VT_UI4;
      V_UI4(&pData[i]) = r | g << 8 | b << 16;
   }
   SafeArrayUnaccessData(psa);

   V_VT(pVal) = VT_ARRAY | VT_VARIANT;
   V_ARRAY(pVal) = psa;

   lock.unlock();
   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_DmdWidth(int *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_DmdHeight(int *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_DmdPixel(int x, int y, int *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_updateDmdPixels(int **buf, int width, int height, int *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_ChangedLampsState(int **buf, int *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_LampsState(int **buf, int *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_ChangedSolenoidsState(int **buf, int *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_SolenoidsState(int **buf, int *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_ChangedGIsState(int **buf, int *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_Switches(VARIANT *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::put_Switches(VARIANT newVal) { return E_NOTIMPL; }

STDMETHODIMP VPinMAMEController::get_GameName(BSTR *pVal)
{
   CComBSTR Val(m_pPinmameGame->name);
   *pVal = Val.Detach();

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_GameName(BSTR newVal)
{
   char szGameName[MAXSTRING];
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, szGameName, sizeof(szGameName), NULL, NULL);
   PLOGI.printf("newVal=%s", szGameName);

   PINMAME_STATUS status = PinmameGetGame(szGameName, &VPinMAMEController::GetGameCallback, this);

   if (status == PINMAME_STATUS::OK) {
      PLOGI.printf("Game found: name=%s, description=%s, manufacturer=%s, year=%s",
         m_pPinmameGame->name, m_pPinmameGame->description, m_pPinmameGame->manufacturer, m_pPinmameGame->year);

      return S_OK;
   }

   if (status == PINMAME_STATUS::GAME_ALREADY_RUNNING) {
      PLOGI.printf("Game already running.");
   }
   else if (status == PINMAME_STATUS::GAME_NOT_FOUND) {
      PLOGI.printf("Game name not found.");
   }

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_ROMName(BSTR *pVal) { return E_NOTIMPL; }

STDMETHODIMP VPinMAMEController::get_HandleKeyboard(VARIANT_BOOL *pVal)
{
   *pVal = PinmameGetHandleKeyboard() ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_HandleKeyboard(VARIANT_BOOL newVal)
{
   PinmameSetHandleKeyboard((newVal == VARIANT_TRUE));

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_Machines(BSTR sMachine, VARIANT *pVal) { return E_NOTIMPL; }

STDMETHODIMP VPinMAMEController::get_Running(VARIANT_BOOL *pVal) { 
   return PinmameIsRunning() ? VARIANT_TRUE : VARIANT_FALSE; 
}

STDMETHODIMP VPinMAMEController::get_ChangedLamps(VARIANT *pVal)
{
   int uCount = PinmameGetChangedLamps(m_pLampBuffer);
   if (uCount <= 0) {
      V_VT(pVal) = VT_EMPTY;
      return S_OK;
   }

   SAFEARRAYBOUND Bounds[] = { { (ULONG)uCount, 0 }, { 2, 0 } };
   SAFEARRAY *psa = SafeArrayCreate(VT_VARIANT, 2, Bounds);
   LONG ix[2];
   VARIANT varValue;
   V_VT(&varValue) = VT_I4;

   for (ix[0] = 0; ix[0] < uCount; ix[0]++) {
      ix[1] = 0;
      V_I4(&varValue) = m_pLampBuffer[ix[0]].lampNo;
      SafeArrayPutElement(psa, ix, &varValue);
      ix[1] = 1;

      if (m_hasRGBs && m_pLampBuffer[ix[0]].lampNo >= 81) {
         V_I4(&varValue) = m_pLampBuffer[ix[0]].state;
      }
      else {
         V_I4(&varValue) = (m_pLampBuffer[ix[0]].state) ? 1 : 0;
      }

      SafeArrayPutElement(psa, ix, &varValue);
   }

   V_VT(pVal) = VT_ARRAY | VT_VARIANT;
   V_ARRAY(pVal) = psa;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_ChangedLEDs(int nHigh, int nLow, int nnHigh, int nnLow, VARIANT *pVal)
{
   uint64_t mask = ((uint64_t)nHigh << 32) | (uint64_t)nLow;
   uint64_t mask2 = ((uint64_t)nnHigh << 32) | (uint64_t)nnLow;
    
   int uCount = PinmameGetChangedLEDs(mask, mask2, m_pLEDBuffer);
   if (uCount <= 0)
   {
      V_VT(pVal) = VT_EMPTY;
      return S_OK;
   }

   SAFEARRAYBOUND Bounds[] = { { (ULONG)uCount, 0 }, { 3, 0 } };
   SAFEARRAY *psa = SafeArrayCreate(VT_VARIANT, 2, Bounds);
   LONG ix[3];
   VARIANT varValue;
   V_VT(&varValue) = VT_I4;

   for (ix[0] = 0; ix[0] < uCount; ix[0]++) {
      ix[1] = 0;
      V_I4(&varValue) = m_pLEDBuffer[ix[0]].ledNo;
      SafeArrayPutElement(psa, ix, &varValue);
      ix[1] = 1;
      V_I4(&varValue) = m_pLEDBuffer[ix[0]].chgSeg;
      SafeArrayPutElement(psa, ix, &varValue);
      ix[1] = 2;
      V_I4(&varValue) = m_pLEDBuffer[ix[0]].state;
      SafeArrayPutElement(psa, ix, &varValue);
   }

   V_VT(pVal) = VT_ARRAY | VT_VARIANT;
   V_ARRAY(pVal) = psa;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_ChangedLEDsState(int nHigh, int nLow, int nnHigh, int nnLow, int **buf, int *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::ShowAboutDialog(LONG_PTR hParentWnd) { return E_NOTIMPL; }

STDMETHODIMP VPinMAMEController::get_HandleMechanics(int *pVal)
{
   *pVal = PinmameGetHandleMechanics();

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_HandleMechanics(int newVal)
{
   PinmameSetHandleMechanics(newVal);

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_GetMech(int mechNo, int *pVal)
{
   *pVal = (m_mechData.count(mechNo) > 0) ? m_mechData[mechNo] : 0;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_Mech(int mechNo, int newVal) {
   if (!m_pPinmameMechConfig)
      m_pPinmameMechConfig = new PinmameMechConfig();

   switch(mechNo) {
      case 0:
         PinmameSetMech(newVal, m_pPinmameMechConfig);

         delete m_pPinmameMechConfig;
         m_pPinmameMechConfig = NULL;

         break;
      case 1:
         m_pPinmameMechConfig->sol1 = newVal;
         break;
      case 2:
         m_pPinmameMechConfig->sol2 = newVal;
         break;
      case 3:
         m_pPinmameMechConfig->length = newVal;
         break;
      case 4:
         m_pPinmameMechConfig->steps = newVal;
         break;
      case 5:
         m_pPinmameMechConfig->type = newVal;
         break;
      case 6:
         m_pPinmameMechConfig->acc = newVal;
         break;
      case 7:
         m_pPinmameMechConfig->ret = newVal;
         break;
      case 8:
         m_pPinmameMechConfig->initialPos = newVal + 1;
         break;
      default:
         if (mechNo % 10 == 0)
            m_pPinmameMechConfig->sw[(mechNo/10)-1].swNo = newVal;
         else if (mechNo % 10 == 1)
            m_pPinmameMechConfig->sw[(mechNo/10)-1].startPos = newVal;
         else if (mechNo % 10 == 2)
            m_pPinmameMechConfig->sw[(mechNo/10)-1].endPos = newVal;
         else if (mechNo % 10 == 3)
            m_pPinmameMechConfig->sw[(mechNo/10)-1].pulse = newVal;
         break;
   }

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_GIString(int nString, int *pVal) { return E_NOTIMPL; }

STDMETHODIMP VPinMAMEController::get_ChangedGIStrings(VARIANT *pVal)
{
   int uCount = PinmameGetChangedGIs(m_pGIBuffer);
   if (uCount <= 0) {
      V_VT(pVal) = VT_EMPTY;
      return S_OK;
   }

   SAFEARRAYBOUND Bounds[] = { { (ULONG)uCount, 0 }, { 2, 0 } };
   SAFEARRAY *psa = SafeArrayCreate(VT_VARIANT, 2, Bounds);
   LONG ix[2];
   VARIANT varValue;
   V_VT(&varValue) = VT_I4;

   for (ix[0] = 0; ix[0] < uCount; ix[0]++) {
      ix[1] = 0;
      V_I4(&varValue) = m_pGIBuffer[ix[0]].giNo;
      SafeArrayPutElement(psa, ix, &varValue);
      ix[1] = 1;
      V_I4(&varValue) = m_pGIBuffer[ix[0]].state;
      SafeArrayPutElement(psa, ix, &varValue);
   }

   V_VT(pVal) = VT_ARRAY | VT_VARIANT;
   V_ARRAY(pVal) = psa;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_ChangedSolenoids(VARIANT *pVal)
{
   int uCount = PinmameGetChangedSolenoids(m_pSolenoidBuffer);
   if (uCount <= 0)
   {
      V_VT(pVal) = VT_EMPTY;
      return S_OK;
   }

   SAFEARRAYBOUND Bounds[] = { { (ULONG)uCount, 0 }, { 2, 0 } };
   SAFEARRAY *psa = SafeArrayCreate(VT_VARIANT, 2, Bounds);
   LONG ix[2];
   VARIANT varValue;
   V_VT(&varValue) = VT_I4;

   for (ix[0] = 0; ix[0] < uCount; ix[0]++) {
      ix[1] = 0;
      V_I4(&varValue) = m_pSolenoidBuffer[ix[0]].solNo;
      SafeArrayPutElement(psa, ix, &varValue);
      ix[1] = 1;
      V_I4(&varValue) = m_pSolenoidBuffer[ix[0]].state;
      SafeArrayPutElement(psa, ix, &varValue);
   }

   V_VT(pVal) = VT_ARRAY | VT_VARIANT;
   V_ARRAY(pVal) = psa;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_SplashInfoLine(BSTR *pVal)
{
   CComBSTR strValue(m_szSplashInfoLine);
   *pVal = strValue.Detach();

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_SplashInfoLine(BSTR newVal)
{
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_szSplashInfoLine, sizeof m_szSplashInfoLine, NULL, NULL);
   PLOGI.printf("newVal=%s", m_szSplashInfoLine);

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_Solenoids(VARIANT *pVal) { return E_NOTIMPL; }

STDMETHODIMP VPinMAMEController::get_Dip(int nNo, int *pVal)
{
   *pVal = PinmameGetDIP(nNo);

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_Dip(int nNo, int newVal)
{
    PinmameSetDIP(nNo, newVal);

    return S_OK;
}

STDMETHODIMP VPinMAMEController::get_GIStrings(VARIANT *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_LockDisplay(VARIANT_BOOL *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::put_LockDisplay(VARIANT_BOOL newVal) { return E_NOTIMPL; }

STDMETHODIMP VPinMAMEController::get_SolMask(int nLow, LONG *pVal)
{
   if (!pVal || (nLow < 0) || (nLow > 1))
      return S_FALSE;

   *pVal = PinmameGetSolenoidMask(nLow);

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_SolMask(int nLow, LONG newVal)
{
   if (!((0 <= nLow && nLow <= 2) || (1000 <= nLow && nLow < 1999)))
      return S_FALSE;

   PinmameSetSolenoidMask(nLow, newVal);

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_Version(BSTR *pVal)
{
   int nVersionNo0 = 99;
   int nVersionNo1 = 99;
   int nVersionNo2 = 99;
   int nVersionNo3 = 99;

   char szVersion[MAXTOKEN];
   sprintf_s(szVersion, sizeof(szVersion), "%02i%02i%02i%02i", nVersionNo0, nVersionNo1, nVersionNo2, nVersionNo3);

   WCHAR wzVersion[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, szVersion, -1, wzVersion, MAXTOKEN);

   *pVal = SysAllocString(wzVersion);

   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_Games(IGames **pVal) 
{ 
   return m_pGames->QueryInterface(IID_IGames, (void**) pVal);
}

STDMETHODIMP VPinMAMEController::get_Settings(IControllerSettings **pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_NewSoundCommands(VARIANT *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::SetDisplayPosition(int x, int y, LONG_PTR hParentWindow) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::ShowOptsDialog(LONG_PTR hParentWnd) { return E_NOTIMPL; }

STDMETHODIMP VPinMAMEController::get_ShowDMDOnly(VARIANT_BOOL *pVal)
{
   *pVal = VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_ShowDMDOnly(VARIANT_BOOL newVal)
{
   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_ShowTitle(VARIANT_BOOL *pVal)
{
   *pVal = VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_ShowTitle(VARIANT_BOOL newVal)
{
   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_ShowFrame(VARIANT_BOOL *pVal)
{
   *pVal = VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_ShowFrame(VARIANT_BOOL newVal)
{
   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_DoubleSize(VARIANT_BOOL *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::put_DoubleSize(VARIANT_BOOL newVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::CheckROMS(/*[in,defaultvalue(0)]*/ int nShowOptions, /*[in,defaultvalue(0)]*/ LONG_PTR hParentWnd, /*[out, retval]*/ VARIANT_BOOL *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::ShowPathesDialog(LONG_PTR hParentWnd) { return E_NOTIMPL; }

STDMETHODIMP VPinMAMEController::get_Hidden(VARIANT_BOOL *pVal)
{
   *pVal = VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP VPinMAMEController::put_Hidden(VARIANT_BOOL newVal)
{
   return S_OK;
}

STDMETHODIMP VPinMAMEController::get_Game(IGame **pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_MechSamples(VARIANT_BOOL *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::put_MechSamples(VARIANT_BOOL newVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::GetWindowRect(LONG_PTR hWnd, VARIANT *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::GetClientRect(LONG_PTR hWnd, VARIANT *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_MasterVolume(int *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::put_MasterVolume(int newVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_EnumAudioDevices(int *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_AudioDevicesCount(int *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_AudioDeviceDescription(int num, BSTR *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_AudioDeviceModule(int num, BSTR *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_CurrentAudioDevice(int *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::put_CurrentAudioDevice(int newVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_FastFrames(int *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::put_FastFrames(int newVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_IgnoreRomCrc(VARIANT_BOOL *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::put_IgnoreRomCrc(VARIANT_BOOL newVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_SoundMode(int *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::put_SoundMode(int newVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_CabinetMode(VARIANT_BOOL *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::put_CabinetMode(VARIANT_BOOL newVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_ShowPinDMD(VARIANT_BOOL *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::put_ShowPinDMD(VARIANT_BOOL newVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::get_ShowWinDMD(VARIANT_BOOL *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEController::put_ShowWinDMD(VARIANT_BOOL newVal) { return E_NOTIMPL; }