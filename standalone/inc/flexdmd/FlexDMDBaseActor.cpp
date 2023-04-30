#include "stdafx.h"
#include "FlexDMDBaseActor.h"
#include "FlexDMDActionFactory.h"

#include "FlexDMDGroupActor.h"

FlexDMDBaseActor::FlexDMDBaseActor()
{
    m_onStage = false;

    m_szName = "";
    m_x = 0;
    m_y = 0;
    m_width = 0;
    m_height = 0;
    m_parent = NULL;
    m_fillParent = false;
    m_clearBackground = false;
    m_prefWidth = 0;
    m_prefHeight = 0;
    m_visible = true;

    CComObject<FlexDMDActionFactory>::CreateInstance(&m_pActionFactory);
    m_pActionFactory->AddRef();
}

FlexDMDBaseActor::~FlexDMDBaseActor()
{
     m_pActionFactory->Release();
}

STDMETHODIMP FlexDMDBaseActor::get_Name(BSTR *pRetVal)
{
    CComBSTR Val(m_szName.c_str());
    *pRetVal = Val.Detach();

    return S_OK;
}

STDMETHODIMP FlexDMDBaseActor::put_Name(BSTR pRetVal)
{
    m_szName = MakeString(pRetVal);

    return S_OK;
}

STDMETHODIMP FlexDMDBaseActor::get_x(single *pRetVal)
{
    *pRetVal = m_x;

    return S_OK;
}

STDMETHODIMP FlexDMDBaseActor::put_x(single pRetVal)
{
    m_x = pRetVal;

    return S_OK;
}

STDMETHODIMP FlexDMDBaseActor::get_y(single *pRetVal)
{
    *pRetVal = m_y;

    return S_OK;
}

STDMETHODIMP FlexDMDBaseActor::put_y(single pRetVal)
{
    m_y = pRetVal;

    return S_OK;
}

STDMETHODIMP FlexDMDBaseActor::get_Width(single *pRetVal)
{
    *pRetVal = m_width;

    return S_OK;
}

STDMETHODIMP FlexDMDBaseActor::put_Width(single pRetVal)
{
    m_width = pRetVal;

    return S_OK;
}

STDMETHODIMP FlexDMDBaseActor::get_Height(single *pRetVal)
{
    *pRetVal = m_height;

    return S_OK;
}

STDMETHODIMP FlexDMDBaseActor::put_Height(single pRetVal)
{
    m_height = pRetVal;

    return S_OK;
}

STDMETHODIMP FlexDMDBaseActor::get_visible(VARIANT_BOOL *pRetVal)
{
    *pRetVal = m_visible ? VARIANT_TRUE : VARIANT_FALSE;

    return S_OK;
}

STDMETHODIMP FlexDMDBaseActor::put_visible(VARIANT_BOOL pRetVal)
{
    m_visible = (pRetVal == VARIANT_TRUE);

    return S_OK;
}

STDMETHODIMP FlexDMDBaseActor::get_FillParent(VARIANT_BOOL *pRetVal)
{
    *pRetVal = m_fillParent ? VARIANT_TRUE : VARIANT_FALSE;

    return S_OK;
}

STDMETHODIMP FlexDMDBaseActor::put_FillParent(VARIANT_BOOL pRetVal)
{
    m_fillParent = (pRetVal == VARIANT_TRUE);

    return S_OK;
}

STDMETHODIMP FlexDMDBaseActor::get_ClearBackground(VARIANT_BOOL *pRetVal)
{
    *pRetVal = m_clearBackground ? VARIANT_TRUE : VARIANT_FALSE;

    return S_OK;
}

STDMETHODIMP FlexDMDBaseActor::put_ClearBackground(VARIANT_BOOL pRetVal)
{
     m_clearBackground = (pRetVal == VARIANT_TRUE);

    return S_OK;
}

STDMETHODIMP FlexDMDBaseActor::SetBounds(single x,single y,single Width,single Height)
{
    m_x = x;
    m_y = y;
    m_width = Width;
    m_height = Height;

    return S_OK;
}

STDMETHODIMP FlexDMDBaseActor::SetPosition(single x,single y)
{
    m_x = x;
    m_y = y;

    return S_OK;
}

STDMETHODIMP FlexDMDBaseActor::SetAlignedPosition(single x, single y, Alignment alignment)
{
    switch (alignment) {
        case Alignment_BottomLeft:
        case Alignment_Left:
        case Alignment_TopLeft:
            m_x = x;
            break;
        case Alignment_Bottom:
        case Alignment_Center:
        case Alignment_Top:
            m_x = x - m_width * 0.5f;
            break;
        case Alignment_BottomRight:
        case Alignment_Right:
        case Alignment_TopRight:
            m_x = x - m_width;
            break;
    }

    switch (alignment)
    {
        case Alignment_BottomLeft:
        case Alignment_Bottom:
        case Alignment_BottomRight:
            m_y = y - m_height;
            break;
        case Alignment_Left:
        case Alignment_Center:
        case Alignment_Right:
            m_y = y - m_height * 0.5f;
            break;
        case Alignment_TopLeft:
        case Alignment_Top:
        case Alignment_TopRight:
            m_y = y;
            break;
    }

    return S_OK;
}

STDMETHODIMP FlexDMDBaseActor::SetSize(single Width,single Height)
{
    m_width = Width;
    m_height = Height;

    return S_OK;
}

STDMETHODIMP FlexDMDBaseActor::get_PrefWidth(single *pRetVal)
{
    *pRetVal = m_prefWidth;

    return S_OK;
}

STDMETHODIMP FlexDMDBaseActor::get_PrefHeight(single *pRetVal)
{
    *pRetVal = m_prefHeight;

    return S_OK;
}

STDMETHODIMP FlexDMDBaseActor::Pack()
{
    m_width = m_prefWidth;
    m_height = m_prefHeight;

    return S_OK;
}

STDMETHODIMP FlexDMDBaseActor::Remove()
{
    if (m_parent != NULL)
        m_parent->RemoveActor((IUnknown*)this);

    return S_OK;
}

STDMETHODIMP FlexDMDBaseActor::get_ActionFactory(IActionFactory **pRetVal)
{
     return m_pActionFactory->QueryInterface(IID_IActionFactory, (void**)pRetVal);
}

STDMETHODIMP FlexDMDBaseActor::AddAction(IUnknown *action)
{
    return S_OK;
}

STDMETHODIMP FlexDMDBaseActor::ClearActions() { return E_NOTIMPL; }

void FlexDMDBaseActor::Update(float secondsElapsed)
{
    if (!m_onStage) assert("Update was called on an actor which is not on stage.");
    //for (int i = 0; i < _actions.Count; i++)
    //    if (_actions[i].Update(secondsElapsed)) 
    //       _actions.RemoveAt(i);

    if (m_fillParent && m_parent != NULL)
        SetBounds(0, 0, m_parent->m_width, m_parent->m_height);
}

void FlexDMDBaseActor::Draw(SDL_Surface* graphics)
{
    if (!m_onStage) assert("Update was called on an actor which is not on stage.");
    if (m_visible && m_clearBackground) {
       SDL_Rect rect = { (int)m_x, (int)m_y, (int)m_width, (int)m_height };
       SDL_FillRect(graphics, &rect, SDL_MapRGB(graphics->format, 0, 0, 0));
    }
}