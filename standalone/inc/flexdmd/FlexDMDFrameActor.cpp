#include "stdafx.h"
#include "FlexDMDFrameActor.h"
#include "FlexDMDActionFactory.h"

#include "FlexDMDGroupActor.h"

FlexDMDFrameActor::FlexDMDFrameActor()
{
    m_thickness = 2;
    m_borderColor = RGB(255, 255, 255);
    m_fill = false;
    m_fillColor = RGB(0, 0, 0);
}

FlexDMDFrameActor::~FlexDMDFrameActor()
{
}

CComObject<FlexDMDFrameActor>* FlexDMDFrameActor::Create(FlexDMD* pFlexDMD, string name)
{
    CComObject<FlexDMDFrameActor>* obj = nullptr;
    CComObject<FlexDMDFrameActor>::CreateInstance(&obj);
    obj->AddRef();

    obj->m_szName = name;
    obj->m_pFlexDMD = pFlexDMD;

    return obj;
}

STDMETHODIMP FlexDMDFrameActor::get_Thickness(LONG *pRetVal)
{
    *pRetVal = m_thickness;

    return S_OK;
}

STDMETHODIMP FlexDMDFrameActor::put_Thickness(LONG pRetVal)
{
    m_thickness = pRetVal;

    return S_OK;
}

STDMETHODIMP FlexDMDFrameActor::get_BorderColor(OLE_COLOR *pRetVal)
{
    *pRetVal = m_borderColor;

    return S_OK;
}

STDMETHODIMP FlexDMDFrameActor::put_BorderColor(OLE_COLOR pRetVal)
{
    m_borderColor = pRetVal;

    return S_OK;
}

STDMETHODIMP FlexDMDFrameActor::get_Fill(VARIANT_BOOL *pRetVal)
{
    *pRetVal = m_fill ? VARIANT_TRUE : VARIANT_FALSE;

    return S_OK;
}

STDMETHODIMP FlexDMDFrameActor::put_Fill(VARIANT_BOOL pRetVal)
{
    m_fill = (pRetVal == VARIANT_TRUE);

    return S_OK;
}

STDMETHODIMP FlexDMDFrameActor::get_FillColor(OLE_COLOR *pRetVal)
{
    *pRetVal = m_fillColor;

    return S_OK;
}

STDMETHODIMP FlexDMDFrameActor::put_FillColor(OLE_COLOR pRetVal)
{
    m_fillColor = pRetVal;

    return S_OK;
}

void FlexDMDFrameActor::Draw(SDL_Surface* graphics)
{
    FlexDMDBaseActor::Draw(graphics);

    if (m_visible) {
        if (m_fill) {
            Uint32 fillColor = SDL_MapRGB(graphics->format, GetRValue(m_fillColor), GetGValue(m_fillColor), GetBValue(m_fillColor));

            SDL_Rect rect = { (int)(m_x + m_thickness), (int)(m_y + m_thickness), (int)(m_width - 2 * m_thickness), (int)(m_height - 2 * m_thickness) };
            SDL_FillRect(graphics, &rect, fillColor);
        }
        if (m_thickness > 0) {
              Uint32 borderColor = SDL_MapRGB(graphics->format, GetRValue(m_borderColor), GetGValue(m_borderColor), GetBValue(m_borderColor));

              SDL_Rect rect = { (int)m_x, (int)m_y, (int)m_width, (int)m_thickness };
              SDL_FillRect(graphics, &rect, borderColor);

              rect = { (int)m_x, (int)(m_y + m_height - m_thickness), (int)m_width, (int)m_thickness };
              SDL_FillRect(graphics, &rect, borderColor);

              rect = { (int)m_x, (int)(m_y + m_thickness), (int)m_thickness, (int)(m_height - 2 * m_thickness) };
              SDL_FillRect(graphics, &rect, borderColor);

              rect = { (int)(m_x + m_width - m_thickness), (int)(m_y + m_thickness), (int)m_thickness, (int)(m_height - 2 * m_thickness) };
              SDL_FillRect(graphics, &rect, borderColor);
        }
    }
}