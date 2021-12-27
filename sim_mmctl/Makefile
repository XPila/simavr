#-----------------------------------------------------------------------------------------------------------------------
# Makefile to compile and link sim_mmctl
PROJECT ?= sim_mmctl

# output folder (output directory tree will be automaticaly created)
OUT := build

# gcc, g++, ld
GCC     := gcc
GPP     := g++
LD      := g++

# output files
OUTDIR := $(OUT)/$(PROJECT).dir # file containing list of created directories
OUTMAP := $(OUT)/$(PROJECT).map # map output file
OUTEXE := $(OUT)/$(PROJECT).exe # exe output file (for simulator/win)

# file path separator (for unix '/', for windows '\\', automaticaly determined from PATH)
PATH_SEPARATOR := $(if $(findstring \,$(PATH)),\,/)
# unix shell or windows shell - determine by PATH_SEPARATOR
SHELL_IS_UNIX := $(findstring /,$(PATH_SEPARATOR))
# shell command for removing directories
SHELL_RMDIR ?= $(if $(SHELL_IS_UNIX),rm -rvf,RMDIR /S /Q)
# shell command for creating directories
SHELL_MKDIR ?= $(if $(SHELL_IS_UNIX),mkdir -p,MKDIR)
# shell command for removing files
SHELL_RM ?= $(if $(SHELL_IS_UNIX),rm,DEL /Q)
# shell command for renaming files/dirs
SHELL_MV ?= $(if $(SHELL_IS_UNIX),mv,REN)
# shell command for copying files
SHELL_CP ?= $(if $(SHELL_IS_UNIX),cp,COPY)
# shell command 'cat'
SHELL_CAT ?= $(if $(SHELL_IS_UNIX),cat,TYPE)
# shell command separator
SHELL_CMDSEP ?= $(if $(SHELL_IS_UNIX), ; , & )

INCLUDES := -I../simavr -I../simavr/cores -I../simavr/sim -I../examples/parts

GCCFLAGS := $(INCLUDES) -Dstrsep=strtok
LDFLAGS := -lelf -lm -lws2_32

# simavr source files (all *.c from folders 'sim' and 'cores')
SIMSRC_SIM := $(wildcard ../simavr/sim/*.c)
SIMSRC_CORES := $(wildcard ../simavr/cores/*.c)
SIMSRC := $(SIMSRC_SIM) $(SIMSRC_CORES)
# exclude core sim_mega328pb.c because is missing
#SIMSRC := $(subst ../simavr/cores/sim_mega328pb.c,,$(SIMSRC))
# exclude run_avr.c (original main)
SIMSRC := $(subst ../simavr/sim/run_avr.c,,$(SIMSRC))
# append uart_pty.c
#SIMSRC := $(SIMSRC) ../examples/parts/uart_pty.c
# append main.c
ALLSRC := $(SIMSRC) main.c uart_tcp.c mmctl_shr16.c mmctl_leds.c
# filter all *.c files
GCCSRC := $(filter %.c, $(ALLSRC))
# filter all *.cpp files
GPPSRC := $(filter %.cpp, $(ALLSRC))

GCCOBJ := $(addprefix $(OUT)/,$(subst ../,,$(GCCSRC:.c=.o)))
GPPOBJ := $(addprefix $(OUT)/,$(subst ../,,$(GPPSRC:.c=.o)))
ALLOBJ := $(GCCOBJ) $(GPPOBJ)


# list of all directories
#ALLDIR := $(addprefix $(OUT)/,$(sort $(subst / , ,$(dir $(subst ../,,$(ALLSRC))) )))
ALLDIR := $(sort $(subst / , ,$(dir $(ALLOBJ)) ))


$(OUTDIR):
	@echo creating output directory tree
#	@$(SHELL_MKDIR) $(subst /,$(PATH_SEPARATOR),$(OUT))
	@$(SHELL_MKDIR) $(subst /,$(PATH_SEPARATOR),$(ALLDIR))
	@echo $(ALLDIR) >$(OUTDIR)

$(OUT)/simavr/%.o: ../simavr/%.c $(OUTDIR)
	@echo compiling $<
	@$(GCC) -c $(GCCFLAGS) -o $@ $<

$(OUT)/examples/%.o: ../examples/%.c $(OUTDIR)
	@echo compiling $<
	@$(GCC) -c $(GCCFLAGS) -o $@ $<

$(OUT)/%.o: %.c $(OUTDIR)
	@echo compiling $<
	@$(GCC) -c $(GCCFLAGS) -o $@ $<

$(OUTEXE): $(GCCOBJ)
	@echo linking $@
	@$(LD) -o $(OUTEXE) $(GCCOBJ) $(LDFLAGS)


all: exe

exe: $(OUTEXE)


clean:
	@echo removing output files $(OUT)
ifneq ("$(wildcard $(OUT))","")
	@$(SHELL_RMDIR) $(subst /,$(PATH_SEPARATOR),$(OUT))
endif


test:
	@echo $(ALLDIR)

printvars:
	@echo PROJECT=$(PROJECT)
	@echo OUT=$(OUT)
	@echo GCC=$(GCC)
	@echo GPP=$(GPP)
	@echo LD=$(LD)
	@echo OUTDIR=$(OUTDIR)
	@echo OUTMAP=$(OUTMAP)
	@echo OUTEXE=$(OUTEXE)
	@echo PATH_SEPARATOR=$(PATH_SEPARATOR)
	@echo SHELL_IS_UNIX=$(SHELL_IS_UNIX)
	@echo SHELL_RMDIR=$(SHELL_RMDIR)
	@echo SHELL_MKDIR=$(SHELL_MKDIR)
	@echo SHELL_RM=$(SHELL_RM)
	@echo SHELL_MV=$(SHELL_MV)
	@echo SHELL_CAT=$(SHELL_CAT)
	@echo SHELL_CMDSEP=$(SHELL_CMDSEP)

MYS2_DLL_PATH := bin/msys2/
copy_msys_dlls:
	$(SHELL_CP) $(MYS2_DLL_PATH)msys-2.0.dll $(OUT)
	$(SHELL_CP) $(MYS2_DLL_PATH)msys-elf-0.dll $(OUT)
	$(SHELL_CP) $(MYS2_DLL_PATH)msys-gcc_s-seh-1.dll $(OUT)
