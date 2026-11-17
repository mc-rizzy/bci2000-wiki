////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: Glue code for Apple CGL functions,
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2023: BCI2000 team and many external contributors ]
//
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
//
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#include "CGLGlue.h"

#import <AppKit/AppKit.h>
#include <OpenGL/gl.h>
#include <stdexcept>

namespace {
  thread_local CGLGlue::Context* spCurrentContext = nullptr;
}

namespace CGLGlue {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

Context::Context()
{
}

Context::Context(Context* pShareContext, View* pView)
{
  NSOpenGLContext* pShare = nil;
  if (pShareContext)
    pShare = static_cast<NSOpenGLContext*>(pShareContext->mNSOpenGLContext);

  NSOpenGLPixelFormatAttribute attrib[] =
  {
    NSOpenGLPFAAlphaSize, 8,
    NSOpenGLPFAColorSize, 24,
    NSOpenGLPFADepthSize, 32,
    NSOpenGLPFADoubleBuffer,
    0
  };
  NSOpenGLPixelFormat* pAttr = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrib];
  if (pAttr) {
    NSOpenGLContext* pContext = [[NSOpenGLContext alloc] initWithFormat:pAttr shareContext:pShare];
    [pAttr release];
    if (pView) {
      pContext.view = reinterpret_cast<NSView*>(pView);
    }
    mNSOpenGLContext = pContext;
  }
  else
  {
    throw std::runtime_error("Could not create OpenGL context");
  }
}


Context::~Context()
{
  NSOpenGLContext* pContext = reinterpret_cast<NSOpenGLContext*>(mNSOpenGLContext);
  [pContext release];
}


int Context::setSwapInterval(int i)
{
  int sync = i;
  NSOpenGLContext* pContext = reinterpret_cast<NSOpenGLContext*>(mNSOpenGLContext);
  CGLContextObj pContextObj = [pContext CGLContextObj];
  ::CGLSetParameter(pContextObj, kCGLCPSwapInterval, &sync);
  ::CGLGetParameter(pContextObj, kCGLCPSwapInterval, &sync);
  return sync;
}

void Context::flushBuffer()
{
  NSOpenGLContext* pContext = reinterpret_cast<NSOpenGLContext*>(mNSOpenGLContext);
  [pContext flushBuffer];
}

void Context::update()
{
  NSOpenGLContext* pContext = reinterpret_cast<NSOpenGLContext*>(mNSOpenGLContext);
  [pContext update];
}

void Context::makeCurrentContext()
{
  NSOpenGLContext* pContext = reinterpret_cast<NSOpenGLContext*>(mNSOpenGLContext);
  [pContext makeCurrentContext];
  spCurrentContext = this;
}

void Context::clearCurrentContext()
{
  [NSOpenGLContext clearCurrentContext];
  spCurrentContext = nullptr;
}

Context* Context::currentContext()
{
  return spCurrentContext;
}

View* Context::view() const
{
  NSOpenGLContext* pContext = reinterpret_cast<NSOpenGLContext*>(mNSOpenGLContext);
  return reinterpret_cast<View*>(pContext.view);
}

void Context::setView(View* pView)
{
  NSOpenGLContext* pContext = reinterpret_cast<NSOpenGLContext*>(mNSOpenGLContext);
  pContext.view = reinterpret_cast<NSView*>(pView);
}

void* Context::nativeContext() const
{
  return mNSOpenGLContext;
}

void Context::setNativeContext(void* pContext)
{
  mNSOpenGLContext = pContext;
}

#pragma clang diagnostic pop

} // namespace CGLGlue
