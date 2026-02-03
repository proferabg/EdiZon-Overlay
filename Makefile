#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/libnx/switch_rules

APP_TITLE		:=	EdiZon
APP_FILENAME	:=  ovlEdiZon
APP_AUTHOR		:=	WerWolv, proferabg, and ppkantorski, Arch9SK7
APP_VERSION		:=	v1.0.15

ifeq ($(RELEASE), 1)
	APP_VERSION	:=	$(APP_VERSION)-$(shell git describe --always)
endif

TARGET			:=	$(APP_TITLE)
OUTDIR			:=	out
BUILD			:=	build
SOURCES_TOP		:=	source
SOURCES			+=  $(foreach dir,$(SOURCES_TOP),$(shell find $(dir) -type d 2>/dev/null))
INCLUDES		:=	include
#EXCLUDES		:=  dmntcht.c
DATA			:=	data

# This location should reflect where you place the libultrahand directory (lib can vary between projects).
include ${TOPDIR}/libs/libultrahand/ultrahand.mk

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
null      	:=
SPACE     	:=  $(null) $(null)

ARCH	:=	-march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIE

CFLAGS	:= -g -Wall -O3 -ffunction-sections -fdata-sections -flto -ffast-math -fomit-frame-pointer \
					-fuse-linker-plugin -finline-small-functions -fno-strict-aliasing -frename-registers -falign-functions=16 \
					$(ARCH) $(DEFINES) -DVERSION_STRING=\"$(subst $(SPACE),\$(SPACE),${APP_VERSION})\"


CFLAGS	+=	$(INCLUDE) -D__SWITCH__ -D__OVERLAY__ -I$(PORTLIBS)/include/freetype2 $(pkg-config --cflags --libs python3) -Wno-deprecated-declarations 
CFLAGS	+=	-DAPP_VERSION=\"$(APP_VERSION)\" -DAPP_TITLE=\"$(APP_TITLE)\" -DAPP_AUTHOR=\""$(APP_AUTHOR)"\"

CXXFLAGS	:= $(CFLAGS) -fexceptions -std=c++26

ASFLAGS	:=	-g $(ARCH)
LDFLAGS	=	-specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

LIBS	:= -lnx

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(CURDIR)/libs/nxpy $(PORTLIBS) $(LIBNX)


#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(OUTDIR)/$(APP_FILENAME)
export TOPDIR	:=	$(CURDIR)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)


CFILES			:=	$(filter-out $(EXCLUDES),$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c))))
CPPFILES		:=	$(filter-out $(EXCLUDES),$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp))))
SFILES			:=	$(filter-out $(EXCLUDES),$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s))))
BINFILES		:=	$(filter-out $(EXCLUDES),$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*))))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES_BIN	:=	$(addsuffix .o,$(BINFILES))
export OFILES_SRC	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES 		:=	$(OFILES_BIN) $(OFILES_SRC)
export HFILES_BIN	:=	$(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

ifeq ($(strip $(CONFIG_JSON)),)
	jsons := $(wildcard *.json)
	ifneq (,$(findstring $(TARGET).json,$(jsons)))
		export APP_JSON := $(TOPDIR)/$(TARGET).json
	else
		ifneq (,$(findstring config.json,$(jsons)))
			export APP_JSON := $(TOPDIR)/config.json
		endif
	endif
else
	export APP_JSON := $(TOPDIR)/$(CONFIG_JSON)
endif

ifeq ($(strip $(ICON)),)
	icons := $(wildcard *.jpg)
	ifneq (,$(findstring $(TARGET).jpg,$(icons)))
		export APP_ICON := $(TOPDIR)/$(TARGET).jpg
	else
		ifneq (,$(findstring icon.jpg,$(icons)))
			export APP_ICON := $(TOPDIR)/icon.jpg
		endif
	endif
else
	export APP_ICON := $(TOPDIR)/$(ICON)
endif

ifeq ($(strip $(NO_ICON)),)
	export NROFLAGS += --icon=$(APP_ICON)
endif

ifeq ($(strip $(NO_NACP)),)
	export NROFLAGS += --nacp=$(OUTPUT).nacp
endif

ifneq ($(APP_TITLEID),)
	export NACPFLAGS += --titleid=$(APP_TITLEID)
endif

ifneq ($(ROMFS),)
	export NROFLAGS += --romfsdir=$(CURDIR)/$(ROMFS)
endif

.PHONY: $(BUILD) clean all install

#---------------------------------------------------------------------------------
all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@ $(BUILD) $(OUTDIR)
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
	@rm -rf SdOut
	@mkdir -p SdOut/switch/.overlays
	@cp -rf $(OUTPUT).ovl SdOut/switch/.overlays/
	@cd $(CURDIR)/SdOut; zip -r -q -9 $(APP_TITLE)-Overlay.zip switch; cd $(CURDIR)

#---------------------------------------------------------------------------------
clean:
	@echo " RM   " $(BUILD) $(OUTDIR) SdOut
	@rm -fr $(BUILD) $(OUTDIR) SdOut

#---------------------------------------------------------------------------------
install: all
	@echo " LFTP " $@
	@lftp -e "put -O /switch/.overlays ./out/$(APP_FILENAME).ovl;bye" $(IP)

#---------------------------------------------------------------------------------
else
.PHONY:	all

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all	:  $(OUTPUT).ovl

$(OUTPUT).ovl	:	$(OUTPUT).nro
	@cp $(OUTPUT).nro $(OUTPUT).ovl
	@printf 'ULTR' >> $(OUTPUT).ovl
	@echo "Ultrahand signature has been added."

$(OUTPUT).nsp	:	$(OUTPUT).nso $(OUTPUT).npdm

$(OUTPUT).nso	:	$(OUTPUT).elf

$(OUTPUT).nro	:	$(OUTPUT).nso $(OUTPUT).nacp

$(OUTPUT).elf	:	$(OFILES)

$(OFILES_SRC)	: $(HFILES_BIN)

#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data
#---------------------------------------------------------------------------------
%.bin.o	%_bin.h :	%.bin
	@echo " BIN  " $@
	@$(bin2o)

%.ttf.o	%_ttf.h :	%.ttf
	@echo " BIN  " $@
	@$(bin2o)

-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
