import { Transaction } from "@mysten/sui/transactions";
import { Ed25519Keypair } from "@mysten/sui/keypairs/ed25519";
import { SuiClient } from "@mysten/sui/client";
import { SUI_CLOCK_OBJECT_ID } from "@mysten/sui/utils";
import dotenv from "dotenv";

// Load environment variables
dotenv.config();

export async function createRecordReadingTransaction(
  packageId: string,
  client: SuiClient,
  sensorId: string,
  senderAddress: string,
  sensorData: {
    value1: number; // Update these field names to match your actual sensor data
    value2: number;
    value3: number;
    value4: number;
    timestamp: number;
  }
): Promise<{ transactionKindBytes: string; sender: string }> {
  try {
    // Create the transaction
    const tx = new Transaction();

    // Set sender (the sensor/microcontroller address)
    tx.setSender(senderAddress);

    tx.moveCall({
      target: `${packageId}::sensor::record_reading`,
      arguments: [
        tx.object(sensorId),
        tx.pure.u16(sensorData.value1),
        tx.pure.u16(sensorData.value2),
        tx.pure.u16(sensorData.value3),
        tx.pure.u16(sensorData.value4),
        tx.pure.u64(sensorData.timestamp),
      ],
    });

    // Serialize the transaction kind
    const transactionKindBytes = Buffer.from(
      await tx.build({
        client: client,
        onlyTransactionKind: true,
      })
    ).toString("hex");

    return {
      transactionKindBytes,
      sender: senderAddress,
    };
  } catch (error) {
    throw new Error(
      `Failed to create transaction: ${
        error instanceof Error ? error.message : "Unknown error"
      }`
    );
  }
}

// Example usage
async function main() {
  const packageId = process.env.PACKAGE_ID;
  const sensorId = process.env.SENSOR_ID;
  const sender = process.env.SENDER;

  if (!packageId || !sensorId || !sender) {
    throw new Error(
      "PACKAGE_ID, SENSOR_ID, and SENDER must be set in .env file"
    );
  }

  const result = await createRecordReadingTransaction(
    packageId,
    new SuiClient({
      url: "https://fullnode.testnet.sui.io:443",
    }),
    sensorId,
    sender,
    { value1: 1111, value2: 2222, value3: 3333, value4: 4444, timestamp: 1234567890 }
  );

  console.log("Transaction created successfully:");
  console.log(JSON.stringify(result, null, 2));
}

main().catch(console.error);
