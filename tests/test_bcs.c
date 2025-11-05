#include "../src/bcs.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define TEST_PASSED() printf("  ✓ %s passed\n", __func__)
#define TEST_FAILED() printf("  ✗ %s FAILED\n", __func__)

void test_write_read_u8() {
    bcs_writer_t writer;
    bcs_writer_init(&writer, 16, 0);

    bcs_write_u8(&writer, 42);
    bcs_write_u8(&writer, 255);

    size_t length;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &length);

    bcs_reader_t reader;
    bcs_reader_init(&reader, bytes, length);

    uint8_t val1, val2;
    bcs_read_u8(&reader, &val1);
    bcs_read_u8(&reader, &val2);

    assert(val1 == 42);
    assert(val2 == 255);

    bcs_writer_free(&writer);
    TEST_PASSED();
}

void test_write_read_u64() {
    bcs_writer_t writer;
    bcs_writer_init(&writer, 16, 0);

    uint64_t test_val = 0x0123456789ABCDEF;
    bcs_write_u64(&writer, test_val);

    size_t length;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &length);

    bcs_reader_t reader;
    bcs_reader_init(&reader, bytes, length);

    uint64_t result;
    bcs_read_u64(&reader, &result);

    assert(result == test_val);

    bcs_writer_free(&writer);
    TEST_PASSED();
}

void test_uleb128() {
    bcs_writer_t writer;
    bcs_writer_init(&writer, 32, 0);

    uint64_t test_values[] = {0, 1, 127, 128, 255, 256, 65535, 1000000};
    size_t num_values = sizeof(test_values) / sizeof(test_values[0]);

    for (size_t i = 0; i < num_values; i++) {
        bcs_write_uleb128(&writer, test_values[i]);
    }

    size_t length;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &length);

    bcs_reader_t reader;
    bcs_reader_init(&reader, bytes, length);

    for (size_t i = 0; i < num_values; i++) {
        uint64_t result;
        bcs_read_uleb128(&reader, &result);
        assert(result == test_values[i]);
    }

    bcs_writer_free(&writer);
    TEST_PASSED();
}

void test_string() {
    bcs_writer_t writer;
    bcs_writer_init(&writer, 64, 0);

    const char *test_str = "Hello, Sui!";
    bcs_write_string(&writer, test_str);

    size_t length;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &length);

    bcs_reader_t reader;
    bcs_reader_init(&reader, bytes, length);

    char result[64];
    size_t actual_length;
    bcs_read_string(&reader, result, sizeof(result), &actual_length);

    assert(strcmp(result, test_str) == 0);
    assert(actual_length == strlen(test_str));

    bcs_writer_free(&writer);
    TEST_PASSED();
}

void test_vector() {
    bcs_writer_t writer;
    bcs_writer_init(&writer, 64, 0);

    uint32_t values[] = {100, 200, 300, 400, 500};
    size_t num_values = sizeof(values) / sizeof(values[0]);

    bcs_write_vec_length(&writer, num_values);
    for (size_t i = 0; i < num_values; i++) {
        bcs_write_u32(&writer, values[i]);
    }

    size_t length;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &length);

    bcs_reader_t reader;
    bcs_reader_init(&reader, bytes, length);

    size_t vec_length;
    bcs_read_vec_length(&reader, &vec_length);
    assert(vec_length == num_values);

    for (size_t i = 0; i < vec_length; i++) {
        uint32_t value;
        bcs_read_u32(&reader, &value);
        assert(value == values[i]);
    }

    bcs_writer_free(&writer);
    TEST_PASSED();
}

void test_option() {
    bcs_writer_t writer;
    bcs_writer_init(&writer, 32, 0);

    // Write Some(42)
    bcs_write_option_some(&writer);
    bcs_write_u32(&writer, 42);

    // Write None
    bcs_write_option_none(&writer);

    size_t length;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &length);

    bcs_reader_t reader;
    bcs_reader_init(&reader, bytes, length);

    // Read Some(42)
    bool has_value;
    bcs_read_option_tag(&reader, &has_value);
    assert(has_value == true);

    uint32_t value;
    bcs_read_u32(&reader, &value);
    assert(value == 42);

    // Read None
    bcs_read_option_tag(&reader, &has_value);
    assert(has_value == false);

    bcs_writer_free(&writer);
    TEST_PASSED();
}

void test_hex_conversion() {
    const char *hex_input = "0x48656c6c6f";
    uint8_t bytes[32];
    size_t actual_bytes;

    bcs_error_t err = bcs_hex_to_bytes(hex_input, bytes, sizeof(bytes), &actual_bytes);
    assert(err == BCS_OK);
    assert(actual_bytes == 5);

    // Should spell "Hello"
    assert(bytes[0] == 'H');
    assert(bytes[1] == 'e');
    assert(bytes[2] == 'l');
    assert(bytes[3] == 'l');
    assert(bytes[4] == 'o');

    // Convert back to hex
    char hex_output[11];
    bcs_bytes_to_hex(bytes, actual_bytes, hex_output);
    assert(strcmp(hex_output, "48656c6c6f") == 0);

    TEST_PASSED();
}

void test_buffer_growth() {
    bcs_writer_t writer;
    bcs_writer_init(&writer, 8, 0); // Small initial size

    // Write more than initial capacity
    for (int i = 0; i < 100; i++) {
        bcs_error_t err = bcs_write_u32(&writer, i);
        assert(err == BCS_OK);
    }

    size_t length;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &length);
    assert(length == 400); // 100 * 4 bytes

    bcs_writer_free(&writer);
    TEST_PASSED();
}

void test_bool() {
    bcs_writer_t writer;
    bcs_writer_init(&writer, 16, 0);

    bcs_write_bool(&writer, true);
    bcs_write_bool(&writer, false);

    size_t length;
    const uint8_t *bytes = bcs_writer_get_bytes(&writer, &length);

    bcs_reader_t reader;
    bcs_reader_init(&reader, bytes, length);

    bool val1, val2;
    bcs_read_bool(&reader, &val1);
    bcs_read_bool(&reader, &val2);

    assert(val1 == true);
    assert(val2 == false);

    bcs_writer_free(&writer);
    TEST_PASSED();
}

int main() {
    printf("Running BCS Tests\n");
    printf("=================\n\n");

    test_write_read_u8();
    test_write_read_u64();
    test_uleb128();
    test_string();
    test_vector();
    test_option();
    test_hex_conversion();
    test_buffer_growth();
    test_bool();

    printf("\n✓ All tests passed!\n");

    return 0;
}
