# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

BCS-C is a lightweight C implementation of Binary Canonical Serialization (BCS) for embedded systems, IoT devices, and sensors that need to interact with the Sui blockchain. The library provides memory-efficient serialization/deserialization suitable for resource-constrained devices.

## Build System

### Building the Library

```bash
make                  # Build static library (libbcs.a)
make clean           # Clean build artifacts
```

### Building and Running Examples

```bash
make examples        # Compile all examples
make run-examples    # Build and run all examples
```

Examples demonstrate:
- `sensor_transaction.c`: Transfer transactions, sensor data serialization, batch readings, address handling, Move calls, deserialization
- `modify_transaction.c`: Full workflow of deserializing, modifying, and rebuilding transactions
- `add_sensor_data.c`: Adding sensor data to existing transactions

### Testing

```bash
make tests          # Compile test binaries
make run-tests      # Build and run all tests
```

Test files:
- `test_bcs.c`: Core BCS functionality tests
- `compatibility_test.c`: Cross-platform compatibility verification

### Installation

```bash
sudo make install   # Install library to /usr/local/lib and header to /usr/local/include
```

## Code Architecture

### Core Components

**BCS Writer (`bcs_writer_t`)** - For serialization
- Dynamically allocated buffer that grows as needed
- Configurable initial capacity and max size limits
- Supports all BCS primitive types (u8, u16, u32, u64, u128, u256, bool)
- Supports compound types (strings, bytes, vectors, options, ULEB128)
- Always free with `bcs_writer_free()` when done

**BCS Reader (`bcs_reader_t`)** - For deserialization
- Zero-allocation reader that works directly with provided buffers
- Reads BCS data in the same order it was written
- No need to free (works on borrowed buffer)

**Error Handling**
- All functions return `bcs_error_t` enum
- Always check return values for proper error handling
- Error codes: BCS_OK, BCS_ERROR_OUT_OF_MEMORY, BCS_ERROR_BUFFER_TOO_SMALL, BCS_ERROR_INVALID_INPUT, BCS_ERROR_OVERFLOW, BCS_ERROR_BUFFER_UNDERFLOW

### File Structure

- `src/bcs.h`: Public API with comprehensive documentation
- `src/bcs.c`: Implementation (single translation unit)
- `examples/`: Working examples of common use cases
- `tests/`: Test suites for verification
- `Makefile`: Build configuration
- `SIGNING_AND_EXECUTION.md`: Complete guide for transaction signing workflow

## Sui Transaction Building

### Transaction Workflow

The typical flow for embedded devices interacting with Sui:

1. **Deserialize** existing transaction bytes with `bcs_reader_t`
2. **Add sensor data** as new Pure inputs
3. **Rebuild transaction** with additional inputs using `bcs_writer_t`
4. **Add metadata** (sender address, gas payment, gas budget/price)
5. **Sign transaction** using Ed25519 (TweetNaCl, mbedTLS, or secure element)
6. **Build final** signed transaction with signature and public key
7. **Send to Sui** via JSON-RPC endpoint

See `SIGNING_AND_EXECUTION.md` for complete code examples of each step.

### Transaction Structure

Sui transactions consist of:
- **Transaction kind**: Usually 0 for ProgrammableTransaction
- **Inputs**: Vector of Pure values or Object references
- **Commands**: Vector of operations (MoveCall, TransferObjects, etc.)
- **Metadata**: Sender, gas payment object, gas budget, gas price
- **Signature**: Ed25519 signature (64 bytes) + public key (32 bytes) + scheme (1 byte)

### Key Serialization Patterns

**Addresses**: Always 32 bytes, use `bcs_write_fixed_bytes()` (no length prefix)

**Sensor data**: Convert floats to fixed-point integers (e.g., temperature * 100)

**Vectors**: Write length with `bcs_write_vec_length()`, then write each element

**Options**: Use `bcs_write_option_some()` + value, or `bcs_write_option_none()`

**Strings**: `bcs_write_string()` handles ULEB128 length prefix automatically

## Important Implementation Notes

### Memory Management

- Writers allocate memory dynamically; always call `bcs_writer_free()` to avoid leaks
- Initialize with appropriate capacity: `bcs_writer_init(&writer, initial_capacity, max_size)`
- Set `max_size` to 0 for unlimited growth, or set a limit for safety
- Readers are stack-based and don't require cleanup

### Endianness

All multi-byte integers (u16, u32, u64, u128, u256) are serialized in **little-endian** format per BCS specification.

### Hex Utilities

- `bcs_hex_to_bytes()`: Convert hex strings (with or without 0x prefix) to bytes
- `bcs_bytes_to_hex()`: Convert bytes to lowercase hex string
- Output buffer for `bcs_bytes_to_hex()` must be at least `length * 2 + 1` bytes

### Platform Compatibility

Standard C99 code with minimal dependencies (stdlib.h, string.h, ctype.h only). Works on:
- Linux, macOS, Windows
- Arduino, ESP32, ESP8266
- ARM Cortex-M microcontrollers
- RISC-V processors

## Security Considerations

**Private key storage**: Never hardcode keys in source. Use secure storage or hardware security modules (HSM/secure elements) in production.

**Recommended crypto libraries for embedded**:
- TweetNaCl: Minimal, audited Ed25519 implementation (~100 lines)
- mbedTLS: Full-featured with ESP32/Arduino support
- WolfSSL: FIPS validated commercial-grade crypto
- Hardware: ATECC608A or similar secure elements for key storage

**Testing**: Always test on devnet/testnet before mainnet deployment.

## NPM Integration

This C library is packaged as an NPM module for distribution:
- Package: `@mysten/bcs-c`
- Build script: `npm run build` (calls `make`)
- Test script: `npm test` (calls `make tests && make run-tests`)
- Clean script: `npm run clean` (calls `make clean`)
