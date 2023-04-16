# References : ../examples/autotest-validate/Makefile
CROSS_COMPILE = 

INCLUDES = -I ../bme280-driver/

ifeq ($(CC),)
	CC ?= $(CROSS_COMPILE)gcc
endif

ifeq ($(CFLAGS),)
	CFLAGS ?= -g -Wall -Werror ${INCLUDES}
endif

ifeq ($(LDFLAGS),)
	LDFLAGS ?= -pthread -lrt
endif

SRC ?= bme280_measure.c
TARGET ?= bme280_measure

all: $(TARGET)

$(TARGET) : ${SRC}
	$(CC) $(CFLAGS) ${SRC} -o ${TARGET} $(LDFLAGS)

clean:
	rm -f *.o ${TARGET}