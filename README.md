# BCS-C - Binary Canonical Serialization for C

A lightweight C implementation of [Binary Canonical Serialization (BCS)](https://github.com/zefchain/bcs) for embedded systems, IoT devices, and sensors that need to interact with the Sui blockchain.

## Overview

This library provides a minimal, memory-efficient implementation of BCS serialization/deserialization suitable for resource-constrained devices like IoT sensors. It enables embedded systems to build and serialize Sui blockchain transactions without requiring a full blockchain client.

## Features

- **Lightweight**: Minimal dependencies (only standard C library)
- **Memory Efficient**: Dynamic buffer allocation with configurable limits
- **Complete**: Supports all BCS primitive and compound types
- **Portable**: Standard C99 code, works on embedded platforms
- **Easy to Use**: Simple API for serialization and deserialization

## Installation

### Building the Library

```bash
make
```

This creates `libbcs.a` which you can link with your application.

### Building and Running Examples

```bash
make examples
make run-examples
```

### Building and Running Tests

```bash
make tests
make run-tests
```

### Installing System-Wide (Optional)

```bash
sudo make install
```

## Quick Start

### Serialization Example

```c
#include "bcs.h"

// Initialize a writer
bcs_writer_t writer;
bcs_writer_init(&writer, 256, 0);

// Write some data
bcs_write_u32(&writer, 1234);
bcs_write_string(&writer, "Hello, Sui!");
bcs_write_bool(&writer, true);

// Get the serialized bytes
size_t length;
const uint8_t *bytes = bcs_writer_get_bytes(&writer, &length);

// Convert to hex for transmission
char hex[length * 2 + 1];
bcs_bytes_to_hex(bytes, length, hex);
printf("Serialized: %s\n", hex);

// Clean up
bcs_writer_free(&writer);
```

### Deserialization Example

```c
#include "bcs.h"

// Initialize a reader with BCS data
bcs_reader_t reader;
bcs_reader_init(&reader, bytes, length);

// Read the data back
uint32_t value;
char str[64];
size_t str_len;
bool flag;

bcs_read_u32(&reader, &value);
bcs_read_string(&reader, str, sizeof(str), &str_len);
bcs_read_bool(&reader, &flag);

printf("Value: %u\n", value);
printf("String: %s\n", str);
printf("Flag: %s\n", flag ? "true" : "false");
```

## Supported Types

### Primitive Types

| Function | Type | Size | Description |
|----------|------|------|-------------|
| `bcs_write_u8()` / `bcs_read_u8()` | `uint8_t` | 1 byte | Unsigned 8-bit integer |
| `bcs_write_u16()` / `bcs_read_u16()` | `uint16_t` | 2 bytes | Unsigned 16-bit integer |
| `bcs_write_u32()` / `bcs_read_u32()` | `uint32_t` | 4 bytes | Unsigned 32-bit integer |
| `bcs_write_u64()` / `bcs_read_u64()` | `uint64_t` | 8 bytes | Unsigned 64-bit integer |
| `bcs_write_u128()` / `bcs_read_u128()` | `uint64_t[2]` | 16 bytes | Unsigned 128-bit integer |
| `bcs_write_u256()` / `bcs_read_u256()` | `uint8_t[32]` | 32 bytes | Unsigned 256-bit integer |
| `bcs_write_bool()` / `bcs_read_bool()` | `bool` | 1 byte | Boolean value |

### Compound Types

| Function | Description |
|----------|-------------|
| `bcs_write_string()` / `bcs_read_string()` | UTF-8 string with ULEB128 length prefix |
| `bcs_write_bytes()` | Byte array with ULEB128 length prefix |
| `bcs_write_fixed_bytes()` | Fixed-size byte array (no length prefix) |
| `bcs_write_vec_length()` / `bcs_read_vec_length()` | Vector length (ULEB128) |
| `bcs_write_option_some()` / `bcs_write_option_none()` | Option type (nullable values) |
| `bcs_write_uleb128()` / `bcs_read_uleb128()` | Variable-length unsigned integer |

### Utility Functions

| Function | Description |
|----------|-------------|
| `bcs_hex_to_bytes()` | Convert hex string to bytes |
| `bcs_bytes_to_hex()` | Convert bytes to hex string |

## Usage Examples

### Example 1: Serialize Sensor Data

```c
typedef struct {
    uint32_t timestamp;
    uint32_t temperature;  // Fixed-point: actual temp * 100
    uint32_t humidity;     // Fixed-point: actual humidity * 100
    uint8_t sensor_id;
} SensorReading;

void serialize_sensor_reading(SensorReading *reading) {
    bcs_writer_t writer;
    bcs_writer_init(&writer, 128, 0);

    bcs_write_u32(&writer, reading->timestamp);
    bcs_write_u32(&writer, reading->temperature);
    bcs_write_u32(&writer, reading->humidity);
    bcs_write_u8(&writer, reading->sensor_id);

    size_t length;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &length);

    // Send bytes to Sui blockchain...
    send_to_blockchain(bytes, length);

    bcs_writer_free(&writer);
}
```

### Example 2: Serialize a Vector of Values

```c
void serialize_sensor_batch(SensorReading *readings, size_t count) {
    bcs_writer_t writer;
    bcs_writer_init(&writer, 512, 0);

    // Write vector length
    bcs_write_vec_length(&writer, count);

    // Write each reading
    for (size_t i = 0; i < count; i++) {
        bcs_write_u32(&writer, readings[i].timestamp);
        bcs_write_u32(&writer, readings[i].temperature);
        bcs_write_u32(&writer, readings[i].humidity);
        bcs_write_u8(&writer, readings[i].sensor_id);
    }

    size_t length;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &length);

    send_to_blockchain(bytes, length);

    bcs_writer_free(&writer);
}
```

### Example 3: Working with Sui Addresses

```c
void example_address() {
    // Convert address from hex
    const char *address_hex = "0x0000000000000000000000000000000000000000000000000000000000000002";
    uint8_t address[32];
    size_t addr_len;

    bcs_hex_to_bytes(address_hex, address, 32, &addr_len);

    // Serialize the address
    bcs_writer_t writer;
    bcs_writer_init(&writer, 64, 0);
    bcs_write_fixed_bytes(&writer, address, 32);

    size_t length;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &length);

    // Use in transaction...

    bcs_writer_free(&writer);
}
```

### Example 4: Option Types

```c
void example_option(bool has_data, uint32_t data) {
    bcs_writer_t writer;
    bcs_writer_init(&writer, 64, 0);

    if (has_data) {
        bcs_write_option_some(&writer);
        bcs_write_u32(&writer, data);
    } else {
        bcs_write_option_none(&writer);
    }

    // ... use serialized data

    bcs_writer_free(&writer);
}
```

## Building Sui Transactions

The library can be used to construct Sui transaction data from sensors. Here's a basic transaction structure:

```c
void build_move_call_transaction() {
    bcs_writer_t writer;
    bcs_writer_init(&writer, 512, 0);

    // Transaction kind
    bcs_write_u8(&writer, 0); // ProgrammableTransaction

    // Inputs
    bcs_write_vec_length(&writer, 1);
    bcs_write_u8(&writer, 0); // Pure input
    bcs_write_vec_length(&writer, 4);
    bcs_write_u32(&writer, sensor_reading);

    // Commands
    bcs_write_vec_length(&writer, 1);
    bcs_write_u8(&writer, 0); // MoveCall

    // Package, module, function
    uint8_t package[32] = { /* package address */ };
    bcs_write_fixed_bytes(&writer, package, 32);
    bcs_write_string(&writer, "sensor_module");
    bcs_write_string(&writer, "record_data");

    // Type arguments (empty)
    bcs_write_vec_length(&writer, 0);

    // Arguments
    bcs_write_vec_length(&writer, 1);
    bcs_write_u16(&writer, 0); // Use input 0

    // ... continue building transaction

    bcs_writer_free(&writer);
}
```

## Error Handling

All write and read functions return `bcs_error_t`:

```c
typedef enum {
    BCS_OK = 0,
    BCS_ERROR_OUT_OF_MEMORY = -1,
    BCS_ERROR_BUFFER_TOO_SMALL = -2,
    BCS_ERROR_INVALID_INPUT = -3,
    BCS_ERROR_OVERFLOW = -4,
    BCS_ERROR_BUFFER_UNDERFLOW = -5,
} bcs_error_t;
```

Always check return values:

```c
bcs_error_t err = bcs_write_u32(&writer, value);
if (err != BCS_OK) {
    printf("Error writing u32: %d\n", err);
    return;
}
```

## Memory Management

The writer allocates memory dynamically and grows as needed:

```c
// Initialize with 256 bytes, no max limit
bcs_writer_init(&writer, 256, 0);

// Initialize with 256 bytes, max 4KB
bcs_writer_init(&writer, 256, 4096);
```

Always free the writer when done:

```c
bcs_writer_free(&writer);
```

The reader does not allocate memory and works directly with provided buffers:

```c
bcs_reader_t reader;
bcs_reader_init(&reader, buffer, buffer_length);
// No need to free reader
```

## Integration with Embedded Systems

### Arduino Example

```c
#include "bcs.h"

void setup() {
    Serial.begin(115200);
}

void loop() {
    float temperature = readTemperature();

    bcs_writer_t writer;
    bcs_writer_init(&writer, 128, 0);

    bcs_write_u32(&writer, millis());
    bcs_write_u32(&writer, (uint32_t)(temperature * 100));

    size_t length;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &length);

    // Send via WiFi/LoRa/etc to Sui network
    sendToSui(bytes, length);

    bcs_writer_free(&writer);

    delay(60000); // Send every minute
}
```

### ESP32 Example

```c
#include "bcs.h"
#include <WiFi.h>

void publishToSui() {
    bcs_writer_t writer;
    bcs_writer_init(&writer, 256, 1024);

    // Serialize sensor data
    bcs_write_u32(&writer, esp_timer_get_time() / 1000000);
    bcs_write_u32(&writer, readSensorValue());

    size_t length;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &length);

    // HTTP POST to Sui RPC endpoint
    HTTPClient http;
    http.begin("https://fullnode.mainnet.sui.io:443");
    http.addHeader("Content-Type", "application/octet-stream");
    http.POST(bytes, length);

    bcs_writer_free(&writer);
}
```

## Compatibility

- **C Standard**: C99 or later
- **Dependencies**: Standard C library only (`stdlib.h`, `string.h`, `ctype.h`)
- **Platforms**: Any platform with a C99 compiler
  - Linux, macOS, Windows
  - Arduino, ESP32, ESP8266
  - ARM Cortex-M microcontrollers
  - RISC-V processors

## License

Apache-2.0

Copyright (c) Mysten Labs, Inc.

## See Also

- [TypeScript BCS Implementation](../bcs/)
- [Sui Documentation](https://docs.sui.io)
- [BCS Specification](https://github.com/zefchain/bcs)
