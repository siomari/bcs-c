/**
 * Compact Sensor Payload Format
 * Minimal data structure for IoT devices to send sensor readings
 */

#ifndef SENSOR_PAYLOAD_H
#define SENSOR_PAYLOAD_H

#include "bcs.h"
#include <stdint.h>
#include <stddef.h>

/**
 * Compact sensor payload structure
 * Total: 14 bytes (just the sensor values)
 */
typedef struct {
    uint16_t value1;      // 2 bytes
    uint16_t value2;      // 2 bytes
    uint16_t value3;      // 2 bytes
    uint16_t value4;      // 2 bytes
    uint64_t timestamp;   // 8 bytes
} __attribute__((packed)) sensor_payload_t;

/**
 * Serialize sensor payload to bytes
 * Output: 14 bytes of binary data (little-endian)
 *
 * @param sensor_data    Sensor readings
 * @param output         Output buffer (must be at least 14 bytes)
 * @return Number of bytes written (always 14)
 */
size_t sensor_payload_serialize(
    const sensor_payload_t *sensor_data,
    uint8_t *output
);

/**
 * Deserialize sensor payload from bytes
 *
 * @param input          Input buffer (14 bytes)
 * @param sensor_data    Output: deserialized sensor readings
 * @return BCS_OK on success, error code otherwise
 */
bcs_error_t sensor_payload_deserialize(
    const uint8_t *input,
    sensor_payload_t *sensor_data
);

/**
 * Convert sensor payload to hex string
 *
 * @param sensor_data    Sensor readings
 * @param output_hex     Output buffer (must be at least 29 bytes for 14*2+1)
 */
void sensor_payload_to_hex(
    const sensor_payload_t *sensor_data,
    char *output_hex
);

#endif // SENSOR_PAYLOAD_H
