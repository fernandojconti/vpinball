#pragma once

#pragma push_macro("_WIN64")
#pragma push_macro("DELETE")
#pragma push_macro("CALLBACK")
#undef _WIN64
#undef DELETE
#undef CALLBACK
#include "libpinmame.h"
#pragma pop_macro("CALLBACK")
#pragma pop_macro("DELETE")
#pragma pop_macro("_WIN64")

#include <map>
#include <mutex>

#include "vpinmame_i.h"

#if !((defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__))
#include "../zedmd/ZeDMD.h"
#endif

class VPinMAMEGames;

class VPinMAMEController :
   public IDispatchImpl<IController, &IID_IController, &LIBID_VPinMAMELib>,
   public CComObjectRoot,
   public CComCoClass<VPinMAMEController, &CLSID_Controller>
{
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);

public:
   static constexpr UINT8 LEVELS_WPC[] = { 0x14, 0x21, 0x43, 0x64 };
   static constexpr UINT8 LEVELS_SAM[] = { 0x00, 0x14, 0x19, 0x1E, 0x23, 0x28, 0x2D, 0x32, 0x37, 0x3C, 0x41, 0x46, 0x4B, 0x50, 0x5A, 0x64 };
   static constexpr UINT8 LEVELS_GTS3[] = { 0x00, 0x1E, 0x23, 0x28, 0x2D, 0x32, 0x37, 0x3C, 0x41, 0x46, 0x4B, 0x50, 0x55, 0x5A, 0x5F, 0x64 };

   VPinMAMEController();
   ~VPinMAMEController();

   BEGIN_COM_MAP(VPinMAMEController)
      COM_INTERFACE_ENTRY(IController)
      COM_INTERFACE_ENTRY(IDispatch)
   END_COM_MAP()

   STDMETHOD(get_ChangedLEDs)(/*[in]*/ int nHigh, int nLow, int nnHigh, int nnLow, /*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(get_ChangedLEDsState)(/*[in]*/ int nHigh, int nLow, int nnHigh, int nnLow, int **buf, /*[out, retval]*/ int *pVal);
   STDMETHOD(get_Settings)(/*[out, retval]*/ IControllerSettings **pVal);
   STDMETHOD(get_Games)(/*[out, retval]*/ IGames **pVal);
   STDMETHOD(get_Version)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_SolMask)(/*[in]*/ int nLow, /*[out, retval]*/ LONG *pVal);
   STDMETHOD(put_SolMask)(/*[in]*/ int nLow, /*[in]*/ LONG newVal);
   STDMETHOD(put_Mech)(/*[in]*/ int param, /*[in]*/ int newVal);
   STDMETHOD(get_LockDisplay)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_LockDisplay)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_GetMech)(/*[in]*/ int mechNo, /*[out, retval]*/ int *pVal);
   STDMETHOD(get_GIStrings)(/*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(get_Dip)(/*[in]*/ int nNo, /*[out, retval]*/ int *pVal);
   STDMETHOD(put_Dip)(/*[in]*/ int nNo, /*[in]*/ int newVal);
   STDMETHOD(get_Solenoids)(/*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(get_SplashInfoLine)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_SplashInfoLine)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_ChangedGIStrings)(/*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(get_GIString)(int nString, /*[out, retval]*/ int *pVal);
   STDMETHOD(get_HandleMechanics)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_HandleMechanics)(/*[in]*/ int newVal);
   STDMETHOD(get_Running)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(get_Machines)(/*[in]*/ BSTR sMachine, /*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(get_Pause)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Pause)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_HandleKeyboard)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_HandleKeyboard)(/*[int]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_GameName)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(put_GameName)(/*[in]*/ BSTR newVal);
   STDMETHOD(get_ChangedSolenoids)(/*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(get_Switches)(/*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(put_Switches)(/*[out, retval]*/ VARIANT newVal);
   STDMETHOD(get_ChangedLamps)(/*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(get_Lamps)(/*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(get_WPCNumbering)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(get_Switch)(/*[in]*/ int nSwitchNo, /*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Switch)(/*[in]*/ int nSwitchNo, /*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Solenoid)(/*[in]*/ int nSolenoid, /*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(get_Lamp)(/*[in]*/ int nLamp, /*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(ShowAboutDialog)(/*[in]*/ LONG_PTR hParentWnd = 0);
   STDMETHOD(Stop)();
   STDMETHOD(Run)(/*[in,defaultvalue(0)]*/ LONG_PTR hParentWnd = 0, /*[in,defaultvalue(100)]*/ int nMinVersion = 0);
   STDMETHOD(get_Hidden)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_Hidden)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_MechSamples)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_MechSamples)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_Game)(/*[out, retval]*/ IGame **pVal);
   STDMETHOD(GetWindowRect)(/*[in,defaultvalue(0)]*/ LONG_PTR hWnd, /*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(GetClientRect)(/*[in,defaultvalue(0)]*/ LONG_PTR hWnd, /*[out, retval]*/ VARIANT *pVal);

   STDMETHOD(get_NVRAM)(/*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(get_ChangedNVRAM)(/*[out, retval]*/ VARIANT *pVal);

   STDMETHOD(get_RawDmdWidth)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_RawDmdHeight)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_RawDmdPixels)(/*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(get_RawDmdColoredPixels)(/*[out, retval]*/ VARIANT *pVal);

   STDMETHOD(get_DmdWidth)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_DmdHeight)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_DmdPixel)(/*[in]*/ int x, /*[in]*/ int y, /*[out, retval]*/ int *pVal);
   STDMETHOD(get_updateDmdPixels)(/*[in]*/ int **buf, /*[in]*/ int width, /*[in]*/ int height, /*[out, retval]*/ int *pVal);
   STDMETHOD(get_ChangedLampsState)(/*[in]*/ int **buf, /*[out, retval]*/ int *pVal);
   STDMETHOD(get_LampsState)(/*[in]*/ int **buf, /*[out, retval]*/ int *pVal);
   STDMETHOD(get_ChangedSolenoidsState)(/*[in]*/ int **buf, /*[out, retval]*/ int *pVal);
   STDMETHOD(get_SolenoidsState)(/*[in]*/ int **buf, /*[out, retval]*/ int *pVal);
   STDMETHOD(get_ChangedGIsState)(/*[in]*/ int **buf, /*[out, retval]*/ int *pVal);
   STDMETHOD(get_MasterVolume)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_MasterVolume)(/*[in]*/ int newVal);
   STDMETHOD(get_EnumAudioDevices)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_AudioDevicesCount)(/*[out, retval]*/ int *pVal);
   STDMETHOD(get_AudioDeviceDescription)(/*[in]*/ int num, /*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_AudioDeviceModule)(/*[in]*/ int num, /*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_CurrentAudioDevice)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_CurrentAudioDevice)(/*[in]*/ int num);

   STDMETHOD(get_ShowPinDMD)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ShowPinDMD)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(get_ShowWinDMD)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ShowWinDMD)(/*[in]*/ VARIANT_BOOL newVal);

   /* deprecated methods/properties */
   STDMETHOD(get_NewSoundCommands)(/*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(ShowPathesDialog)(/*[in,defaultvalue(0)]*/ LONG_PTR hParentWnd);
   STDMETHOD(SetDisplayPosition)(/*[in]*/ int x, /*[in]*/ int y, /*[in]*/ LONG_PTR hParentWindow);
   STDMETHOD(get_DoubleSize)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_DoubleSize)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_ShowFrame)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ShowFrame)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_ShowDMDOnly)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ShowDMDOnly)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(CheckROMS)(/*[in,defaultvalue(0)]*/ int nShowOptions, /*[in,defaultvalue(0)]*/ LONG_PTR hParentWnd, /*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(get_ShowTitle)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_ShowTitle)(/*[in]*/ VARIANT_BOOL newpVal);
   STDMETHOD(ShowOptsDialog)(/*[in]*/ LONG_PTR hParentWnd = 0);
   STDMETHOD(get_FastFrames)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_FastFrames)(/*[in]*/ int newVal);
   STDMETHOD(get_IgnoreRomCrc)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_IgnoreRomCrc)(/*[in]*/ VARIANT_BOOL newVal);
   STDMETHOD(get_CabinetMode)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(put_CabinetMode)(/*[in]*/ VARIANT_BOOL newVal);

   STDMETHOD(get_SoundMode)(/*[out, retval]*/ int *pVal);
   STDMETHOD(put_SoundMode)(/*[in]*/ int newVal);

   STDMETHOD(get_ROMName)(/*[out, retval]*/ BSTR *pVal);

   static void CALLBACK GetGameCallback(PinmameGame* pPinmameGame, const void* pUserData);
   static void CALLBACK OnStateUpdated(int state, const void* pUserData);
   static void CALLBACK OnDisplayAvailable(int index, int displayCount, PinmameDisplayLayout* p_displayLayout, const void* pUserData);
   static void CALLBACK OnDisplayUpdated(int index, void* p_displayData, PinmameDisplayLayout* p_displayLayout, const void* pUserData);
   static int CALLBACK OnAudioAvailable(PinmameAudioInfo* p_audioInfo, const void* pUserData);
   static int CALLBACK OnAudioUpdated(void* p_buffer, int samples, const void* pUserData);
   static void CALLBACK OnMechAvailable(int mechNo, PinmameMechInfo* p_mechInfo, const void* pUserData);
   static void CALLBACK OnMechUpdated(int mechNo, PinmameMechInfo* p_mechInfo, const void* pUserData);
   static void CALLBACK OnConsoleDataUpdated(void* p_data, int size, const void* pUserData);
   static int CALLBACK IsKeyPressed(PINMAME_KEYCODE keycode, const void* pUserData);
   static void CALLBACK OnLogMessage(const char* format, va_list args, const void* pUserData);

private:
   string m_szPath;
   char m_szSplashInfoLine[MAXSTRING];

   PinmameGame* m_pPinmameGame;
   PinmameMechConfig* m_pPinmameMechConfig;

   PinmameSolenoidState* m_pSolenoidBuffer;
   PinmameLampState* m_pLampBuffer;
   PinmameGIState* m_pGIBuffer;
   PinmameLEDState* m_pLEDBuffer;
  
   int m_dmdDepth;
   int m_dmdWidth;
   int m_dmdHeight;
   OLE_COLOR m_dmdColor;

   UINT8* m_pLevels;
   UINT8* m_pDmdData;
   std::mutex m_dmdMutex;
   bool m_hasDMDUpdates;

   unsigned int m_serumTrigger;
   UINT8 m_dmdPalette[64 * 3];
   UINT8 m_dmdRotations[24];
   bool m_hasSerum;

   bool m_hasRGBs;

   std::map<int, int> m_mechData;

   AudioPlayer* m_audio;
   int m_audioChannels;

   CComObject<VPinMAMEGames>* m_pGames;

#if !((defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__))
   ZeDMD m_zedmd;
#endif
};
