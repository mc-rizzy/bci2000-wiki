////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: Glue code for Apple CGL functions,
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2026: BCI2000 team and many external contributors ]
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
#ifndef CGL_GLUE_H
#define CGL_GLUE_H

namespace CGLGlue {

struct View;

class Context
{
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(const Context&) = delete;

public:
    Context();
    Context(Context* shareContext, View* view);
    virtual ~Context();

    int setSwapInterval(int);
    void flushBuffer();

    void update();

    void makeCurrentContext();
    static void clearCurrentContext();
    static Context* currentContext();

    View* view() const;
    void setView(View*);

    void setNativeContext(void*);
    void* nativeContext() const;

private:
    void* mNSOpenGLContext = nullptr;
};

} // namespace CGLGlue

#endif // CGL_GLUE_H
