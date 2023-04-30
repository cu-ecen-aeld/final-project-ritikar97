# References : ../examples/autotest-validate/Makefile
CROSS_COMPILE = 

ifeq ($(CC),)
	CC ?= $(CROSS_COMPILE)gcc
endif

ifeq ($(CFLAGS),)
	CFLAGS ?= -g -Wall -Werror ${INCLUDES}
endif

ifeq ($(LDFLAGS),)
	LDFLAGS ?= -lpaho-mqtt3c -pthread
endif

SRC ?= mqtt_sub_rpi.c
TARGET ?= mqtt_sub_rpi

all: $(TARGET)

$(TARGET) : ${SRC}
	$(CC) $(CFLAGS) ${SRC} -o ${TARGET} -lpaho-mqtt3c -pthread

clean:
	rm -f *.o ${TARGET}