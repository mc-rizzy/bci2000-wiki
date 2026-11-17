# -*- coding: utf-8 -*-

# ///////////////////////////////////////////////////////////////////////////
# $Id: __init__.py 9513 2026-08-17 17:54:24Z jhill $
# Author: jeremy.hill@neurotechcenter.org
# Description: root namespace of the BCI2000Tools package
#
# $BEGIN_BCI2000_LICENSE$
#
# This file is part of BCI2000, a platform for real-time bio-signal research.
# [ Copyright (C) 2000-2022: BCI2000 team and many external contributors ]
#
# BCI2000 is free software: you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# BCI2000 is distributed in the hope that it will be useful, but
#                         WITHOUT ANY WARRANTY
# - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
# A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <http://www.gnu.org/licenses/>.
#
# $END_BCI2000_LICENSE$
# ///////////////////////////////////////////////////////////////////////////

__version__ = '2.0.2'

from . import Bootstrapping; from .Bootstrapping import bci2000root, bci2000path, bci2000_exe_extension, USE_NUMPY_MATRIX

# NB: do NOT import .Remote automatically (it has side effects and may fail, depending on bci2000root() setting)
