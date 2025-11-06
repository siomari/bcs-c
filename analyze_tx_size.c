#include <stdio.h>

int main() {
    printf("=== Transaction Size Breakdown ===\n\n");
    
    printf("TransactionData structure:\n");
    printf("  Version byte: 1 byte\n");
    printf("  TransactionKind type: 1 byte\n\n");
    
    printf("Inputs (6 total):\n");
    printf("  - 1 Object (SharedObject):\n");
    printf("    - Type: 1 byte\n");
    printf("    - Variant: 1 byte\n");
    printf("    - ObjectID: 32 bytes\n");
    printf("    - InitialSharedVersion: ~2 bytes (ULEB128)\n");
    printf("    - Mutable flag: 1 byte\n");
    printf("    Subtotal: ~37 bytes\n\n");
    
    printf("  - 5 Pure values:\n");
    printf("    - Each u16 (value1-4): 1 (type) + 1 (len) + 2 (data) = 4 bytes × 4 = 16 bytes\n");
    printf("    - One u64 (timestamp): 1 (type) + 1 (len) + 8 (data) = 10 bytes\n");
    printf("    Subtotal: 26 bytes\n\n");
    
    printf("Commands (1 MoveCall):\n");
    printf("  - Command type: 1 byte\n");
    printf("  - Package ID: 32 bytes\n");
    printf("  - Module name ('sensor'): 1 (len) + 6 (chars) = 7 bytes\n");
    printf("  - Function name ('record_reading'): 1 (len) + 14 (chars) = 15 bytes\n");
    printf("  - Type args: 1 byte (empty vec)\n");
    printf("  - Arguments: 1 (vec len) + 6 (arg indices) = 7 bytes\n");
    printf("  Subtotal: ~63 bytes\n\n");
    
    printf("Gas Payment & Metadata:\n");
    printf("  - Sender address: 32 bytes\n");
    printf("  - Gas payment vector:\n");
    printf("    - Vec length: 1 byte\n");
    printf("    - ObjectID: 32 bytes\n");
    printf("    - Version: ~2 bytes (ULEB128)\n");
    printf("    - Digest: 32 bytes\n");
    printf("    Subtotal: 67 bytes\n");
    printf("  - Gas budget: ~4 bytes (ULEB128)\n");
    printf("  - Gas price: ~4 bytes (ULEB128)\n");
    printf("  Subtotal: ~75 bytes\n\n");
    
    int total = 2 + 37 + 26 + 63 + 75;
    printf("TOTAL: ~%d bytes\n\n", total);
    
    printf("With signature for transmission:\n");
    printf("  Transaction: 303 bytes\n");
    printf("  Signature: 64 bytes\n");
    printf("  Public key: 32 bytes\n");
    printf("  Scheme: 1 byte\n");
    printf("  TOTAL: 400 bytes\n\n");
    
    printf("After compression: 277 bytes (still > 200)\n\n");
    
    printf("=== Options to reach <200 bytes ===\n\n");
    printf("1. Backend signing (recommended for IoT):\n");
    printf("   - Send only sensor data: 14 bytes ✓\n");
    printf("   - Backend builds & signs transaction\n\n");
    
    printf("2. Send only unsigned transaction + sign on backend:\n");
    printf("   - Compressed unsigned tx: ~274 bytes (still > 200)\n\n");
    
    printf("3. Custom protocol:\n");
    printf("   - Send sensor ID index (1 byte) + data (14 bytes) = 15 bytes ✓\n");
    printf("   - Backend has sensor-to-ObjectID mapping\n");
    
    return 0;
}
