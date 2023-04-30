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

class VPinMAMERoms :
   public IDispatchImpl<IRoms, &IID_IRoms, &LIBID_VPinMAMELib>,
   public CComObjectRoot,
   public CComCoClass<VPinMAMERoms, &CLSID_Roms>
{
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);

public:
   VPinMAMERoms();
   ~VPinMAMERoms();

   BEGIN_COM_MAP(VPinMAMERoms)
      COM_INTERFACE_ENTRY(IRoms)
      COM_INTERFACE_ENTRY(IDispatch)
   END_COM_MAP()

   STDMETHOD(Audit)(/*[in]*/ VARIANT_BOOL fStrict);
   STDMETHOD(get_StateDescription)(/*[out, retval]*/ BSTR *pVal);
   STDMETHOD(get_State)(/*[out, retval]*/ LONG *pVal);
   STDMETHOD(get_Available)(/*[out, retval]*/ VARIANT_BOOL *pVal);
   STDMETHOD(get_Count)(LONG* pnCount);
   STDMETHOD(get_Item)(VARIANT *pKey, IRom **pRom);
   STDMETHOD(get__NewEnum)(IUnknown** ppunkEnum);
};
