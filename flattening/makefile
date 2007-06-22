.SUFFIXES:

%.o: %.cpp
	@echo $(notdir $<)
	$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(CXXFLAGS) -c $< -o $@

%.o: %.c
	@echo $(notdir $<)
	$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d $(CFLAGS) -c $< -o $@

%.class: %.java
	@echo $(notdir $<)
	$(JAVAC) $(JAVAFLAGS) $< -d .

export CC		:=	gcc
export CXX		:=	g++
export JAVAC	:=	javac

SMTOUT	:= smt
VIEWEROUT	:= viewer

UNAME	:=	$(shell uname -s)

# General *nix flags (will be overwritten for Cygwin)
LIBS	:=	-L/usr/local/lib/ -lboost_program_options -lGL -lGLU -lglut -lstdc++ -lm -ljpeg
JNILDFLAGS	:=	-Wl -shared -D_UNIX -fPIC
LDFLAGS		:=
CFLAGS	:=	-O3 -DDEBUG

# Linux-specific flags
ifneq (,$(findstring Linux,$(UNAME)))	
	JDK		:=	/usr/lib/jvm/java-1.5.0-sun
	JAVAGL	:= JavaGL.so
	JAVAINCLUDE	:=	-I$(JDK)/include -I$(JDK)/include/linux
	CFLAGS	+= `pkg-config --cflags opencv`
	LIBS	+= `pkg-config --libs opencv`
endif
# MinGW-specific flags
ifneq (,$(findstring MINGW,$(UNAME)))
	LIBS	:=	-lopengl32 -lglu32 -lglut32 -ljpeg -lstdc++
	CFLAGS	:=	-D_STDCALL_SUPPORTED -D_M_IX86	
	JAVAGL	:=	smt.exe
endif
# Cygwin-specific flags
ifneq (,$(findstring CYGWIN,$(UNAME)))
	SMTOUT	:= smt.exe
	VIEWEROUT	:= viewer.exe
	LIBS	:=	-lboost_program_options-gcc-mt -lhighgui -lcv -lcxcore -lglui -lopengl32 -lglu32 -lglut32 -ljpeg -lstdc++
	JNILDFLAGS	:=	-Wl,--add-stdcall-alias -shared
	CFLAGS	:=	-I/usr/include/opencv -I/usr/include/boost-1_33_1	
	JDK		:=	/cygdrive/c/j2sdk1.4.2_10
	JAVAGL	:= JavaGL.dll
	JAVAINCLUDE	:=	-I$(JDK)/include -I$(JDK)/include/win32
endif
# Mac-specific flags
ifneq (,$(findstring Darwin,$(UNAME)))
	# export CC		:=	/usr/local/bin/gcc
	# export CXX		:=	/usr/local/bin/g++
	# CFLAGS	+=  -ftree-vectorize -fopenmp
	CFLAGS	+=	-I/usr/local/include/opencv -I/opt/local/include
	LIBS	:=	-L/opt/local/lib -lboost_program_options -lhighgui -lcv -lcxcore -lstdc++ -lm -ljpeg -framework OpenGL -framework GLUT -framework Foundation
endif

# gprof flags
# CFLAGS	+= -pg -g
# LDFLAGS	:= -pg

BUILD	:=	build

BACKENDSOURCES	:=	source/backend
SMTSOURCES		:=	source/smt
VIEWERSOURCES		:=	source/viewer
SMTJAVASOURCES	:=	source/smtjava
INCLUDES		:=	$(BACKENDSOURCES) $(SMTSOURCES) $(SMTJAVASOURCES)

CFLAGS		+= $(INCLUDE)
CXXFLAGS	:= $(CFLAGS)
JAVAFLAGS	:= -sourcepath $(SOURCEPATH)

ifneq ($(BUILD),$(notdir $(CURDIR))) # not in build directory

export JAVAGLBIN	:=	$(CURDIR)/$(JAVAGL)
export SMTBIN	:=	$(CURDIR)/$(SMTOUT)
export VIEWERBIN	:=	$(CURDIR)/$(VIEWEROUT)
export DEPSDIR	:=	$(CURDIR)/$(BUILD)

export VPATH	:=	$(foreach dir,$(BACKENDSOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(SMTSOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(SMTJAVASOURCES),$(CURDIR)/$(dir))

BACKENDCPPFILES	:=	$(foreach dir,$(BACKENDSOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
BACKENDCFILES	:=	$(foreach dir,$(BACKENDSOURCES),$(notdir $(wildcard $(dir)/*.c)))
SMTCPPFILES		:=	$(foreach dir,$(SMTSOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
VIEWERCPPFILES		:=	$(foreach dir,$(VIEWERSOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SMTJAVACPPFILES	:=	$(foreach dir,$(SMTJAVASOURCES),$(notdir $(wildcard $(dir)/*.cpp)))

SMTJAVAJAVAILES	:=	$(foreach dir,$(SMTJAVASOURCES),$(notdir $(wildcard $(dir)/*.java)))

# Use CXX for linking
export LD	:=	$(CXX)

export BACKENDOFILES	:=	$(BACKENDCPPFILES:.cpp=.o) \
							$(BACKENDCFILES:.c=.o)
export SMTOFILES		:=	$(SMTCPPFILES:.cpp=.o)
export VIEWEROFILES		:=	$(SMTCPPFILES:.cpp=.o)
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
	rm -f $(SMTOUT)
	rm -f $(VIEWEROUT)
	rm -f $(JAVAGL)
	rm -rf $(BUILD)

else # in build directory

DEPENDS	:=	$(BACKENDOFILES:.o=.d) \
			$(SMTOFILES:.o=.d) \
			$(SMTJAVAOFILES:.o=.d)

all: $(SMTBIN) $(VIEWERBIN)
#smtjava

$(SMTBIN): $(BACKENDOFILES) $(SMTOFILES)
	$(LD) $(LDFLAGS) $(SMTOFILES) $(BACKENDOFILES) $(LIBPATHS) $(LIBS) -o $@

$(VIEWERBIN): $(BACKENDOFILES) $(VIEWEROFILES)
	$(LD) $(LDFLAGS) $(VIEWEROFILES) $(BACKENDOFILES) $(LIBPATHS) $(LIBS) -o $@


.PHONY: smtjava
smtjava: $(JAVAGLBIN) smt.class

$(JAVAGLBIN): $(BACKENDOFILES) $(SMTJAVAOFILES)
	$(LD) $(JAVAINCLUDE) $(JNILDFLAGS) $(BACKENDOFILES) $(SMTJAVAOFILES) $(LIBS) -o $@

-include $(DEPENDS)

endif
