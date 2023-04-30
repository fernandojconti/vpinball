#pragma once

#include "FlexDMDBaseActor.h"
#include "Layout.h"

class FlexDMDAnimatedActor : public FlexDMDBaseActor 
{
public:
   FlexDMDAnimatedActor();
   ~FlexDMDAnimatedActor();

   virtual void Update(float delta);

   void Seek(float posInSeconds);
   void Advance(float delta);
   void Rewind();

   virtual void ReadNextFrame() = 0;

protected:
   Scaling m_scaling;
   Alignment m_alignment;
   bool m_paused;
   bool m_loop;
   float m_length;
   float m_playSpeed;

   float m_frameTime;
   float m_frameDuration;
   float m_time;
   bool m_endOfAnimation;
};