///////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: A Matlab (mex) subroutine that reads BCI2000 data through
//   the SignalSharing interface.
//
//  ok = signal_sharing('listen', '127.0.0.1:1905');
//  while ok
//      [signal, states, parameters, ok] = signal_sharing('acquire', 1000);
//  end
//  ok = signal_sharing('shutdown');
//
//  Reads signal, state, and parameter data through a socket connection as
//  given by the 'listen' argument.
//  Typically, only one of the signal, states, parameters arguments is
//  nonempty upon return from 'acquire'.
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
///////////////////////////////////////////////////////////////////////////////
#include "libmex.imports.h"

#include "BCIException.h"
#include "BCIStream.h"
#include "SignalSharingClient.h"

#include "mexutils.h"

static SignalSharingClient* spClient;

static mxArray* SignalToMatrix(const GenericSignal & signal)
{
    mwSize dim[] = { signal.Elements(), signal.Channels() };
    mxArray* matrix = ::mxCreateNumericArray(2, dim, mxDOUBLE_CLASS, mxREAL);
    if (!matrix)
        throw bciexception << "Out of memory when allocating space for an output variable.";
    auto pMatrixData = ::mxGetDoubles(matrix);
    auto pSignalData = signal.ConstData();
    for (int i = 0; i < dim[0] * dim[1]; ++i)
        *pMatrixData++ = *pSignalData++;
    return matrix;
}

static mxArray* EmptyMatrix()
{
    mwSize dim[] = { 0, 0 };
    return ::mxCreateNumericArray(2, dim, mxDOUBLE_CLASS, mxREAL);
}

static void DoListen(int nargout, mxArray * varargout[], int nargin, const mxArray * varargin[])
{
    if (nargin < 2 || mxGetClassID(varargin[1]) != mxCHAR_CLASS)
        throw bciexception << "Listening address in the form '<ip>:<port>' expected in the second argument.";
    char* stringArg = mxArrayToString(varargin[1]);
    if (stringArg == NULL)
        throw bciexception << "Out of memory when reading string argument.";

    if (spClient)
        throw bciexception << "Already listening at " << spClient->ListeningAddress() << ".";
    spClient = new SignalSharingClient(stringArg);
    if (nargout > 0)
        varargout[0] = ::mxCreateDoubleScalar(1);
}

static void DoShutdown(int nargout, mxArray* varargout[], int nargin, const mxArray* varargin[])
{
    delete spClient;
    spClient = nullptr;
    if (nargout > 0)
        varargout[0] = ::mxCreateDoubleScalar(1);
}

static void ReturnSignal(int nargout, mxArray* varargout[])
{
    if (nargout > 0)
        varargout[0] = SignalToMatrix(spClient->Signal());
    if (nargout > 1)
        varargout[1] = EmptyMatrix();
    if (nargout > 2)
        varargout[2] = EmptyMatrix();
}

static void ReturnStates(int nargout, mxArray* varargout[])
{
    if (nargout > 0)
        varargout[0] = EmptyMatrix();
    if (nargout > 1)
        varargout[1] = SignalToMatrix(spClient->States());
    if (nargout > 2)
        varargout[2] = EmptyMatrix();
}

static void ReturnParameters(int nargout, mxArray* varargout[])
{
    if (nargout > 0)
        varargout[0] = EmptyMatrix();
    if (nargout > 1)
        varargout[1] = EmptyMatrix();
    if (nargout > 2)
        varargout[2] = ParamlistToStruct(spClient->Parameters());
}

static void ReturnEmpty(int nargout, mxArray* varargout[])
{
    if (nargout > 0)
        varargout[0] = EmptyMatrix();
    if (nargout > 1)
        varargout[1] = EmptyMatrix();
    if (nargout > 2)
        varargout[2] = EmptyMatrix();
}

static void DoAcquire(int nargout, mxArray* varargout[], int nargin, const mxArray* varargin[])
{
    if (!spClient)
        throw bciexception << "Not listening, execute 'listen' first";

    int timeout = 1000;
    if (nargin > 1) {
        bool ok = ::mxGetClassID(varargin[1]) == mxDOUBLE_CLASS;
        if (ok) {
            auto dim = ::mxGetDimensions(varargin[1]);
            if (dim[0] != 1 || dim[1] != 1)
                ok = false;
        }
        if (ok) {
            timeout = std::ceil(*::mxGetDoubles(varargin[1]));
            if (timeout < 0)
                ok = false;
        }
        if (!ok)
            throw bciexception << "When a second argument is given, a timeout in ms is expected";
    }

    int kind = spClient->WaitForData(timeout);
    switch (kind)
    {
        case SignalSharingClient::signal:
            ReturnSignal(nargout, varargout);
            break;
        case SignalSharingClient::states:
            ReturnStates(nargout, varargout);
            break;
        case SignalSharingClient::parameters:
            ReturnParameters(nargout, varargout);
            break;
        case SignalSharingClient::none:
        default:
            ReturnEmpty(nargout, varargout);
            break;
    }
    if (nargout > 3)
        varargout[3] = ::mxCreateDoubleScalar(kind);
}

void BCIMexFunction(int nargout, mxArray *varargout[], int nargin, const mxArray *varargin[])
{
    if (PrintVersion(__FILE__, nargin, varargin))
        return;

    if (nargin < 1 || mxGetClassID(varargin[0]) != mxCHAR_CLASS)
        throw bciexception << "One of 'listen', 'acquire', 'shutdown' expected in first argument.";
    char* stringArg = mxArrayToString(varargin[0]);
    if (stringArg == NULL)
        throw bciexception << "Out of memory when reading string argument.";
    if (!::stricmp(stringArg, "listen"))
        DoListen(nargout, varargout, nargin, varargin);
    else if (!::stricmp(stringArg, "acquire"))
        DoAcquire(nargout, varargout, nargin, varargin);
    else if (!::stricmp(stringArg, "shutdown"))
        DoShutdown(nargout, varargout, nargin, varargin);
    else
        throw bciexception << "Unknown command, commands are: 'listen', 'acquire', 'shutdown'.";
}
