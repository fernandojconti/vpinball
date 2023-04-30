#include "stdafx.h"
#include "VPinMAMERoms.h"

VPinMAMERoms::VPinMAMERoms()
{
}

VPinMAMERoms::~VPinMAMERoms()
{
}

STDMETHODIMP VPinMAMERoms::get_Count(LONG* pnCount) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMERoms::get_Item(VARIANT *pKey, IRom **pRom) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMERoms::get__NewEnum(IUnknown** ppunkEnum) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMERoms::get_Available(VARIANT_BOOL *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMERoms::get_State(LONG *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMERoms::get_StateDescription(BSTR *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMERoms::Audit(VARIANT_BOOL fStrict) { return E_NOTIMPL; }