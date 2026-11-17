/////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: Capture a rectangular area of the entire desktop.
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
/////////////////////////////////////////////////////////////////////////////
#ifndef CAPTURE_H
#define CAPTURE_H

class Capture
{
public:
    Capture();
    ~Capture();

    Capture(const Capture&) = delete;
    Capture& operator=(const Capture&) = delete;

    Capture& SetRect(int x, int y, int width, int height);
    Capture& SetDecimation(float);
    
    // This function only exists to accommodate Wayland requirements
    Capture& StartupCaptureStream();

    int GetOutputWidth() const;
    int GetOutputHeight() const;
    int GetImageBytes() const;

    // pMemory: Memory to hold image data in 32-bit BGRA format, must be at least GetImageBytes() in size
    void DoCapture(void* pMemory);

private:
    struct Private;
    Private* p = nullptr;
};

#endif // CAPTURE_H
