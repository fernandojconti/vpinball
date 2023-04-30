#pragma once

#include "flexdmd_i.h"

class FlexDMDCompositeAction :
   public IDispatchImpl<ICompositeAction, &IID_ICompositeAction, &LIBID_FlexDMD>,
   public CComObjectRoot,
   public CComCoClass<FlexDMDCompositeAction, &CLSID_FlexDMD>
{
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);

public:
   FlexDMDCompositeAction();
   ~FlexDMDCompositeAction();

   BEGIN_COM_MAP(FlexDMDCompositeAction)
      COM_INTERFACE_ENTRY(ICompositeAction)
      COM_INTERFACE_ENTRY(IDispatch)
   END_COM_MAP()

   STDMETHOD(Add)(IUnknown *action, ICompositeAction **pRetVal);
};
