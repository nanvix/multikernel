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

#===============================================================================
# Binaries Sources and Objects
#===============================================================================

# Object Files
OBJ = $(SRC:.c=.$(OBJ_SUFFIX).o)

#===============================================================================

LINKER_SCRIPT=

# Builds All Object Files
all: spawn0 spawn1 spawn2 spawn3

spawn0: $(OBJ) spawn0.$(OBJ_SUFFIX).o
ifeq ($(VERBOSE), no)
	@echo [CC] nanvix-$@.$(OBJ_SUFFIX)
	@$(CC) $(LDFLAGS) $(LINKER_SCRIPT) -o $(BINDIR)/nanvix-$@.$(OBJ_SUFFIX) $^ $(LIBS)
else
	$(CC) $(LDFLAGS) $(LINKER_SCRIPT) -o $(BINDIR)/nanvix-$@.$(OBJ_SUFFIX) $^ $(LIBS)
endif

spawn1: $(OBJ) spawn1.$(OBJ_SUFFIX).o
ifeq ($(VERBOSE), no)
	@echo [CC] nanvix-$@.$(OBJ_SUFFIX)
	@$(CC) $(LDFLAGS) $(LINKER_SCRIPT) -o $(BINDIR)/nanvix-$@.$(OBJ_SUFFIX) $^ $(LIBS)
else
	$(CC) $(LDFLAGS) $(LINKER_SCRIPT) -o $(BINDIR)/nanvix-$@.$(OBJ_SUFFIX) $^ $(LIBS)
endif

spawn2: $(OBJ) spawn2.$(OBJ_SUFFIX).o
ifeq ($(VERBOSE), no)
	@echo [CC] nanvix-$@.$(OBJ_SUFFIX)
	@$(CC) $(LDFLAGS) $(LINKER_SCRIPT) -o $(BINDIR)/nanvix-$@.$(OBJ_SUFFIX) $^ $(LIBS)
else
	$(CC) $(LDFLAGS) $(LINKER_SCRIPT) -o $(BINDIR)/nanvix-$@.$(OBJ_SUFFIX) $^ $(LIBS)
endif

spawn3: $(OBJ) spawn3.$(OBJ_SUFFIX).o
ifeq ($(VERBOSE), no)
	@echo [CC] nanvix-$@.$(OBJ_SUFFIX)
	@$(CC) $(LDFLAGS) $(LINKER_SCRIPT) -o $(BINDIR)/nanvix-$@.$(OBJ_SUFFIX) $^ $(LIBS)
else
	$(CC) $(LDFLAGS) $(LINKER_SCRIPT) -o $(BINDIR)/nanvix-$@.$(OBJ_SUFFIX) $^ $(LIBS)
endif

# Cleans All Object Files
clean:
ifeq ($(VERBOSE), no)
	@echo [CLEAN] $(OBJ)
	@rm -rf $(OBJ)
else
	rm -rf $(OBJ)
endif

# Cleans Everything
distclean: clean
ifeq ($(VERBOSE), no)
	@echo [CLEAN] nanvix-spawn?.$(OBJ_SUFFIX)
	@rm -rf $(BINDIR)/nanvix-spawn?.$(OBJ_SUFFIX)
else
	rm -rf $(BINDIR)/nanvix-spawn?.$(OBJ_SUFFIX)
endif

# Builds a C source file.
%.$(OBJ_SUFFIX).o: %.c
ifeq ($(VERBOSE), no)
	@echo [CC] $@
	@$(CC) $(CFLAGS) $< -c -o $@
else
	$(CC) $(CFLAGS) $< -c -o $@
endif
