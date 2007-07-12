%: %.cpp
	g++ $< -o $@ -O0 -g `pkg-config --cflags --libs opencv`

all: calibrate colorstrip thumbprint threshtemp opticalflow opticaltri undistort
