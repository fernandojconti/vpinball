#ifndef KERNING_H
#define KERNING_H

class Kerning {
public:
   Kerning();
   ~Kerning();

   int GetHash();

   char m_firstCharacter;
   char m_secondCharacter;
   int m_amount;
};

#endif
