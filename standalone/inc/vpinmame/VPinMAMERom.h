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

class VPinMAMERom :
   public IDispatchImpl<IRom, &IID_IRom, &LIBID_VPinMAMELib>,
   public CComObjectRoot,
   public CComCoClass<VPinMAMERom, &CLSID_Rom>
{
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);

public:
   VPinMAMERom();
   ~VPinMAMERom();

   BEGIN_COM_MAP(VPinMAMERom)
      COM_INTERFACE_ENTRY(IRom)
      COM_INTERFACE_ENTRY(IDispatch)
   END_COM_MAP()

   STDMETHOD(Audit)(/*[in]*/ VARIANT_BOOL fStrict);
   STDMETHOD(get_StateDescription)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_State)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(get_Name)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_Flags)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(get_ExpChecksum)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(get_Checksum)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(get_ExpLength)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(get_Length)(/*[out, retval]*/ LONG *pVal);
};