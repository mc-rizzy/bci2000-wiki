///////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: A Matlab (mex) subroutine that reads BCI2000 .prm files into
//  a Matlab workspace variables.
//
//  parameters = load_bciprm( 'filename1', 'filename2', ... )
//
//  loads parameter data from the files whose names are given as function 
//  arguments.
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

    ParamList allParams;
    int i = 0;
    while (i < nargin)
    {
        if (mxGetClassID(varargin[i]) != mxCHAR_CLASS)
            throw bciexception << "File name expected in argument " << i + 1 << ".";
        char *stringArg = mxArrayToString(varargin[i]);
        if (stringArg == nullptr)
        {
            throw bciexception << "Out of memory when reading string argument.";
        }
        else
        {
            ParamList fileParams;
            bool ok = fileParams.Load(stringArg);
            if (!ok)
                ok = fileParams.Load(std::string(stringArg) + ".dat");
            if (!ok)
                throw bciexception << "Could not open \"" << stringArg << "\" as a BCI2000 parameter file.";
            allParams.CopyFrom(fileParams);
        }
        mxFree(stringArg);
        ++i;
    }

    if (nargout > 0)
        varargout[0] = ParamlistToStruct(allParams);
}
