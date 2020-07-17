#
# MIT License
#
# Copyright(c) 2011-2020 The Maintainers of Nanvix
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

#===============================================================================
# Tweak Files
#===============================================================================

# Source Files
export SRC += $(wildcard $(CURDIR)/common/*.c)

#===============================================================================

# Builds Everything
all: all-name all-rmem all-hello all-vfs all-sysv

# Cleans Build Objects
clean: clean-name clean-rmem clean-hello clean-vfs clean-sysv

# Cleans Everything
distclean: distclean-name distclean-rmem distclean-hello distclean-vfs \
	distclean-sysv

#===============================================================================
# Name Server
#===============================================================================

# Builds Name Server.
all-name:
	$(MAKE) -C pm/name all

# Cleans Name Server Build objects.
clean-name:
	$(MAKE) -C pm/name clean

# Cleans Name Server build.
distclean-name:
	$(MAKE) -C pm/name distclean

#===============================================================================
# RMem Server
#===============================================================================

# Builds RMem Server.
all-rmem:
	$(MAKE) -C mm/rmem all

# Cleans RMem Server Build objects.
clean-rmem:
	$(MAKE) -C mm/rmem clean

# Cleans RMem Server build.
distclean-rmem:
	$(MAKE) -C mm/rmem distclean

#===============================================================================
# SYSV Server
#===============================================================================

# Builds SYSV Server.
all-sysv:
	$(MAKE) -C pm/sysv all

# Cleans build objects.
clean-sysv:
	$(MAKE) -C pm/sysv clean

# Cleans build.
distclean-sysv:
	$(MAKE) -C pm/sysv distclean

#===============================================================================
# VFS Server
#===============================================================================

# Builds VFS Server.
all-vfs:
	$(MAKE) -C fs/vfs all

# Cleans build objects.
clean-vfs:
	$(MAKE) -C fs/vfs clean

# Cleans build.
distclean-vfs:
	$(MAKE) -C fs/vfs distclean

#===============================================================================
# Hello Server
#===============================================================================

# Builds Hello Server.
all-hello:
	$(MAKE) -C hello all

# Cleans Hello Server Build objects.
clean-hello:
	$(MAKE) -C hello clean

# Cleans Hello Server build.
distclean-hello:
	$(MAKE) -C hello distclean
