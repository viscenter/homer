.SUFFIXES:

%.o: %.cpp
	@echo $(notdir $<)
	$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(CXXFLAGS) -c $< -o $@

%.o: %.c
	@echo $(notdir $<)
	$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d $(CFLAGS) -c $< -o $@

export CC		:=	gcc
export CXX		:=	g++

REMAPOUT	:= remap

UNAME	:=	$(shell uname -s)

# General *nix flags (will be overwritten for Cygwin)
LIBS	:=	 
LDFLAGS		:=
CFLAGS	:=	-O3 

# gprof flags
# CFLAGS	+= -pg -g
# LDFLAGS	+= -pg

# Linux-specific flags
ifneq (,$(findstring Linux,$(UNAME)))	
	CFLAGS	+= `pkg-config --cflags opencv` `pkg-config --cflags gsl`
	LIBS	+= `pkg-config --libs opencv` `pkg-config --libs gsl`
endif
# Cygwin-specific flags
ifneq (,$(findstring CYGWIN,$(UNAME)))
	REMAPOUT	+= .exe
	LIBS	:=	-lhighgui -lcv -lcxcore 
	CFLAGS	+=	-I/usr/include/opencv -I/usr/include/boost-1_33_1
endif
# Mac-specific flags
ifneq (,$(findstring Darwin,$(UNAME)))
	# export CC		:=	/usr/local/bin/gcc
	# export CXX		:=	/usr/local/bin/g++
	# CFLAGS	+=  -ftree-vectorize -fopenmp
	CFLAGS	+=	-I/usr/local/include/opencv -I/opt/local/include -I/opt/local/include/boost-1_34
	LIBS	:=	-L/opt/local/lib -lhighgui -lcv -lcxcore
endif

BUILD	:=	build

REMAPSOURCES		:=	source/remap
SIFTSOURCES		:=	source/sift
INCLUDES		+=	$(REMAPSOURCES) $(SIFTSOURCES)

CFLAGS		+= $(INCLUDE)
CXXFLAGS	:= $(CFLAGS)

ifneq ($(BUILD),$(notdir $(CURDIR))) # not in build directory

export REMAPBIN	:=	$(CURDIR)/$(REMAPOUT)
export DEPSDIR	:=	$(CURDIR)/$(BUILD)

export VPATH	:=	$(foreach dir,$(REMAPSOURCES),$(CURDIR)/$(dir)) \
									$(foreach dir,$(SIFTSOURCES),$(CURDIR)/$(dir))

REMAPCPPFILES	:=	$(foreach dir,$(REMAPSOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
REMAPCFILES	:=	$(foreach dir,$(REMAPSOURCES),$(notdir $(wildcard $(dir)/*.c)))
SIFTCPPFILES	:=	$(foreach dir,$(SIFTSOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SIFTCFILES	:=	$(foreach dir,$(SIFTSOURCES),$(notdir $(wildcard $(dir)/*.c)))

# Use CXX for linking
export LD	:=	$(CXX)

export REMAPOFILES	:=	$(REMAPCPPFILES:.cpp=.o) \
							$(REMAPCFILES:.c=.o)
export SIFTOFILES	:=	$(SIFTCPPFILES:.cpp=.o) \
							$(SIFTCFILES:.c=.o)

export BUILDBINS :=	$(REMAPBIN)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir))

.PHONY: $(BUILD) clean

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	rm -f $(BUILDBINS)
	rm -rf $(BUILD)

else # in build directory

DEPENDS	:=	$(REMAPOFILES:.o=.d) $(SIFTOFILES:.o=.d)

all: $(BUILDBINS)

$(REMAPBIN): $(REMAPOFILES) $(SIFTOFILES)
	$(LD) $(LDFLAGS) $(SIFTOFILES) $(REMAPOFILES) $(LIBPATHS) $(LIBS) -o $@

-include $(DEPENDS)

endif
