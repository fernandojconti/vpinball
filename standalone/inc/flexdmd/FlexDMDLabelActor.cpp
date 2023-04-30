#include "stdafx.h"
#include "FlexDMDLabelActor.h"
#include "FlexDMDActionFactory.h"

#include <sstream>
#include <regex>

FlexDMDLabelActor::FlexDMDLabelActor()
{
    m_autopack = false;
    m_alignment = Alignment_Center;
}

FlexDMDLabelActor::~FlexDMDLabelActor()
{
}

CComObject<FlexDMDLabelActor>* FlexDMDLabelActor::Create(FlexDMD* pFlexDMD, string name, FlexDMDFont* pFont, string text)
{
    CComObject<FlexDMDLabelActor>* obj = nullptr;
    CComObject<FlexDMDLabelActor>::CreateInstance(&obj);
    obj->AddRef();

    obj->m_pFlexDMD = pFlexDMD;
    obj->m_szName = name;
    obj->m_pFont = pFont;
    obj->m_szText = text;

    LONG runtimeVersion;
    pFlexDMD->get_RuntimeVersion(&runtimeVersion);

    obj->m_autopack = runtimeVersion <= 1008;

    obj->ProcessText();
    obj->Pack();

    return obj;
}

STDMETHODIMP FlexDMDLabelActor::get_AutoPack(VARIANT_BOOL *pRetVal)
{
    *pRetVal = m_autopack ? VARIANT_TRUE : VARIANT_FALSE;

    return S_OK;
}

STDMETHODIMP FlexDMDLabelActor::put_AutoPack(VARIANT_BOOL pRetVal)
{
    m_autopack = (pRetVal == VARIANT_TRUE);

    return S_OK;
}

void FlexDMDLabelActor::Missing36() { }
void FlexDMDLabelActor::Missing37() { }

STDMETHODIMP FlexDMDLabelActor::get_Font(IUnknown **pRetVal)
{
    if (m_pFont)
       return m_pFont->QueryInterface(IID_FlexDMDFontInterface, (void**)pRetVal);

    return S_OK;
}

STDMETHODIMP FlexDMDLabelActor::putref_Font(IUnknown *pRetVal)
{
    m_pFont = (FlexDMDFont*)pRetVal;
    UpdateBounds();

    return S_OK;
}

STDMETHODIMP FlexDMDLabelActor::get_Text(BSTR *pRetVal)
{
    CComBSTR Val(m_szText.c_str());
    *pRetVal = Val.Detach();

    return S_OK;
}

STDMETHODIMP FlexDMDLabelActor::put_Text(BSTR pRetVal)
{
   m_szText = MakeString(pRetVal);
   ProcessText();

   return S_OK;
}

void FlexDMDLabelActor::ProcessText()
{
   std::regex pattern(R"(\r\n|\n\r|\n|\r)");
   m_szText = std::regex_replace(m_szText, pattern, "\r\n");

   m_lines.clear();

   std::stringstream ss(m_szText);
   string line;

   while (std::getline(ss, line, '\n'))
       m_lines.push_back(line);

   UpdateBounds();
}

void FlexDMDLabelActor::UpdateBounds()
{
   if (!m_pFont)
      return;

   SDL_Rect size = m_pFont->MeasureFont(m_szText);
   m_textWidth = size.w;
   m_textHeight = size.h;
   m_prefWidth = m_textWidth;
   m_prefHeight = m_textHeight;

   if (m_autopack)
      Pack();
}

void FlexDMDLabelActor::Draw(SDL_Surface* pSurface)
{
    FlexDMDBaseActor::Draw(pSurface);

    if (m_visible && m_pFont) {
         if (m_lines.size() > 1 && m_alignment != Alignment_Left && m_alignment != Alignment_BottomLeft && m_alignment != Alignment_TopLeft) {
            float x = 0;
            float y = 0;
            Layout::Align(m_alignment, m_textWidth, m_textHeight, m_width, m_height, x, y);

            for (const string& line : m_lines) {
               float lx = 0;
               float ly = 0;
               Layout::Align(m_alignment, m_pFont->MeasureFont(line).w, m_textHeight, m_width, m_height, lx, ly);
               m_pFont->DrawString(pSurface, (int)floor(m_x + lx), (int)floor(m_y + ly), line);
               y += m_pFont->m_bitmapFont->m_lineHeight;
            }
         }
         else {
            float x = 0;
            float y = 0;
            Layout::Align(m_alignment, m_textWidth, m_textHeight, m_width, m_height, x, y);
            m_pFont->DrawString(pSurface, (int)floor(m_x + x), (int)floor(m_y + y), m_szText);
         }
    }
}