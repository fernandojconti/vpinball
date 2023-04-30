#include "stdafx.h"
#include "VPinMAMEGameSettings.h"

VPinMAMEGameSettings::VPinMAMEGameSettings()
{
    m_pGame = nullptr;
}

VPinMAMEGameSettings::~VPinMAMEGameSettings()
{
   if (m_pGame)
      m_pGame->Release();
}

void VPinMAMEGameSettings::Init(IGame* pGame)
{
   if (!pGame)
      return;

   m_pGame = pGame;
   m_pGame->AddRef();

   CComBSTR sROM;
   m_pGame->get_Name(&sROM);
   WideCharToMultiByte(CP_ACP, 0, sROM, -1, m_szROM, sizeof m_szROM, NULL, NULL);
}

STDMETHODIMP VPinMAMEGameSettings::ShowSettingsDlg(LONG_PTR hParentWnd) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMEGameSettings::Clear() { return E_NOTIMPL; }

STDMETHODIMP VPinMAMEGameSettings::get_Value(BSTR sName, VARIANT *pVal)
{
   char szName[MAXSTRING];
   WideCharToMultiByteNull(CP_ACP, 0, sName, -1, szName, 256, nullptr, nullptr);

   V_VT(pVal) = VT_BSTR;
   V_BSTR(pVal) = ::SysAllocString(L"");

   PLOGI.printf("name=%s, value=%s", szName, "");

   return S_OK;
}

STDMETHODIMP VPinMAMEGameSettings::put_Value(BSTR sName, VARIANT newVal)
{
   char szName[MAXSTRING];
   WideCharToMultiByteNull(CP_ACP, 0, sName, -1, szName, 256, nullptr, nullptr);
 
   VARIANT var0;
   V_VT(&var0) = VT_EMPTY;
   VariantChangeType(&var0, &newVal, 0, VT_BSTR);

   char szValue[MAXSTRING * 4];
   WideCharToMultiByte(CP_ACP, 0, V_BSTR(&var0), -1, szValue, sizeof(szValue), NULL, NULL);

   PLOGI.printf("name=%s, value=%s", szName, szValue);

   VariantClear(&var0);

   return S_OK;
}

STDMETHODIMP VPinMAMEGameSettings::SetDisplayPosition(VARIANT newValX, VARIANT newValY, LONG_PTR hWnd) { return E_NOTIMPL; }
