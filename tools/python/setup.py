#!/usr/bin/env -S python #
# -*- coding: utf-8 -*-

# ///////////////////////////////////////////////////////////////////////////
# $Id: setup.py 9161 2025-11-04 18:10:45Z jhill $
# Author: jeremy.hill@neurotechcenter.org
# Description: Python pip/setuptools helper file for installing tools/python suite
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

# From the BCI2000 point of view:
"""
The purpose of this file is to make the contents of `tools/python`
available to your Python interpreter (i.e. listed on its `sys.path`) in any
future Python session.

The recommended way to use this file is to open a command prompt, change
your working directory to the location of this `setup.py` file, and then::

    python -m pip install -e .

The "editable" flag `-e` causes Python's setuptools to simply store a link
to the present location of this subdirectory. The advantage of an editable
installation is that you still have only one definitive copy of the code
(possibly under version control) and any update to those files will then be
adopted automatically following a new Python launch. Another advantage is
that the Python code will automatically be able to find other parts of the
same BCI2000 distribution (for example,  `BCI2000Tools.Chain.bci2000chain()`
relies on the executables in `tools/cmdline` for running offline analyses,
and `BCI2000Tools.Remote.BCI2000` relies on `prog/BCI2000Remote.py` for
talking directly to a running instance of BCI2000). The downside is that
Python will fail to find the files if you ever move the BCI2000 distribution
away from its current location (or rename it, or rename any of its parent
directories).

You could remove the `-e` flag if you want setuptools to make a static
*copy* of the modules/packages from the `tools/python` directory, within the
`site-packages` of your Python distribution or in your virtual environment.
This is the usual/expected way of using `pip` (downloading the package from
pypi.org), but it means you give up the advantages mentioned above. Upgrades
and bugfixes will not be released on pypi.org as frequently as on the definitive
bci2000.org version control. You will also not be able to find your complete
BCI2000 distribution programmatically (this might be fine if the only part you
are *ever* going to use is the FileReader, EventRelated analysis tool, and/or
Plotting utilities).
"""

# And now, from the pypi.org point of view:
long_description = """
This toolbox is part of BCI2000, a platform for real-time biosignal acquisition,
analysis and feedback for brain-computer interfacing and related neurotechnologies.
These files provide a mixed collection of tools for reading BCI2000 .dat files,
for performing simple analyses of biosignal data, and for interacting with a
running BCI2000 session.

BCI2000Tools' relationship with your BCI2000 distribution
=========================================================

Some of BCI2000Tools' functionality (reading ``.dat`` files, rudimentary signal
analysis, computing spatial filters, plotting, ...) is self-contained. For this
reason, we decided to release BCI2000Tools as a self-contained package on PyPI,
to take its place in the Python-centric ecosystem.

By contrast, some of the toolbox's functionality relies on BCI2000 binaries that
are not included in the PyPI release: for example, the ``BCI2000Tools.Chain``
submodule is designed as a Python wrapper around the C++-compiled binaries found
in the ``tools/cmdline`` subdirectory of a full BCI2000 distribution;
``BCI2000Tools.Remote`` is a wrapper around BCI2000's ``prog/BCI2000Remote.py``,
which in turn wraps a dynamic library from, and sends messages to the binaries in,
the ``prog`` directory. For this reason, the canonical way to install and manage
BCI2000Tools is not via PyPI at all, but via the bci2000.org subversion repository::

    # (1) register for a free username on bci2000.org;
    # (2) install an svn client; then:
    svn checkout https://bci2000.org/svn/trunk  bci2000-svn
    cd bci2000-svn/tools/python  # BCI2000Tools and its setup.py both live here
    python -m pip install -e .   # install in-place ("editable")

and if you do it that way, and leave BCI2000Tools in that location, BCI2000Tools
will automatically find the corresponding ``tools/cmdline/*`` and ``prog/*``
elements (provided you have built them).

If you install another way (e.g. from PyPI via a standard non-editable install,
``python -m pip install BCI2000Tools``), you would need to specify the root
directory of your BCI2000 distribution explicitly---certainly before you import
``BCI2000Tools.Remote`` and ideally also before you import ``BCI2000Tools.Chain``.
Here is how to do that::

    from BCI2000Tools import bci2000root
    bci2000root('/PATH/TO/BCI2000/DISTRO/WITH/BINARIES')
    
    from BCI2000Tools.Remote import BCI2000
    from BCI2000Tools.Chain import bci2000chain

If you do ``from BCI2000Tools.AllTools import *`` Python will attempt to import
all the submodules, and you will see a message telling you which imports did and
did not succeed, along with any reasons for failure.

"""
import os
import re
import ast
import sys
import inspect
import setuptools


try: __file__
except:
	frame = inspect.currentframe()
	try: __file__ = inspect.getfile( frame )
	finally: del frame  # https://docs.python.org/3/library/inspect.html#the-interpreter-stack
HERE = os.path.realpath( os.path.dirname( __file__ ) ) # full path to the parent directory of this setup.py file

package_dir = '.' # the directory that would get added to the path, expressed relative to the location of this setup.py file

# import the package itself - but make sure it's the to-be-installed version and not some legacy version hanging over
#sys.path.insert( 0, os.path.realpath( os.path.join( HERE, package_dir ) ) )
#import BCI2000Tools  # could use this to get the __version__ etc
#sys.path.pop( 0 )

setup_args = dict(
	package_dir={ '' : package_dir },
	packages=[ 'BCI2000Tools' ],
	name='BCI2000Tools', # NB: if we ever add other whole packages at the level of BCI2000Tools, this should be changed
	description = 'A suite of Python tools for working with BCI2000 data files, processing chains, and live sessions',
	long_description = long_description,
	long_description_content_type='text/x-rst',
	license = "GPL v3+",
	url = 'https://bci2000.org',
	#author = meta[ 'author' ],
	#author_email = meta[ 'email' ],
	#python_requires = '>=2.7, !=3.0.*, !=3.1.*, !=3.2.*, <4',
	install_requires=[ 'numpy' ], # matplotlib is also a nice-to-have too but not essential
)

version_finder = re.compile( r'^__version__\s*=\s*(\S*)\s*(#.*)?$' )
with open( os.path.join( HERE, package_dir, 'BCI2000Tools', '__init__.py' ) ) as fh:
	for line in fh:
		match = re.match( version_finder, line )
		if match: setup_args[ 'version' ] = ast.literal_eval( match.group( 1 ) ); break

if __name__ == '__main__' and getattr( sys, 'argv', [] )[ 1: ]:
	setuptools.setup( **setup_args )
else:
	sys.stderr.write( """
The BCI2000Tools setup.py file should not be run or imported directly.
Instead, it is typically used implicitly, as follows::

    python -m pip install -e  "%s"

""" % HERE )

