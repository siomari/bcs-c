# Integration Guide - Using BCS-C in Your Project

This guide shows how to integrate the BCS-C library into your IoT/embedded project for modifying Sui transactions.

## Quick Start

### Files to Copy

Copy these 4 files to your project:

```
your-project/
├── bcs.h                  # Core BCS library header
├── bcs.c                  # Core BCS library implementation
├── sui_transaction.h      # Sui transaction helpers header
└── sui_transaction.c      # Sui transaction helpers implementation
```

From this repository:
- `src/bcs.h` → `your-project/bcs.h`
- `src/bcs.c` → `your-project/bcs.c`
- `src/sui_transaction.h` → `your-project/sui_transaction.h`
- `src/sui_transaction.c` → `your-project/sui_transaction.c`

### Minimal Example

```c
#include "sui_transaction.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    // 1. Get transaction template from your backend
    const char *template_hex = "00060101e3b83e..."; // From TypeScript

    // 2. Read sensor values
    sensor_data_t sensor_data = {
        .value1 = 2350,      // 23.50°C * 100
        .value2 = 6540,      // 65.40% * 100
        .value3 = 10132,     // 1013.2 hPa * 10
        .value4 = 850,       // 8.50V * 100
        .timestamp = 1730822400
    };

    // 3. Modify transaction
    char *modified_tx;
    size_t modified_len;

    bcs_error_t err = sui_modify_transaction_with_sensor_data(
        template_hex,
        &sensor_data,
        &modified_tx,
        &modified_len
    );

    if (err != BCS_OK) {
        printf("Error: %d\n", err);
        return 1;
    }

    // 4. Send to backend for signing
    send_to_backend(modified_tx);

    // 5. Cleanup
    free(modified_tx);

    return 0;
}
```

### Compile

```bash
# Compile all files together
gcc -o my_sensor my_sensor.c bcs.c sui_transaction.c

# Or create a static library first
gcc -c bcs.c sui_transaction.c
ar rcs libsui.a bcs.o sui_transaction.o
gcc -o my_sensor my_sensor.c libsui.a
```

## API Reference

### `sui_modify_transaction_with_sensor_data()`

The main function for modifying transactions with sensor data.

```c
bcs_error_t sui_modify_transaction_with_sensor_data(
    const char *hex_tx,              // Input: transaction template (hex)
    const sensor_data_t *sensor_data, // Input: your sensor readings
    char **output_hex,                // Output: modified transaction (caller must free)
    size_t *output_length             // Output: length of modified transaction
);
```

**Parameters:**
- `hex_tx`: Transaction template from TypeScript (hex string)
- `sensor_data`: Pointer to sensor_data_t structure with your readings
- `output_hex`: Pointer to receive allocated hex string (you must free this)
- `output_length`: Pointer to receive the length in bytes

**Returns:** `BCS_OK` on success, error code otherwise

**Error codes:**
- `BCS_OK` (0) - Success
- `BCS_ERROR_INVALID_INPUT` - NULL pointer or invalid input
- `BCS_ERROR_OUT_OF_MEMORY` - Memory allocation failed
- `BCS_ERROR_BUFFER_TOO_SMALL` - Buffer overflow
- `BCS_ERROR_BUFFER_UNDERFLOW` - Unexpected end of data

### `sensor_data_t` Structure

Default structure for 5 sensor values:

```c
typedef struct {
    uint16_t value1;      // First sensor (e.g., temperature)
    uint16_t value2;      // Second sensor (e.g., humidity)
    uint16_t value3;      // Third sensor (e.g., pressure)
    uint16_t value4;      // Fourth sensor (e.g., voltage)
    uint64_t timestamp;   // Unix timestamp
} sensor_data_t;
```

**Customize for your needs:**
- Edit `src/sui_transaction.h` to match your Move contract
- Add/remove fields as needed
- Update the implementation in `src/sui_transaction.c`

### Advanced: `sui_modify_transaction_with_pure_values()`

For more control, use the lower-level function:

```c
bcs_error_t sui_modify_transaction_with_pure_values(
    const char *hex_tx,
    const uint8_t **pure_values,    // Array of byte arrays
    const size_t *pure_lengths,     // Length of each value
    size_t num_pures,               // Number of values
    char **output_hex,
    size_t *output_length
);
```

Example with custom values:

```c
// Custom data: 3 values (u32, u64, string)
uint8_t value1[4] = {0x12, 0x34, 0x56, 0x78};  // u32
uint8_t value2[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};  // u64
uint8_t value3[5] = {0x68, 0x65, 0x6c, 0x6c, 0x6f};  // "hello"

const uint8_t *values[] = {value1, value2, value3};
const size_t lengths[] = {4, 8, 5};

sui_modify_transaction_with_pure_values(
    hex_tx, values, lengths, 3, &output, &output_len
);
```

## Platform-Specific Notes

### Arduino / ESP32

```cpp
#include "sui_transaction.h"

void setup() {
    Serial.begin(115200);
}

void loop() {
    // Read sensors
    sensor_data_t data;
    data.value1 = analogRead(A0);
    data.timestamp = millis();

    // Get template from WiFi/server
    String template_hex = fetchTemplate();

    char *modified_tx;
    size_t modified_len;

    sui_modify_transaction_with_sensor_data(
        template_hex.c_str(),
        &data,
        &modified_tx,
        &modified_len
    );

    // Send to server
    sendToServer(modified_tx);
    free(modified_tx);

    delay(60000); // Every minute
}
```

### Memory-Constrained Devices

The library is designed for minimal memory usage:

- **Stack usage**: < 500 bytes
- **Heap usage**:
  - Transaction buffer: ~150-200 bytes (your transaction size)
  - Working buffer: ~512 bytes (configurable)
  - Output buffer: ~300-400 bytes (2× transaction size in hex)
- **Total**: < 2KB for typical sensor transactions

To reduce memory further:
1. Edit `sui_transaction.c`, change `bcs_writer_init(&writer, 512, 0)` to smaller size (e.g., 256)
2. Process transactions in chunks if needed
3. Reuse buffers across multiple operations

## Workflow

### Production Setup

```
┌─────────────┐
│   Backend   │ 1. Creates transaction template
│  (TypeScript)│    with placeholder values
└──────┬──────┘
       │ hex template
       ▼
┌─────────────┐
│  IoT Device │ 2. Reads sensors and modifies
│    (C code) │    Pure input values
└──────┬──────┘
       │ modified hex
       ▼
┌─────────────┐
│   Backend   │ 3. Signs and submits to Sui
│  (TypeScript)│
└─────────────┘
```

### Step-by-Step

1. **Backend creates template** (once, or when contract changes):
   ```typescript
   // create_transaction.ts
   const tx = new Transaction();
   tx.moveCall({
       target: `${packageId}::sensor::record_reading`,
       arguments: [
           tx.object(sensorId),
           tx.pure.u16(0),  // Placeholder
           tx.pure.u16(0),
           tx.pure.u16(0),
           tx.pure.u16(0),
           tx.pure.u64(0),
       ],
   });
   const template = await tx.build({ client, onlyTransactionKind: true });
   ```

2. **Device modifies with real data**:
   ```c
   sensor_data_t data = read_all_sensors();
   sui_modify_transaction_with_sensor_data(template, &data, &output, &len);
   send_to_backend(output);
   ```

3. **Backend signs and submits**:
   ```typescript
   // sign_and_submit.ts
   const tx = Transaction.fromKind(Buffer.from(modified_hex, 'hex'));
   tx.setSender(sender);
   await client.signAndExecuteTransaction({ transaction: tx, signer: keypair });
   ```

## Customization

### Different Number of Values

Edit `src/sui_transaction.h`:

```c
typedef struct {
    uint16_t temperature;
    uint16_t humidity;
    // Add more or remove fields as needed
    uint64_t timestamp;
} sensor_data_t;
```

Edit `src/sui_transaction.c` to match:

```c
// In sui_modify_transaction_with_sensor_data()
const uint8_t *pure_values[] = {
    temp_bytes,
    humidity_bytes,
    // Add more as needed
    timestamp_bytes
};

const size_t pure_lengths[] = { 2, 2, /* ... */ 8 };
```

### Different Data Types

For other types (u32, u128, strings, etc.), use the low-level BCS functions:

```c
bcs_writer_t writer;
bcs_writer_init(&writer, 64, 0);

// Write different types
bcs_write_u32(&writer, my_u32_value);
bcs_write_u128(&writer, my_u128_value);
bcs_write_string(&writer, "my_string");

size_t len;
const uint8_t *bytes = bcs_writer_get_bytes(&writer, &len);
// Use bytes in sui_modify_transaction_with_pure_values()

bcs_writer_free(&writer);
```

See `src/bcs.h` for all available functions.

## Troubleshooting

### "Error: 1" (BCS_ERROR_INVALID_INPUT)
- Check that hex_tx is not NULL
- Verify hex string is valid (even length, only 0-9a-f)

### "Error: 2" (BCS_ERROR_OUT_OF_MEMORY)
- Reduce transaction size
- Increase available heap
- Check for memory leaks (did you free previous outputs?)

### Transaction fails on-chain
- Verify transaction template matches current contract
- Check that sensor data types match Move function signature
- Ensure object references haven't changed

### Compilation errors
- Make sure all 4 files are included
- Use C99 standard: `gcc -std=c99 ...`
- Include all necessary headers (`<string.h>`, `<stdlib.h>`, etc.)

## Next Steps

- See `examples/modify_inline_args.c` for complete example
- See `e2e-test/` for full TypeScript integration
- See `src/bcs.h` for low-level BCS operations
- See `SIGNING_AND_EXECUTION.md` for signing details

## Support

For issues or questions:
- Check the examples in `examples/`
- Read the full documentation in `README.md`
- Review the source code (heavily commented)
