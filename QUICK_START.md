# BCS-C Quick Start Guide

## Installation (One-time)

```bash
make
sudo make install
```

## Use in Your Project (3 Ways)

### 1️⃣ With pkg-config (Easiest)

```bash
gcc my_sensor.c $(pkg-config --cflags --libs bcs-c) -o my_sensor
```

### 2️⃣ Manual flags

```bash
gcc my_sensor.c -I/usr/local/include -L/usr/local/lib -lbcs -o my_sensor
```

### 3️⃣ Copy library

```bash
cp libbcs.a src/bcs.h src/sui_transaction.h /your/project/
gcc my_sensor.c libbcs.a -o my_sensor
```

## Minimal Code Example

```c
#include <bcs.h>
#include <sui_transaction.h>

int main() {
    transaction_builder_t params = {0};
    size_t bytes_read;
    
    // Your package and module
    bcs_hex_to_bytes("YOUR_PACKAGE_ID_HEX", params.package_id, 32, &bytes_read);
    params.module_name = "sensor";
    params.function_name = "record_reading";
    
    // Sensor object
    bcs_hex_to_bytes("YOUR_SENSOR_OBJECT_ID", params.sensor_object_id, 32, &bytes_read);
    params.sensor_initial_shared_version = 123456789;  // From blockchain
    params.sensor_mutable = true;
    
    // Your sensor data
    params.sensor_data.value1 = 2500;  // 25.00°C
    params.sensor_data.value2 = 6500;  // 65.00%
    params.sensor_data.value3 = 10130; // 1013.0 hPa
    params.sensor_data.value4 = 850;   // 8.50V
    params.sensor_data.timestamp = time(NULL);
    
    // Your wallet
    bcs_hex_to_bytes("YOUR_WALLET_ADDRESS", params.sender, 32, &bytes_read);
    
    // Gas object (query fresh from blockchain!)
    bcs_hex_to_bytes("YOUR_GAS_COIN_ID", params.gas_object.object_id, 32, &bytes_read);
    params.gas_object.version = 987654321;  // From blockchain
    bcs_hex_to_bytes("YOUR_GAS_DIGEST", params.gas_object.digest, 32, &bytes_read);
    
    params.gas_budget = 100000000;  // 0.1 SUI
    params.gas_price = 1000;
    
    // Build!
    char *tx_hex;
    size_t tx_len;
    if (sui_build_sensor_transaction(&params, &tx_hex, &tx_len) == BCS_OK) {
        printf("Transaction: %s\n", tx_hex);
        // Now sign with Ed25519 and submit!
        free(tx_hex);
    }
    
    return 0;
}
```

## 📖 Full Documentation

- **USAGE.md** - Complete integration guide
- **demo-project/** - Working example
- **INTEGRATION_SUMMARY.md** - Feature overview
- **SIGNING_AND_EXECUTION.md** - How to sign and submit

## ✨ Features

✅ 302-byte transactions built in pure C  
✅ Works on IoT devices (ESP32, Arduino, etc.)  
✅ Tested on Sui blockchain  
✅ Zero JavaScript dependencies  
✅ pkg-config support  
✅ Cross-platform (Linux, macOS, Windows, embedded)

## 🎯 That's It!

You're ready to build Sui transactions from C code!
