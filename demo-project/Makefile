# Demo Project Makefile
# Shows how to link against installed BCS-C library

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2

# Try to use pkg-config if available
PKGCONFIG := $(shell command -v pkg-config 2> /dev/null)

ifdef PKGCONFIG
    # Use pkg-config if available
    CFLAGS += $(shell pkg-config --cflags bcs-c 2>/dev/null || echo "-I/usr/local/include")
    LDFLAGS = $(shell pkg-config --libs bcs-c 2>/dev/null || echo "-L/usr/local/lib -lbcs")
else
    # Fallback to manual flags
    CFLAGS += -I/usr/local/include
    LDFLAGS = -L/usr/local/lib -lbcs
endif

TARGET = my_sensor_app

all: $(TARGET)

$(TARGET): my_sensor_app.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@
	@echo ""
	@echo "✓ Build complete! Run with: ./$(TARGET)"

clean:
	rm -f $(TARGET)

.PHONY: all clean
