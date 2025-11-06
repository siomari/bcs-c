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
): Promise<{ transactionBytes: string; sender: string; gasCoin: string }> {
  try {
    // Get gas coins for the sender
    const coins = await client.getCoins({
      owner: senderAddress,
      coinType: "0x2::sui::SUI",
    });

    if (coins.data.length === 0) {
      throw new Error("No gas coins available for sender");
    }

    // Use the first coin with sufficient balance
    const gasCoin = coins.data[0];

    // Create the transaction
    const tx = new Transaction();

    // Set sender (the sensor/microcontroller address)
    tx.setSender(senderAddress);

    // Set the specific gas coin to use
    tx.setGasPayment([
      {
        objectId: gasCoin.coinObjectId,
        version: gasCoin.version,
        digest: gasCoin.digest,
      },
    ]);

    // Set gas budget
    tx.setGasBudget(100000000); // 0.1 SUI

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

    // Build the full transaction with gas coin
    const transactionBytes = Buffer.from(
      await tx.build({
        client: client,
      })
    ).toString("hex");

    return {
      transactionBytes,
      sender: senderAddress,
      gasCoin: gasCoin.coinObjectId,
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
