#include "stdafx.h"
#include "FlexDMDVideoActor.h"
#include "FlexDMDActionFactory.h"

FlexDMDVideoActor::FlexDMDVideoActor()
{
    m_visible = true;
}

FlexDMDVideoActor::~FlexDMDVideoActor()
{
}

void FlexDMDVideoActor::Missing34() { };
void FlexDMDVideoActor::Missing35() { };
void FlexDMDVideoActor::Missing36() { };
void FlexDMDVideoActor::Missing37() { };
STDMETHODIMP FlexDMDVideoActor::get_Length(single *pRetVal) { return E_NOTIMPL; }
STDMETHODIMP FlexDMDVideoActor::get_Loop(VARIANT_BOOL *pRetVal) { return E_NOTIMPL; }
STDMETHODIMP FlexDMDVideoActor::put_Loop(VARIANT_BOOL pRetVal) { return E_NOTIMPL; }
STDMETHODIMP FlexDMDVideoActor::get_Paused(VARIANT_BOOL *pRetVal) { return E_NOTIMPL; }
STDMETHODIMP FlexDMDVideoActor::put_Paused(VARIANT_BOOL pRetVal) { return E_NOTIMPL; }
STDMETHODIMP FlexDMDVideoActor::get_PlaySpeed(single *pRetVal) { return E_NOTIMPL; }
STDMETHODIMP FlexDMDVideoActor::put_PlaySpeed(single pRetVal) { return E_NOTIMPL; }
STDMETHODIMP FlexDMDVideoActor::Seek(single posInSeconds) { return E_NOTIMPL; }

void FlexDMDVideoActor::Draw(SDL_Surface* pSurface)
{
}

void FlexDMDVideoActor::ReadNextFrame()
{
}