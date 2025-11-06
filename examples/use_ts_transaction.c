/**
 * Example: Update sensor values in transaction bytes
 * This allows embedded devices to modify Pure input values before signing
 */

#include "../src/bcs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    printf("=== Modifying Sensor Values in Transaction ===\n\n");

    // Transaction bytes from TypeScript
    const char *hex_tx = "00060101e3b83e0616d2c47961ca802a5169a94896e65063252ae9f2a3603afea35b42c34e7a39160000000001000257040002ae080002050d00025c110008d2029649000000000100b045c6512cef5ad7d20377ec3fe79488b97697a72041b87bb8d7d8c1887de6980673656e736f720e7265636f72645f72656164696e670006010000010100010200010300010400010500";

    size_t max_length = strlen(hex_tx) / 2;
    uint8_t *tx_bytes = malloc(max_length);
    size_t tx_length;

    bcs_error_t err = bcs_hex_to_bytes(hex_tx, tx_bytes, max_length, &tx_length);
    if (err != BCS_OK) {
        printf("Error: %d\n", err);
        free(tx_bytes);
        return 1;
    }

    printf("Original: %zu bytes\n\n", tx_length);

    // ========== SET YOUR SENSOR VALUES HERE ==========
    uint16_t new_value1 = 2350;   // e.g., 23.50°C * 100
    uint16_t new_value2 = 6540;   // e.g., 65.40% * 100
    uint16_t new_value3 = 10132;  // e.g., 1013.2 hPa * 10
    uint16_t new_value4 = 850;    // e.g., 8.50V * 100
    uint64_t new_timestamp = 1730822400;

    printf("New values:\n");
    printf("  value1: %u\n", new_value1);
    printf("  value2: %u\n", new_value2);
    printf("  value3: %u\n", new_value3);
    printf("  value4: %u\n", new_value4);
    printf("  timestamp: %llu\n\n", (unsigned long long)new_timestamp);

    // Parse transaction
    bcs_reader_t reader;
    bcs_reader_init(&reader, tx_bytes, tx_length);

    uint8_t kind;
    bcs_read_u8(&reader, &kind);

    uint64_t num_inputs;
    bcs_read_uleb128(&reader, &num_inputs);


    // Rebuild transaction
    bcs_writer_t writer;
    bcs_writer_init(&writer, 512, 0);

    bcs_write_u8(&writer, kind);
    bcs_write_uleb128(&writer, num_inputs);

    // Process each input
    size_t pure_idx = 0;
    for (uint64_t i = 0; i < num_inputs; i++) {
        uint8_t input_type;
        bcs_read_u8(&reader, &input_type);
        bcs_write_u8(&writer, input_type);

        if (input_type == 0) { // Pure
            // Read old value (discard)
            uint64_t old_len;
            bcs_read_uleb128(&reader, &old_len);
            uint8_t *old_data = malloc((size_t)old_len);
            bcs_read_bytes(&reader, old_data, (size_t)old_len);
            free(old_data);

            // Write new value
            if (pure_idx == 0) {
                bcs_write_uleb128(&writer, 2);
                bcs_write_u16(&writer, new_value1);
            } else if (pure_idx == 1) {
                bcs_write_uleb128(&writer, 2);
                bcs_write_u16(&writer, new_value2);
            } else if (pure_idx == 2) {
                bcs_write_uleb128(&writer, 2);
                bcs_write_u16(&writer, new_value3);
            } else if (pure_idx == 3) {
                bcs_write_uleb128(&writer, 2);
                bcs_write_u16(&writer, new_value4);
            } else if (pure_idx == 4) {
                bcs_write_uleb128(&writer, 8);
                bcs_write_u64(&writer, new_timestamp);
            }
            pure_idx++;

        } else if (input_type == 1) { // Object
            uint8_t variant;
            bcs_read_u8(&reader, &variant);
            bcs_write_u8(&writer, variant);

            uint8_t obj_id[32];
            bcs_read_bytes(&reader, obj_id, 32);
            bcs_write_fixed_bytes(&writer, obj_id, 32);

            if (variant == 0) { // ImmOrOwnedObject
                uint64_t version;
                bcs_read_u64(&reader, &version);
                bcs_write_u64(&writer, version);

                uint8_t digest[32];
                bcs_read_bytes(&reader, digest, 32);
                bcs_write_fixed_bytes(&writer, digest, 32);

            } else if (variant == 1) { // SharedObject
                uint64_t initial_shared_version;
                bcs_read_u64(&reader, &initial_shared_version);
                bcs_write_u64(&writer, initial_shared_version);

                uint8_t mutable;
                bcs_read_u8(&reader, &mutable);
                bcs_write_u8(&writer, mutable);

            } else if (variant == 2) { // Receiving
                uint64_t version;
                bcs_read_u64(&reader, &version);
                bcs_write_u64(&writer, version);

                uint8_t digest[32];
                bcs_read_bytes(&reader, digest, 32);
                bcs_write_fixed_bytes(&writer, digest, 32);
            }
        }
    }

    // Read and copy commands vector
    uint64_t num_commands;
    bcs_read_uleb128(&reader, &num_commands);
    bcs_write_uleb128(&writer, num_commands);

    // Copy remaining bytes (all command data - no modification needed)
    size_t remaining = bcs_reader_remaining(&reader);
    if (remaining > 0) {
        uint8_t *rest = malloc(remaining);
        bcs_read_bytes(&reader, rest, remaining);
        bcs_write_fixed_bytes(&writer, rest, remaining);
        free(rest);
    }

    // Get result
    size_t new_length;
    const uint8_t *new_bytes = bcs_writer_get_bytes(&writer, &new_length);

    printf("Modified: %zu bytes\n\n", new_length);

    char *hex_out = malloc(new_length * 2 + 1);
    bcs_bytes_to_hex(new_bytes, new_length, hex_out);

    printf("=== NEW TRANSACTION BYTES ===\n%s\n\n", hex_out);

    printf("✓ Done! Copy hex above for signing.\n");

    // Cleanup
    free(tx_bytes);
    free(hex_out);
    bcs_writer_free(&writer);

    return 0;
}
