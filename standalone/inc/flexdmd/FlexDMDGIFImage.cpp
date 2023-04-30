#include "stdafx.h"
#include "FlexDMDGIFImage.h"
#include <cmath>

FlexDMDGIFImage::FlexDMDGIFImage()
{
   m_pAnimation = NULL;
   m_pos = 0;
   m_length = 0;

   m_pSurface = NULL;
   m_bitmap = false;
}

FlexDMDGIFImage::~FlexDMDGIFImage()
{
   if (m_pAnimation)
      IMG_FreeAnimation(m_pAnimation);
}

CComObject<FlexDMDGIFImage>* FlexDMDGIFImage::Create(FlexDMD* pFlexDMD, string name, string video)
{
   CComObject<FlexDMDGIFImage>* obj = nullptr;
   CComObject<FlexDMDGIFImage>::CreateInstance(&obj);

   obj->m_pFlexDMD = pFlexDMD;
   obj->m_szName = name;
   obj->m_szVideo = video;

   obj->AddRef();

   string path = g_pvp->m_currentTablePath + pFlexDMD->GetProjectFolder() + video;

   IMG_Animation* pAnimation = IMG_LoadAnimation(path.c_str());

   if (pAnimation) {
      obj->m_pAnimation = pAnimation;

      obj->m_prefWidth = pAnimation->w;
      obj->m_prefHeight = pAnimation->h;

      for (int index = 0; index < pAnimation->count; index++)
         obj->m_length += pAnimation->delays[index] / 1000.f;

      obj->Rewind();
      obj->Pack();

      PLOGI.printf("%s loaded: name=%s, w=%d, h=%d, count=%d, length=%.2f",
         video.c_str(), name.c_str(), pAnimation->w, pAnimation->h, pAnimation->count, obj->m_length);
   }
   else {
      PLOGE.printf("%s failed to load: name=%s", video.c_str(), name.c_str());
   }

   obj->m_bitmap = false;
    
   return obj;
}

void FlexDMDGIFImage::Rewind() 
{
   if (!m_pAnimation)
      return;

   FlexDMDVideoActor::Rewind();
   m_pos = 0;
   m_frameDuration = m_pAnimation->delays[0] / 1000.0f;
}

void FlexDMDGIFImage::ReadNextFrame()
{
   if (!m_pAnimation)
      return;

   if (m_pos >= m_pAnimation->count - 1)
      m_endOfAnimation = true;
   else {
      m_pos++;
      m_frameTime = 0;
      for (int i = 0; i < m_pos; i++)
         m_frameTime += m_pAnimation->delays[i] / 1000.0f;
        
      m_frameDuration = m_pAnimation->delays[m_pos] / 1000.0f;
      UpdateFrame();
   }
}

void FlexDMDGIFImage::UpdateFrame() 
{
   if (m_pAnimation && m_bitmap)
      m_pSurface = m_pAnimation->frames[m_pos];
}

void FlexDMDGIFImage::Draw(SDL_Surface* graphics)
{
   if (m_visible && m_bitmap && m_pSurface) {
      float w = 0;
      float h = 0;
      Layout::Scale(m_scaling, m_prefWidth, m_prefHeight, m_width, m_height, w, h);
      float x = 0;
      float y = 0;
      Layout::Align(m_alignment, w, h, m_width, m_height, x, y);
      SDL_Rect srcRect = { 0, 0, m_pSurface->w, m_pSurface->h };
      SDL_Rect dstRect = { (int)(m_x + x), (int)(m_y + y), (int)w, (int)h };
      SDL_BlitScaled(m_pSurface, &srcRect, graphics, &dstRect);
   }
}

void FlexDMDGIFImage::OnStageStateChanged()
{
   m_bitmap = m_onStage;
   UpdateFrame();
}