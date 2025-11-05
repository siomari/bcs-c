/**
 * Example: Building a Sui transaction from a sensor
 *
 * This example demonstrates how to use the BCS library to serialize
 * transaction data from an IoT sensor that needs to submit data to
 * the Sui blockchain.
 */

#include "../src/bcs.h"
#include <stdio.h>
#include <string.h>

// Simulated sensor reading structure
typedef struct {
    uint32_t timestamp;
    float temperature;
    float humidity;
    uint8_t sensor_id;
} SensorReading;

// Example: Serialize a simple transfer transaction
void example_transfer_transaction() {
    printf("=== Example: Transfer Transaction ===\n");

    bcs_writer_t writer;
    bcs_error_t err;

    // Initialize writer with 256 bytes initial capacity
    err = bcs_writer_init(&writer, 256, 0);
    if (err != BCS_OK) {
        printf("Failed to initialize writer: %d\n", err);
        return;
    }

    // Serialize a recipient address (32 bytes)
    uint8_t recipient[32] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
    };
    bcs_write_fixed_bytes(&writer, recipient, 32);

    // Serialize amount (u64)
    bcs_write_u64(&writer, 1000000); // 1 SUI

    // Get serialized bytes
    size_t length;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &length);

    // Print result
    printf("Serialized %zu bytes: ", length);
    char hex[length * 2 + 1];
    bcs_bytes_to_hex(bytes, length, hex);
    printf("%s\n\n", hex);

    bcs_writer_free(&writer);
}

// Example: Serialize sensor data for on-chain storage
void example_sensor_data() {
    printf("=== Example: Sensor Data Serialization ===\n");

    SensorReading reading = {
        .timestamp = 1699564800,  // Unix timestamp
        .temperature = 23.5,      // 23.5°C
        .humidity = 65.2,         // 65.2%
        .sensor_id = 42
    };

    bcs_writer_t writer;
    bcs_error_t err;

    err = bcs_writer_init(&writer, 128, 0);
    if (err != BCS_OK) {
        printf("Failed to initialize writer: %d\n", err);
        return;
    }

    // Serialize sensor reading
    bcs_write_u32(&writer, reading.timestamp);

    // Convert float to fixed-point for blockchain storage
    // (multiply by 100 to preserve 2 decimal places)
    uint32_t temp_fixed = (uint32_t)(reading.temperature * 100);
    uint32_t humidity_fixed = (uint32_t)(reading.humidity * 100);

    bcs_write_u32(&writer, temp_fixed);
    bcs_write_u32(&writer, humidity_fixed);
    bcs_write_u8(&writer, reading.sensor_id);

    // Get result
    size_t length;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &length);

    printf("Sensor reading serialized to %zu bytes\n", length);
    printf("Timestamp: %u\n", reading.timestamp);
    printf("Temperature: %.1f°C\n", reading.temperature);
    printf("Humidity: %.1f%%\n", reading.humidity);
    printf("Sensor ID: %u\n", reading.sensor_id);

    char hex[length * 2 + 1];
    bcs_bytes_to_hex(bytes, length, hex);
    printf("Hex: %s\n\n", hex);

    bcs_writer_free(&writer);
}

// Example: Serialize a vector of sensor readings
void example_sensor_batch() {
    printf("=== Example: Batch Sensor Readings ===\n");

    SensorReading readings[] = {
        {.timestamp = 1699564800, .temperature = 23.5, .humidity = 65.2, .sensor_id = 1},
        {.timestamp = 1699564860, .temperature = 23.7, .humidity = 64.8, .sensor_id = 1},
        {.timestamp = 1699564920, .temperature = 23.9, .humidity = 64.5, .sensor_id = 1},
    };

    size_t num_readings = sizeof(readings) / sizeof(readings[0]);

    bcs_writer_t writer;
    bcs_error_t err;

    err = bcs_writer_init(&writer, 256, 0);
    if (err != BCS_OK) {
        printf("Failed to initialize writer: %d\n", err);
        return;
    }

    // Write vector length
    bcs_write_vec_length(&writer, num_readings);

    // Write each reading
    for (size_t i = 0; i < num_readings; i++) {
        bcs_write_u32(&writer, readings[i].timestamp);
        bcs_write_u32(&writer, (uint32_t)(readings[i].temperature * 100));
        bcs_write_u32(&writer, (uint32_t)(readings[i].humidity * 100));
        bcs_write_u8(&writer, readings[i].sensor_id);
    }

    size_t length;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &length);

    printf("Serialized %zu readings to %zu bytes\n", num_readings, length);
    char hex[length * 2 + 1];
    bcs_bytes_to_hex(bytes, length, hex);
    printf("Hex: %s\n\n", hex);

    bcs_writer_free(&writer);
}

// Example: Working with addresses (hex conversion)
void example_address_handling() {
    printf("=== Example: Address Handling ===\n");

    const char *address_hex = "0x0000000000000000000000000000000000000000000000000000000000000002";

    uint8_t address_bytes[32];
    size_t actual_bytes;

    bcs_error_t err = bcs_hex_to_bytes(address_hex, address_bytes, 32, &actual_bytes);
    if (err != BCS_OK) {
        printf("Failed to convert hex to bytes: %d\n", err);
        return;
    }

    printf("Converted address: %s\n", address_hex);
    printf("To %zu bytes\n", actual_bytes);

    // Serialize the address
    bcs_writer_t writer;
    bcs_writer_init(&writer, 64, 0);
    bcs_write_fixed_bytes(&writer, address_bytes, 32);

    size_t length;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &length);

    char result_hex[length * 2 + 1];
    bcs_bytes_to_hex(bytes, length, result_hex);
    printf("Serialized: %s\n\n", result_hex);

    bcs_writer_free(&writer);
}

// Example: Building a Move call transaction structure
void example_move_call() {
    printf("=== Example: Move Call Transaction ===\n");

    bcs_writer_t writer;
    bcs_writer_init(&writer, 512, 0);

    // Transaction kind (0 for ProgrammableTransaction)
    bcs_write_u8(&writer, 0);

    // Number of inputs (vector length)
    bcs_write_vec_length(&writer, 2);

    // Input 1: Pure value (sensor data)
    bcs_write_u8(&writer, 0); // Pure variant
    bcs_write_vec_length(&writer, 4); // 4 bytes for u32
    bcs_write_u32(&writer, 1699564800); // Timestamp

    // Input 2: Object reference
    bcs_write_u8(&writer, 1); // Object variant

    // Commands (vector of commands)
    bcs_write_vec_length(&writer, 1);

    // MoveCall command
    bcs_write_u8(&writer, 0); // MoveCall variant

    // Package address
    uint8_t package[32] = {0};
    package[31] = 0x02; // 0x...02
    bcs_write_fixed_bytes(&writer, package, 32);

    // Module name
    bcs_write_string(&writer, "sensor_data");

    // Function name
    bcs_write_string(&writer, "record_reading");

    // Type arguments (empty vector)
    bcs_write_vec_length(&writer, 0);

    // Arguments (reference the inputs)
    bcs_write_vec_length(&writer, 2);
    bcs_write_u16(&writer, 0); // Input index 0
    bcs_write_u16(&writer, 1); // Input index 1

    size_t length;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &length);

    printf("Move call transaction serialized to %zu bytes\n", length);
    char hex[length * 2 + 1];
    bcs_bytes_to_hex(bytes, length, hex);
    printf("Hex: %s\n\n", hex);

    bcs_writer_free(&writer);
}

// Example: Reading back serialized data
void example_deserialization() {
    printf("=== Example: Deserialization ===\n");

    // First, serialize some data
    bcs_writer_t writer;
    bcs_writer_init(&writer, 64, 0);

    bcs_write_u32(&writer, 1699564800);
    bcs_write_u32(&writer, 2350); // 23.50 * 100
    bcs_write_u8(&writer, 42);

    size_t length;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &length);

    // Now deserialize it
    bcs_reader_t reader;
    bcs_reader_init(&reader, bytes, length);

    uint32_t timestamp, temperature;
    uint8_t sensor_id;

    bcs_read_u32(&reader, &timestamp);
    bcs_read_u32(&reader, &temperature);
    bcs_read_u8(&reader, &sensor_id);

    printf("Deserialized data:\n");
    printf("  Timestamp: %u\n", timestamp);
    printf("  Temperature: %.2f°C\n", temperature / 100.0);
    printf("  Sensor ID: %u\n", sensor_id);
    printf("  Remaining bytes: %zu\n\n", bcs_reader_remaining(&reader));

    bcs_writer_free(&writer);
}

int main() {
    printf("BCS Sensor Transaction Examples\n");
    printf("================================\n\n");

    example_transfer_transaction();
    example_sensor_data();
    example_sensor_batch();
    example_address_handling();
    example_move_call();
    example_deserialization();

    printf("All examples completed successfully!\n");

    return 0;
}
