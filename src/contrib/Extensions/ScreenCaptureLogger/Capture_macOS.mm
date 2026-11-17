/////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: A function that captures a screen area into memory as a
//   BGRA image.
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
#include "Capture.h"

#include <stdexcept>
#include <algorithm>

#include <unistd.h>
#import <ScreenCaptureKit/ScreenCaptureKit.h>

struct Capture::Private
{
    int x = 0, y = 0, width = 0, height = 0;
    float decimation = 1;
    CGColorSpaceRef colorSpace = nil;
};

Capture::Capture()
: p(new Private)
{
    p->colorSpace = ::CGColorSpaceCreateDeviceRGB();
}

Capture::~Capture()
{
    CGColorSpaceRelease(p->colorSpace);
    delete p;
}

Capture& Capture::SetRect(int x, int y, int width, int height)
{
    p->x = x;
    p->y = y;
    p->width = width;
    p->height = height;
    return *this;
}

Capture& Capture::SetDecimation(float d)
{
    p->decimation = std::max(d, 1.f);
    return *this;
}

Capture& Capture::StartupCaptureStream()
{
    return *this;
}

int Capture::GetOutputWidth() const
{
    return p->width / p->decimation;
}

int Capture::GetOutputHeight() const
{
    return p->height / p->decimation;
}

int Capture::GetImageBytes() const
{
    return GetOutputWidth() * GetOutputHeight() * 4;
}

void Capture::DoCapture(void* pMemory)
{
    __block CGImageRef image = nil;
    __block NSError* error = nil;
    __block bool done = false;
    void (^captureComplete)(CGImageRef, NSError*) = ^(CGImageRef inImage, NSError* inError)
    {
        image = inImage;
        error = inError;
        done = true;
    };
    CGRect cgRect = ::CGRectMake(p->x, p->y, p->width, p->height);
    [SCScreenshotManager captureImageInRect: cgRect completionHandler: captureComplete];
    while (!done) {
        ::usleep(500);
    }
    if (error) {
        throw std::runtime_error("Error capturing image");
    }
    int outWidth = GetOutputWidth();
    int outHeight = GetOutputHeight();
    NSUInteger bytesPerPixel = 4;
    NSUInteger bytesPerRow = bytesPerPixel * outWidth;
    NSUInteger bitsPerComponent = 8;
    CGContextRef context = ::CGBitmapContextCreate(
      pMemory, outWidth, outHeight,
      bitsPerComponent, bytesPerRow, p->colorSpace,
      kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little
    );
    ::CGContextDrawImage(context, ::CGRectMake(0, 0, outWidth, outHeight), image);
    ::CGContextRelease(context);
    ::CGImageRelease(image);
}
