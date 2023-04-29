# References : ../examples/autotest-validate/Makefile
CROSS_COMPILE = 


ifeq ($(CC),)
	CC ?= $(CROSS_COMPILE)gcc
endif

ifeq ($(CFLAGS),)
	CFLAGS ?= -g -Wall -Werror ${INCLUDES}
endif

ifeq ($(LDFLAGS),)
	LDFLAGS ?= -lmosquitto -lrt
endif

SRC ?= mqtt_sub_rpi.c
TARGET ?= mqtt_sub_rpi

all: $(TARGET)

$(TARGET) : ${SRC}
	$(CC) $(CFLAGS) ${SRC} -o ${TARGET} $(LDFLAGS)

clean:
	rm -f *.o ${TARGET}