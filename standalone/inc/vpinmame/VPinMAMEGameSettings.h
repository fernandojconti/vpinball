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

class VPinMAMEGameSettings :
   public IDispatchImpl<IGameSettings, &IID_IGameSettings, &LIBID_VPinMAMELib>,
   public CComObjectRoot,
   public CComCoClass<VPinMAMEGameSettings, &CLSID_GameSettings>
{
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);

public:
   VPinMAMEGameSettings();
   ~VPinMAMEGameSettings();

   BEGIN_COM_MAP(VPinMAMEGameSettings)
      COM_INTERFACE_ENTRY(IGameSettings)
      COM_INTERFACE_ENTRY(IDispatch)
   END_COM_MAP()

   STDMETHOD(get_Value)(/*[in]*/ BSTR sName, /*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(put_Value)(/*[in]*/ BSTR sName, /*[in]*/ VARIANT newVal);
   STDMETHOD(Clear)();
   STDMETHOD(ShowSettingsDlg)(/* [in,defaultvalue(0)] */ LONG_PTR hParentWnd);
   STDMETHOD(SetDisplayPosition)(/*[in]*/ VARIANT newValX, /*[in]*/ VARIANT newValY,/* [in,defaultvalue(0)] */ LONG_PTR hWnd);

   void Init(IGame* pGame);

private:
   IGame* m_pGame;
   char m_szROM[MAXSTRING];
};
