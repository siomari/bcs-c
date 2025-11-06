/**
 * Modify sensor values in full TransactionData (with gas payment)
 *
 * This version handles the complete TransactionData structure including:
 * - TransactionKind (with inputs and commands)
 * - Sender address
 * - Gas payment
 * - Gas budget and price
 */

#include "../src/sui_transaction.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    printf("=== Modifying Full Transaction (with gas payment) ===\n\n");

    // Full transaction from TypeScript (includes gas payment, sender, etc.)
    const char *hex_tx = "0000060101e3b83e0616d2c47961ca802a5169a94896e65063252ae9f2a3603afea35b42c34e7a39160000000001000257040002ae080002050d00025c110008d2029649000000000100b045c6512cef5ad7d20377ec3fe79488b97697a72041b87bb8d7d8c1887de6980673656e736f720e7265636f72645f72656164696e6700060100000101000102000103000104000105006c389948ab60dc14b7c18e0e4e3dd08bf2ced62f7a34c1f7ceaeb24d967db7c701052c8d054356c92a3b62cb4dd75339c8f465eac608393cd1f7180cd2ea0c1a4a587a391600000000206f3a7c4925d994bf8cab59fb842e4659f492f0ecf2ae5d39c24cdfa6d29336cb6c389948ab60dc14b7c18e0e4e3dd08bf2ced62f7a34c1f7ceaeb24d967db7c7e80300000000000000e1f5050000000000";

    printf("Original: %zu bytes\n\n", strlen(hex_tx) / 2);

    // New sensor values
    sensor_data_t sensor_data = {
        .value1 = 2350,
        .value2 = 6540,
        .value3 = 10132,
        .value4 = 850,
        .timestamp = 1730822400
    };

    printf("New sensor values:\n");
    printf("  value1: %u\n", sensor_data.value1);
    printf("  value2: %u\n", sensor_data.value2);
    printf("  value3: %u\n", sensor_data.value3);
    printf("  value4: %u\n", sensor_data.value4);
    printf("  timestamp: %llu\n\n", (unsigned long long)sensor_data.timestamp);

    // Convert hex to bytes
    size_t max_length = strlen(hex_tx) / 2;
    uint8_t *tx_bytes = malloc(max_length);
    size_t tx_length;

    bcs_hex_to_bytes(hex_tx, tx_bytes, max_length, &tx_length);

    // Parse and modify
    bcs_reader_t reader;
    bcs_reader_init(&reader, tx_bytes, tx_length);

    bcs_writer_t writer;
    bcs_writer_init(&writer, 512, 0);

    // Copy TransactionData version
    uint8_t version;
    bcs_read_u8(&reader, &version);
    bcs_write_u8(&writer, version);

    // Copy TransactionKind type
    uint8_t kind;
    bcs_read_u8(&reader, &kind);
    bcs_write_u8(&writer, kind);

    // Copy number of inputs
    uint64_t num_inputs;
    bcs_read_uleb128(&reader, &num_inputs);
    bcs_write_uleb128(&writer, num_inputs);

    // Process inputs - modify Pure values, copy Objects
    size_t pure_idx = 0;
    for (uint64_t i = 0; i < num_inputs; i++) {
        uint8_t input_type;
        bcs_read_u8(&reader, &input_type);
        bcs_write_u8(&writer, input_type);

        if (input_type == 0) { // Pure
            uint64_t old_len;
            bcs_read_uleb128(&reader, &old_len);
            uint8_t *old_data = malloc((size_t)old_len);
            bcs_read_bytes(&reader, old_data, (size_t)old_len);
            free(old_data);

            // Write new value
            if (pure_idx == 0) {
                bcs_write_uleb128(&writer, 2);
                bcs_write_u16(&writer, sensor_data.value1);
            } else if (pure_idx == 1) {
                bcs_write_uleb128(&writer, 2);
                bcs_write_u16(&writer, sensor_data.value2);
            } else if (pure_idx == 2) {
                bcs_write_uleb128(&writer, 2);
                bcs_write_u16(&writer, sensor_data.value3);
            } else if (pure_idx == 3) {
                bcs_write_uleb128(&writer, 2);
                bcs_write_u16(&writer, sensor_data.value4);
            } else if (pure_idx == 4) {
                bcs_write_uleb128(&writer, 8);
                bcs_write_u64(&writer, sensor_data.timestamp);
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
                uint64_t obj_version;
                bcs_read_u64(&reader, &obj_version);
                bcs_write_u64(&writer, obj_version);

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
                uint64_t obj_version;
                bcs_read_u64(&reader, &obj_version);
                bcs_write_u64(&writer, obj_version);

                uint8_t digest[32];
                bcs_read_bytes(&reader, digest, 32);
                bcs_write_fixed_bytes(&writer, digest, 32);
            }
        }
    }

    // Copy the rest (commands, sender, gas payment, etc.)
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
    printf("✓ Done! Use this for signing (Transaction.from()).\n");

    // Cleanup
    free(tx_bytes);
    free(hex_out);
    bcs_writer_free(&writer);

    return 0;
}
