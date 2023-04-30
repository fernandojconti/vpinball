#include "stdafx.h"
#include "BitmapFont.h"

#include <iostream>
#include <fstream>
#include <sstream>

BitmapFont::BitmapFont()
{
   m_padding = { 0, 0, 0, 0 };
   m_spacing = { 0, 0 };
   m_textureSize = { 0, 0, 0, 0 };
}

BitmapFont::~BitmapFont()
{
   for (auto it = m_characters.begin(); it != m_characters.end(); ++it)
      delete it->second;

   for (auto it = m_kernings.begin(); it != m_kernings.end(); ++it)
      delete it->second;

   for (auto it = m_pages.begin(); it != m_pages.end(); ++it)
      delete *it;
}

int BitmapFont::GetKerning(char previous, char current)
{
    int result = 0;

    Kerning key;
    key.m_firstCharacter = previous;
    key.m_secondCharacter = current;

    auto it = m_kernings.find(key.GetHash());
    if (it != m_kernings.end())
       result = it->second->m_amount;

    return result;
}

void BitmapFont::Load(string fileName)
{
   std::ifstream fontFile;
   fontFile.open(fileName, std::ifstream::in);
   if (fontFile.is_open())
   {
      string line;
      while (std::getline(fontFile, line))
      {
         vector<string> parts;
         std::stringstream ss(line);
         string token;

         while (std::getline(ss, token, ' '))
            parts.push_back(token);

         if (parts.size() != 0) {
            if (parts[0] == "info") {
                m_szFamilyName = GetNamedString(&parts, "face");
                m_fontSize = GetNamedInt(&parts, "size");
                m_bold = GetNamedBool(&parts, "bold");
                m_italic = GetNamedBool(&parts, "italic");
                m_szCharset = GetNamedString(&parts, "charset");
                m_unicode = GetNamedBool(&parts, "unicode");
                m_stretchedHeight = GetNamedInt(&parts, "stretchH");
                m_smoothed = GetNamedBool(&parts, "smooth");
                m_superSampling = GetNamedInt(&parts, "aa");
                m_padding = ParsePadding(GetNamedString(&parts, "padding"));
                m_spacing = ParsePoint(GetNamedString(&parts, "spacing"));
                m_outlineSize = GetNamedInt(&parts, "outline");
            }
            else if (parts[0] == "common") {
                m_lineHeight = GetNamedInt(&parts, "lineHeight");
                m_baseHeight = GetNamedInt(&parts, "base");
                m_textureSize = { 0, 0, GetNamedInt(&parts, "scaleW"),
                                        GetNamedInt(&parts, "scaleH") };
                m_packed = GetNamedBool(&parts, "packed");
                m_alphaChannel = GetNamedInt(&parts, "alphaChnl");
                m_redChannel = GetNamedInt(&parts, "redChnl");
                m_greenChannel = GetNamedInt(&parts, "greenChnl");
                m_blueChannel = GetNamedInt(&parts, "blueChnl");
            }
            else if (parts[0] == "page") {
                int id = GetNamedInt(&parts, "id");
                string name = GetNamedString(&parts, "file");

                Page* page = new Page();
                page->m_id = id;
                page->m_szFilename = name;

                m_pages.push_back(page);
            }
            else if (parts[0] == "char") {
                Character* character = new Character();

                character->m_char = (char)GetNamedInt(&parts, "id");
                character->m_bounds = { GetNamedInt(&parts, "x"),
                                        GetNamedInt(&parts, "y"),
                                        GetNamedInt(&parts, "width"),
                                        GetNamedInt(&parts, "height") };
                character->m_offset = { GetNamedInt(&parts, "xoffset"),
                                        GetNamedInt(&parts, "yoffset") };
                character->m_xadvance = GetNamedInt(&parts, "xadvance");
                character->m_texturePage = GetNamedInt(&parts, "page");
                character->m_channel = GetNamedInt(&parts, "chnl");

                m_characters[character->m_char] = character;
            }
            else if (parts[0] == "kerning") {
                Kerning* kerning = new Kerning();

                kerning->m_firstCharacter = (char)GetNamedInt(&parts, "first");
                kerning->m_secondCharacter = (char)GetNamedInt(&parts, "second");
                kerning->m_amount = GetNamedInt(&parts, "amount");

                auto it = m_kernings.find(kerning->GetHash());
                if (it == m_kernings.end())
                   m_kernings[kerning->GetHash()] = kerning;
            }
         }
      }

      std::sort(m_pages.begin(), m_pages.end(), [](const Page* a, const Page* b) {
        return a->m_id < b->m_id;
      });
   }
}

SDL_Rect BitmapFont::MeasureFont(string text, double maxWidth)
{
   SDL_Rect result = { 0, 0, 0, 0 };

   if (text.length() > 0) {
      char previousCharacter = ' ';
      int currentLineWidth = 0;
      int currentLineHeight = m_lineHeight;
      int blockWidth = 0;
      int blockHeight = 0;
      int length = text.length();
      vector<int> lineHeights;

      for (int i = 0; i < length; i++) {
         const char* chars = text.c_str();

         char character = chars[i];

         if (character == '\n' || character == '\r') {
            if (character == '\n' || i + 1 == length || chars[i + 1] != '\n') {
               lineHeights.push_back(currentLineHeight);
               blockWidth = max(blockWidth, currentLineWidth);
               currentLineWidth = 0;
               currentLineHeight = m_lineHeight;
            }
         }
         else {
            Character* data;
            int width;

            auto it = m_characters.find(character);
            if (it != m_characters.end()) {
               data = m_characters[character];
               width = data->m_xadvance + GetKerning(previousCharacter, character);

               if (maxWidth != NoMaxWidth && currentLineWidth + width >= maxWidth) {
                  lineHeights.push_back(currentLineHeight);
                  blockWidth = max(blockWidth, currentLineWidth);
                  currentLineWidth = 0;
                  currentLineHeight = m_lineHeight;
               }

               currentLineWidth += width;
               currentLineHeight = max(currentLineHeight, data->m_bounds.h + data->m_offset.y);
            }
            previousCharacter = character;
         }
      }

      if (currentLineHeight != 0)
         lineHeights.push_back(currentLineHeight);

      for (int i = 0; i < lineHeights.size() - 1; i++)
         lineHeights[i] = m_lineHeight;

      for (int lineHeight : lineHeights)
        blockHeight += lineHeight;

      result = { 0, 0, max(currentLineWidth, blockWidth), blockHeight };
   }
  
   return result;
}

string BitmapFont::GetNamedString(vector<string>* parts, string name)
{
   string result = "";

   for (const auto& part : *parts) {
      int nameEndIndex = part.find('=');
      if (nameEndIndex != -1) {
         string namePart = part.substr(0, nameEndIndex);
         string valuePart = part.substr(nameEndIndex + 1);

         if (name.compare(namePart) == 0) {
            int length = valuePart.length();

            if (length > 1 && valuePart[0] == '"' && valuePart[length - 1] == '"')
               valuePart = valuePart.substr(1, length - 2);

            result = valuePart;
            break;
         }
      }
   }

   return result;
}

int BitmapFont::StringToInt(string s, int defaultValue) {
   int value;
   try {
      if (s.length() > 0)
         s = s.substr(s.find_first_not_of(" \t\r\n"), s.find_last_not_of(" \t\r\n") - s.find_first_not_of(" \t\r\n") + 1);
      value = std::stoi(s);
   }
   catch (...) {
      value = defaultValue;
   }
   return value;
}

int BitmapFont::GetNamedInt(vector<string>* parts, string name, int defaultValue)
{
   return StringToInt(GetNamedString(parts, name), defaultValue);
}

bool BitmapFont::GetNamedBool(vector<string>* parts, string name, bool defaultValue)
{
   return StringToInt(GetNamedString(parts, name), defaultValue ? 1 : 0) > 0;
}

SDL_Rect BitmapFont::ParsePadding(string s)
{
   vector<string> parts;
   std::stringstream ss(s);
   string token;

   while (std::getline(ss, token, ','))
      parts.push_back(token);

   SDL_Rect rect = { StringToInt(parts[3]),
                     StringToInt(parts[0]),
                     StringToInt(parts[1]),
                     StringToInt(parts[2]) };
   return rect;
}

SDL_Point BitmapFont::ParsePoint(string s)
{
   vector<string> parts;
   std::stringstream ss(s);
   string token;

   while (std::getline(ss, token, ','))
      parts.push_back(token);

   SDL_Point point = { StringToInt(parts[0]),
                       StringToInt(parts[1]) };
   return point;
}

