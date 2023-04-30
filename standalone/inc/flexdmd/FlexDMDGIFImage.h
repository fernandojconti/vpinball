#pragma once

#include "FlexDMDVideoActor.h"
#include <SDL2/SDL_image.h>

class FlexDMDGIFImage : public FlexDMDVideoActor 
{
public:
   FlexDMDGIFImage();
   ~FlexDMDGIFImage();

   static CComObject<FlexDMDGIFImage>* Create(FlexDMD* pFlexDMD, string name, string video);
   
   virtual void Rewind();
   virtual void ReadNextFrame();
   virtual void UpdateFrame();

   virtual void Draw(SDL_Surface* graphics);

   virtual void OnStageStateChanged();

protected:
   int m_pos;
   IMG_Animation* m_pAnimation;
   SDL_Surface* m_pSurface;
   bool m_bitmap;
};