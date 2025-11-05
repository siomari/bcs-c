CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -I.
LDFLAGS =

# Source files
SRC = src/bcs.c
OBJ = $(SRC:.c=.o)

# Targets
LIB = libbcs.a
EXAMPLE1 = examples/sensor_transaction
EXAMPLE2 = examples/modify_transaction
EXAMPLE3 = examples/add_sensor_data
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
examples: $(EXAMPLE1) $(EXAMPLE2) $(EXAMPLE3)

$(EXAMPLE1): $(EXAMPLE1).c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -o $@ $(LDFLAGS)

$(EXAMPLE2): $(EXAMPLE2).c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -o $@ $(LDFLAGS)

$(EXAMPLE3): $(EXAMPLE3).c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -o $@ $(LDFLAGS)

# Build tests
tests: $(TEST) $(COMPAT_TEST)

$(TEST): tests/test_bcs.c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -o $@ $(LDFLAGS)

$(COMPAT_TEST): tests/compatibility_test.c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -o $@ $(LDFLAGS)

# Run examples
run-examples: $(EXAMPLE1) $(EXAMPLE2) $(EXAMPLE3)
	@echo "Running sensor_transaction example..."
	./$(EXAMPLE1)
	@echo "\n===================="
	@echo "Running modify_transaction example..."
	./$(EXAMPLE2)
	@echo "\n===================="
	@echo "Running add_sensor_data example..."
	./$(EXAMPLE3)

# Run tests
run-tests: $(TEST) $(COMPAT_TEST)
	./$(TEST)
	@echo "\n===================="
	./$(COMPAT_TEST)

# Clean build artifacts
clean:
	rm -f $(OBJ) $(LIB) $(EXAMPLE1) $(EXAMPLE2) $(EXAMPLE3) $(TEST) $(COMPAT_TEST)

# Install (optional)
install: $(LIB)
	install -d $(DESTDIR)/usr/local/lib
	install -d $(DESTDIR)/usr/local/include
	install -m 644 $(LIB) $(DESTDIR)/usr/local/lib/
	install -m 644 src/bcs.h $(DESTDIR)/usr/local/include/
