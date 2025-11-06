/**
 * Sui Transaction Helpers - Implementation
 */

#include "sui_transaction.h"
#include <stdlib.h>
#include <string.h>

bcs_error_t sui_build_sensor_transaction(
    const transaction_builder_t *params,
    char **output_hex,
    size_t *output_length
) {
    if (!params || !output_hex || !output_length) {
        return BCS_ERROR_INVALID_INPUT;
    }

    bcs_writer_t writer;
    bcs_writer_init(&writer, 512, 0);
    bcs_error_t err = BCS_OK;

    // ========== TransactionData V1 ==========
    bcs_write_u8(&writer, 0x00);  // Version: V1

    // ========== TransactionKind: ProgrammableTransaction ==========
    bcs_write_u8(&writer, 0x00);  // Kind: ProgrammableTransaction

    // ========== Inputs (6 total: 1 Object + 5 Pure values) ==========
    bcs_write_uleb128(&writer, 6);

    // Input 0: Object - SharedObject (sensor)
    bcs_write_u8(&writer, 0x01);  // CallArg::Object
    bcs_write_u8(&writer, 0x01);  // ObjectArg::SharedObject variant
    bcs_write_fixed_bytes(&writer, params->sensor_object_id, 32);
    bcs_write_u64(&writer, params->sensor_initial_shared_version);
    bcs_write_u8(&writer, params->sensor_mutable ? 0x01 : 0x00);

    // Input 1: Pure - value1 (u16)
    bcs_write_u8(&writer, 0x00);  // CallArg::Pure
    bcs_write_uleb128(&writer, 2);
    bcs_write_u16(&writer, params->sensor_data.value1);

    // Input 2: Pure - value2 (u16)
    bcs_write_u8(&writer, 0x00);  // CallArg::Pure
    bcs_write_uleb128(&writer, 2);
    bcs_write_u16(&writer, params->sensor_data.value2);

    // Input 3: Pure - value3 (u16)
    bcs_write_u8(&writer, 0x00);  // CallArg::Pure
    bcs_write_uleb128(&writer, 2);
    bcs_write_u16(&writer, params->sensor_data.value3);

    // Input 4: Pure - value4 (u16)
    bcs_write_u8(&writer, 0x00);  // CallArg::Pure
    bcs_write_uleb128(&writer, 2);
    bcs_write_u16(&writer, params->sensor_data.value4);

    // Input 5: Pure - timestamp (u64)
    bcs_write_u8(&writer, 0x00);  // CallArg::Pure
    bcs_write_uleb128(&writer, 8);
    bcs_write_u64(&writer, params->sensor_data.timestamp);

    // ========== Commands (1 MoveCall) ==========
    bcs_write_uleb128(&writer, 1);  // 1 command

    // Command: MoveCall
    bcs_write_u8(&writer, 0x00);  // Command::MoveCall

    // Package ID
    bcs_write_fixed_bytes(&writer, params->package_id, 32);

    // Module name
    bcs_write_string(&writer, params->module_name);

    // Function name
    bcs_write_string(&writer, params->function_name);

    // Type arguments (empty)
    bcs_write_uleb128(&writer, 0);

    // Arguments (6 inputs: indices 0, 1, 2, 3, 4, 5)
    bcs_write_uleb128(&writer, 6);
    bcs_write_u8(&writer, 0x01);  // Argument::Input
    bcs_write_u16(&writer, 0);  // Input index 0 (u16, not ULEB128!)
    bcs_write_u8(&writer, 0x01);  // Argument::Input
    bcs_write_u16(&writer, 1);  // Input index 1
    bcs_write_u8(&writer, 0x01);  // Argument::Input
    bcs_write_u16(&writer, 2);  // Input index 2
    bcs_write_u8(&writer, 0x01);  // Argument::Input
    bcs_write_u16(&writer, 3);  // Input index 3
    bcs_write_u8(&writer, 0x01);  // Argument::Input
    bcs_write_u16(&writer, 4);  // Input index 4
    bcs_write_u8(&writer, 0x01);  // Argument::Input
    bcs_write_u16(&writer, 5);  // Input index 5

    // ========== Sender ==========
    bcs_write_fixed_bytes(&writer, params->sender, 32);

    // ========== Gas Payment (vector of ObjectRef) ==========
    bcs_write_uleb128(&writer, 1);  // 1 gas coin
    bcs_write_fixed_bytes(&writer, params->gas_object.object_id, 32);
    bcs_write_u64(&writer, params->gas_object.version);
    bcs_write_u8(&writer, 32);  // Digest length byte (0x20)
    bcs_write_fixed_bytes(&writer, params->gas_object.digest, 32);

    // ========== Gas Owner (same as sender) ==========
    bcs_write_fixed_bytes(&writer, params->sender, 32);

    // ========== Gas Price (comes BEFORE gas budget!) ==========
    bcs_write_u64(&writer, params->gas_price);

    // ========== Gas Budget ==========
    bcs_write_u64(&writer, params->gas_budget);

    // ========== Get result ==========
    size_t result_length;
    const uint8_t *result_bytes = bcs_writer_get_bytes(&writer, &result_length);

    // Convert to hex
    *output_hex = malloc(result_length * 2 + 1);
    if (!*output_hex) {
        err = BCS_ERROR_OUT_OF_MEMORY;
        goto cleanup;
    }

    bcs_bytes_to_hex(result_bytes, result_length, *output_hex);
    *output_length = result_length;

cleanup:
    bcs_writer_free(&writer);
    return err;
}

bcs_error_t sui_modify_transaction_with_pure_values(
    const char *hex_tx,
    const uint8_t **pure_values,
    const size_t *pure_lengths,
    size_t num_pures,
    char **output_hex,
    size_t *output_length
) {
    if (!hex_tx || !pure_values || !pure_lengths || !output_hex || !output_length) {
        return BCS_ERROR_INVALID_INPUT;
    }

    // Convert hex to bytes
    size_t max_length = strlen(hex_tx) / 2;
    uint8_t *tx_bytes = malloc(max_length);
    if (!tx_bytes) {
        return BCS_ERROR_OUT_OF_MEMORY;
    }

    size_t tx_length;
    bcs_error_t err = bcs_hex_to_bytes(hex_tx, tx_bytes, max_length, &tx_length);
    if (err != BCS_OK) {
        free(tx_bytes);
        return err;
    }

    // Parse transaction
    bcs_reader_t reader;
    bcs_reader_init(&reader, tx_bytes, tx_length);

    // Read TransactionData version byte
    uint8_t version;
    err = bcs_read_u8(&reader, &version);
    if (err != BCS_OK) {
        free(tx_bytes);
        return err;
    }

    // Read TransactionKind type
    uint8_t kind;
    err = bcs_read_u8(&reader, &kind);
    if (err != BCS_OK) {
        free(tx_bytes);
        return err;
    }

    uint64_t num_inputs;
    err = bcs_read_uleb128(&reader, &num_inputs);
    if (err != BCS_OK) {
        free(tx_bytes);
        return err;
    }

    // Rebuild transaction
    bcs_writer_t writer;
    bcs_writer_init(&writer, 512, 0);

    // Write TransactionData version byte
    bcs_write_u8(&writer, version);

    // Write TransactionKind type
    bcs_write_u8(&writer, kind);
    bcs_write_uleb128(&writer, num_inputs);

    // Process inputs
    size_t pure_idx = 0;
    for (uint64_t i = 0; i < num_inputs; i++) {
        uint8_t input_type;
        err = bcs_read_u8(&reader, &input_type);
        if (err != BCS_OK) goto cleanup;
        bcs_write_u8(&writer, input_type);

        if (input_type == 0) { // Pure - replace with new value
            // Read old value (discard)
            uint64_t old_len;
            err = bcs_read_uleb128(&reader, &old_len);
            if (err != BCS_OK) goto cleanup;

            uint8_t *old_data = malloc((size_t)old_len);
            if (!old_data) {
                err = BCS_ERROR_OUT_OF_MEMORY;
                goto cleanup;
            }
            err = bcs_read_bytes(&reader, old_data, (size_t)old_len);
            free(old_data);
            if (err != BCS_OK) goto cleanup;

            // Write new value
            if (pure_idx < num_pures) {
                bcs_write_uleb128(&writer, pure_lengths[pure_idx]);
                bcs_write_fixed_bytes(&writer, pure_values[pure_idx], pure_lengths[pure_idx]);
            } else {
                // Not enough pure values provided - keep old value
                bcs_write_uleb128(&writer, old_len);
                bcs_write_fixed_bytes(&writer, old_data, (size_t)old_len);
            }
            pure_idx++;

        } else if (input_type == 1) { // Object - copy unchanged
            uint8_t variant;
            err = bcs_read_u8(&reader, &variant);
            if (err != BCS_OK) goto cleanup;
            bcs_write_u8(&writer, variant);

            uint8_t obj_id[32];
            err = bcs_read_bytes(&reader, obj_id, 32);
            if (err != BCS_OK) goto cleanup;
            bcs_write_fixed_bytes(&writer, obj_id, 32);

            if (variant == 0) { // ImmOrOwnedObject
                uint64_t version;
                err = bcs_read_u64(&reader, &version);
                if (err != BCS_OK) goto cleanup;
                bcs_write_u64(&writer, version);

                uint8_t digest[32];
                err = bcs_read_bytes(&reader, digest, 32);
                if (err != BCS_OK) goto cleanup;
                bcs_write_fixed_bytes(&writer, digest, 32);

            } else if (variant == 1) { // SharedObject
                uint64_t initial_shared_version;
                err = bcs_read_u64(&reader, &initial_shared_version);
                if (err != BCS_OK) goto cleanup;
                bcs_write_u64(&writer, initial_shared_version);

                uint8_t mutable;
                err = bcs_read_u8(&reader, &mutable);
                if (err != BCS_OK) goto cleanup;
                bcs_write_u8(&writer, mutable);

            } else if (variant == 2) { // Receiving
                uint64_t version;
                err = bcs_read_u64(&reader, &version);
                if (err != BCS_OK) goto cleanup;
                bcs_write_u64(&writer, version);

                uint8_t digest[32];
                err = bcs_read_bytes(&reader, digest, 32);
                if (err != BCS_OK) goto cleanup;
                bcs_write_fixed_bytes(&writer, digest, 32);
            }
        }
    }

    // Copy the rest of the transaction (commands, sender, gas payment, gas budget/price)
    size_t remaining = bcs_reader_remaining(&reader);
    if (remaining > 0) {
        uint8_t *rest = malloc(remaining);
        if (!rest) {
            err = BCS_ERROR_OUT_OF_MEMORY;
            goto cleanup;
        }
        err = bcs_read_bytes(&reader, rest, remaining);
        if (err != BCS_OK) {
            free(rest);
            goto cleanup;
        }
        bcs_write_fixed_bytes(&writer, rest, remaining);
        free(rest);
    }

    // Get result
    size_t result_length;
    const uint8_t *result_bytes = bcs_writer_get_bytes(&writer, &result_length);

    // Convert to hex
    *output_hex = malloc(result_length * 2 + 1);
    if (!*output_hex) {
        err = BCS_ERROR_OUT_OF_MEMORY;
        goto cleanup;
    }

    bcs_bytes_to_hex(result_bytes, result_length, *output_hex);
    *output_length = result_length;

    err = BCS_OK;

cleanup:
    free(tx_bytes);
    bcs_writer_free(&writer);
    return err;
}

bcs_error_t sui_modify_transaction_with_sensor_data(
    const char *hex_tx,
    const sensor_data_t *sensor_data,
    char **output_hex,
    size_t *output_length
) {
    if (!sensor_data) {
        return BCS_ERROR_INVALID_INPUT;
    }

    // Serialize sensor values to bytes
    uint8_t value1_bytes[2];
    value1_bytes[0] = sensor_data->value1 & 0xFF;
    value1_bytes[1] = (sensor_data->value1 >> 8) & 0xFF;

    uint8_t value2_bytes[2];
    value2_bytes[0] = sensor_data->value2 & 0xFF;
    value2_bytes[1] = (sensor_data->value2 >> 8) & 0xFF;

    uint8_t value3_bytes[2];
    value3_bytes[0] = sensor_data->value3 & 0xFF;
    value3_bytes[1] = (sensor_data->value3 >> 8) & 0xFF;

    uint8_t value4_bytes[2];
    value4_bytes[0] = sensor_data->value4 & 0xFF;
    value4_bytes[1] = (sensor_data->value4 >> 8) & 0xFF;

    uint8_t timestamp_bytes[8];
    for (int i = 0; i < 8; i++) {
        timestamp_bytes[i] = (sensor_data->timestamp >> (i * 8)) & 0xFF;
    }

    // Create arrays for the low-level function
    const uint8_t *pure_values[] = {
        value1_bytes,
        value2_bytes,
        value3_bytes,
        value4_bytes,
        timestamp_bytes
    };

    const size_t pure_lengths[] = { 2, 2, 2, 2, 8 };

    return sui_modify_transaction_with_pure_values(
        hex_tx,
        pure_values,
        pure_lengths,
        5,
        output_hex,
        output_length
    );
}
