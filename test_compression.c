#include <stdio.h>
#include <string.h>
#include <zlib.h>

int main() {
    // Full transaction hex from our example
    const char *hex = "0000060101e3b83e0616d2c47961ca802a5169a94896e65063252ae9f2a3603afea35b42c34e7a39160000000100022e0900028c190002942700025203000800412a67000000000100b045c6512cef5ad7d20377ec3fe79488b97697a72041b87bb8d7d8c1887de6980673656e736f720e7265636f72645f72656164696e6700060100000101000102000103000104000105006c389948ab60dc14b7c18e0e4e3dd08bf2ced62f7a34c1f7ceaeb24d967db7c701052c8d054356c92a3b62cb4dd75339c8f465eac608393cd1f7180cd2ea0c1a4a587a391600000000206f3a7c4925d994bf8cab59fb842e4659f492f0ecf2ae5d39c24cdfa6d29336cb6c389948ab60dc14b7c18e0e4e3dd08bf2ced62f7a34c1f7ceaeb24d967db7c7e80300000000000000e1f5050000000000";
    
    size_t hex_len = strlen(hex);
    size_t data_len = hex_len / 2;
    
    // Convert hex to bytes
    unsigned char data[512];
    for (size_t i = 0; i < data_len; i++) {
        sscanf(hex + 2*i, "%2hhx", &data[i]);
    }
    
    // Compress
    unsigned char compressed[512];
    uLongf compressed_len = sizeof(compressed);
    
    int result = compress2(compressed, &compressed_len, data, data_len, 9); // Max compression
    
    if (result == Z_OK) {
        printf("Original size: %zu bytes\n", data_len);
        printf("Compressed size: %lu bytes\n", compressed_len);
        printf("Reduction: %.1f%%\n", (1.0 - (double)compressed_len / data_len) * 100);
        printf("\n✓ Fits in 200 bytes: %s\n", compressed_len <= 200 ? "YES" : "NO");
    } else {
        printf("Compression failed\n");
    }
    
    return 0;
}
