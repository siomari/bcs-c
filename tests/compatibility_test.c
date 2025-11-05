/**
 * BCS Compatibility Test
 *
 * This test verifies that our C BCS implementation produces
 * exactly the same bytes as the TypeScript BCS library.
 *
 * Test vectors were generated using @mysten/bcs TypeScript library
 */

#include "../src/bcs.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Helper to compare hex strings
int compare_hex(const char *expected, const uint8_t *actual, size_t len) {
    char actual_hex[len * 2 + 1];
    bcs_bytes_to_hex(actual, len, actual_hex);

    if (strcmp(expected, actual_hex) == 0) {
        printf("  ✓ MATCH: %s\n", expected);
        return 1;
    } else {
        printf("  ✗ MISMATCH:\n");
        printf("    Expected: %s\n", expected);
        printf("    Got:      %s\n", actual_hex);
        return 0;
    }
}

void test_u8_compatibility() {
    printf("\n=== u8 Compatibility ===\n");

    bcs_writer_t writer;
    bcs_writer_init(&writer, 16, 0);

    // Test value: 42
    bcs_write_u8(&writer, 42);

    size_t len;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &len);

    // Expected from TypeScript: bcs.u8().serialize(42).toHex()
    assert(compare_hex("2a", bytes, len));

    bcs_writer_free(&writer);
}

void test_u32_compatibility() {
    printf("\n=== u32 Compatibility ===\n");

    bcs_writer_t writer;
    bcs_writer_init(&writer, 16, 0);

    // Test value: 1000000
    bcs_write_u32(&writer, 1000000);

    size_t len;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &len);

    // Expected from TypeScript: bcs.u32().serialize(1000000).toHex()
    assert(compare_hex("40420f00", bytes, len));

    bcs_writer_free(&writer);
}

void test_u64_compatibility() {
    printf("\n=== u64 Compatibility ===\n");

    bcs_writer_t writer;
    bcs_writer_init(&writer, 16, 0);

    // Test value: 1000000000
    bcs_write_u64(&writer, 1000000000ULL);

    size_t len;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &len);

    // Expected from TypeScript: bcs.u64().serialize(1000000000).toHex()
    assert(compare_hex("00ca9a3b00000000", bytes, len));

    bcs_writer_free(&writer);
}

void test_bool_compatibility() {
    printf("\n=== bool Compatibility ===\n");

    bcs_writer_t writer;
    bcs_writer_init(&writer, 16, 0);

    // Test true
    bcs_write_bool(&writer, true);
    // Test false
    bcs_write_bool(&writer, false);

    size_t len;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &len);

    // Expected from TypeScript:
    // bcs.bool().serialize(true).toHex() + bcs.bool().serialize(false).toHex()
    assert(compare_hex("0100", bytes, len));

    bcs_writer_free(&writer);
}

void test_string_compatibility() {
    printf("\n=== string Compatibility ===\n");

    bcs_writer_t writer;
    bcs_writer_init(&writer, 32, 0);

    // Test value: "hello"
    bcs_write_string(&writer, "hello");

    size_t len;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &len);

    // Expected from TypeScript: bcs.string().serialize("hello").toHex()
    // Format: [length as ULEB128][UTF-8 bytes]
    // "hello" = 5 bytes: 0x68 0x65 0x6C 0x6C 0x6F
    // Length 5 = 0x05
    assert(compare_hex("0568656c6c6f", bytes, len));

    bcs_writer_free(&writer);
}

void test_vector_u32_compatibility() {
    printf("\n=== vector<u32> Compatibility ===\n");

    bcs_writer_t writer;
    bcs_writer_init(&writer, 32, 0);

    // Test value: [100, 200, 300]
    bcs_write_vec_length(&writer, 3);
    bcs_write_u32(&writer, 100);
    bcs_write_u32(&writer, 200);
    bcs_write_u32(&writer, 300);

    size_t len;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &len);

    // Expected from TypeScript: bcs.vector(bcs.u32()).serialize([100, 200, 300]).toHex()
    // Format: [length=3][100][200][300]
    assert(compare_hex("0364000000c80000002c010000", bytes, len));

    bcs_writer_free(&writer);
}

void test_option_some_compatibility() {
    printf("\n=== option<u32> Some Compatibility ===\n");

    bcs_writer_t writer;
    bcs_writer_init(&writer, 32, 0);

    // Test value: Some(42)
    bcs_write_option_some(&writer);
    bcs_write_u32(&writer, 42);

    size_t len;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &len);

    // Expected from TypeScript: bcs.option(bcs.u32()).serialize(42).toHex()
    // Format: [1 for Some][value]
    assert(compare_hex("012a000000", bytes, len));

    bcs_writer_free(&writer);
}

void test_option_none_compatibility() {
    printf("\n=== option<u32> None Compatibility ===\n");

    bcs_writer_t writer;
    bcs_writer_init(&writer, 32, 0);

    // Test value: None
    bcs_write_option_none(&writer);

    size_t len;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &len);

    // Expected from TypeScript: bcs.option(bcs.u32()).serialize(null).toHex()
    // Format: [0 for None]
    assert(compare_hex("00", bytes, len));

    bcs_writer_free(&writer);
}

void test_address_compatibility() {
    printf("\n=== address (32 bytes) Compatibility ===\n");

    bcs_writer_t writer;
    bcs_writer_init(&writer, 64, 0);

    // Test value: 0x0000000000000000000000000000000000000000000000000000000000000001
    uint8_t address[32] = {0};
    address[31] = 0x01;

    bcs_write_fixed_bytes(&writer, address, 32);

    size_t len;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &len);

    // Expected: exact 32 bytes with last byte = 0x01
    assert(compare_hex(
        "0000000000000000000000000000000000000000000000000000000000000001",
        bytes, len
    ));

    bcs_writer_free(&writer);
}

void test_uleb128_compatibility() {
    printf("\n=== ULEB128 Compatibility ===\n");

    struct {
        uint64_t value;
        const char *expected_hex;
    } test_cases[] = {
        {0, "00"},
        {1, "01"},
        {127, "7f"},
        {128, "8001"},
        {255, "ff01"},
        {256, "8002"},
        {16383, "ff7f"},
        {16384, "808001"},
    };

    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        bcs_writer_t writer;
        bcs_writer_init(&writer, 16, 0);

        bcs_write_uleb128(&writer, test_cases[i].value);

        size_t len;
        const uint8_t *bytes = bcs_writer_get_bytes(&writer, &len);

        printf("  ULEB128(%llu): ", test_cases[i].value);
        assert(compare_hex(test_cases[i].expected_hex, bytes, len));

        bcs_writer_free(&writer);
    }
}

void test_struct_compatibility() {
    printf("\n=== struct Compatibility ===\n");

    bcs_writer_t writer;
    bcs_writer_init(&writer, 64, 0);

    // Simulate: struct Coin { id: u64, value: u64 }
    // Test value: { id: 1, value: 1000000 }
    bcs_write_u64(&writer, 1);
    bcs_write_u64(&writer, 1000000);

    size_t len;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &len);

    // Expected from TypeScript:
    // const Coin = bcs.struct('Coin', { id: bcs.u64(), value: bcs.u64() });
    // Coin.serialize({ id: 1, value: 1000000 }).toHex()
    assert(compare_hex("010000000000000040420f0000000000", bytes, len));

    bcs_writer_free(&writer);
}

void test_complex_structure_compatibility() {
    printf("\n=== Complex Structure Compatibility ===\n");

    bcs_writer_t writer;
    bcs_writer_init(&writer, 128, 0);

    // Simulate a sensor reading structure:
    // struct SensorReading {
    //     timestamp: u32,
    //     temperature: u32,
    //     readings: vector<u8>
    // }
    // Value: { timestamp: 1699564800, temperature: 2350, readings: [1, 2, 3] }

    bcs_write_u32(&writer, 1699564800);  // timestamp
    bcs_write_u32(&writer, 2350);        // temperature

    // Vector of u8
    bcs_write_vec_length(&writer, 3);
    bcs_write_u8(&writer, 1);
    bcs_write_u8(&writer, 2);
    bcs_write_u8(&writer, 3);

    size_t len;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &len);

    printf("  Sensor reading structure: ");
    char hex[len * 2 + 1];
    bcs_bytes_to_hex(bytes, len, hex);
    printf("%s\n", hex);

    // Verify structure: 4 bytes timestamp + 4 bytes temp + 1 byte length + 3 bytes data
    assert(len == 12);
    assert(bytes[0] == 0x00); // timestamp low byte (1699564800 = 0x654D4D00)
    assert(bytes[1] == 0x4D);
    assert(bytes[2] == 0x4D);
    assert(bytes[3] == 0x65);
    assert(bytes[4] == 0x2E); // temperature low byte (2350 = 0x092E)
    assert(bytes[5] == 0x09);
    assert(bytes[6] == 0x00);
    assert(bytes[7] == 0x00);
    assert(bytes[8] == 0x03); // vector length = 3
    assert(bytes[9] == 0x01);  // reading[0]
    assert(bytes[10] == 0x02); // reading[1]
    assert(bytes[11] == 0x03); // reading[2]

    printf("  ✓ Structure validated!\n");

    bcs_writer_free(&writer);
}

int main() {
    printf("========================================\n");
    printf("BCS Compatibility Test Suite\n");
    printf("========================================\n");
    printf("\nVerifying C BCS implementation matches\n");
    printf("TypeScript @mysten/bcs output...\n");

    test_u8_compatibility();
    test_u32_compatibility();
    test_u64_compatibility();
    test_bool_compatibility();
    test_string_compatibility();
    test_vector_u32_compatibility();
    test_option_some_compatibility();
    test_option_none_compatibility();
    test_address_compatibility();
    test_uleb128_compatibility();
    test_struct_compatibility();
    test_complex_structure_compatibility();

    printf("\n========================================\n");
    printf("✓ ALL COMPATIBILITY TESTS PASSED!\n");
    printf("========================================\n");
    printf("\nConclusion: C BCS implementation produces\n");
    printf("byte-for-byte identical output to the\n");
    printf("TypeScript SDK. Sui validators WILL\n");
    printf("understand these bytes correctly.\n\n");

    return 0;
}
