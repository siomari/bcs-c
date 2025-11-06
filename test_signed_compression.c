#include <stdio.h>
#include <string.h>
#include <zlib.h>

int main() {
    // Simulate a signed transaction structure
    // Transaction (303 bytes) + Signature (64 bytes) + PubKey (32 bytes) + Scheme (1 byte) = 400 bytes
    
    // Full transaction hex (303 bytes)
    const char *tx_hex = "0000060101e3b83e0616d2c47961ca802a5169a94896e65063252ae9f2a3603afea35b42c34e7a39160000000100022e0900028c190002942700025203000800412a67000000000100b045c6512cef5ad7d20377ec3fe79488b97697a72041b87bb8d7d8c1887de6980673656e736f720e7265636f72645f72656164696e6700060100000101000102000103000104000105006c389948ab60dc14b7c18e0e4e3dd08bf2ced62f7a34c1f7ceaeb24d967db7c701052c8d054356c92a3b62cb4dd75339c8f465eac608393cd1f7180cd2ea0c1a4a587a391600000000206f3a7c4925d994bf8cab59fb842e4659f492f0ecf2ae5d39c24cdfa6d29336cb6c389948ab60dc14b7c18e0e4e3dd08bf2ced62f7a34c1f7ceaeb24d967db7c7e80300000000000000e1f5050000000000";
    
    // Mock signature (64 bytes) + pubkey (32 bytes) + scheme (1 byte)
    unsigned char signed_tx[400];
    size_t tx_len = strlen(tx_hex) / 2;
    
    // Convert tx hex to bytes
    for (size_t i = 0; i < tx_len; i++) {
        sscanf(tx_hex + 2*i, "%2hhx", &signed_tx[i]);
    }
    
    // Add mock signature data (97 bytes)
    memset(signed_tx + tx_len, 0xAB, 97);
    size_t total_len = tx_len + 97;
    
    printf("Signed transaction size: %zu bytes\n\n", total_len);
    
    // Test different compression levels
    for (int level = 1; level <= 9; level++) {
        unsigned char compressed[512];
        uLongf compressed_len = sizeof(compressed);
        
        int result = compress2(compressed, &compressed_len, signed_tx, total_len, level);
        
        if (result == Z_OK) {
            printf("Level %d: %lu bytes (%.1f%% reduction)\n", 
                   level, compressed_len, 
                   (1.0 - (double)compressed_len / total_len) * 100);
        }
    }
    
    // Best case
    unsigned char compressed[512];
    uLongf compressed_len = sizeof(compressed);
    compress2(compressed, &compressed_len, signed_tx, total_len, 9);
    
    printf("\n✓ Best compression: %lu bytes\n", compressed_len);
    printf("✓ Fits in 200 bytes: %s\n", compressed_len <= 200 ? "YES" : "NO");
    
    return 0;
}
