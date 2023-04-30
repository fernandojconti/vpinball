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

class VPinMAMEGame :
   public IDispatchImpl<IGame, &IID_IGame, &LIBID_VPinMAMELib>,
   public CComObjectRoot,
   public CComCoClass<VPinMAMEGame, &CLSID_Game>
{
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);

public:
   VPinMAMEGame();
   ~VPinMAMEGame();

   BEGIN_COM_MAP(VPinMAMEGame)
      COM_INTERFACE_ENTRY(IGame)
      COM_INTERFACE_ENTRY(IDispatch)
   END_COM_MAP()

   STDMETHOD(get_IsSupported)(/* [out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(get_Name)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_Manufacturer)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_Year)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_Description)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_CloneOf)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_Roms)(/*[out, retval]*/ IRoms* *pVal);
   STDMETHOD(ShowInfoDlg)(/*[in,defaultvalue(0)]*/ int nShowOptions, /*[in,defaultvalue(0)]*/ LONG_PTR hParentWnd, /*[out, retval]*/ int *pVal);
   STDMETHOD(get_Settings)(/*[out, retval]*/ IGameSettings * *pVal);

   HRESULT Init(PinmameGame* pPinmameGame);

private:
   PinmameGame m_pinmameGame;
};