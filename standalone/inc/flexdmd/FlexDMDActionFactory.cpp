#include "stdafx.h"
#include "FlexDMDActionFactory.h"
#include "FlexDMDCompositeAction.h"

FlexDMDActionFactory::FlexDMDActionFactory()
{
    CComObject<FlexDMDCompositeAction>::CreateInstance(&m_pCompositeAction);
    m_pCompositeAction->AddRef();
}

FlexDMDActionFactory::~FlexDMDActionFactory()
{
    m_pCompositeAction->Release();
}

STDMETHODIMP FlexDMDActionFactory::Wait(single secondsToWait, IUnknown **pRetVal)
{
    return this->QueryInterface(IID_IUnknown, (void**)pRetVal);
}

STDMETHODIMP FlexDMDActionFactory::Delayed(single secondsToWait, IUnknown *action, IUnknown **pRetVal)
{
    return this->QueryInterface(IID_IUnknown, (void**)pRetVal);
}

STDMETHODIMP FlexDMDActionFactory::Parallel(ICompositeAction **pRetVal)
{
    return m_pCompositeAction->QueryInterface(IID_ICompositeAction, (void**)pRetVal);
}

STDMETHODIMP FlexDMDActionFactory::Sequence(ICompositeAction **pRetVal)
{
    return m_pCompositeAction->QueryInterface(IID_ICompositeAction, (void**)pRetVal);
}

STDMETHODIMP FlexDMDActionFactory::Repeat(IUnknown *action, LONG count, IUnknown **pRetVal)
{
    return this->QueryInterface(IID_IUnknown, (void**)pRetVal);
}

STDMETHODIMP FlexDMDActionFactory::Blink(single secondsShow, single secondsHide, LONG Repeat, IUnknown **pRetVal)
{
    return this->QueryInterface(IID_IUnknown, (void**)pRetVal);
}

STDMETHODIMP FlexDMDActionFactory::Show(VARIANT_BOOL visible, IUnknown **pRetVal)
{
    return this->QueryInterface(IID_IUnknown, (void**)pRetVal);
}

STDMETHODIMP FlexDMDActionFactory::AddTo(IGroupActor *parent, IUnknown **pRetVal)
{
    return this->QueryInterface(IID_IUnknown, (void**)pRetVal);
}

STDMETHODIMP FlexDMDActionFactory::RemoveFromParent(IUnknown **pRetVal)
{
    return S_OK;
}

STDMETHODIMP FlexDMDActionFactory::AddChild(IUnknown *child, IUnknown **pRetVal)
{
    return this->QueryInterface(IID_IUnknown, (void**)pRetVal);
}

STDMETHODIMP FlexDMDActionFactory::RemoveChild(IUnknown *child, IUnknown **pRetVal)
{
    return this->QueryInterface(IID_IUnknown, (void**)pRetVal);
}

STDMETHODIMP FlexDMDActionFactory::Seek(single pos, IUnknown **pRetVal)
{
    return this->QueryInterface(IID_IUnknown, (void**)pRetVal);
}

STDMETHODIMP FlexDMDActionFactory::MoveTo(single x, single y, single duration, ITweenAction **pRetVal)
{
    return S_OK;
}
