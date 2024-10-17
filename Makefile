#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
# Set toolchain location in an environment var for future use, this will change
# to use a system environment var in the future.
#---------------------------------------------------------------------------------
ifeq ($(strip $(FXCGSDK)),)
$(error FXCGSDK is not set)
endif

include $(FXCGSDK)/toolchain/prizm_rules


#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:=	$(notdir $(CURDIR))
BUILD		:=	build
SOURCES		:=	src
DATA		:=	data  
INCLUDES	:=
GENERATED	:=	src/charset/gen.hpp

#---------------------------------------------------------------------------------
# options for code and add-in generation
#---------------------------------------------------------------------------------

MKG3AFLAGS := -n basic:methodology -i uns:../icons/unselected.bmp -i sel:../icons/selected.bmp
MKG3AFILES := ../icons/unselected.bmp ../icons/selected.bmp

# Optional: add -flto to CFLAGS and LDFLAGS to enable link-time optimization
# (LTO). Doing so will usually allow the compiler to generate much better code
# (smaller and/or faster), but may expose bugs in your code that don't cause
# any trouble without LTO enabled.
CFLAGS	= -Os -Wall $(MACHDEP) $(INCLUDE) -ffunction-sections -fdata-sections
CXXFLAGS	=	$(CFLAGS) -fno-exceptions

LDFLAGS	= $(MACHDEP) -T$(FXCGSDK)/toolchain/prizm.x -Wl,-static -Wl,-gc-sections

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:=	 -lc -lfxcg -lgcc

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) \
					$(sFILES:.s=.o) $(SFILES:.S=.o)

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES), -iquote $(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD) -I$(LIBFXCG_INC)

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
					-L$(LIBFXCG_LIB)

export OUTPUT	:=	$(CURDIR)/$(TARGET)
.PHONY: g3a clean cleangen \
		cleanmethodlib cleanmethods methodgen \
		all cleanall \
		force

#---------------------------------------------------------------------------------
g3a: $(BUILD) $(GENERATED)
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

$(BUILD):
	@mkdir $@

src/charset/gen.hpp: prizmunicode/charmap.py prizmunicode/searchmap.py prizmunicode/genhpp.py
	py -m prizmunicode.genhpp create_hpp src/charset/gen.hpp

#---------------------------------------------------------------------------------
export CYGWIN := nodosfilewarning
clean:
	$(call rmdir,$(BUILD))
	$(call rm,$(OUTPUT).bin)
	$(call rm,$(OUTPUT).g3a)

cleangen:
	$(foreach gen,${GENERATED},$(call rm,$(gen));)

#---------------------------------------------------------------------------------
METHODS_URL	:=	https://methods.cccbr.org.uk/xml/CCCBR_methods.xml.zip
METHODS_DIR	:=	methods
METHODS_XML	:=	CCCBR_methods.xml.zip

methodgen: $(METHODS_XML) methodconv.py prizmunicode/charmap.py prizmunicode/searchmap.py | $(METHODS_DIR)
	py methodconv.py $(METHODS_XML)

$(METHODS_DIR):
	@mkdir $@

$(METHODS_XML): force
# -z only updates if the remote has been updated
	curl "$(METHODS_URL)" -o $(METHODS_XML) -Rz $(METHODS_XML)

cleanmethodlib:
	$(call rm,$(METHODS_XML))

cleanmethods:
	$(call rmdir,$(METHODS_DIR))

#---------------------------------------------------------------------------------
all: g3a methodgen
cleanall: clean cleangen cleanmethods cleanmethodlib

#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).g3a: $(OUTPUT).bin $(MKG3AFILES)
$(OUTPUT).bin: $(OFILES)


-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
