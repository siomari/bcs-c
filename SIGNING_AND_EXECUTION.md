# Transaction Signing and Execution Guide

This guide explains how to deserialize, modify, sign, and execute Sui transactions from a sensor or embedded device.

## Workflow Overview

```
1. Receive transaction bytes → 2. Deserialize → 3. Add sensor data →
4. Rebuild transaction → 5. Add metadata → 6. Sign → 7. Send to Sui
```

## Step-by-Step Process

### 1. Deserialize an Existing Transaction

```c
bcs_reader_t reader;
bcs_reader_init(&reader, tx_bytes, tx_length);

// Read transaction kind
uint8_t kind;
bcs_read_u8(&reader, &kind);

// Read inputs
uint64_t num_inputs;
bcs_read_uleb128(&reader, &num_inputs);

// Parse each input...
```

### 2. Add New Pure Values (Sensor Data)

```c
// Create new pure inputs for sensor readings
bcs_writer_t temp_writer;
bcs_writer_init(&temp_writer, 32, 0);

// Temperature reading
bcs_write_u32(&temp_writer, 2350); // 23.50°C * 100

size_t len;
const uint8_t *bytes = bcs_writer_get_bytes(&temp_writer, &len);
```

### 3. Rebuild Transaction with New Inputs

```c
bcs_writer_t writer;
bcs_writer_init(&writer, 512, 0);

// Write kind
bcs_write_u8(&writer, kind);

// Write new total number of inputs
bcs_write_uleb128(&writer, original_inputs + new_inputs);

// Write all inputs (original + new)
// ... write each input

// Write commands (unchanged from original)
// ... write commands
```

### 4. Complete Transaction Structure

A complete Sui transaction needs:

```c
typedef struct {
    // Transaction data (what you built above)
    uint8_t *transaction_data;
    size_t transaction_data_length;

    // Transaction metadata
    uint8_t sender[32];           // Sender address
    uint8_t gas_payment[32];      // Gas coin object ID
    uint64_t gas_budget;          // Gas budget
    uint64_t gas_price;           // Gas price

    // Signature (Ed25519)
    uint8_t signature[64];        // 64 bytes signature
    uint8_t public_key[32];       // 32 bytes public key
    uint8_t signature_scheme;     // 0 for Ed25519
} SignedTransaction;
```

### 5. Serialize Complete Transaction with Metadata

```c
void serialize_transaction_with_metadata(
    const uint8_t *tx_data,
    size_t tx_data_len,
    const uint8_t sender[32],
    uint64_t gas_budget,
    uint8_t **output,
    size_t *output_len
) {
    bcs_writer_t writer;
    bcs_writer_init(&writer, 1024, 0);

    // Write sender
    bcs_write_fixed_bytes(&writer, sender, 32);

    // Write transaction data
    bcs_write_bytes(&writer, tx_data, tx_data_len);

    // Write gas data
    bcs_write_u64(&writer, gas_budget);
    // ... write gas payment, price, etc.

    size_t length;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &length);

    *output = malloc(length);
    memcpy(*output, bytes, length);
    *output_len = length;

    bcs_writer_free(&writer);
}
```

### 6. Sign the Transaction

For embedded systems, you'll need to implement Ed25519 signing. Here are options:

#### Option A: Use TweetNaCl (Recommended for embedded)

```c
#include "tweetnacl.h"

void sign_transaction(
    const uint8_t *tx_bytes,
    size_t tx_length,
    const uint8_t secret_key[64],
    uint8_t signature[64]
) {
    crypto_sign_detached(
        signature,
        NULL,
        tx_bytes,
        tx_length,
        secret_key
    );
}
```

#### Option B: Use mbedTLS (For ESP32/Arduino)

```c
#include "mbedtls/ed25519.h"

void sign_transaction_mbedtls(
    const uint8_t *tx_bytes,
    size_t tx_length,
    const uint8_t private_key[32],
    uint8_t signature[64]
) {
    mbedtls_ed25519_sign(
        signature,
        private_key,
        tx_bytes,
        tx_length
    );
}
```

#### Option C: Use a Secure Element (Best for production)

```c
// For hardware security modules like ATECC608A
void sign_with_secure_element(
    const uint8_t *tx_bytes,
    size_t tx_length,
    uint8_t signature[64]
) {
    // Use hardware signing
    // Private key never leaves the secure element
    atcab_sign(0, tx_bytes, signature);
}
```

### 7. Build Final Signed Transaction

```c
void build_signed_transaction(
    const uint8_t *tx_bytes,
    size_t tx_length,
    const uint8_t signature[64],
    const uint8_t public_key[32],
    uint8_t **output,
    size_t *output_len
) {
    bcs_writer_t writer;
    bcs_writer_init(&writer, 1024, 0);

    // Write transaction bytes
    bcs_write_bytes(&writer, tx_bytes, tx_length);

    // Write signature scheme (0 = Ed25519)
    bcs_write_u8(&writer, 0);

    // Write signature (64 bytes)
    bcs_write_fixed_bytes(&writer, signature, 64);

    // Write public key (32 bytes)
    bcs_write_fixed_bytes(&writer, public_key, 32);

    size_t length;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &length);

    *output = malloc(length);
    memcpy(*output, bytes, length);
    *output_len = length;

    bcs_writer_free(&writer);
}
```

### 8. Send to Sui Network

```c
// HTTP POST to Sui RPC endpoint
void send_transaction_to_sui(
    const uint8_t *signed_tx,
    size_t tx_length
) {
    // Convert to base64 for JSON-RPC
    char *base64_tx = base64_encode(signed_tx, tx_length);

    // Build JSON-RPC request
    char json[2048];
    snprintf(json, sizeof(json),
        "{\"jsonrpc\":\"2.0\","
        "\"id\":1,"
        "\"method\":\"sui_executeTransactionBlock\","
        "\"params\":[\"%s\",{\"showEffects\":true}]}",
        base64_tx
    );

    // Send via HTTP
    http_post("https://fullnode.mainnet.sui.io:443", json);

    free(base64_tx);
}
```

## Complete Example

```c
#include "bcs.h"
#include "tweetnacl.h"

void process_and_send_transaction(
    const uint8_t *original_tx,
    size_t original_length,
    uint32_t sensor_reading
) {
    // 1. Parse original transaction
    ParsedTransaction parsed;
    parse_transaction(original_tx, original_length, &parsed);

    // 2. Create new pure input with sensor data
    bcs_writer_t input_writer;
    bcs_writer_init(&input_writer, 32, 0);
    bcs_write_u32(&input_writer, sensor_reading);
    size_t input_len;
    const uint8_t *input_bytes = bcs_writer_get_bytes(&input_writer, &input_len);

    // 3. Rebuild transaction with new input
    uint8_t *modified_tx;
    size_t modified_length;
    const uint8_t *new_inputs[] = {input_bytes};
    size_t new_lengths[] = {input_len};

    rebuild_transaction_with_new_inputs(
        &parsed,
        new_inputs,
        new_lengths,
        1,
        &modified_tx,
        &modified_length
    );

    // 4. Add metadata (sender, gas, etc.)
    uint8_t sender[32] = { /* your address */ };
    uint8_t *tx_with_metadata;
    size_t metadata_length;

    serialize_transaction_with_metadata(
        modified_tx,
        modified_length,
        sender,
        10000000, // gas budget
        &tx_with_metadata,
        &metadata_length
    );

    // 5. Sign transaction
    uint8_t signature[64];
    uint8_t secret_key[64] = { /* your secret key */ };

    sign_transaction(
        tx_with_metadata,
        metadata_length,
        secret_key,
        signature
    );

    // 6. Build final signed transaction
    uint8_t public_key[32] = { /* your public key */ };
    uint8_t *signed_tx;
    size_t signed_length;

    build_signed_transaction(
        tx_with_metadata,
        metadata_length,
        signature,
        public_key,
        &signed_tx,
        &signed_length
    );

    // 7. Send to Sui network
    send_transaction_to_sui(signed_tx, signed_length);

    // Cleanup
    free(modified_tx);
    free(tx_with_metadata);
    free(signed_tx);
    free_parsed_transaction(&parsed);
    bcs_writer_free(&input_writer);
}
```

## Security Considerations

### Private Key Storage

**DO NOT** hardcode private keys in your code! Instead:

1. **For Development**: Store in secure configuration file
2. **For Production**: Use hardware security module (HSM) or secure element
3. **For ESP32**: Use secure boot and flash encryption

```c
// Bad - DON'T DO THIS
uint8_t private_key[32] = {0x01, 0x02, 0x03...}; // INSECURE!

// Good - Use secure storage
read_private_key_from_secure_storage(private_key);

// Better - Use secure element
// Private key never leaves the hardware
sign_with_secure_element(tx_bytes, signature);
```

### Recommended Libraries for Embedded Systems

1. **TweetNaCl** - Minimal, audited crypto library (~100 lines)
   - Website: https://tweetnacl.cr.yp.to/
   - Perfect for constrained devices

2. **mbedTLS** - Full-featured crypto for embedded
   - Supports ESP32, Arduino, ARM Cortex-M
   - Hardware acceleration support

3. **WolfSSL** - Commercial-grade embedded crypto
   - FIPS validated
   - Extensive hardware support

## Example for ESP32

```c
#include <WiFi.h>
#include <HTTPClient.h>
#include "bcs.h"
#include "mbedtls/ed25519.h"

void setup() {
    Serial.begin(115200);
    WiFi.begin("SSID", "password");
}

void loop() {
    // Read sensor
    float temp = readTemperature();

    // Build transaction with sensor data
    uint8_t *tx;
    size_t tx_len;
    build_sensor_transaction(temp, &tx, &tx_len);

    // Sign
    uint8_t signature[64];
    sign_transaction(tx, tx_len, signature);

    // Send to Sui
    HTTPClient http;
    http.begin("https://fullnode.mainnet.sui.io:443");
    http.addHeader("Content-Type", "application/json");

    char json[2048];
    build_json_rpc_request(tx, signature, json);
    http.POST(json);

    free(tx);

    delay(60000); // Every minute
}
```

## Testing

Always test on devnet first:

```c
const char *DEVNET_RPC = "https://fullnode.devnet.sui.io:443";
const char *TESTNET_RPC = "https://fullnode.testnet.sui.io:443";
const char *MAINNET_RPC = "https://fullnode.mainnet.sui.io:443";

// Use devnet for development
send_to_network(tx, tx_len, DEVNET_RPC);
```

## See Also

- [BCS C README](README.md) - Main library documentation
- [Modify Transaction Example](examples/modify_transaction.c) - Full working example
- [Sui Documentation](https://docs.sui.io) - Sui blockchain docs
