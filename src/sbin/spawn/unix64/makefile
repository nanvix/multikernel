#
# MIT License
#
# Copyright(c) 2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
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

include $(BUILDDIR)/makefile.config

#===============================================================================
# Binaries Sources and Objects
#===============================================================================

# Binaries
SPAWN0_EXEC = nanvix-spawn0.$(OBJ_SUFFIX)
SPAWN1_EXEC = nanvix-spawn1.$(OBJ_SUFFIX)
SPAWN2_EXEC = nanvix-spawn2.$(OBJ_SUFFIX)
SPAWN3_EXEC = nanvix-spawn3.$(OBJ_SUFFIX)

# Source Files
SPAWN0_SRC = $(SRC) spawn0.c
SPAWN1_SRC = $(SRC) spawn1.c
SPAWN2_SRC = $(SRC) spawn2.c
SPAWN3_SRC = $(SRC) spawn3.c

# Object Files
SPAWN0_OBJ = $(SPAWN0_SRC:.c=.$(OBJ_SUFFIX).o)
SPAWN1_OBJ = $(SPAWN1_SRC:.c=.$(OBJ_SUFFIX).o)
SPAWN2_OBJ = $(SPAWN2_SRC:.c=.$(OBJ_SUFFIX).o)
SPAWN3_OBJ = $(SPAWN3_SRC:.c=.$(OBJ_SUFFIX).o)

#===============================================================================
# Intermediate rules 
#===============================================================================

# Builds All Object Files
all-spawn:
	$(MAKE) all OBJ="$(SPAWN0_OBJ)" EXEC=$(SPAWN0_EXEC)
	$(MAKE) all OBJ="$(SPAWN1_OBJ)" EXEC=$(SPAWN1_EXEC)
	$(MAKE) all OBJ="$(SPAWN2_OBJ)" EXEC=$(SPAWN2_EXEC)
	$(MAKE) all OBJ="$(SPAWN3_OBJ)" EXEC=$(SPAWN3_EXEC)

# Cleans All Object Files
clean-spawn:
	$(MAKE) clean OBJ="$(SPAWN0_OBJ)" EXEC=$(SPAWN0_EXEC)
	$(MAKE) clean OBJ="$(SPAWN1_OBJ)" EXEC=$(SPAWN1_EXEC)
	$(MAKE) clean OBJ="$(SPAWN2_OBJ)" EXEC=$(SPAWN2_EXEC)
	$(MAKE) clean OBJ="$(SPAWN3_OBJ)" EXEC=$(SPAWN3_EXEC)

# Cleans Everything
distclean-spawn:
	$(MAKE) distclean OBJ="$(SPAWN0_OBJ)" EXEC=$(SPAWN0_EXEC)
	$(MAKE) distclean OBJ="$(SPAWN1_OBJ)" EXEC=$(SPAWN1_EXEC)
	$(MAKE) distclean OBJ="$(SPAWN2_OBJ)" EXEC=$(SPAWN2_EXEC)
	$(MAKE) distclean OBJ="$(SPAWN3_OBJ)" EXEC=$(SPAWN3_EXEC)

#===============================================================================

include $(BUILDDIR)/makefile.executable
