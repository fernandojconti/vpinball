#pragma once

#include "FlexDMDBaseActor.h"

class FlexDMDGroupActor :
   public IDispatchImpl<IGroupActor, &IID_IGroupActor, &LIBID_FlexDMD>,
   public CComObjectRoot,
   public CComCoClass<FlexDMDGroupActor, &CLSID_FlexDMD>,
   public FlexDMDBaseActor
{
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);
   STDMETHOD(GetDocumentation)(INT index, BSTR *pBstrName, BSTR *pBstrDocString, DWORD *pdwHelpContext, BSTR *pBstrHelpFile);

public:
   FlexDMDGroupActor();
   ~FlexDMDGroupActor();

   BEGIN_COM_MAP(FlexDMDGroupActor)
      COM_INTERFACE_ENTRY(IGroupActor)
      COM_INTERFACE_ENTRY(IDispatch)
   END_COM_MAP()

   STDMETHOD(get_Name)(BSTR *pRetVal) { return FlexDMDBaseActor::get_Name(pRetVal); }
   STDMETHOD(put_Name)(BSTR pRetVal) { return FlexDMDBaseActor::put_Name(pRetVal); };
   STDMETHOD(get_x)(single *pRetVal) { return FlexDMDBaseActor::get_x(pRetVal); };
   STDMETHOD(put_x)(single pRetVal) { return FlexDMDBaseActor::put_x(pRetVal); };
   STDMETHOD(get_y)(single *pRetVal) { return FlexDMDBaseActor::get_y(pRetVal); };
   STDMETHOD(put_y)(single pRetVal) { return FlexDMDBaseActor::put_y(pRetVal); };
   STDMETHOD(get_Width)(single *pRetVal) { return FlexDMDBaseActor::get_Width(pRetVal); };
   STDMETHOD(put_Width)(single pRetVal) { return FlexDMDBaseActor::put_Width(pRetVal); };
   STDMETHOD(get_Height)(single *pRetVal) { return FlexDMDBaseActor::get_Height(pRetVal); };
   STDMETHOD(put_Height)(single pRetVal) { return FlexDMDBaseActor::put_Height(pRetVal); };
   STDMETHOD(get_visible)(VARIANT_BOOL *pRetVal) { return FlexDMDBaseActor::get_visible(pRetVal); };
   STDMETHOD(put_visible)(VARIANT_BOOL pRetVal) { return FlexDMDBaseActor::put_visible(pRetVal); };
   STDMETHOD(get_FillParent)(VARIANT_BOOL *pRetVal) { return FlexDMDBaseActor::get_FillParent(pRetVal); };
   STDMETHOD(put_FillParent)(VARIANT_BOOL pRetVal) { return FlexDMDBaseActor::put_FillParent(pRetVal); };
   STDMETHOD(get_ClearBackground)(VARIANT_BOOL *pRetVal) { return FlexDMDBaseActor::get_ClearBackground(pRetVal); };
   STDMETHOD(put_ClearBackground)(VARIANT_BOOL pRetVal) { return FlexDMDBaseActor::put_ClearBackground(pRetVal); };
   STDMETHOD(SetBounds)(single x, single y, single Width, single Height) { return FlexDMDBaseActor::SetBounds(x, y, Width, Height); };
   STDMETHOD(SetPosition)(single x, single y) { return FlexDMDBaseActor::SetPosition(x, y); };
   STDMETHOD(SetAlignedPosition)(single x, single y, Alignment alignment) { return FlexDMDBaseActor::SetAlignedPosition(x, y, alignment); };
   STDMETHOD(SetSize)(single Width, single Height) { return FlexDMDBaseActor::SetSize(Width, Height); };
   STDMETHOD(get_PrefWidth)(single *pRetVal) { return FlexDMDBaseActor::get_PrefWidth(pRetVal); };
   STDMETHOD(get_PrefHeight)(single *pRetVal) { return FlexDMDBaseActor::get_PrefHeight(pRetVal); };
   STDMETHOD(Pack)() { return FlexDMDBaseActor::Pack(); };
   STDMETHOD(Remove)() { return FlexDMDBaseActor::Remove(); };
   STDMETHOD(get_ActionFactory)(IActionFactory **pRetVal) { return FlexDMDBaseActor::get_ActionFactory(pRetVal); };
   STDMETHOD(AddAction)(IUnknown *action) { return FlexDMDBaseActor::AddAction(action); };
   STDMETHOD(ClearActions)() { return FlexDMDBaseActor::ClearActions(); };

   STDMETHOD(get_Clip)(VARIANT_BOOL *pRetVal);
   STDMETHOD(put_Clip)(VARIANT_BOOL pRetVal);
   STDMETHOD(get_ChildCount)(LONG *pRetVal);
   STDMETHOD(HasChild)(BSTR Name, VARIANT_BOOL *pRetVal);
   STDMETHOD(GetGroup)(BSTR Name, IGroupActor **pRetVal);
   STDMETHOD(GetFrame)(BSTR Name, IFrameActor **pRetVal);
   STDMETHOD(GetLabel)(BSTR Name, ILabelActor **pRetVal);
   STDMETHOD(GetVideo)(BSTR Name, IVideoActor **pRetVal);
   STDMETHOD(GetImage)(BSTR Name, IImageActor **pRetVal);
   STDMETHOD(RemoveAll)();
   STDMETHOD(AddActor)(IUnknown *child);
   STDMETHOD(RemoveActor)(IUnknown *child);

   static CComObject<FlexDMDGroupActor>* Create(FlexDMD* pFlexDMD, string name);

   virtual void OnStageStateChanged();
   virtual void Update(float delta);
   virtual void Draw(SDL_Surface* graphics);

private:
   FlexDMDGroupActor* GetRoot();
   FlexDMDBaseActor* Get(string name);

   FlexDMD* m_pFlexDMD;
   bool m_clip;
   vector<FlexDMDBaseActor*> m_children;
};