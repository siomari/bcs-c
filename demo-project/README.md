# Demo Project: Using BCS-C Library

This is a simple example showing how to use the BCS-C library in your own project.

## Prerequisites

1. **Install BCS-C library** (from parent directory):

```bash
cd ..
make
sudo make install
cd demo-project
```

This installs the library to `/usr/local/lib/libbcs.a` and headers to `/usr/local/include/`.

## Building

### Option 1: Using Makefile (Recommended)

```bash
make
```

The Makefile automatically uses pkg-config if available, or falls back to manual flags.

### Option 2: Manual Compilation

Using pkg-config:
```bash
gcc my_sensor_app.c $(pkg-config --cflags --libs bcs-c) -o my_sensor_app
```

Without pkg-config:
```bash
gcc my_sensor_app.c -I/usr/local/include -L/usr/local/lib -lbcs -o my_sensor_app
```

## Running

```bash
./my_sensor_app
```

Expected output:
```
=== My Sensor Application ===

Reading sensor values...
  Temperature: 23.50°C
  Humidity: 65.40%
  Pressure: 1013.2 hPa
  Battery: 8.50V
  Timestamp: 1730822400

Building Sui transaction...
✓ Transaction built: 302 bytes

Transaction hex:
0000060101e3b83e...

Next steps:
1. Sign this transaction with your Ed25519 private key
2. Submit to Sui blockchain via JSON-RPC
3. See SIGNING_AND_EXECUTION.md for complete workflow
```

## Customizing for Your Project

Edit `my_sensor_app.c` and replace:

1. **Package ID** - Your deployed Move package
2. **Sensor Object ID** - Your sensor object on Sui
3. **Sender Address** - Your wallet address
4. **Gas Object** - Query from blockchain before each transaction
5. **Sensor Data** - Read from actual sensors

## Troubleshooting

### Library not found error

If you get `error while loading shared libraries: libbcs.a: cannot open shared object file`:

```bash
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

Or use static linking (already done in this Makefile).

### Permission denied

If installation failed, make sure you used `sudo`:

```bash
cd ..
sudo make install
```

### Custom installation path

If you installed to a custom prefix:

```bash
# When you installed
cd ..
make install PREFIX=/opt/bcs

# When compiling this demo
export PKG_CONFIG_PATH=/opt/bcs/lib/pkgconfig:$PKG_CONFIG_PATH
make
```

## Next Steps

- See `../USAGE.md` for complete API documentation
- Check `../examples/` for more examples
- Read `../SIGNING_AND_EXECUTION.md` for signing workflow
- Test with `../tests/` to verify installation
