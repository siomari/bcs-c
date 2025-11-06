CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -I.
LDFLAGS =

# Source files
SRC = src/bcs.c src/sui_transaction.c
OBJ = $(SRC:.c=.o)

# Targets
LIB = libbcs.a
EXAMPLE1 = examples/sensor_transaction
EXAMPLE2 = examples/modify_transaction
EXAMPLE3 = examples/add_sensor_data
EXAMPLE4 = examples/modify_inline_args
EXAMPLE5 = examples/build_transaction
TEST = tests/test_bcs
COMPAT_TEST = tests/compatibility_test

.PHONY: all clean examples tests

all: $(LIB) examples

# Build static library
$(LIB): $(OBJ)
	ar rcs $@ $^

# Build object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build examples
examples: $(EXAMPLE1) $(EXAMPLE2) $(EXAMPLE3) $(EXAMPLE4) $(EXAMPLE5)

$(EXAMPLE1): $(EXAMPLE1).c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -o $@ $(LDFLAGS)

$(EXAMPLE2): $(EXAMPLE2).c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -o $@ $(LDFLAGS)

$(EXAMPLE3): $(EXAMPLE3).c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -o $@ $(LDFLAGS)

$(EXAMPLE4): $(EXAMPLE4).c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -o $@ $(LDFLAGS)

$(EXAMPLE5): $(EXAMPLE5).c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -o $@ $(LDFLAGS)

# Build tests
tests: $(TEST) $(COMPAT_TEST)

$(TEST): tests/test_bcs.c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -o $@ $(LDFLAGS)

$(COMPAT_TEST): tests/compatibility_test.c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -o $@ $(LDFLAGS)

# Run examples
run-examples: $(EXAMPLE1) $(EXAMPLE2) $(EXAMPLE3) $(EXAMPLE4) $(EXAMPLE5)
	@echo "Running sensor_transaction example..."
	./$(EXAMPLE1)
	@echo "\n===================="
	@echo "Running modify_transaction example..."
	./$(EXAMPLE2)
	@echo "\n===================="
	@echo "Running add_sensor_data example..."
	./$(EXAMPLE3)
	@echo "\n===================="
	@echo "Running modify_inline_args example..."
	./$(EXAMPLE4)
	@echo "\n===================="
	@echo "Running build_transaction example..."
	./$(EXAMPLE5)

# Run tests
run-tests: $(TEST) $(COMPAT_TEST)
	./$(TEST)
	@echo "\n===================="
	./$(COMPAT_TEST)

# Clean build artifacts
clean:
	rm -f $(OBJ) $(LIB) $(EXAMPLE1) $(EXAMPLE2) $(EXAMPLE3) $(EXAMPLE4) $(EXAMPLE5) $(TEST) $(COMPAT_TEST)

# Installation paths
PREFIX ?= /usr/local
LIBDIR ?= $(PREFIX)/lib
INCLUDEDIR ?= $(PREFIX)/include
PKGCONFIGDIR ?= $(LIBDIR)/pkgconfig

# Install
install: $(LIB)
	@echo "Installing BCS-C library to $(PREFIX)..."
	install -d $(DESTDIR)$(LIBDIR)
	install -d $(DESTDIR)$(INCLUDEDIR)
	install -d $(DESTDIR)$(PKGCONFIGDIR)
	install -m 644 $(LIB) $(DESTDIR)$(LIBDIR)/
	install -m 644 src/bcs.h $(DESTDIR)$(INCLUDEDIR)/
	install -m 644 src/sui_transaction.h $(DESTDIR)$(INCLUDEDIR)/
	sed -e 's|@PREFIX@|$(PREFIX)|g' \
	    -e 's|@LIBDIR@|$(LIBDIR)|g' \
	    -e 's|@INCLUDEDIR@|$(INCLUDEDIR)|g' \
	    bcs.pc.in > $(DESTDIR)$(PKGCONFIGDIR)/bcs.pc
	@echo "✓ Installation complete!"
	@echo "  Library: $(LIBDIR)/libbcs.a"
	@echo "  Headers: $(INCLUDEDIR)/bcs.h, $(INCLUDEDIR)/sui_transaction.h"
	@echo "  pkg-config: $(PKGCONFIGDIR)/bcs.pc"

# Uninstall
uninstall:
	@echo "Uninstalling BCS-C library..."
	rm -f $(DESTDIR)$(LIBDIR)/$(LIB)
	rm -f $(DESTDIR)$(INCLUDEDIR)/bcs.h
	rm -f $(DESTDIR)$(INCLUDEDIR)/sui_transaction.h
	rm -f $(DESTDIR)$(PKGCONFIGDIR)/bcs.pc
	@echo "✓ Uninstallation complete!"
