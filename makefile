.SUFFIXES:

%.o: %.cpp
	@echo $(notdir $<)
	$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(CXXFLAGS) -c $< -o $@

%.class: %.java
	@echo $(notdir $<)
	$(JAVAC) $(JAVAFLAGS) $< -d .

export CC		:=	gcc
export CXX		:=	g++
export JAVAC	:=	javac

UNAME	:=	$(shell uname -s)

# General *nix flags (will be overwritten for Cygwin)
LIBS	:=	-lGL -lGLU -lglut -lstdc++ -lm -ljpeg
JNILDFLAGS	:=	-Wl -shared -D_UNIX -fPIC
LDFLAGS		:=
CFLAGS	:=	-O3

# Linux-specific flags
ifneq (,$(findstring Linux,$(UNAME)))	
	JDK		:=	/usr/lib/jvm/java-1.5.0-sun
	JAVAGL	:= JavaGL.so
	JAVAINCLUDE	:=	-I$(JDK)/include -I$(JDK)/include/linux
endif
# Cygwin-specific flags
ifneq (,$(findstring CYGWIN,$(UNAME)))
	LIBS	:=	-lopengl32 -lglu32 -lglut32 -lstdc++
	JNILDFLAGS	:=	-Wl,--add-stdcall-alias -shared
	CFLAGS	:=	-g -mno-cygwin
	JDK		:=	/cygdrive/c/j2sdk1.4.2_10
	JAVAGL	:= JavaGL.dll
	JAVAINCLUDE	:=	-I$(JDK)/include -I$(JDK)/include/win32
endif
# Mac-specific flags
ifneq (,$(findstring Darwin,$(UNAME)))
	export CC		:=	/usr/local/bin/gcc
	export CXX		:=	/usr/local/bin/g++
	CFLAGS	+=  -ftree-vectorize -fopenmp
	LIBS	:=	-fopenmp -lstdc++ -lm -ljpeg -framework OpenGL -framework GLUT -framework Foundation
endif

# gprof flags
# CFLAGS	+= -pg -g
# LDFLAGS	:= -pg

BUILD	:=	build

BACKENDSOURCES	:=	source/backend
SMTSOURCES		:=	source/smt
SMTJAVASOURCES	:=	source/smtjava
INCLUDES		:=	$(BACKENDSOURCES) $(SMTSOURCES) $(SMTJAVASOURCES)

CFLAGS		+= $(INCLUDE)
CXXFLAGS	:= $(CFLAGS)
JAVAFLAGS	:= -sourcepath $(SOURCEPATH)

ifneq ($(BUILD),$(notdir $(CURDIR))) # not in build directory

export JAVAGLBIN	:=	$(CURDIR)/$(JAVAGL)
export SMTBIN	:=	$(CURDIR)/smt
export DEPSDIR	:=	$(CURDIR)/$(BUILD)

export VPATH	:=	$(foreach dir,$(BACKENDSOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(SMTSOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(SMTJAVASOURCES),$(CURDIR)/$(dir))

BACKENDCPPFILES	:=	$(foreach dir,$(BACKENDSOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SMTCPPFILES		:=	$(foreach dir,$(SMTSOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SMTJAVACPPFILES	:=	$(foreach dir,$(SMTJAVASOURCES),$(notdir $(wildcard $(dir)/*.cpp)))

SMTJAVAJAVAILES	:=	$(foreach dir,$(SMTJAVASOURCES),$(notdir $(wildcard $(dir)/*.java)))

# Use CXX for linking
export LD	:=	$(CXX)

export BACKENDOFILES	:=	$(BACKENDCPPFILES:.cpp=.o)
export SMTOFILES		:=	$(SMTCPPFILES:.cpp=.o)
export SMTJAVAOFILES	:=	$(SMTJAVACPPFILES:.cpp=.o)
export SMTJAVACLASSFILES	:=	$(SMTJAVACPPFILES:.java=.class)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir))
export SOURCEPATH	:=	$(CURDIR)/$(SMTJAVASOURCES)

.PHONY: $(BUILD) smt clean

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/makefile

smt:
	@[ -d $(BUILD) ] || mkdir -p $(BUILD)
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/makefile $(SMTBIN)

clean:
	rm -f smt
	rm -f $(JAVAGL)
	rm -rf $(BUILD)

else # in build directory

DEPENDS	:=	$(BACKENDOFILES:.o=.d) \
			$(SMTOFILES:.o=.d) \
			$(SMTJAVAOFILES:.o=.d)

all: $(SMTBIN) smtjava

$(SMTBIN): $(BACKENDOFILES) $(SMTOFILES)
	$(LD) $(LDFLAGS) $(SMTOFILES) $(BACKENDOFILES) $(LIBPATHS) $(LIBS) -o $@

.PHONY: smtjava
smtjava: $(JAVAGLBIN) smt.class

$(JAVAGLBIN): $(BACKENDOFILES) $(SMTJAVAOFILES)
	$(LD) $(JAVAINCLUDE) $(JNILDFLAGS) $(BACKENDOFILES) $(SMTJAVAOFILES) $(LIBS) -o $@

-include $(DEPENDS)

endif
