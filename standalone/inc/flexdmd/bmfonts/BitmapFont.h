#pragma once

#include "Kerning.h"
#include "Character.h"
#include "Page.h"

#include <map>

class BitmapFont
{
public:
   BitmapFont();
   ~BitmapFont();
   
   static const int NoMaxWidth = -1;

   int GetKerning(char previous, char current);
   void Load(string fileName);

   SDL_Rect MeasureFont(string text, double maxWidth = NoMaxWidth);

   static string GetNamedString(vector<string>* parts, string name);
   static int StringToInt(string s, int defaultValue = 0);
   static int GetNamedInt(vector<string>* parts, string name, int defaultValue = 0);
   static bool GetNamedBool(vector<string>* parts, string name, bool defaultValue = false);
   static SDL_Rect ParsePadding(string s);
   static SDL_Point ParsePoint(string s);

   int m_alphaChannel;
   int m_baseHeight;
   int m_blueChannel;
   bool m_bold;
   std::map<char, Character*> m_characters;
   string m_szCharset;
   string m_szFamilyName;
   int m_fontSize;
   int m_greenChannel;
   bool m_italic;
   std::map<int, Kerning*> m_kernings;
   int m_lineHeight;
   int m_outlineSize;
   bool m_packed;
   SDL_Rect m_padding;
   vector<Page*> m_pages;
   int m_redChannel;
   bool m_smoothed;
   SDL_Point m_spacing;
   int m_stretchedHeight;
   int m_superSampling;
   SDL_Rect m_textureSize;
   bool m_unicode;
};