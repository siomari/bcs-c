# E2E Test - TypeScript + C Integration

This directory contains TypeScript scripts for creating, signing, and submitting Sui transactions that work with the C code.

## Setup

```bash
npm install
# or
pnpm install
```

Create a `.env` file with:
```
PACKAGE_ID=0x...        # Your deployed Move package
SENSOR_ID=0x...         # Your sensor object ID
SENDER=0x...            # Transaction sender address (same as PRIVATE_KEY owner)
PRIVATE_KEY=suiprivkey1...
```

## Scripts

### `create_transaction.ts`
Creates a transaction template with placeholder sensor values.

```bash
npm exec tsx create_transaction.ts
```

**Output:** Transaction kind bytes (hex string)

**What it does:**
- Creates a MoveCall to `sensor::record_reading`
- Uses placeholder values (1111, 2222, 3333, 4444, 1234567890)
- Returns serialized transaction bytes

**Copy the output hex** and paste it into the C examples (e.g., `modify_inline_args.c` line 30).

### `sign_and_submit.ts`
Signs and submits a transaction to Sui testnet.

```bash
npm exec tsx sign_and_submit.ts
```

**What it does:**
- Takes transaction bytes (from C code or TypeScript)
- Rebuilds Transaction from kind bytes
- Signs with private key
- Submits to Sui testnet
- Shows transaction result and explorer link

**Update line 71** with the modified transaction hex from C code output.

## Workflow

### Full End-to-End Flow

1. **Create template** (do once, or when contract changes):
   ```bash
   npm exec tsx create_transaction.ts
   ```
   Copy the hex output.

2. **Update C code** with the template:
   Edit `../examples/modify_inline_args.c` line 30, paste the hex.

3. **Run C code** with real sensor data:
   ```bash
   cd ..
   ./examples/modify_inline_args
   ```
   Copy the output hex.

4. **Sign and submit**:
   Edit `sign_and_submit.ts` line 71, paste the hex from C.
   ```bash
   npm exec tsx sign_and_submit.ts
   ```

5. **Check result** on Sui explorer (link printed in output).

## For Production

In production, automate this flow:

1. **Backend/Server**: Runs `create_transaction.ts` and sends hex to device
2. **IoT Device**: Runs C code to modify with sensor readings, sends back
3. **Backend/Server**: Runs `sign_and_submit.ts` to submit to blockchain

Or sign on the device if you have secure key storage (HSM/secure element).
