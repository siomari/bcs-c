/**
 * Demo: Using BCS-C library in your own project
 * 
 * This shows how to use the installed BCS-C library
 * to build Sui transactions from your own application.
 */

#include <bcs.h>
#include <sui_transaction.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("=== My Sensor Application ===\n\n");
    
    // Initialize transaction builder
    transaction_builder_t params = {0};
    size_t bytes_read;
    
    // REPLACE THESE WITH YOUR ACTUAL VALUES:
    
    // Package ID (replace with your deployed Move package)
    bcs_hex_to_bytes(
        "b045c6512cef5ad7d20377ec3fe79488b97697a72041b87bb8d7d8c1887de698",
        params.package_id, 32, &bytes_read
    );
    
    params.module_name = "sensor";
    params.function_name = "record_reading";
    
    // Sensor object ID (replace with your sensor object)
    bcs_hex_to_bytes(
        "e3b83e0616d2c47961ca802a5169a94896e65063252ae9f2a3603afea35b42c3",
        params.sensor_object_id, 32, &bytes_read
    );
    params.sensor_initial_shared_version = 372865614;
    params.sensor_mutable = true;
    
    // Read sensor data (replace with actual sensor readings)
    printf("Reading sensor values...\n");
    params.sensor_data.value1 = 2350;    // Temperature: 23.50°C
    params.sensor_data.value2 = 6540;    // Humidity: 65.40%
    params.sensor_data.value3 = 10132;   // Pressure: 1013.2 hPa
    params.sensor_data.value4 = 850;     // Battery: 8.50V
    params.sensor_data.timestamp = 1730822400;
    
    printf("  Temperature: %.2f°C\n", params.sensor_data.value1 / 100.0);
    printf("  Humidity: %.2f%%\n", params.sensor_data.value2 / 100.0);
    printf("  Pressure: %.1f hPa\n", params.sensor_data.value3 / 10.0);
    printf("  Battery: %.2fV\n", params.sensor_data.value4 / 100.0);
    printf("  Timestamp: %llu\n\n", (unsigned long long)params.sensor_data.timestamp);
    
    // Sender address (replace with your wallet address)
    bcs_hex_to_bytes(
        "6c389948ab60dc14b7c18e0e4e3dd08bf2ced62f7a34c1f7ceaeb24d967db7c7",
        params.sender, 32, &bytes_read
    );
    
    // Gas object (query from blockchain before each transaction)
    bcs_hex_to_bytes(
        "052c8d054356c92a3b62cb4dd75339c8f465eac608393cd1f7180cd2ea0c1a4a",
        params.gas_object.object_id, 32, &bytes_read
    );
    params.gas_object.version = 372865625;
    bcs_hex_to_bytes(
        "ca72a304d601a8adcbd514507d1b7050d2ff8770933a4ccdc01ee5a78f33ca98",
        params.gas_object.digest, 32, &bytes_read
    );
    
    params.gas_budget = 100000000;  // 0.1 SUI
    params.gas_price = 1000;
    
    // Build transaction
    printf("Building Sui transaction...\n");
    char *tx_hex;
    size_t tx_len;
    bcs_error_t err = sui_build_sensor_transaction(&params, &tx_hex, &tx_len);
    
    if (err != BCS_OK) {
        fprintf(stderr, "Error building transaction: %d\n", err);
        return 1;
    }
    
    printf("✓ Transaction built: %zu bytes\n\n", tx_len);
    printf("Transaction hex:\n%s\n\n", tx_hex);
    
    printf("Next steps:\n");
    printf("1. Sign this transaction with your Ed25519 private key\n");
    printf("2. Submit to Sui blockchain via JSON-RPC\n");
    printf("3. See SIGNING_AND_EXECUTION.md for complete workflow\n");
    
    // Cleanup
    free(tx_hex);
    
    return 0;
}
