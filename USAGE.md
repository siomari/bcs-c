# Using BCS-C Library in Your Project

This guide shows how to use the BCS-C library in your own C projects.

## Installation

### System-wide Installation (Recommended)

Install the library and headers to your system:

```bash
cd /path/to/bcs-c
make
sudo make install
```

This installs:
- **Library**: `/usr/local/lib/libbcs.a`
- **Headers**: `/usr/local/include/bcs.h`, `/usr/local/include/sui_transaction.h`
- **pkg-config**: `/usr/local/lib/pkgconfig/bcs.pc`

To uninstall:

```bash
sudo make uninstall
```

### Custom Installation Path

Install to a custom location:

```bash
make install PREFIX=/opt/bcs
```

Then set `PKG_CONFIG_PATH` when compiling your projects:

```bash
export PKG_CONFIG_PATH=/opt/bcs/lib/pkgconfig:$PKG_CONFIG_PATH
```

## Using in Your Project

### Method 1: Using pkg-config (Recommended)

Create your project file `my_sensor.c`:

```c
#include <bcs.h>
#include <sui_transaction.h>
#include <stdio.h>

int main() {
    // Initialize transaction builder
    transaction_builder_t params = {0};
    
    // Set your sensor data
    params.sensor_data.value1 = 2500;
    params.sensor_data.value2 = 7000;
    params.sensor_data.value3 = 10200;
    params.sensor_data.value4 = 900;
    params.sensor_data.timestamp = 1730822400;
    
    // Set package ID, module, function
    size_t bytes_read;
    bcs_hex_to_bytes("YOUR_PACKAGE_ID", params.package_id, 32, &bytes_read);
    params.module_name = "sensor";
    params.function_name = "record_reading";
    
    // Set sensor object ID and shared version
    bcs_hex_to_bytes("YOUR_SENSOR_ID", params.sensor_object_id, 32, &bytes_read);
    params.sensor_initial_shared_version = 372865614;
    params.sensor_mutable = true;
    
    // Set sender and gas object
    bcs_hex_to_bytes("YOUR_SENDER_ADDRESS", params.sender, 32, &bytes_read);
    bcs_hex_to_bytes("YOUR_GAS_OBJECT_ID", params.gas_object.object_id, 32, &bytes_read);
    params.gas_object.version = 372865625;
    bcs_hex_to_bytes("YOUR_GAS_DIGEST", params.gas_object.digest, 32, &bytes_read);
    
    params.gas_budget = 100000000;  // 0.1 SUI
    params.gas_price = 1000;
    
    // Build transaction
    char *tx_hex;
    size_t tx_len;
    bcs_error_t err = sui_build_sensor_transaction(&params, &tx_hex, &tx_len);
    
    if (err == BCS_OK) {
        printf("Transaction built: %zu bytes\n", tx_len);
        printf("Hex: %s\n", tx_hex);
        free(tx_hex);
    }
    
    return 0;
}
```

Compile with pkg-config:

```bash
gcc my_sensor.c $(pkg-config --cflags --libs bcs-c) -o my_sensor
```

### Method 2: Manual Compilation

Without pkg-config:

```bash
gcc my_sensor.c -I/usr/local/include -L/usr/local/lib -lbcs -o my_sensor
```

### Method 3: Using Makefile

Create a `Makefile` for your project:

```makefile
CC = gcc
CFLAGS = $(shell pkg-config --cflags bcs-c) -Wall -Wextra -std=c99
LDFLAGS = $(shell pkg-config --libs bcs-c)

my_sensor: my_sensor.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

clean:
	rm -f my_sensor
```

Then just run:

```bash
make
./my_sensor
```

### Method 4: Copy Library Directly

If you don't want to install system-wide, copy the library:

```bash
# Copy library and headers to your project
cp /path/to/bcs-c/libbcs.a ./
cp /path/to/bcs-c/src/bcs.h ./
cp /path/to/bcs-c/src/sui_transaction.h ./

# Compile
gcc my_sensor.c libbcs.a -o my_sensor
```

## Core APIs

### BCS Writer (Serialization)

```c
#include <bcs.h>

bcs_writer_t writer;
bcs_writer_init(&writer, 512, 0);  // 512 byte initial capacity, unlimited max

// Write primitives
bcs_write_u8(&writer, 42);
bcs_write_u16(&writer, 1234);
bcs_write_u32(&writer, 567890);
bcs_write_u64(&writer, 1234567890);
bcs_write_string(&writer, "Hello");
bcs_write_fixed_bytes(&writer, data, 32);

// Get result
size_t len;
const uint8_t *bytes = bcs_writer_get_bytes(&writer, &len);

// Always cleanup
bcs_writer_free(&writer);
```

### BCS Reader (Deserialization)

```c
#include <bcs.h>

bcs_reader_t reader;
bcs_reader_init(&reader, buffer, buffer_len);

uint8_t val8;
uint16_t val16;
uint32_t val32;

bcs_read_u8(&reader, &val8);
bcs_read_u16(&reader, &val16);
bcs_read_u32(&reader, &val32);

// No cleanup needed (reader doesn't allocate)
```

### Sui Transaction Builder

```c
#include <sui_transaction.h>

transaction_builder_t params = {0};

// Fill in all parameters (see example above)

char *tx_hex;
size_t tx_len;
bcs_error_t err = sui_build_sensor_transaction(&params, &tx_hex, &tx_len);

if (err == BCS_OK) {
    // Use tx_hex...
    free(tx_hex);
}
```

## Example Projects

Check the `examples/` directory for complete examples:

- `build_transaction.c` - Build complete transaction from scratch
- `sensor_transaction.c` - Simple sensor data serialization
- `modify_transaction.c` - Modify existing transactions

## Platform Support

BCS-C works on:
- ✅ Linux (x86_64, ARM, RISC-V)
- ✅ macOS (Intel, Apple Silicon)
- ✅ Windows (MinGW, Cygwin)
- ✅ ESP32, ESP8266 (Arduino)
- ✅ ARM Cortex-M microcontrollers
- ✅ RISC-V processors

## Troubleshooting

### Library not found

```bash
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

### pkg-config not finding bcs.pc

```bash
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH
```

### Permission denied during install

Use `sudo`:

```bash
sudo make install
```

## Support

- **Documentation**: See `CLAUDE.md` and `SIGNING_AND_EXECUTION.md`
- **Examples**: Check `examples/` directory
- **Tests**: Run `make tests && make run-tests`

## License

See LICENSE file in the repository.
