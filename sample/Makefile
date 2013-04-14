#ROOT_DIR = $(shell pwd)
ROOT_DIR = $(PWD)
UGLES2_HOST_DIR = $(ROOT_DIR)/ugles2/host
UGLES2_RASPBERRYPI_DIR = $(ROOT_DIR)/ugles2/raspberrypi

MESA_SRCS = main.c platform/mesa_x.c
MESA_UGLES2_LIB = $(UGLES2_HOST_DIR)/lib/libugles2.a

RASPBERRYPI_SRCS = main.c platform/raspberrypi.c
RASPBERRYPI_UGLES2_LIB = $(UGLES2_RASPBERRYPI_DIR)/lib/libugles2.a
RASPBERRYPI_VC_DIR = /usr/local/gcc-linaro-arm-linux-gnueabihf-raspbian/vc
RASPBERRYPI_LIB_DIR = /usr/local/gcc-linaro-arm-linux-gnueabihf-raspbian/dev

#all: raspberrypi
all: mesa_x

mesa_x: $(MESA_SRCS) $(MESA_UGLES2_LIB)
	cd build-ugles2/host && make all install
	gcc -DMESA_X $(MESA_SRCS) $(MESA_UGLES2_LIB) -lGLESv2 -lEGL -lpng -ljpeg -L/usr/lib/i386-linux-gnu -lfreetype -lpthread -lm -o $@

raspberrypi: $(RASPBERRYPI_SRCS) $(RASPBERRYPI_UGLES2_LIB)
	cd build-ugles2/raspberrypi && make all install
	arm-linux-gnueabihf-gcc -DRASPBERRYPI -I/usr/local/gcc-linaro-arm-linux-gnueabihf-raspbian/vc/include -I/usr/local/gcc-linaro-arm-linux-gnueabihf-raspbian/vc/include/interface/vcos/pthreads $(RASPBERRYPI_SRCS) $(RASPBERRYPI_UGLES2_LIB) -L/usr/local/gcc-linaro-arm-linux-gnueabihf-raspbian/vc/lib -L/usr/local/gcc-linaro-arm-linux-gnueabihf-raspbian/dev/lib -lGLESv2 -lEGL -lbcm_host -lm -lvcos -lvchiq_arm -lpng -ljpeg -lz -lfreetype -lpthread -lm -o $@

clean:
	rm -rf mesa_x build-ugles2 ugles2 raspberrypi

$(MESA_UGLES2_LIB):
	mkdir -p build-ugles2/host && cd build-ugles2/host && ../../../configure --prefix=$(UGLES2_HOST_DIR) --enable-png --enable-jpeg --enable-freetype --with-includes=/usr/include/freetype2 && make all install

$(RASPBERRYPI_UGLES2_LIB):
	mkdir -p build-ugles2/raspberrypi && cd build-ugles2/raspberrypi && ../../../configure --prefix=$(UGLES2_RASPBERRYPI_DIR) --host=arm-linux-gnueabihf --enable-png --enable-jpeg --enable-freetype --with-includes=$(RASPBERRYPI_VC_DIR)/include:$(RASPBERRYPI_VC_DIR)/include/interface/vcos/pthreads:$(RASPBERRYPI_LIB_DIR)/include:$(RASPBERRYPI_LIB_DIR)/include/freetype2 && make all install

.PHONY: clean host build-host mesa_x raspberrypi


