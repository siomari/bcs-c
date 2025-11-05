/**
 * Example: Adding sensor readings to an existing transaction
 *
 * Use case: You receive a pre-built transaction from your backend/server
 * that already has gas coins and all necessary objects. You just need to
 * add your sensor readings as pure inputs and update the command arguments.
 */

#include "../src/bcs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Simple helper to add pure inputs to an existing transaction
 * and update command arguments to reference them
 */
bcs_error_t add_sensor_readings_to_transaction(
    const uint8_t *original_tx,
    size_t original_tx_len,
    float temperature,
    float humidity,
    uint32_t timestamp,
    uint8_t **output_tx,
    size_t *output_tx_len
) {
    // Step 1: Parse the original transaction to get the structure
    bcs_reader_t reader;
    bcs_reader_init(&reader, original_tx, original_tx_len);

    // Read transaction kind
    uint8_t tx_kind;
    bcs_read_u8(&reader, &tx_kind);
    printf("Transaction kind: %u\n", tx_kind);

    // Read number of existing inputs
    uint64_t num_existing_inputs;
    bcs_read_uleb128(&reader, &num_existing_inputs);
    printf("Existing inputs: %llu\n", num_existing_inputs);

    // Save position and remaining data
    size_t inputs_start_pos = reader.position;
    size_t remaining_len = bcs_reader_remaining(&reader);
    uint8_t *remaining_data = malloc(remaining_len);
    bcs_read_bytes(&reader, remaining_data, remaining_len);

    // Step 2: Prepare sensor data as pure inputs
    bcs_writer_t sensor_writer;
    bcs_writer_init(&sensor_writer, 128, 0);

    // Sensor reading 1: Temperature (convert to fixed point: temp * 100)
    uint32_t temp_fixed = (uint32_t)(temperature * 100);
    bcs_write_u32(&sensor_writer, temp_fixed);

    // Sensor reading 2: Humidity (convert to fixed point: humidity * 100)
    uint32_t humidity_fixed = (uint32_t)(humidity * 100);
    bcs_write_u32(&sensor_writer, humidity_fixed);

    // Sensor reading 3: Timestamp
    bcs_write_u32(&sensor_writer, timestamp);

    size_t sensor_data_len;
    const uint8_t *sensor_data = bcs_writer_get_bytes(&sensor_writer, &sensor_data_len);

    printf("Sensor data prepared: %zu bytes\n", sensor_data_len);
    printf("  Temperature: %.2f°C -> %u\n", temperature, temp_fixed);
    printf("  Humidity: %.2f%% -> %u\n", humidity, humidity_fixed);
    printf("  Timestamp: %u\n", timestamp);

    // Step 3: Rebuild transaction with original inputs + new sensor inputs
    bcs_writer_t new_tx_writer;
    bcs_writer_init(&new_tx_writer, 1024, 0);

    // Write transaction kind
    bcs_write_u8(&new_tx_writer, tx_kind);

    // Write NEW total number of inputs (original + 3 new sensor inputs)
    size_t total_inputs = num_existing_inputs + 3;
    bcs_write_uleb128(&new_tx_writer, total_inputs);
    printf("New total inputs: %zu\n", total_inputs);

    // Write all original inputs (copy from original transaction)
    const uint8_t *original_inputs = original_tx + inputs_start_pos;

    // We need to copy the original inputs section
    // Create a reader just for the inputs
    bcs_reader_t inputs_reader;
    bcs_reader_init(&inputs_reader, original_tx, original_tx_len);
    bcs_read_u8(&inputs_reader, &tx_kind); // skip kind
    bcs_read_uleb128(&inputs_reader, &num_existing_inputs); // skip input count

    // Now parse and copy each original input
    for (uint64_t i = 0; i < num_existing_inputs; i++) {
        uint8_t input_type;
        bcs_read_u8(&inputs_reader, &input_type);
        bcs_write_u8(&new_tx_writer, input_type);

        if (input_type == 0) { // Pure input
            uint64_t data_len;
            bcs_read_uleb128(&inputs_reader, &data_len);
            bcs_write_uleb128(&new_tx_writer, data_len);

            uint8_t *data = malloc(data_len);
            bcs_read_bytes(&inputs_reader, data, data_len);
            bcs_write_fixed_bytes(&new_tx_writer, data, data_len);
            free(data);
        } else if (input_type == 1) { // Object input
            // Object inputs have object reference structure
            // For simplicity, copy the next 32 bytes (ObjectID)
            uint8_t obj_data[32];
            bcs_read_bytes(&inputs_reader, obj_data, 32);
            bcs_write_fixed_bytes(&new_tx_writer, obj_data, 32);

            // Version (u64)
            uint64_t version;
            bcs_read_u64(&inputs_reader, &version);
            bcs_write_u64(&new_tx_writer, version);

            // Digest (32 bytes)
            uint8_t digest[32];
            bcs_read_bytes(&inputs_reader, digest, 32);
            bcs_write_fixed_bytes(&new_tx_writer, digest, 32);
        }
    }

    // Write 3 new pure inputs with sensor data
    // Input 1: Temperature
    bcs_write_u8(&new_tx_writer, 0); // Pure type
    bcs_write_uleb128(&new_tx_writer, 4); // 4 bytes
    bcs_write_u32(&new_tx_writer, temp_fixed);

    // Input 2: Humidity
    bcs_write_u8(&new_tx_writer, 0); // Pure type
    bcs_write_uleb128(&new_tx_writer, 4); // 4 bytes
    bcs_write_u32(&new_tx_writer, humidity_fixed);

    // Input 3: Timestamp
    bcs_write_u8(&new_tx_writer, 0); // Pure type
    bcs_write_uleb128(&new_tx_writer, 4); // 4 bytes
    bcs_write_u32(&new_tx_writer, timestamp);

    // Write the rest of the transaction (commands, etc.)
    // Copy remaining data from where we left off
    size_t commands_start = inputs_reader.position;
    size_t commands_len = original_tx_len - commands_start;
    bcs_write_fixed_bytes(&new_tx_writer, original_tx + commands_start, commands_len);

    // Get final transaction
    size_t final_len;
    const uint8_t *final_tx = bcs_writer_get_bytes(&new_tx_writer, &final_len);

    *output_tx = malloc(final_len);
    memcpy(*output_tx, final_tx, final_len);
    *output_tx_len = final_len;

    printf("Rebuilt transaction: %zu bytes\n", final_len);

    // Cleanup
    free(remaining_data);
    bcs_writer_free(&sensor_writer);
    bcs_writer_free(&new_tx_writer);

    return BCS_OK;
}

/**
 * Simpler version: Just append sensor data to transaction
 * Assumes you'll update command arguments separately
 */
bcs_error_t append_sensor_inputs_simple(
    const uint8_t *original_tx,
    size_t original_tx_len,
    float temperature,
    float humidity,
    uint32_t timestamp,
    uint8_t **output_tx,
    size_t *output_tx_len
) {
    printf("\n=== Simple Append Method ===\n");

    // Parse to find where to inject
    bcs_reader_t reader;
    bcs_reader_init(&reader, original_tx, original_tx_len);

    uint8_t tx_kind;
    bcs_read_u8(&reader, &tx_kind);

    uint64_t num_inputs;
    bcs_read_uleb128(&reader, &num_inputs);

    printf("Adding sensor data to transaction with %llu existing inputs\n", num_inputs);

    // Build new transaction
    bcs_writer_t writer;
    bcs_writer_init(&writer, 2048, 0);

    // Write kind
    bcs_write_u8(&writer, tx_kind);

    // Write new input count
    bcs_write_uleb128(&writer, num_inputs + 3);

    // Copy original inputs section
    // This is a simplified approach - copy everything after input count
    size_t copy_start = reader.position;
    size_t copy_len = original_tx_len - copy_start;

    // First, we need to find where inputs end and commands begin
    // For this simple version, let's just copy the original inputs
    bcs_reader_t temp_reader;
    bcs_reader_init(&temp_reader, original_tx, original_tx_len);
    bcs_read_u8(&temp_reader, &tx_kind);
    bcs_read_uleb128(&temp_reader, &num_inputs);

    size_t inputs_start = temp_reader.position;

    // Parse through inputs to find where they end
    for (uint64_t i = 0; i < num_inputs; i++) {
        uint8_t input_type;
        bcs_read_u8(&temp_reader, &input_type);

        if (input_type == 0) { // Pure
            uint64_t len;
            bcs_read_uleb128(&temp_reader, &len);
            uint8_t *data = malloc(len);
            bcs_read_bytes(&temp_reader, data, len);
            free(data);
        } else if (input_type == 1) { // Object
            uint8_t skip[72]; // 32 + 8 + 32 bytes
            bcs_read_bytes(&temp_reader, skip, 72);
        }
    }

    size_t inputs_end = temp_reader.position;
    size_t inputs_section_len = inputs_end - inputs_start;

    // Write original inputs
    bcs_write_fixed_bytes(&writer, original_tx + inputs_start, inputs_section_len);

    // Add new sensor inputs
    uint32_t temp_fixed = (uint32_t)(temperature * 100);
    uint32_t humidity_fixed = (uint32_t)(humidity * 100);

    bcs_write_u8(&writer, 0); // Pure
    bcs_write_uleb128(&writer, 4);
    bcs_write_u32(&writer, temp_fixed);

    bcs_write_u8(&writer, 0); // Pure
    bcs_write_uleb128(&writer, 4);
    bcs_write_u32(&writer, humidity_fixed);

    bcs_write_u8(&writer, 0); // Pure
    bcs_write_uleb128(&writer, 4);
    bcs_write_u32(&writer, timestamp);

    printf("Added 3 sensor inputs:\n");
    printf("  - Temperature: %.2f°C\n", temperature);
    printf("  - Humidity: %.2f%%\n", humidity);
    printf("  - Timestamp: %u\n", timestamp);

    // Copy rest of transaction (commands)
    size_t remaining_len = original_tx_len - inputs_end;
    bcs_write_fixed_bytes(&writer, original_tx + inputs_end, remaining_len);

    // Get result
    size_t final_len;
    const uint8_t *final_bytes = bcs_writer_get_bytes(&writer, &final_len);

    *output_tx = malloc(final_len);
    memcpy(*output_tx, final_bytes, final_len);
    *output_tx_len = final_len;

    printf("New transaction size: %zu bytes (was %zu bytes)\n", final_len, original_tx_len);

    bcs_writer_free(&writer);

    return BCS_OK;
}

// Create a sample transaction for testing
void create_test_transaction(uint8_t **tx, size_t *tx_len) {
    bcs_writer_t writer;
    bcs_writer_init(&writer, 512, 0);

    // ProgrammableTransaction
    bcs_write_u8(&writer, 0);

    // 2 existing inputs (gas coin + object)
    bcs_write_vec_length(&writer, 2);

    // Input 0: Object (gas coin)
    bcs_write_u8(&writer, 1); // Object type
    uint8_t obj_id[32] = {0};
    obj_id[31] = 0x01;
    bcs_write_fixed_bytes(&writer, obj_id, 32);
    bcs_write_u64(&writer, 1000); // version
    uint8_t digest[32] = {0};
    bcs_write_fixed_bytes(&writer, digest, 32);

    // Input 1: Pure value (some parameter)
    bcs_write_u8(&writer, 0); // Pure type
    bcs_write_vec_length(&writer, 4);
    bcs_write_u32(&writer, 12345);

    // Commands
    bcs_write_vec_length(&writer, 1);
    bcs_write_u8(&writer, 0); // MoveCall

    uint8_t package[32] = {0};
    package[31] = 0x02;
    bcs_write_fixed_bytes(&writer, package, 32);
    bcs_write_string(&writer, "sensor_contract");
    bcs_write_string(&writer, "record_data");
    bcs_write_vec_length(&writer, 0); // type args

    // Arguments - will reference the NEW sensor inputs we'll add
    bcs_write_vec_length(&writer, 4); // 4 arguments
    bcs_write_u16(&writer, 1); // Input 1 (existing parameter)
    bcs_write_u16(&writer, 2); // Input 2 (temperature - to be added)
    bcs_write_u16(&writer, 3); // Input 3 (humidity - to be added)
    bcs_write_u16(&writer, 4); // Input 4 (timestamp - to be added)

    size_t len;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &len);

    *tx = malloc(len);
    memcpy(*tx, bytes, len);
    *tx_len = len;

    bcs_writer_free(&writer);
}

int main() {
    printf("=== Add Sensor Data to Existing Transaction ===\n\n");

    // Simulate receiving a transaction from your backend
    printf("Creating test transaction (simulating backend)...\n");
    uint8_t *original_tx;
    size_t original_len;
    create_test_transaction(&original_tx, &original_len);

    printf("Received transaction: %zu bytes\n", original_len);
    char hex1[original_len * 2 + 1];
    bcs_bytes_to_hex(original_tx, original_len, hex1);
    printf("Original hex: %s\n\n", hex1);

    // Get sensor readings
    float temperature = 23.45;
    float humidity = 65.8;
    uint32_t timestamp = 1699564800;

    printf("Current sensor readings:\n");
    printf("  Temperature: %.2f°C\n", temperature);
    printf("  Humidity: %.2f%%\n", humidity);
    printf("  Timestamp: %u\n\n", timestamp);

    // Add sensor data to transaction
    uint8_t *modified_tx;
    size_t modified_len;

    bcs_error_t err = append_sensor_inputs_simple(
        original_tx,
        original_len,
        temperature,
        humidity,
        timestamp,
        &modified_tx,
        &modified_len
    );

    if (err != BCS_OK) {
        printf("Error: %d\n", err);
        free(original_tx);
        return 1;
    }

    printf("\nModified transaction: %zu bytes\n", modified_len);
    char hex2[modified_len * 2 + 1];
    bcs_bytes_to_hex(modified_tx, modified_len, hex2);
    printf("Modified hex: %s\n\n", hex2);

    printf("✓ Success! Transaction now includes sensor readings.\n");
    printf("\nNext steps:\n");
    printf("  1. Sign this modified transaction with your private key\n");
    printf("  2. Send to Sui network for execution\n");
    printf("  3. The Move function will receive your sensor data\n");

    // Cleanup
    free(original_tx);
    free(modified_tx);

    return 0;
}
