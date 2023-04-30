#ifndef CHARACTER_H
#define CHARACTER_H

class Character {
public:
   Character();
   ~Character();

   SDL_Rect m_bounds;
   int m_channel;
   char m_char;
   SDL_Point m_offset;
   int m_texturePage;
   int m_xadvance;
};

#endif
