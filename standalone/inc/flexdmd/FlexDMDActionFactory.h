#pragma once

#include "flexdmd_i.h"

class FlexDMDCompositeAction;

class FlexDMDActionFactory :
   public IDispatchImpl<IActionFactory, &IID_IActionFactory, &LIBID_FlexDMD>,
   public CComObjectRoot,
   public CComCoClass<FlexDMDActionFactory, &CLSID_FlexDMD>
{
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);

public:
   FlexDMDActionFactory();
   ~FlexDMDActionFactory();

   BEGIN_COM_MAP(FlexDMDActionFactory)
      COM_INTERFACE_ENTRY(IActionFactory)
      COM_INTERFACE_ENTRY(IDispatch)
   END_COM_MAP()

   STDMETHOD(Wait)(single secondsToWait, IUnknown **pRetVal);
   STDMETHOD(Delayed)(single secondsToWait, IUnknown *action, IUnknown **pRetVal);
   STDMETHOD(Parallel)(ICompositeAction **pRetVal);
   STDMETHOD(Sequence)(ICompositeAction **pRetVal);
   STDMETHOD(Repeat)(IUnknown *action, LONG count, IUnknown **pRetVal);
   STDMETHOD(Blink)(single secondsShow, single secondsHide, LONG Repeat, IUnknown **pRetVal);
   STDMETHOD(Show)(VARIANT_BOOL visible, IUnknown **pRetVal);
   STDMETHOD(AddTo)(IGroupActor *parent, IUnknown **pRetVal);
   STDMETHOD(RemoveFromParent)(IUnknown **pRetVal);
   STDMETHOD(AddChild)(IUnknown *child, IUnknown **pRetVal);
   STDMETHOD(RemoveChild)(IUnknown *child, IUnknown **pRetVal);
   STDMETHOD(Seek)(single pos, IUnknown **pRetVal);
   STDMETHOD(MoveTo)(single x, single y, single duration, ITweenAction **pRetVal);

private:
   CComObject<FlexDMDCompositeAction>* m_pCompositeAction;
};