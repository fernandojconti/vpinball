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

class VPinMAMEControllerSettings :
   public IDispatchImpl<IControllerSettings, &IID_IControllerSettings, &LIBID_VPinMAMELib>,
   public CComObjectRoot,
   public CComCoClass<VPinMAMEControllerSettings, &CLSID_ControllerSettings>
{
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);

public:
   VPinMAMEControllerSettings();
   ~VPinMAMEControllerSettings();

   BEGIN_COM_MAP(VPinMAMEControllerSettings)
      COM_INTERFACE_ENTRY(IControllerSettings)
      COM_INTERFACE_ENTRY(IDispatch)
   END_COM_MAP()

   STDMETHOD(get_InstallDir)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_Value)(/*[in]*/ BSTR sName, /*[out, retval]*/ VARIANT *pVal);
   STDMETHOD(put_Value)(/*[in]*/ BSTR sName, /*[in]*/ VARIANT newVal);
   STDMETHOD(Clear)();
   STDMETHOD(ShowSettingsDlg)(LONG_PTR hParentWnd);
};