/**
 * Example: Modify sensor values in Sui transaction
 *
 * This example shows how to use the sui_transaction library to modify
 * Pure input values in a FULL transaction (TransactionData with gas payment)
 * created by TypeScript.
 *
 * Usage in your project:
 * 1. Include "sui_transaction.h"
 * 2. Create sensor_data_t with your readings
 * 3. Call sui_modify_transaction_with_sensor_data()
 * 4. Send the result for signing (use Transaction.from() in TypeScript)
 */

#include "../src/sui_transaction.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    printf("=== Modifying Sensor Values in Sui Transaction ===\n\n");

    // Full transaction from TypeScript (includes gas payment, sender, etc.)
    const char *hex_tx = "0000060101e3b83e0616d2c47961ca802a5169a94896e65063252ae9f2a3603afea35b42c34e7a39160000000001000257040002ae080002050d00025c110008d2029649000000000100b045c6512cef5ad7d20377ec3fe79488b97697a72041b87bb8d7d8c1887de6980673656e736f720e7265636f72645f72656164696e6700060100000101000102000103000104000105006c389948ab60dc14b7c18e0e4e3dd08bf2ced62f7a34c1f7ceaeb24d967db7c701052c8d054356c92a3b62cb4dd75339c8f465eac608393cd1f7180cd2ea0c1a4a587a391600000000206f3a7c4925d994bf8cab59fb842e4659f492f0ecf2ae5d39c24cdfa6d29336cb6c389948ab60dc14b7c18e0e4e3dd08bf2ced62f7a34c1f7ceaeb24d967db7c7e80300000000000000e1f5050000000000";

    printf("Original transaction: %zu bytes\n\n", strlen(hex_tx) / 2);

    // ========== SET YOUR SENSOR VALUES HERE ==========
    sensor_data_t sensor_data = {
        .value1 = 2350,      // e.g., 23.50°C * 100
        .value2 = 6540,      // e.g., 65.40% * 100
        .value3 = 10132,     // e.g., 1013.2 hPa * 10
        .value4 = 850,       // e.g., 8.50V * 100
        .timestamp = 1730822400
    };

    printf("Sensor readings:\n");
    printf("  value1: %u\n", sensor_data.value1);
    printf("  value2: %u\n", sensor_data.value2);
    printf("  value3: %u\n", sensor_data.value3);
    printf("  value4: %u\n", sensor_data.value4);
    printf("  timestamp: %llu\n\n", (unsigned long long)sensor_data.timestamp);

    // Modify transaction with sensor data
    char *modified_tx;
    size_t modified_len;

    bcs_error_t err = sui_modify_transaction_with_sensor_data(
        hex_tx,
        &sensor_data,
        &modified_tx,
        &modified_len
    );

    if (err != BCS_OK) {
        printf("Error modifying transaction: %d\n", err);
        return 1;
    }

    printf("Modified transaction: %zu bytes\n\n", modified_len);
    printf("=== NEW TRANSACTION BYTES ===\n%s\n\n", modified_tx);
    printf("✓ Done! Copy hex above for signing.\n");

    // Cleanup
    free(modified_tx);

    return 0;
}
