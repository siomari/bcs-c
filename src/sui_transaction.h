/**
 * Sui Transaction Helpers
 * High-level functions for modifying Sui transactions with sensor data
 */

#ifndef SUI_TRANSACTION_H
#define SUI_TRANSACTION_H

#include "bcs.h"
#include <stdint.h>
#include <stddef.h>

/**
 * Sensor data structure
 * Adjust fields to match your Move contract
 */
typedef struct {
    uint16_t value1;      // e.g., temperature * 100
    uint16_t value2;      // e.g., humidity * 100
    uint16_t value3;      // e.g., pressure * 10
    uint16_t value4;      // e.g., voltage * 100
    uint64_t timestamp;   // Unix timestamp
} sensor_data_t;

/**
 * Modify a Sui transaction with sensor data
 *
 * Takes a transaction template from TypeScript and replaces Pure input values
 * with actual sensor readings. Preserves Object references and commands.
 *
 * @param hex_tx         Input transaction in hex format (from TypeScript)
 * @param sensor_data    Sensor readings to inject
 * @param output_hex     Output: modified transaction hex (caller must free)
 * @param output_length  Output: length of modified transaction
 * @return BCS_OK on success, error code otherwise
 *
 * Example:
 *   sensor_data_t data = { 2350, 6540, 10132, 850, 1730822400 };
 *   char *modified_tx;
 *   size_t modified_len;
 *
 *   bcs_error_t err = sui_modify_transaction_with_sensor_data(
 *       template_hex, &data, &modified_tx, &modified_len);
 *
 *   if (err == BCS_OK) {
 *       send_to_backend(modified_tx);
 *       free(modified_tx);
 *   }
 */
bcs_error_t sui_modify_transaction_with_sensor_data(
    const char *hex_tx,
    const sensor_data_t *sensor_data,
    char **output_hex,
    size_t *output_length
);

/**
 * Lower-level function: Modify transaction with custom Pure values
 *
 * For more control, you can provide an array of Pure values to inject.
 *
 * @param hex_tx         Input transaction hex
 * @param pure_values    Array of byte arrays for Pure values
 * @param pure_lengths   Array of lengths for each Pure value
 * @param num_pures      Number of Pure values
 * @param output_hex     Output: modified transaction hex (caller must free)
 * @param output_length  Output: length of modified transaction
 * @return BCS_OK on success, error code otherwise
 */
bcs_error_t sui_modify_transaction_with_pure_values(
    const char *hex_tx,
    const uint8_t **pure_values,
    const size_t *pure_lengths,
    size_t num_pures,
    char **output_hex,
    size_t *output_length
);

#endif // SUI_TRANSACTION_H
