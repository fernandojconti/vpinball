#include "stdafx.h"
#include "VPinMAMERom.h"

VPinMAMERom::VPinMAMERom()
{
}

VPinMAMERom::~VPinMAMERom()
{
}

STDMETHODIMP VPinMAMERom::get_Name(BSTR *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMERom::get_State(LONG *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMERom::get_StateDescription(BSTR *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMERom::Audit(VARIANT_BOOL fStrict) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMERom::get_Length(LONG *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMERom::get_ExpLength(LONG *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMERom::get_Checksum(LONG *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMERom::get_ExpChecksum(LONG *pVal) { return E_NOTIMPL; }
STDMETHODIMP VPinMAMERom::get_Flags(LONG *pVal) { return E_NOTIMPL; }