////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A ref-counted object that may be shared across dynamic library
//   modules, even if those are using different versions of the runtime library.
//   At creation, a handle has a ref count of 1.
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
#ifndef BCI_HANDLE_H
#define BCI_HANDLE_H

#include <atomic>
#include "Debugging.h"
#include "GenericSignal.h"

template<class T> void* CreateBCIHandle(T*);

struct BCIHandle
{
    template<typename T> friend void* CreateBCIHandle(T*);

public:
    BCIHandle() {}
    BCIHandle(const BCIHandle&) = delete;
    BCIHandle& operator=(const BCIHandle&) = delete;

private:
    ~BCIHandle()
    {
        if (mDeleter)
            mDeleter(mpObject);
    }

public:
    void* Object()
    {
        return mpObject;
    }

    void AddRef()
    {
        Assert(mRefCount > 0);
        ++mRefCount;
    }
    void Release()
    {
        if (--mRefCount == 0)
            delete this;
    }

private:
    void* mpObject = nullptr;
    std::atomic<int32_t> mRefCount = 1;
    void(*mDeleter)(void*) = nullptr;
};

template<class T> void DeleteObject(void* obj)
{
    delete static_cast<T*>(obj);
}

template<class T> void* CreateBCIHandle(T* t)
{
    BCIHandle* handle = new BCIHandle;
    handle->mDeleter = &DeleteObject<T>;
    handle->mpObject = t;
    return handle;
}

/////////////////////////////////////////////////
struct SignalDataWrapper
{
    SignalDataWrapper(const GenericSignal& signal)
    : mSignal(signal)
    {
        mSignal.EnsureDeepCopy();
    }
    GenericSignal mSignal;
};

#endif // BCI_HANDLE_H
