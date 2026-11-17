function save_bciprm(varargin)
%SAVE_BCIPRM Save a Matlab parameter struct into a BCI2000 parameter file. 
%
%  save_bciprm( 'filename', parameters )
%
%  Saves parameter data into the named file.
%  The parameter argument must be a Matlab struct as 
%  created by the <tt>load_bcidat</tt>, or <tt>convert_bciprm</tt> mex
%  files.
%
%  The save_bciprm function is part of the BCI2000 project.
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
%  save_bciprm.mex.
%  $Id$
%
missing_mex_file;
