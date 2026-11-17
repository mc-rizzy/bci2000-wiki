function parameters = load_bciprm(varargin)
%LOAD_BCIPRM Load parameters from one or more BCI2000 parameter files
%  into a Matlab workspace variable.
%
%  parameters = load_bciprm('filename1', 'filename2', ...)
%
%  loads parameter data from the files whose names are given
%  as function arguments. The output is a Matlab struct, formatted in 
%  the same ways as the parameter output of load_bcidat.
%
%  The load_bciprm function is part of the BCI2000 project. 
%
%  $BEGIN_BCI2000_LICENSE$
%  
%  This file is part of BCI2000, a platform for real-time bio-signal research.
%  [ Copyright (C) 2000-2026: BCI2000 team and many external contributors ]
%  
%  BCI2000 is free software: you can redistribute it and/or modify it under the
%  terms of the GNU General Public License as published by the Free Software
%  Foundation, either version 3 of the License, or (at your option) any later
%  version.
%  
%  BCI2000 is distributed in the hope that it will be useful, but
%                          WITHOUT ANY WARRANTY
%  - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
%  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
%  
%  You should have received a copy of the GNU General Public License along with
%  this program.  If not, see <http://www.gnu.org/licenses/>.
%  
%  $END_BCI2000_LICENSE$

%  This is a help file documenting the functionality contained in
%  load_bciprm.mex.
%  $Id$
%
missing_mex_file;
