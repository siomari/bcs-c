/**
 * Example: Build complete Sui transaction from scratch in C
 *
 * This example shows how to build a full TransactionData structure
 * from sensor data, object IDs, and gas payment information.
 *
 * Usage:
 * 1. Provide sensor data
 * 2. Provide sensor object ID and initial shared version
 * 3. Provide gas coin data (object ID, version, digest)
 * 4. Build complete transaction ready for signing
 */

#include "../src/sui_transaction.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    printf("=== Building Sui Transaction from Scratch ===\n\n");

    // Initialize transaction builder parameters
    transaction_builder_t params = {0};

    // Package ID: 0xb045c6512cef5ad7d20377ec3fe79488b97697a72041b87bb8d7d8c1887de698
    size_t bytes_read;
    bcs_hex_to_bytes(
        "b045c6512cef5ad7d20377ec3fe79488b97697a72041b87bb8d7d8c1887de698",
        params.package_id, 32, &bytes_read
    );

    // Module and function
    params.module_name = "sensor";
    params.function_name = "record_reading";

    // Sensor object ID: 0xe3b83e0616d2c47961ca802a5169a94896e65063252ae9f2a3603afea35b42c3
    bcs_hex_to_bytes(
        "e3b83e0616d2c47961ca802a5169a94896e65063252ae9f2a3603afea35b42c3",
        params.sensor_object_id, 32, &bytes_read
    );
    params.sensor_initial_shared_version = 372865614;  // From blockchain
    params.sensor_mutable = true;

    // Sensor data
    params.sensor_data.value1 = 2350;       // 23.50°C * 100
    params.sensor_data.value2 = 6540;       // 65.40% * 100
    params.sensor_data.value3 = 10132;      // 1013.2 hPa * 10
    params.sensor_data.value4 = 850;        // 8.50V * 100
    params.sensor_data.timestamp = 1730822400;

    printf("Sensor readings:\n");
    printf("  value1: %u\n", params.sensor_data.value1);
    printf("  value2: %u\n", params.sensor_data.value2);
    printf("  value3: %u\n", params.sensor_data.value3);
    printf("  value4: %u\n", params.sensor_data.value4);
    printf("  timestamp: %llu\n\n", (unsigned long long)params.sensor_data.timestamp);

    // Sender address: 0x6c389948ab60dc14b7c18e0e4e3dd08bf2ced62f7a34c1f7ceaeb24d967db7c7
    bcs_hex_to_bytes(
        "6c389948ab60dc14b7c18e0e4e3dd08bf2ced62f7a34c1f7ceaeb24d967db7c7",
        params.sender, 32, &bytes_read
    );

    // Gas object: 0x052c8d054356c92a3b62cb4dd75339c8f465eac608393cd1f7180cd2ea0c1a4a
    bcs_hex_to_bytes(
        "052c8d054356c92a3b62cb4dd75339c8f465eac608393cd1f7180cd2ea0c1a4a",
        params.gas_object.object_id, 32, &bytes_read
    );
    params.gas_object.version = 372865621;

    // Gas object digest: 0xca72a304d601a8adcbd514507d1b7050d2ff8770933a4ccdc01ee5a78f33ca98
    bcs_hex_to_bytes(
        "ca72a304d601a8adcbd514507d1b7050d2ff8770933a4ccdc01ee5a78f33ca98",
        params.gas_object.digest, 32, &bytes_read
    );

    // Gas budget and price
    params.gas_budget = 100000000;  // 0.1 SUI
    params.gas_price = 1000;

    // Build transaction
    char *tx_hex;
    size_t tx_len;

    printf("Building transaction...\n");
    bcs_error_t err = sui_build_sensor_transaction(&params, &tx_hex, &tx_len);

    if (err != BCS_OK) {
        printf("Error building transaction: %d\n", err);
        return 1;
    }

    printf("Transaction built: %zu bytes\n\n", tx_len);
    printf("=== TRANSACTION BYTES ===\n%s\n\n", tx_hex);
    printf("✓ Done! This transaction is ready for signing.\n");
    printf("  Next step: Sign with Ed25519 key and submit to Sui.\n");

    // Cleanup
    free(tx_hex);

    return 0;
}
