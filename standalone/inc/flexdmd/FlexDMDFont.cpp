#include "stdafx.h"
#include "FlexDMDFont.h"
#include <sstream>

FlexDMDFont::FlexDMDFont()
{
}

FlexDMDFont::~FlexDMDFont()
{
   if (m_bitmapFont) {
      for (int i = 0; i < m_bitmapFont->m_pages.size(); ++i) {
         if (m_textures[i])
            SDL_FreeSurface(m_textures[i]);
      }

      delete m_bitmapFont;
   }
}

FlexDMDFont* FlexDMDFont::Create(FlexDMD* pFlexDMD, string font, OLE_COLOR tint, OLE_COLOR borderTint, LONG borderSize)
{
   static string szFlexResources = "FlexDMD.Resources.";

   FlexDMDFont* obj = new FlexDMDFont();

   obj->m_pFlexDMD = pFlexDMD;
   obj->m_font = font;
   obj->m_tint = tint;
   obj->m_borderTint = borderTint;
   obj->m_borderSize = borderSize;

   vector<string> parts;
   std::stringstream ss(font);
   string token;

   while (std::getline(ss, token, '&'))
      parts.push_back(token);

   string basePath;
   string path;

   if (parts[0].starts_with(szFlexResources)) {
      basePath = g_pvp->m_szMyPath + "flexdmd" + PATH_SEPARATOR_CHAR;
      path = basePath + parts[0].substr(szFlexResources.length());
   }
   else {
      basePath = g_pvp->m_currentTablePath + pFlexDMD->GetProjectFolder();
      path = basePath + parts[0];

      std::size_t pos = parts[0].find_last_of(PATH_SEPARATOR_CHAR);
      if (pos != std::string::npos)
         basePath += parts[0].substr(0, pos + 1);
   }

   obj->m_bitmapFont = new BitmapFont();
   obj->m_bitmapFont->Load(path);

   obj->m_textures = new SDL_Surface*[obj->m_bitmapFont->m_pages.size()];
   memset((char*)obj->m_textures, 0, sizeof(SDL_Surface*) * obj->m_bitmapFont->m_pages.size());

   for (int i = 0; i < obj->m_bitmapFont->m_pages.size(); i++) {
      path = basePath + obj->m_bitmapFont->m_pages[i]->m_szFilename;

      SDL_Surface* pSurface = IMG_Load(path.c_str());

      if (pSurface) {
         obj->m_textures[i] = pSurface;

         PLOGI.printf("%s loaded", obj->m_bitmapFont->m_pages[i]->m_szFilename.c_str());
      }
      else {
         PLOGE.printf("%s failed to load", obj->m_bitmapFont->m_pages[i]->m_szFilename.c_str());
      }
   }

   return obj;
}

void FlexDMDFont::DrawCharacter(SDL_Surface* graphics, char character, char previousCharacter, float& x, float& y)
{
   if (character == '\n') {
      x = 0;
      y += m_bitmapFont->m_lineHeight;
   }
   else {
        auto it = m_bitmapFont->m_characters.find(character);
        if (it != m_bitmapFont->m_characters.end()) {
           Character* data = it->second;

           int kerning = m_bitmapFont->GetKerning(previousCharacter, character);

           if (graphics) {
              SDL_Surface* pSource = m_textures[data->m_texturePage];

              if (pSource) {
                 SDL_Rect dstRect = { (int)(x + data->m_offset.x + kerning), (int)(y + data->m_offset.y), data->m_bounds.w, data->m_bounds.h };
                 SDL_BlitScaled(pSource, &data->m_bounds, graphics, &dstRect);
              }
           }
           x += data->m_xadvance + kerning;
        }
        else if ('a' <= character && character <= 'z' && m_bitmapFont->m_characters.find(toupper(character)) != m_bitmapFont->m_characters.end()) {
            m_bitmapFont->m_characters[character] = m_bitmapFont->m_characters[toupper(character)];
            DrawCharacter(graphics, character, previousCharacter, x, y);
        }
        else if (m_bitmapFont->m_characters.find(' ') != m_bitmapFont->m_characters.end()) {
            PLOGE.printf("Missing character %c replaced by ' '", character);
            m_bitmapFont->m_characters[character] = m_bitmapFont->m_characters[' '];
            DrawCharacter(graphics, character, previousCharacter, x, y);
        }
   }
}

SDL_Rect FlexDMDFont::MeasureFont(string text)
{
   DrawString(NULL, 0, 0, text);
   return m_bitmapFont->MeasureFont(text);
}

void FlexDMDFont::DrawString(SDL_Surface* graphics, float x, float y, string text)
{
   char previousCharacter = ' ';
   for (int i = 0; i < text.length(); i++) {
        char character = text[i];
        DrawCharacter(graphics, character, previousCharacter, x, y);
        previousCharacter = character;
   }
}