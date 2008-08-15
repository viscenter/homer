.SUFFIXES:

%.exe: %.cpp
	g++ $< -o $@ $(CFLAGS) $(LIBS)

CFLAGS += -O0 -g

UNAME	:=	$(shell uname -s)

# Linux-specific flags
ifneq (,$(findstring Linux,$(UNAME)))
	CFLAGS += `pkg-config --cflags opencv`
	LIBS += `pkg-config --libs opencv`
endif
# Cygwin-specific flags
ifneq (,$(findstring CYGWIN,$(UNAME)))
	LIBS += -L/usr/local/lib -lcv -lcxcore -lhighgui
endif
# Mac-specific flags
ifneq (,$(findstring Darwin,$(UNAME)))
	LIBS = -lcv -lcxcore -lhighgui
endif

all: calibration.exe colorstrip.exe thumbprint.exe calibrate2obj.exe diffimage.exe opticalflow.exe opticaltri.exe andimage.exe
