#include "stdafx.h"
#include "RenderFrame.h"
#include "RenderPass.h"
#include "RenderCommand.h"
#include "RenderDevice.h"

RenderFrame::RenderFrame(RenderDevice* renderDevice)
   : m_rd(renderDevice)
{
}

RenderFrame::~RenderFrame()
{
   for (auto item : m_commandPool)
      delete item;
   for (auto item : m_passPool)
      delete item;
   for (auto item : m_passes)
      delete item;
   delete m_basicShaderState;
   delete m_DMDShaderState;
   delete m_FBShaderState;
   delete m_flasherShaderState;
   delete m_lightShaderState;
   delete m_ballShaderState;
}

RenderPass* RenderFrame::AddPass(const string& name, RenderTarget* const rt, const bool ignoreStereo)
{
   RenderPass* pass;
   if (m_passPool.size() == 0)
   {
      pass = new RenderPass(name, rt, ignoreStereo);
   }
   else
   {
      pass = m_passPool.back();
      m_passPool.pop_back();
      pass->Reset(name, rt, ignoreStereo);
   }
   m_passes.push_back(pass);
   return pass;
}

RenderCommand* RenderFrame::NewCommand()
{
   if (m_commandPool.size() == 0)
   {
      return new RenderCommand(m_rd);
   }
   else
   {
      RenderCommand* item = m_commandPool.back();
      m_commandPool.pop_back();
      return item;
   }
}

void RenderFrame::Execute(const bool log)
{
   if (m_passes.size() == 0)
      return;

   // Save render/shader states
   if (m_basicShaderState == nullptr)
   {
      m_basicShaderState = new Shader::ShaderState(m_rd->basicShader);
      m_DMDShaderState = new Shader::ShaderState(m_rd->DMDShader);
      m_FBShaderState = new Shader::ShaderState(m_rd->FBShader);
      m_flasherShaderState = new Shader::ShaderState(m_rd->flasherShader);
      m_lightShaderState = new Shader::ShaderState(m_rd->lightShader);
      m_ballShaderState = new Shader::ShaderState(g_pplayer->m_ballShader);
   }
   RenderState prevState;
   m_rd->CopyRenderStates(true, prevState);
   m_rd->basicShader->m_state->CopyTo(true, m_basicShaderState);
   m_rd->DMDShader->m_state->CopyTo(true, m_DMDShaderState);
   m_rd->FBShader->m_state->CopyTo(true, m_FBShaderState);
   m_rd->flasherShader->m_state->CopyTo(true, m_flasherShaderState);
   m_rd->lightShader->m_state->CopyTo(true, m_lightShaderState);
   g_pplayer->m_ballShader->m_state->CopyTo(true, m_ballShaderState);

   // Sort passes to avoid useless render target switching, and allow merging passes for better draw call sorting/batching
   vector<RenderPass*> sortedPasses;
   sortedPasses.reserve(m_passes.size());
   if (log)
   {
      const U64 start = usec();
      m_passes.back()->Sort(sortedPasses);
      const U64 length = usec() - start;
      PLOGI << "Rendering Frame [" << sortedPasses.size() << " passes out of " << m_passes.size() << " submitted passes]";
      PLOGI << "> Pass sort took " << std::fixed << std::setw(8) << std::setprecision(3) << length << "us";
   }
   else
      m_passes.back()->Sort(sortedPasses);
   
   #ifndef ENABLE_SDL
   CHECKD3D(m_rd->GetCoreDevice()->BeginScene());
   #endif
   for (RenderPass* pass : sortedPasses)
   {
      pass->Execute(log);
      if (m_commandPool.size() < 1024)
         pass->RecycleCommands(m_commandPool);
      m_passPool.push_back(pass);
   }
   #ifndef ENABLE_SDL
   CHECKD3D(m_rd->GetCoreDevice()->EndScene());
   #endif
   m_passes.clear();

   // Restore render/shader states
   m_rd->CopyRenderStates(false, prevState);
   m_rd->basicShader->m_state->CopyTo(false, m_basicShaderState);
   m_rd->DMDShader->m_state->CopyTo(false, m_DMDShaderState);
   m_rd->FBShader->m_state->CopyTo(false, m_FBShaderState);
   m_rd->flasherShader->m_state->CopyTo(false, m_flasherShaderState);
   m_rd->lightShader->m_state->CopyTo(false, m_lightShaderState);
   g_pplayer->m_ballShader->m_state->CopyTo(false, m_ballShaderState);
}
