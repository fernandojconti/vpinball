#include "stdafx.h"
#include "ZeDMD.h"

/**
 * Derived from https://github.com/freezy/dmd-extensions/blob/master/LibDmd/Output/ZeDMD/ZeDMD.cs
 */

ZeDMD::ZeDMD()
{
   m_width = 0;
   m_height = 0;
   
   m_pCache = NULL;
   m_pPlanes = NULL;
   m_pTemp = NULL;
}

ZeDMD::~ZeDMD()
{
   m_dmd.Disconnect();

   if (m_pCache)
      delete m_pCache;

   if (m_pPlanes)
      delete m_pPlanes;
      
   if (m_pTemp)
      delete m_pTemp;
}

void ZeDMD::Open(int width, int height)
{
   if (m_dmd.Connect()) {
      m_pCache = (UINT8*)malloc(width * height * 3);
      m_pPlanes = (UINT8*)malloc(width * height * 3);
      m_pTemp = (UINT8*)malloc(width * height * 3);
     
      m_width = width;
      m_height = height;   

      // m_dmd.QueueCommand(ZEDMD_COMMAND::DebugEnable);

      SetColor(RGB(255, 88, 32));

      m_dmd.Run();
   }
}

void ZeDMD::SetColor(OLE_COLOR color)
{
   UINT8 r = color & 0xFF;
   UINT8 g = (color >> 8) & 0xFF;
   UINT8 b = (color >> 16) & 0xFF;

   int pos = 0;
   for (int i = 0; i < 4; i++) {
      float perc = calc_brightness(i / 3.0f);
      m_palette2[pos++] = (UINT8)(r * perc);
      m_palette2[pos++] = (UINT8)(g * perc);
      m_palette2[pos++] = (UINT8)(b * perc);
   }

   pos = 0;
   for (int i = 0; i < 16; i++) {
      float perc = calc_brightness(i / 15.0f);
      m_palette4[pos++] = (UINT8)(r * perc);
      m_palette4[pos++] = (UINT8)(g * perc);
      m_palette4[pos++] = (UINT8)(b * perc);
   }
}

void ZeDMD::RenderGray2(UINT8* frame, int width, int height)
{
   if (!m_pCache)
      return;

   int size = width * height;

   if (memcmp(m_pCache, frame, size)) {
      memcpy(m_pCache, frame, size);

      int paletteSize = 12;
      int bufferSize = size / 4;

      Split(width, height, 2, frame, m_pPlanes);

      memcpy(m_pTemp, m_palette2, paletteSize);
      memcpy(m_pTemp + paletteSize, m_pPlanes, bufferSize);

      m_dmd.QueueCommand(ZEDMD_COMMAND::Gray2, m_pTemp, paletteSize + bufferSize);
   }
}

void ZeDMD::RenderGray4(UINT8* frame, int width, int height)
{
   if (!m_pCache)
      return;

   int size = width * height;

   if (memcmp(m_pCache, frame, size)) {
      memcpy(m_pCache, frame, size);

      int paletteSize = 48;
      int bufferSize = size / 2;

      Split(width, height, 4, frame, m_pPlanes);

      memcpy(m_pTemp, m_palette4, paletteSize);
      memcpy(m_pTemp + paletteSize, m_pPlanes, bufferSize);

      m_dmd.QueueCommand(ZEDMD_COMMAND::ColGray4, m_pTemp, paletteSize + bufferSize);
   }
}

void ZeDMD::RenderRgb24(UINT8* frame, int width, int height)
{
   if (!m_pCache)
      return;

   int size = width * height * 3;
   
   if (memcmp(m_pCache, frame, size)) {
      memcpy(m_pCache, frame, size);
      m_dmd.QueueCommand(ZEDMD_COMMAND::RGB24, frame, size);
   }
}

/**
 * Derived from https://github.com/freezy/dmd-extensions/blob/master/LibDmd/Common/FrameUtil.cs
 */

void ZeDMD::Split(int width, int height, int bitlen, UINT8* frame, UINT8* planes)
{
   int planeSize = width * height / 8;
   int pos = 0;
   UINT8 bd[bitlen];

   for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x += 8) {
          memset(bd, 0, bitlen * sizeof(UINT8));

          for (int v = 7; v >= 0; v--) {
             UINT8 pixel = frame[(y * width) + (x + v)];
             for (int i = 0; i < bitlen; i++) {
                bd[i] <<= 1;
                if ((pixel & (1 << i)) != 0)
                   bd[i] |= 1;
             }
          }

          for (int i = 0; i < bitlen; i++)
             planes[i * planeSize + pos] = bd[i];

          pos++;
       }
    }
}