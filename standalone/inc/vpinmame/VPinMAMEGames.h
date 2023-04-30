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

#include "vpinmame_i.h"

class VPinMAMEGame;

class VPinMAMEGames :
   public IDispatchImpl<IGames, &IID_IGames, &LIBID_VPinMAMELib>,
   public CComObjectRoot,
   public CComCoClass<VPinMAMEGames, &CLSID_Games>
{
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);

public:
   VPinMAMEGames();
   ~VPinMAMEGames();

   BEGIN_COM_MAP(VPinMAMEGames)
      COM_INTERFACE_ENTRY(IGames)
      COM_INTERFACE_ENTRY(IDispatch)
   END_COM_MAP()

   STDMETHOD(get_Count)(LONG* pnCount);
   STDMETHOD(get_Item)(VARIANT *pKey, IGame **pGame);
   STDMETHOD(get__NewEnum)(IUnknown** ppunkEnum);

   static void CALLBACK GetGameCallback(PinmameGame* pPinmameGame, const void* pUserData);

private:
   PinmameGame m_pinmameGame;
};
