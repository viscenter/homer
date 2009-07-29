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
LIBS	:=	-L/usr/local/lib/ -lboost_program_options -lGL -lGLU -lglut -lstdc++ -lm 
JNILDFLAGS	:=	-Wl -shared -D_UNIX -fPIC
LDFLAGS		:=
CFLAGS	+=	-O3 -g
# CFLAGS  += -DUSE_DISPLAY_LISTS 

# Linux-specific flags
ifneq (,$(findstring Linux,$(UNAME)))	
	JDK		:=	/usr/lib/jvm/java-1.5.0-sun
	JAVAGL	:= JavaGL.so
	JAVAINCLUDE	:=	-I$(JDK)/include -I$(JDK)/include/linux
	CFLAGS	+= `pkg-config --cflags opencv`
	LIBS	+= `pkg-config --libs opencv`
endif
# MinGW-specific flags
# Get GLUT from http://mywebpage.netscape.com/PtrPck/glut.htm
# Use it to build GLUI
ifneq (,$(findstring MINGW,$(UNAME)))
	SMTOUT	:= smt.exe
	VIEWEROUT	:= viewer.exe
	LIBS  :=	 -L/c/Program\ Files/OpenCV/lib
	LIBS	+=	-lhighgui -lcv -lcxcore -lglut32 -lopengl32 -lglu32 -lstdc++
	CFLAGS	+=	-DWIN32 -D_STDCALL_SUPPORTED -D_M_IX86	
	CFLAGS  +=	-I/c/Program\ Files/OpenCV/cv/include -I/c/Program\ Files/OpenCV/cxcore/include \
							-I/c/Program\ Files/OpenCV/otherlibs/highgui
	JAVAGL	:=	smt.exe
endif
# Cygwin-specific flags
ifneq (,$(findstring CYGWIN,$(UNAME)))
	SMTOUT	:= smt.exe
	VIEWEROUT	:= viewer.exe
	LIBS	:=	-lboost_program_options-gcc-mt -lhighgui -lcv -lcxcore -lopengl32 -lglu32 -lglut32 -lstdc++
	JNILDFLAGS	:=	-Wl,--add-stdcall-alias -shared
	CFLAGS	+=	-DWIN32 -I/usr/local/include/opencv -I/usr/include/boost-1_33_1
	CFLAGS += -mno-cygwin
	LDFLAGS +=  -mno-cygwin
	JDK		:=	/cygdrive/c/j2sdk1.4.2_10
	JAVAGL	:= JavaGL.dll
	JAVAINCLUDE	:=	-I$(JDK)/include -I$(JDK)/include/win32
endif
# Mac-specific flags
ifneq (,$(findstring Darwin,$(UNAME)))
	export CC		:=	/usr/bin/gcc
	export CXX		:=	/usr/bin/g++
	# CFLAGS	+=  -ftree-vectorize -fopenmp
	CFLAGS	+=	-I/opt/local/include/opencv -I/opt/local/include -I/opt/local/include/boost
  # LIBS	:=	-L/opt/local/lib -lboost_program_options -lhighgui -lcv -lcxcore 
	LIBS	:=	/opt/local/lib/libboost_program_options-mt.a \
						/opt/local/lib/libhighgui.dylib \
						/opt/local/lib/libcv.dylib \
						/opt/local/lib/libcxcore.dylib \
						/opt/local/lib/libjpeg.a \
						/opt/local/lib/libpng.a \
						/opt/local/lib/libtiff.a \
						/opt/local/lib/libz.a
	LIBS  += -lm -framework OpenGL -framework GLUT -framework Foundation -framework Carbon
endif

# gprof flags
# CFLAGS	+= -pg -g
# LDFLAGS	:= -pg

BUILD	:=	build

BACKENDSOURCES	:=	source/backend source/glee source/glui
SMTSOURCES		:=	source/smt
VIEWERSOURCES		:=	source/viewer
SMTJAVASOURCES	:=	source/smtjava
TUIOSOURCES   	:=	source/TUIO source/TUIO/ip source/TUIO/ip/posix source/TUIO/osc 
INCLUDES		+=	$(BACKENDSOURCES) $(SMTSOURCES) $(VIEWERSOURCES) $(SMTJAVASOURCES) $(TUIOSOURCES)

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
					$(foreach dir,$(VIEWERSOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(SMTJAVASOURCES),$(CURDIR)/$(dir))

BACKENDCPPFILES	:=	$(foreach dir,$(BACKENDSOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
BACKENDCFILES	:=	$(foreach dir,$(BACKENDSOURCES),$(notdir $(wildcard $(dir)/*.c)))
SMTCPPFILES		:=	$(foreach dir,$(SMTSOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
VIEWERCPPFILES		:=	$(foreach dir,$(VIEWERSOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SMTJAVACPPFILES	:=	$(foreach dir,$(SMTJAVASOURCES),$(notdir $(wildcard $(dir)/*.cpp)))

SMTJAVAJAVAILES	:=	$(foreach dir,$(SMTJAVASOURCES),$(notdir $(wildcard $(dir)/*.java)))
TUIOCPPFILES	:=	$(foreach dir,$(TUIOSOURCES),$(notdir $(wildcard $(dir)/*.cpp)))

# Use CXX for linking
export LD	:=	$(CXX)

export BACKENDOFILES	:=	$(BACKENDCPPFILES:.cpp=.o) \
							$(BACKENDCFILES:.c=.o)
export SMTOFILES		:=	$(SMTCPPFILES:.cpp=.o)
export TUIOOFILES		:=	$(TUIOCPPFILES:.cpp=.o)
export VIEWEROFILES		:=	$(VIEWERCPPFILES:.cpp=.o)
# export TUIOLIBFILES		:=	../$(TUIOSOURCES)/libTUIO.a
export SMTJAVAOFILES	:=	$(SMTJAVACPPFILES:.cpp=.o)
export SMTJAVACLASSFILES	:=	$(SMTJAVACPPFILES:.java=.class)

ifneq (,$(findstring MINGW,$(UNAME)))
	export BUILDBINS :=	$(VIEWERBIN)
else
	export BUILDBINS := $(SMTBIN) $(VIEWERBIN)
endif

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir))
export SOURCEPATH	:=	$(CURDIR)/$(SMTJAVASOURCES)

.PHONY: $(BUILD) smt clean cacheclean

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/makefile

smt:
	@[ -d $(BUILD) ] || mkdir -p $(BUILD)
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/makefile $(SMTBIN)

clean:
	rm -f $(BUILDBINS)
	rm -rf $(BUILD)

cacheclean:
	rm venetus/cache/*

else # in build directory

DEPENDS	:=	$(BACKENDOFILES:.o=.d) \
			$(SMTOFILES:.o=.d) \
			$(TUIOOFILES:.o=.d) \
#			$(SMTJAVAOFILES:.o=.d)

all: $(BUILDBINS)
#all: $(SMTBIN) $(VIEWERBIN)
#smtjava

$(SMTBIN): $(BACKENDOFILES) $(SMTOFILES) $(TUIOOFILES)
	$(LD) $(LDFLAGS) $(SMTOFILES) $(BACKENDOFILES) $(LIBPATHS) $(LIBS) -o $@

$(VIEWERBIN): $(BACKENDOFILES) $(VIEWEROFILES) $(TUIOOFILES)
	$(LD) $(LDFLAGS) $(TUIOOFILES) $(VIEWEROFILES) $(BACKENDOFILES) $(LIBPATHS) $(LIBS) -o $@ $(TUIOLIBFILES)


# .PHONY: smtjava
# smtjava: $(JAVAGLBIN) smt.class

# $(JAVAGLBIN): $(BACKENDOFILES) $(SMTJAVAOFILES)
# 	$(LD) $(JAVAINCLUDE) $(JNILDFLAGS) $(BACKENDOFILES) $(SMTJAVAOFILES) $(LIBS) -o $@

-include $(DEPENDS)

endif
