///////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: A Matlab (mex) subroutine that writes BCI2000 parameters from
//   a Matlab struct into a BCI2000 .prm file.
//
//  save_bciprm( parameters, 'filename1' )
//
//  saves parameter data from the 'parameters' struct into a file whose name
//  are given as the next function argument.
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
#include <cstring>
#include "mexutils.h"

void BCIMexFunction(int nargout, mxArray *varargout[], int nargin, const mxArray *varargin[])
{
    if (PrintVersion(__FILE__, nargin, varargin))
        return;

    if (nargin != 2)
        throw bciexception << "Expected two arguments, an output file name, and a parameter struct";

    if (mxGetClassID(varargin[0]) != mxCHAR_CLASS)
        throw bciexception << "File name expected in first argument";

    if (mxGetClassID(varargin[1]) != mxSTRUCT_CLASS)
        throw bciexception << "Parameter struct expected in second argument";

    char* stringArg = mxArrayToString(varargin[0]);
    if (stringArg == nullptr)
        throw bciexception << "Out of memory when reading string argument";

    std::string filename = stringArg;
    mxFree(stringArg);

    ParamList params;
    StructToParamlist(varargin[1], params);

    if (!params.Save(filename))
        throw bciexception << "Could not save parameters to " << filename;
}