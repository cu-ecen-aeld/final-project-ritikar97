# References : ../examples/autotest-validate/Makefile
CROSS_COMPILE = 


ifeq ($(CC),)
	CC ?= $(CROSS_COMPILE)gcc
endif

ifeq ($(CFLAGS),)
	CFLAGS ?= -g -Wall -Werror ${INCLUDES}
endif

SRC ?= bme280_measure.c
TARGET ?= bme280_measure

all: $(TARGET)

$(TARGET) : ${SRC}
	$(CC) $(CFLAGS) ${SRC} -o ${TARGET} $(LDFLAGS) -pthread -lrt -lpaho-mqtt3c

clean:
	rm -f *.o ${TARGET}