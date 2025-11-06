# BCS-C Examples

This directory contains example programs demonstrating how to use the BCS-C library for Sui blockchain transactions.

## For IoT Sensors (Recommended)

### `modify_inline_args.c` ⭐ **USE THIS ONE**
**Best choice for embedded devices** - Modifies sensor values in transactions from TypeScript.

- Parses transaction created by TypeScript SDK
- Replaces placeholder sensor values with real readings
- Handles all Object types (ImmOrOwned, SharedObject, Receiving)
- Works with transactions created by `e2e-test/create_transaction.ts`

**Workflow:**
1. TypeScript creates transaction template with placeholder values
2. C code reads sensor data and modifies Pure inputs
3. TypeScript signs and submits to Sui

```bash
# Compile and run
make examples
./examples/modify_inline_args
```

### `use_ts_transaction.c`
Simpler alternative to `modify_inline_args.c` with the same functionality but less documentation.

## Other Examples

### `sensor_transaction.c`
Basic examples showing:
- Creating sensor reading transactions from scratch
- Serializing addresses
- Working with Move calls
- Batch sensor readings

### `modify_transaction.c`
Full workflow demonstration:
- Deserializing transactions
- Modifying transaction data
- Rebuilding transactions
- Useful for understanding the complete process

### `add_sensor_data.c`
Alternative approach showing how to add sensor data to existing transactions.

## Build Instructions

```bash
# Build all examples
make examples

# Build a specific example
gcc -Wall -Wextra -std=c99 -O2 -I../src modify_inline_args.c ../libbcs.a -o modify_inline_args

# Run
./modify_inline_args
```

## Integration Guide

To integrate into your IoT project:

1. Copy `src/bcs.h` and `src/bcs.c` to your project
2. Use the core logic from `modify_inline_args.c`
3. Replace hardcoded transaction hex with your TypeScript template
4. Call the function with your sensor readings

See the main README.md for detailed integration instructions.
