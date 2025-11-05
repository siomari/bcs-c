/**
 * Example: Deserializing, modifying, and re-serializing a Sui transaction
 *
 * This example shows how to:
 * 1. Deserialize an existing transaction
 * 2. Add additional pure value inputs
 * 3. Re-serialize the modified transaction
 * 4. Prepare it for signing and execution
 */

#include "../src/bcs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Structure to hold parsed transaction data
typedef struct {
    uint8_t kind;

    // Inputs
    size_t num_inputs;
    uint8_t **input_data;
    size_t *input_lengths;
    uint8_t *input_types;

    // Commands
    size_t num_commands;
    uint8_t *commands_data;
    size_t commands_length;
} ParsedTransaction;

// Parse a simple programmable transaction
bcs_error_t parse_transaction(const uint8_t *tx_bytes, size_t tx_length, ParsedTransaction *parsed) {
    bcs_reader_t reader;
    bcs_reader_init(&reader, tx_bytes, tx_length);

    // Read transaction kind
    bcs_error_t err = bcs_read_u8(&reader, &parsed->kind);
    if (err != BCS_OK) return err;

    printf("Transaction kind: %u (0 = ProgrammableTransaction)\n", parsed->kind);

    // Read number of inputs
    uint64_t num_inputs;
    err = bcs_read_uleb128(&reader, &num_inputs);
    if (err != BCS_OK) return err;

    parsed->num_inputs = (size_t)num_inputs;
    printf("Number of inputs: %zu\n", parsed->num_inputs);

    // Allocate arrays for inputs
    parsed->input_data = malloc(sizeof(uint8_t*) * parsed->num_inputs);
    parsed->input_lengths = malloc(sizeof(size_t) * parsed->num_inputs);
    parsed->input_types = malloc(sizeof(uint8_t) * parsed->num_inputs);

    // Parse each input
    for (size_t i = 0; i < parsed->num_inputs; i++) {
        err = bcs_read_u8(&reader, &parsed->input_types[i]);
        if (err != BCS_OK) return err;

        printf("  Input %zu type: %u ", i, parsed->input_types[i]);

        if (parsed->input_types[i] == 0) { // Pure input
            uint64_t data_len;
            err = bcs_read_uleb128(&reader, &data_len);
            if (err != BCS_OK) return err;

            parsed->input_lengths[i] = (size_t)data_len;
            parsed->input_data[i] = malloc(data_len);

            err = bcs_read_bytes(&reader, parsed->input_data[i], data_len);
            if (err != BCS_OK) return err;

            printf("(Pure, %zu bytes)\n", (size_t)data_len);
        } else {
            // For other input types, you'd parse accordingly
            printf("(Other type - not fully parsed in this example)\n");
        }
    }

    // Store remaining data (commands, etc.)
    parsed->commands_length = bcs_reader_remaining(&reader);
    if (parsed->commands_length > 0) {
        parsed->commands_data = malloc(parsed->commands_length);
        bcs_read_bytes(&reader, parsed->commands_data, parsed->commands_length);
    } else {
        parsed->commands_data = NULL;
    }

    printf("Remaining data (commands, etc.): %zu bytes\n\n", parsed->commands_length);

    return BCS_OK;
}

// Free parsed transaction data
void free_parsed_transaction(ParsedTransaction *parsed) {
    if (parsed->input_data) {
        for (size_t i = 0; i < parsed->num_inputs; i++) {
            if (parsed->input_data[i]) {
                free(parsed->input_data[i]);
            }
        }
        free(parsed->input_data);
    }
    if (parsed->input_lengths) free(parsed->input_lengths);
    if (parsed->input_types) free(parsed->input_types);
    if (parsed->commands_data) free(parsed->commands_data);
}

// Add new pure inputs to the transaction
bcs_error_t rebuild_transaction_with_new_inputs(
    ParsedTransaction *parsed,
    const uint8_t **new_inputs,
    const size_t *new_input_lengths,
    size_t num_new_inputs,
    uint8_t **output_tx,
    size_t *output_length
) {
    bcs_writer_t writer;
    bcs_writer_init(&writer, 512, 0);

    // Write transaction kind
    bcs_write_u8(&writer, parsed->kind);

    // Write total number of inputs (original + new)
    size_t total_inputs = parsed->num_inputs + num_new_inputs;
    bcs_write_uleb128(&writer, total_inputs);

    printf("Rebuilding with %zu total inputs (%zu original + %zu new)\n",
           total_inputs, parsed->num_inputs, num_new_inputs);

    // Write original inputs
    for (size_t i = 0; i < parsed->num_inputs; i++) {
        bcs_write_u8(&writer, parsed->input_types[i]);
        if (parsed->input_types[i] == 0) { // Pure
            bcs_write_uleb128(&writer, parsed->input_lengths[i]);
            bcs_write_fixed_bytes(&writer, parsed->input_data[i], parsed->input_lengths[i]);
        }
    }

    // Write new pure inputs
    for (size_t i = 0; i < num_new_inputs; i++) {
        bcs_write_u8(&writer, 0); // Pure input type
        bcs_write_uleb128(&writer, new_input_lengths[i]);
        bcs_write_fixed_bytes(&writer, new_inputs[i], new_input_lengths[i]);
        printf("  Added new input %zu: %zu bytes\n", i, new_input_lengths[i]);
    }

    // Write the rest of the transaction (commands, etc.)
    if (parsed->commands_data && parsed->commands_length > 0) {
        bcs_write_fixed_bytes(&writer, parsed->commands_data, parsed->commands_length);
    }

    // Get the final transaction bytes
    size_t length;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &length);

    // Allocate output buffer
    *output_tx = malloc(length);
    memcpy(*output_tx, bytes, length);
    *output_length = length;

    bcs_writer_free(&writer);
    printf("Rebuilt transaction: %zu bytes\n\n", length);

    return BCS_OK;
}

// Create a simple transaction for testing
void create_sample_transaction(uint8_t **tx_bytes, size_t *tx_length) {
    bcs_writer_t writer;
    bcs_writer_init(&writer, 256, 0);

    // Transaction kind: ProgrammableTransaction
    bcs_write_u8(&writer, 0);

    // Number of inputs: 2
    bcs_write_vec_length(&writer, 2);

    // Input 0: Pure value (u32: 1000)
    bcs_write_u8(&writer, 0); // Pure type
    bcs_write_vec_length(&writer, 4); // 4 bytes
    bcs_write_u32(&writer, 1000);

    // Input 1: Pure value (u64: 5000000)
    bcs_write_u8(&writer, 0); // Pure type
    bcs_write_vec_length(&writer, 8); // 8 bytes
    bcs_write_u64(&writer, 5000000);

    // Commands: 1 command (simplified)
    bcs_write_vec_length(&writer, 1);
    bcs_write_u8(&writer, 0); // MoveCall

    // Package address (32 bytes)
    uint8_t package[32] = {0};
    package[31] = 0x02;
    bcs_write_fixed_bytes(&writer, package, 32);

    // Module name
    bcs_write_string(&writer, "sensor");

    // Function name
    bcs_write_string(&writer, "process");

    // Type arguments (empty)
    bcs_write_vec_length(&writer, 0);

    // Arguments (use inputs 0 and 1)
    bcs_write_vec_length(&writer, 2);
    bcs_write_u16(&writer, 0); // Input 0
    bcs_write_u16(&writer, 1); // Input 1

    // Get bytes
    size_t length;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &length);

    *tx_bytes = malloc(length);
    memcpy(*tx_bytes, bytes, length);
    *tx_length = length;

    bcs_writer_free(&writer);
}

int main() {
    printf("=== Transaction Modification Example ===\n\n");

    // Step 1: Create a sample transaction
    printf("Step 1: Creating sample transaction...\n");
    uint8_t *original_tx;
    size_t original_length;
    create_sample_transaction(&original_tx, &original_length);

    printf("Original transaction: %zu bytes\n", original_length);
    char hex1[original_length * 2 + 1];
    bcs_bytes_to_hex(original_tx, original_length, hex1);
    printf("Hex: %s\n\n", hex1);

    // Step 2: Parse/deserialize the transaction
    printf("Step 2: Parsing transaction...\n");
    ParsedTransaction parsed = {0};
    bcs_error_t err = parse_transaction(original_tx, original_length, &parsed);
    if (err != BCS_OK) {
        printf("Error parsing transaction: %d\n", err);
        free(original_tx);
        return 1;
    }

    // Step 3: Add new pure values (sensor readings)
    printf("Step 3: Adding new sensor data as pure inputs...\n");

    // New input 1: timestamp (u32)
    uint8_t new_input1[4];
    bcs_writer_t temp_writer;
    bcs_writer_init(&temp_writer, 16, 0);
    bcs_write_u32(&temp_writer, 1699564800);
    size_t len1;
    const uint8_t *bytes1 = bcs_writer_get_bytes(&temp_writer, &len1);
    memcpy(new_input1, bytes1, len1);

    // New input 2: temperature (u32)
    uint8_t new_input2[4];
    bcs_writer_t temp_writer2;
    bcs_writer_init(&temp_writer2, 16, 0);
    bcs_write_u32(&temp_writer2, 2350); // 23.50°C * 100
    size_t len2;
    const uint8_t *bytes2 = bcs_writer_get_bytes(&temp_writer2, &len2);
    memcpy(new_input2, bytes2, len2);

    const uint8_t *new_inputs[] = {new_input1, new_input2};
    size_t new_input_lengths[] = {len1, len2};

    // Step 4: Rebuild transaction with new inputs
    printf("\nStep 4: Rebuilding transaction...\n");
    uint8_t *modified_tx;
    size_t modified_length;

    err = rebuild_transaction_with_new_inputs(
        &parsed,
        new_inputs,
        new_input_lengths,
        2,
        &modified_tx,
        &modified_length
    );

    if (err != BCS_OK) {
        printf("Error rebuilding transaction: %d\n", err);
        free(original_tx);
        free_parsed_transaction(&parsed);
        bcs_writer_free(&temp_writer);
        bcs_writer_free(&temp_writer2);
        return 1;
    }

    printf("Modified transaction: %zu bytes\n", modified_length);
    char hex2[modified_length * 2 + 1];
    bcs_bytes_to_hex(modified_tx, modified_length, hex2);
    printf("Hex: %s\n\n", hex2);

    // Step 5: Show what you would do next
    printf("Step 5: Next steps for execution:\n");
    printf("  1. Add transaction metadata (sender, gas, etc.)\n");
    printf("  2. Sign the transaction bytes with your private key\n");
    printf("  3. Serialize the signature\n");
    printf("  4. Send to Sui RPC endpoint for execution\n\n");

    printf("Transaction successfully modified!\n");
    printf("Original: %zu bytes, Modified: %zu bytes\n", original_length, modified_length);

    // Cleanup
    free(original_tx);
    free(modified_tx);
    free_parsed_transaction(&parsed);
    bcs_writer_free(&temp_writer);
    bcs_writer_free(&temp_writer2);

    return 0;
}
