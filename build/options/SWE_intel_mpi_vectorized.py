#!/usr/bin/python

# @file
# This file is part of SWE.
#
# @author Sebastian Rettenberger (rettenbs AT in.tum.de, http://www5.in.tum.de/wiki/index.php/Sebastian_Rettenberger,_M.Sc.)
#
# @section LICENSE
#
# SWE is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# SWE is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with SWE.  If not, see <http://www.gnu.org/licenses/>.
#
#
# @section DESCRIPTION
#
# Example build parameters using MPI; with vectorized solver; writes netCDF files
#

# Build options
compiler='intel'
parallelization='mpi'
vectorize='on'
solver='fwavevec'
writeNetCDF='yes'

# Print vectorization information (default is 'off')
#showVectorization='on'

# Library paths (only required of not installed in default path)
#netCDFDir=''
