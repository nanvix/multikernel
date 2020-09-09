#===============================================================================
# Toolchain Configuration
#===============================================================================

# Compiler Options
ifneq ($(LIBLWIP),)
CFLAGS += -I $(INCDIR)/posix
endif

# Libraries
LIBS := -Wl,--whole-archive
LIBS += $(LIBDIR)/$(LIBHAL)
LIBS += $(LIBDIR)/$(LIBKERNEL)
LIBS += -Wl,--no-whole-archive
LIBS += $(LIBDIR)/$(LIBRUNTIME)
LIBS += $(LIBDIR)/$(LIBC)
LIBS += $(LIBDIR)/$(LIBNANVIX)
ifneq ($(LIBLWIP),)
LIBS += $(LIBDIR)/$(LIBLWIP)
endif
LIBS += $(LIBDIR)/$(BARELIB) $(THEIR_LIBS)

#===============================================================================
# Binaries Sources and Objects
#===============================================================================

# Binary
EXEC = nanvix-benchmark-model.$(OBJ_SUFFIX)

# C Source Files
SRC = $(wildcard *.c)                \
      $(wildcard name/*.c)           \
      $(wildcard posix/*.c)          \
      $(wildcard rmem/manager/*.c)   \
      $(wildcard rmem/cache/*.c)     \
      $(wildcard rmem/stub/*.c)

# Object Files
OBJ = $(SRC:.c=.$(OBJ_SUFFIX).o)

#===============================================================================

ifeq ($(TARGET),unix64)
LINKER_SCRIPT=
else
LINKER_SCRIPT = -L $(LINKERDIR)/ -T link.ld
endif

# Builds All Object Files
all: $(OBJ)
ifeq ($(VERBOSE), no)
	@echo [CC] $(EXEC)
	@$(CC) $(LDFLAGS) $(LINKER_SCRIPT) -o $(BINDIR)/$(EXEC) $(OBJ) $(LIBS)
else
	$(CC) $(LDFLAGS) $(LINKER_SCRIPT) -o $(BINDIR)/$(EXEC) $(OBJ) $(LIBS)
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
	@echo [CLEAN] $(EXEC)
	@rm -rf $(BINDIR)/$(EXEC)
else
	rm -rf $(BINDIR)/$(EXEC)
endif

# Builds a C source file.
%.$(OBJ_SUFFIX).o: %.c
ifeq ($(VERBOSE), no)
	@echo [CC] $@
	@$(CC) $(CFLAGS) $< -c -o $@
else
	$(CC) $(CFLAGS) $< -c -o $@
endif
