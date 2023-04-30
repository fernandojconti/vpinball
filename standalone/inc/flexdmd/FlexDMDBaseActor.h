#pragma once

#include "flexdmd_i.h"

#include "FlexDMD.h"
#include "FlexDMDActionFactory.h"
#include "Layout.h"

class FlexDMDBaseActor 
{
public:
   FlexDMDBaseActor();
   ~FlexDMDBaseActor();

   STDMETHOD(get_Name)(BSTR *pRetVal);
   STDMETHOD(put_Name)(BSTR pRetVal);
   STDMETHOD(get_x)(single *pRetVal);
   STDMETHOD(put_x)(single pRetVal);
   STDMETHOD(get_y)(single *pRetVal);
   STDMETHOD(put_y)(single pRetVal);
   STDMETHOD(get_Width)(single *pRetVal);
   STDMETHOD(put_Width)(single pRetVal);
   STDMETHOD(get_Height)(single *pRetVal);
   STDMETHOD(put_Height)(single pRetVal);
   STDMETHOD(get_visible)(VARIANT_BOOL *pRetVal);
   STDMETHOD(put_visible)(VARIANT_BOOL pRetVal);
   STDMETHOD(get_FillParent)(VARIANT_BOOL *pRetVal);
   STDMETHOD(put_FillParent)(VARIANT_BOOL pRetVal);
   STDMETHOD(get_ClearBackground)(VARIANT_BOOL *pRetVal);
   STDMETHOD(put_ClearBackground)(VARIANT_BOOL pRetVal);
   STDMETHOD(SetBounds)(single x, single y, single Width, single Height);
   STDMETHOD(SetPosition)(single x, single y);
   STDMETHOD(SetAlignedPosition)(single x, single y, Alignment alignment);
   STDMETHOD(SetSize)(single Width, single Height);
   STDMETHOD(get_PrefWidth)(single *pRetVal);
   STDMETHOD(get_PrefHeight)(single *pRetVal);
   STDMETHOD(Pack)();
   STDMETHOD(Remove)();
   STDMETHOD(get_ActionFactory)(IActionFactory **pRetVal);
   STDMETHOD(AddAction)(IUnknown *action);
   STDMETHOD(ClearActions)();

   string GetName() { return m_szName; };
   void SetParent(FlexDMDGroupActor* parent) { m_parent = parent; };
   void SetOnStage(bool onStage) { m_onStage = onStage; this->OnStageStateChanged(); };

   virtual void OnStageStateChanged() {};
   virtual void Update(float secondsElapsed);
   virtual void Draw(SDL_Surface* graphics);

protected:
   bool m_onStage;

   string m_szName;
   single m_x;
   single m_y;
   single m_width;
   single m_height;
   FlexDMDGroupActor* m_parent;
   bool m_fillParent;
   bool m_clearBackground;
   single m_prefWidth;
   single m_prefHeight;
   bool m_visible;

   CComObject<FlexDMDActionFactory>* m_pActionFactory;
};