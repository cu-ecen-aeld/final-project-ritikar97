# References : ../examples/autotest-validate/Makefile
CROSS_COMPILE = 


ifeq ($(CC),)
	CC ?= $(CROSS_COMPILE)gcc
endif

ifeq ($(CFLAGS),)
	CFLAGS ?= -g -Wall -Werror ${INCLUDES}
endif

ifeq ($(LDFLAGS),)
	LDFLAGS ?= -pthread -lrt -lpaho-mqtt3c
endif

SRC ?= bme280_measure.c
TARGET ?= bme280_measure

all: $(TARGET)

$(TARGET) : ${SRC}
	$(CC) $(CFLAGS) ${SRC} -o ${TARGET} $(LDFLAGS)

clean:
	rm -f *.o ${TARGET}