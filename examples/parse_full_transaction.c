/**
 * Debug tool: Parse full TransactionData structure
 * Shows the complete structure including gas payment
 */

#include "../src/bcs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void print_hex_addr(const uint8_t *data, size_t len) {
    printf("0x");
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
}

int main() {
    // Full transaction from TypeScript (with gas payment)
    const char *hex_tx = "0000060101e3b83e0616d2c47961ca802a5169a94896e65063252ae9f2a3603afea35b42c34e7a39160000000001000257040002ae080002050d00025c110008d2029649000000000100b045c6512cef5ad7d20377ec3fe79488b97697a72041b87bb8d7d8c1887de6980673656e736f720e7265636f72645f72656164696e6700060100000101000102000103000104000105006c389948ab60dc14b7c18e0e4e3dd08bf2ced62f7a34c1f7ceaeb24d967db7c701052c8d054356c92a3b62cb4dd75339c8f465eac608393cd1f7180cd2ea0c1a4a587a391600000000206f3a7c4925d994bf8cab59fb842e4659f492f0ecf2ae5d39c24cdfa6d29336cb6c389948ab60dc14b7c18e0e4e3dd08bf2ced62f7a34c1f7ceaeb24d967db7c7e80300000000000000e1f5050000000000";

    size_t max_length = strlen(hex_tx) / 2;
    uint8_t *tx_bytes = malloc(max_length);
    size_t tx_length;

    bcs_hex_to_bytes(hex_tx, tx_bytes, max_length, &tx_length);

    printf("=== Full TransactionData Structure ===\n");
    printf("Total: %zu bytes\n\n", tx_length);

    bcs_reader_t reader;
    bcs_reader_init(&reader, tx_bytes, tx_length);

    // TransactionData V1
    uint8_t version;
    bcs_read_u8(&reader, &version);
    printf("TransactionData version: %u\n\n", version);

    // --- TransactionKind ---
    printf("=== TransactionKind ===\n");
    uint8_t kind;
    bcs_read_u8(&reader, &kind);
    printf("Kind: %u (ProgrammableTransaction)\n", kind);

    // Inputs
    uint64_t num_inputs;
    bcs_read_uleb128(&reader, &num_inputs);
    printf("Inputs: %llu\n\n", (unsigned long long)num_inputs);

    for (uint64_t i = 0; i < num_inputs; i++) {
        uint8_t input_type;
        bcs_read_u8(&reader, &input_type);
        printf("Input %llu: type %u ", (unsigned long long)i, input_type);

        if (input_type == 0) { // Pure
            uint64_t len;
            bcs_read_uleb128(&reader, &len);
            uint8_t *data = malloc((size_t)len);
            bcs_read_bytes(&reader, data, (size_t)len);

            if (len == 2) {
                uint16_t val = data[0] | (data[1] << 8);
                printf("(Pure) = %u\n", val);
            } else if (len == 8) {
                uint64_t val = 0;
                for (int j = 0; j < 8; j++) {
                    val |= ((uint64_t)data[j]) << (j * 8);
                }
                printf("(Pure) = %llu\n", (unsigned long long)val);
            } else {
                printf("(Pure) length=%llu\n", (unsigned long long)len);
            }
            free(data);

        } else if (input_type == 1) { // Object
            uint8_t variant;
            bcs_read_u8(&reader, &variant);

            uint8_t obj_id[32];
            bcs_read_bytes(&reader, obj_id, 32);

            printf("(Object variant %u) = ", variant);
            print_hex_addr(obj_id, 32);
            printf("\n");

            if (variant == 1) { // SharedObject
                uint64_t version;
                bcs_read_u64(&reader, &version);
                uint8_t mutable;
                bcs_read_u8(&reader, &mutable);
            }
        }
    }

    // Commands
    uint64_t num_commands;
    bcs_read_uleb128(&reader, &num_commands);
    printf("\nCommands: %llu\n", (unsigned long long)num_commands);

    // Skip commands parsing for now
    size_t pos_after_inputs = tx_length - bcs_reader_remaining(&reader);
    printf("Commands start at byte: %zu\n", pos_after_inputs);

    // Find end of commands by looking for sender address
    // (we know it's 32 bytes and comes after commands)
    // Skip to after commands
    while (bcs_reader_remaining(&reader) >= 32 + 1 + 32 + 8 + 8 + 8) {
        uint8_t peek;
        bcs_read_u8(&reader, &peek);

        // Check if this could be start of sender (looking ahead 32 bytes for address pattern)
        if (bcs_reader_remaining(&reader) >= 32) {
            uint8_t temp[32];
            size_t saved_pos = tx_length - bcs_reader_remaining(&reader);
            bcs_read_bytes(&reader, temp, 32);

            // Check if next sections make sense
            if (bcs_reader_remaining(&reader) >= 1 + 32 + 8 + 8 + 8) {
                // Likely found sender, rewind
                bcs_reader_init(&reader, tx_bytes, tx_length);
                // Skip to this position
                uint8_t dummy;
                for (size_t i = 0; i < saved_pos; i++) {
                    bcs_read_u8(&reader, &dummy);
                }
                break;
            }
        }
    }

    // --- Sender ---
    printf("\n=== Sender ===\n");
    uint8_t sender[32];
    bcs_read_bytes(&reader, sender, 32);
    printf("Sender: ");
    print_hex_addr(sender, 32);
    printf("\n");

    // --- Gas Payment ---
    printf("\n=== Gas Payment ===\n");
    uint64_t num_gas_coins;
    bcs_read_uleb128(&reader, &num_gas_coins);
    printf("Number of gas coins: %llu\n", (unsigned long long)num_gas_coins);

    for (uint64_t i = 0; i < num_gas_coins; i++) {
        uint8_t gas_obj_id[32];
        bcs_read_bytes(&reader, gas_obj_id, 32);

        uint64_t gas_version;
        bcs_read_u64(&reader, &gas_version);

        uint8_t gas_digest[32];
        bcs_read_bytes(&reader, gas_digest, 32);

        printf("Gas coin %llu:\n", (unsigned long long)i);
        printf("  ObjectID: ");
        print_hex_addr(gas_obj_id, 32);
        printf("\n");
        printf("  Version: %llu\n", (unsigned long long)gas_version);
        printf("  Digest: ");
        print_hex_addr(gas_digest, 32);
        printf("\n");
    }

    // --- Gas Price & Budget ---
    printf("\n=== Gas Config ===\n");
    uint64_t gas_budget;
    bcs_read_u64(&reader, &gas_budget);
    printf("Gas budget: %llu\n", (unsigned long long)gas_budget);

    uint64_t gas_price;
    bcs_read_u64(&reader, &gas_price);
    printf("Gas price: %llu\n", (unsigned long long)gas_price);

    printf("\nRemaining bytes: %zu\n", bcs_reader_remaining(&reader));

    free(tx_bytes);
    return 0;
}
