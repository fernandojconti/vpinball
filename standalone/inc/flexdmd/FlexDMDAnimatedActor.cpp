#include "stdafx.h"
#include "FlexDMDAnimatedActor.h"
#include <cmath>

FlexDMDAnimatedActor::FlexDMDAnimatedActor()
{
    m_paused = false;
    m_loop = true;
    m_playSpeed = 1.0f;
    m_alignment = Alignment_Center;
    m_scaling = Scaling_Stretch;
}

FlexDMDAnimatedActor::~FlexDMDAnimatedActor()
{
}

void FlexDMDAnimatedActor::Update(float delta)
{
   FlexDMDBaseActor::Update(delta);

   if (!m_visible)
      return;

   if (!m_paused)
      Advance(delta * m_playSpeed);
}

void FlexDMDAnimatedActor::Seek(float posInSeconds)
{
    Rewind();
    Advance(posInSeconds);
}

void FlexDMDAnimatedActor::Advance(float delta)
{
   m_time += delta;
   while (!m_endOfAnimation && m_time >= m_frameTime + m_frameDuration)
      ReadNextFrame();

   if (m_endOfAnimation && m_loop) {
      float length = m_frameTime + m_frameDuration;
      m_time = fmod(m_time, length);
      Rewind();
   }
}

void FlexDMDAnimatedActor::Rewind()
{
    m_time = 0;
    m_frameTime = 0;
    m_endOfAnimation = false;
}