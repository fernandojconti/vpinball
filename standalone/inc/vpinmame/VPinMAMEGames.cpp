#include "stdafx.h"
#include "VPinMAMEGames.h"
#include "VPinMAMEGame.h"

void CALLBACK VPinMAMEGames::GetGameCallback(PinmameGame* pPinmameGame, const void* pUserData)
{
   VPinMAMEGames* pGames = (VPinMAMEGames*)pUserData;

   memcpy(&pGames->m_pinmameGame, pPinmameGame, sizeof(PinmameGame));
}

VPinMAMEGames::VPinMAMEGames()
{
}

VPinMAMEGames::~VPinMAMEGames()
{
}

STDMETHODIMP VPinMAMEGames::get_Count(LONG* pnCount) { return E_NOTIMPL; }

STDMETHODIMP VPinMAMEGames::get_Item(VARIANT *pKey, IGame **pGame)
{
   if (V_VT(pKey) & VT_BSTR) {
      const WCHAR * const wzT = V_BSTR(pKey);
      char szName[256];
      WideCharToMultiByteNull(CP_ACP, 0, wzT, -1, szName, 256, nullptr, nullptr);

      if (PinmameGetGame(szName, &GetGameCallback, this) == PINMAME_STATUS::OK) {
         CComObject<VPinMAMEGame>* pObj;

         HRESULT hr = CComObject<VPinMAMEGame>::CreateInstance(&pObj);
         if (FAILED(hr))
            return hr;

         pObj->AddRef();
         pObj->Init(&m_pinmameGame);

         *pGame = pObj;

         return S_OK;
      }
   }

   return E_INVALIDARG;
}

STDMETHODIMP VPinMAMEGames::get__NewEnum(IUnknown** ppunkEnum) { return E_NOTIMPL; }
