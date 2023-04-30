#include "stdafx.h"
#ifndef __STANDALONE__
#include "Intshcut.h"
#endif

#ifdef __STANDALONE__
#include "standalone/PoleStorage.h"
#endif

unsigned long long tinymt64state[2] = { 'T', 'M' };


float sz2f(const string& sz)
{
   const int len = (int)sz.length()+1;
   WCHAR * const wzT = new WCHAR[len];
   MultiByteToWideCharNull(CP_ACP, 0, sz.c_str(), -1, wzT, len);

   CComVariant var = wzT;

   float result;
   if (SUCCEEDED(VariantChangeType(&var, &var, 0, VT_R4)))
   {
      result = V_R4(&var);
      VariantClear(&var);
   }
   else
      result = 0.0f; //!! use inf or NaN instead?

   delete[] wzT;

   return result;
}

string f2sz(const float f)
{
   CComVariant var = f;

   if (SUCCEEDED(VariantChangeType(&var, &var, 0, VT_BSTR)))
   {
      const WCHAR * const wzT = V_BSTR(&var);
      char tmp[256];
      WideCharToMultiByteNull(CP_ACP, 0, wzT, -1, tmp, 256, nullptr, nullptr);
      VariantClear(&var);
      return tmp;
   }
   else
      return "0.0"s; //!! must this be somehow localized, i.e. . vs ,
}

void WideStrNCopy(const WCHAR *wzin, WCHAR *wzout, const DWORD wzoutMaxLen)
{
   DWORD i = 0;
   while (*wzin && (++i < wzoutMaxLen)) { *wzout++ = *wzin++; }
   *wzout = 0;
}

void WideStrCat(const WCHAR *wzin, WCHAR *wzout, const DWORD wzoutMaxLen)
{
   DWORD i = lstrlenW(wzout);
   wzout += i;
   while (*wzin && (++i < wzoutMaxLen)) { *wzout++ = *wzin++; }
   *wzout = 0;
}

int WideStrCmp(const WCHAR *wz1, const WCHAR *wz2)
{
   while (*wz1 != L'\0')
   {
      if (*wz1 != *wz2)
      {
         if (*wz1 > *wz2)
            return 1; // If *wz2 == 0, then wz1 will return as higher, which is correct
         else if (*wz1 < *wz2)
            return -1;
      }
      wz1++;
      wz2++;
   }
   if (*wz2 != L'\0')
      return -1; // wz2 is longer - and therefore higher
   return 0;
}

int WzSzStrCmp(const WCHAR *wz1, const char *sz2)
{
   while (*wz1 != L'\0')
      if (*wz1++ != *sz2++)
         return 1;
   if (*sz2 != L'\0')
      return 1;
   return 0;
}

int WzSzStrNCmp(const WCHAR *wz1, const char *sz2, const DWORD maxComparisonLen)
{
   DWORD i = 0;

   while (*wz1 != L'\0' && i < maxComparisonLen)
   {
      if (*wz1++ != *sz2++)
         return 1;
      i++;
   }
   if (*sz2 != L'\0')
      return 1;
   return 0;
}

LocalString::LocalString(const int resid)
{
#ifndef __STANDALONE__
   if (resid > 0)
      /*const int cchar =*/ LoadString(g_pvp->theInstance, resid, m_szbuffer, sizeof(m_szbuffer));
#else
   static robin_hood::unordered_map<int, const char*> ids_map = {
     { IDS_SCRIPT, "Script" },
     { IDS_TB_BUMPER, "Bumper" },
     { IDS_TB_DECAL, "Decal" },
     { IDS_TB_DISPREEL, "EMReel" },
     { IDS_TB_FLASHER, "Flasher" },
     { IDS_TB_FLIPPER, "Flipper" },
     { IDS_TB_GATE, "Gate" },
     { IDS_TB_KICKER, "Kicker" },
     { IDS_TB_LIGHT, "Light" },
     { IDS_TB_LIGHTSEQ, "LightSeq" },
     { IDS_TB_PLUNGER, "Plunger" },
     { IDS_TB_PRIMITIVE, "Primitive" },
     { IDS_TB_WALL, "Wall" },
     { IDS_TB_RAMP, "Ramp" },
     { IDS_TB_RUBBER, "Rubber" },
     { IDS_TB_SPINNER, "Spinner" },
     { IDS_TB_TEXTBOX, "TextBox" },
     { IDS_TB_TIMER, "Timer" },
     { IDS_TB_TRIGGER, "Trigger" },
     { IDS_TB_TARGET, "Target" }
   };
   const robin_hood::unordered_map<int, const char*>::iterator it = ids_map.find(resid);
   if (it != ids_map.end())
   {
      const char* sz = it->second;
      strncpy(m_szbuffer, sz, sizeof(m_szbuffer) - 1);
   }
#endif
}

LocalStringW::LocalStringW(const int resid)
{
#ifndef __STANDALONE__
   if (resid > 0)
      LoadStringW(g_pvp->theInstance, resid, m_szbuffer, sizeof(m_szbuffer)/sizeof(WCHAR));
#else
   static robin_hood::unordered_map<int, const char*> ids_map = {
     { IDS_SCRIPT, "Script" },
     { IDS_TB_BUMPER, "Bumper" },
     { IDS_TB_DECAL, "Decal" },
     { IDS_TB_DISPREEL, "EMReel" },
     { IDS_TB_FLASHER, "Flasher" },
     { IDS_TB_FLIPPER, "Flipper" },
     { IDS_TB_GATE, "Gate" },
     { IDS_TB_KICKER, "Kicker" },
     { IDS_TB_LIGHT, "Light" },
     { IDS_TB_LIGHTSEQ, "LightSeq" },
     { IDS_TB_PLUNGER, "Plunger" },
     { IDS_TB_PRIMITIVE, "Primitive" },
     { IDS_TB_WALL, "Wall" },
     { IDS_TB_RAMP, "Ramp" },
     { IDS_TB_RUBBER, "Rubber" },
     { IDS_TB_SPINNER, "Spinner" },
     { IDS_TB_TEXTBOX, "TextBox" },
     { IDS_TB_TIMER, "Timer" },
     { IDS_TB_TRIGGER, "Trigger" },
     { IDS_TB_TARGET, "Target" }
   };
   const robin_hood::unordered_map<int, const char*>::iterator it = ids_map.find(resid);
   if (it != ids_map.end())
   {
      const char* sz = it->second;
      const int len = strlen(sz)+1;
      MultiByteToWideCharNull(CP_ACP, 0, sz, -1, m_szbuffer, len);
   }
#endif
}

WCHAR *MakeWide(const string& sz)
{
   const int len = (int)sz.length()+1;
   WCHAR * const wzT = new WCHAR[len];
   MultiByteToWideCharNull(CP_ACP, 0, sz.c_str(), -1, wzT, len);

   return wzT;
}

char *MakeChar(const WCHAR * const wz)
{
   const int len = lstrlenW(wz);
   char * const szT = new char[len + 1];
   WideCharToMultiByteNull(CP_ACP, 0, wz, -1, szT, len + 1, nullptr, nullptr);

   return szT;
}

string MakeString(const WCHAR * const wz)
{
   char* szT = MakeChar(wz);
   string sz = szT;
   delete[] szT;

   return sz;
}

HRESULT OpenURL(const string& szURL)
{
#ifndef __STANDALONE__
   IUniformResourceLocator* pURL;

   HRESULT hres = CoCreateInstance(CLSID_InternetShortcut, nullptr, CLSCTX_INPROC_SERVER, IID_IUniformResourceLocator, (void**)&pURL);
   if (!SUCCEEDED(hres))
   {
      return hres;
   }

   hres = pURL->SetURL(szURL.c_str(), IURL_SETURL_FL_GUESS_PROTOCOL);

   if (!SUCCEEDED(hres))
   {
      pURL->Release();
      return hres;
   }

   //Open the URL by calling InvokeCommand
   URLINVOKECOMMANDINFO ivci;
   ivci.dwcbSize = sizeof(URLINVOKECOMMANDINFO);
   ivci.dwFlags = IURL_INVOKECOMMAND_FL_ALLOW_UI;
   ivci.hwndParent = g_pvp->GetHwnd();
   ivci.pcszVerb = "open";
   hres = pURL->InvokeCommand(&ivci);
   pURL->Release();
   return (hres);
#else
   return 0L;
#endif
}

char* replace(const char* const original, const char* const pattern, const char* const replacement)
{
  const size_t replen = strlen(replacement);
  const size_t patlen = strlen(pattern);
  const size_t orilen = strlen(original);

  size_t patcnt = 0;
  const char * patloc;

  // find how many times the pattern occurs in the original string
  for (const char* oriptr = original; (patloc = strstr(oriptr, pattern)); oriptr = patloc + patlen)
    patcnt++;

  {
    // allocate memory for the new string
    const size_t retlen = orilen + patcnt * (replen - patlen);
    char * const returned = new char[retlen + 1];

    //if (returned != nullptr)
    {
      // copy the original string, 
      // replacing all the instances of the pattern
      char * retptr = returned;
      const char* oriptr;
      for (oriptr = original; (patloc = strstr(oriptr, pattern)); oriptr = patloc + patlen)
      {
        const size_t skplen = patloc - oriptr;
        // copy the section until the occurence of the pattern
        strncpy(retptr, oriptr, skplen);
        retptr += skplen;
        // copy the replacement 
        strncpy(retptr, replacement, replen);
        retptr += replen;
      }
      // copy the rest of the string.
      strcpy(retptr, oriptr);
    }
    return returned;
  }
}

// Helper function for IsOnWine
//
// This exists such that we only check if we're on wine once, and assign the result of this function to a static const var
static bool IsOnWineInternal()
{
#ifndef __STANDALONE__
   // See https://www.winehq.org/pipermail/wine-devel/2008-September/069387.html
   const HMODULE ntdllHandle = GetModuleHandleW(L"ntdll.dll");
   assert(ntdllHandle != nullptr && "Could not GetModuleHandleW(L\"ntdll.dll\")");
   return GetProcAddress(ntdllHandle, "wine_get_version") != nullptr;
#else
   return false;
#endif
}

bool IsOnWine()
{
   static const bool result = IsOnWineInternal();
   return result;
}

#ifdef __STANDALONE__

float calc_brightness(float x)
{
   // function to improve the brightness with fx=ax²+bc+c, f(0)=0, f(1)=1, f'(1.1)=0
   return (-x * x + 2.1f * x) / 1.1f;
}

const char* glToString(GLuint value)
{
   static robin_hood::unordered_map<GLuint, const char*> value_map = {
     { (GLuint)GL_RGB, "GL_RGB" },
     { (GLuint)GL_RGBA, "GL_RGBA" },
     { (GLuint)GL_RGB8, "GL_RGB8" },
     { (GLuint)GL_RGBA8, "GL_RGBA8" },
     { (GLuint)GL_SRGB8, "GL_SRGB8" },
     { (GLuint)GL_SRGB8_ALPHA8, "GL_SRGB8_ALPHA8" },
     { (GLuint)GL_RGB16F, "GL_RGB16F" },
     { (GLuint)GL_UNSIGNED_BYTE, "GL_UNSIGNED_BYTE" },
     { (GLuint)GL_HALF_FLOAT, "GL_HALF_FLOAT" },
   };

   const robin_hood::unordered_map<GLuint, const char*>::iterator it = value_map.find(value);
   if (it != value_map.end()) {
      return it->second;
   }
   return (const char*)"Unknown";
}

HRESULT external_open_storage(const OLECHAR* pwcsName, IStorage* pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage** ppstgOpen)
{
   char szName[1024];
   WideCharToMultiByte(CP_ACP, 0, pwcsName, -1, szName, sizeof(szName), NULL, NULL);

   return PoleStorage::Create(szName, "/", (IStorage**)ppstgOpen);
}

#include "standalone/inc/vpinmame/VPinMAMEController.h"
#include "standalone/inc/wmp/WMPCore.h"
#include "standalone/inc/flexdmd/FlexDMD.h"
#include "standalone/inc/pup/PinUpPlayerPinDisplay.h"

HRESULT external_create_object(const WCHAR* progid, IClassFactory* cf, IUnknown* obj)
{
   HRESULT hres = E_NOTIMPL;

   if (!wcsicmp(progid, L"VPinMAME.Controller")) {
      CComObject<VPinMAMEController>* pObj = nullptr;
      if (SUCCEEDED(CComObject<VPinMAMEController>::CreateInstance(&pObj))) {
         hres = pObj->QueryInterface(IID_IController, (void**)obj);
      }
   }
   else if (!wcsicmp(progid, L"WMPlayer.OCX")) {
      CComObject<WMPCore>* pObj = nullptr;
      if (SUCCEEDED(CComObject<WMPCore>::CreateInstance(&pObj))) {
         hres = pObj->QueryInterface(IID_IWMPCore, (void**)obj);
      }
   }
   else if (!wcsicmp(progid, L"FlexDMD.FlexDMD")) {
      CComObject<FlexDMD>* pObj = nullptr;
      if (SUCCEEDED(CComObject<FlexDMD>::CreateInstance(&pObj))) {
         hres = pObj->QueryInterface(IID_IFlexDMD, (void**)obj);
      }
   }
   else if (!wcsicmp(progid, L"PinUpPlayer.PinDisplay")) {
      CComObject<PinUpPlayerPinDisplay>* pObj = nullptr;
      if (SUCCEEDED(CComObject<PinUpPlayerPinDisplay>::CreateInstance(&pObj))) {
         hres = pObj->QueryInterface(IID_IPinDisplay, (void**)obj);
      }
   }
   else if (!wcsicmp(progid, L"Shell.Application")) {
   }
   else if (!wcsicmp(progid, L"WScript.Shell")) {
   }
   else if (!wcsicmp(progid, L"B2S.Server")) {
   }
   else if (!wcsicmp(progid, L"VPROC.Controller")) {
   }
   else if (!wcsicmp(progid, L"UltraDMD.DMDObject")) {
   }
   else if (!wcsicmp(progid, L"PUPDMDControl.DMD")) {
   }

   const char* const szT = MakeChar(progid);
   PLOGI.printf("progid=%s, hres=0x%08x", szT, hres);
   delete[] szT;

   return hres;
}

void external_log_info(const char* format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    PLOGI << buffer;
}

void external_log_debug(const char* format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    PLOGD << buffer;
}
#endif