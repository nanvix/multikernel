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

# Builds Everything
all: all-zombie all-test all-model

# Cleans Build Objects
clean: clean-zombie clean-test clean-model

# Cleans Everything
distclean: distclean-zombie distclean-test distclean-model

#===============================================================================
# Zombie Server
#===============================================================================

# Builds Zombie Server.
all-zombie:
	$(MAKE) -C zombie all

# Cleans Zombie Server Build objects.
clean-zombie:
	$(MAKE) -C zombie clean

# Cleans Zombie Server build.
distclean-zombie:
	$(MAKE) -C zombie distclean

#===============================================================================
# Test Server
#===============================================================================

# Builds Test Server.
all-test:
	$(MAKE) -C test all

# Cleans Test Server Build objects.
clean-test:
	$(MAKE) -C test clean

# Cleans Test Server build.
distclean-test:
	$(MAKE) -C test distclean

#===============================================================================
# Model
#===============================================================================

# Builds Model Server.
all-model:
	$(MAKE) -C model all

# Cleans Model Server Build objects.
clean-model:
	$(MAKE) -C model clean

# Cleans Model Server build.
distclean-model:
	$(MAKE) -C model distclean
