import { createRecordReadingTransaction } from "./create_transaction";
import { SuiClient } from "@mysten/sui/client";
import dotenv from "dotenv";

dotenv.config();

async function main() {
  const packageId = process.env.PACKAGE_ID!;
  const sensorId = process.env.SENSOR_ID!;
  const sender = process.env.SENDER!;

  // EXACT SAME VALUES as build_transaction.c
  const sensorData = {
    value1: 2350,
    value2: 6540,
    value3: 10132,
    value4: 850,
    timestamp: 1730822400,
  };

  const result = await createRecordReadingTransaction(
    packageId,
    new SuiClient({ url: "https://fullnode.testnet.sui.io:443" }),
    sensorId,
    sender,
    sensorData
  );

  console.log("Transaction created with MATCHING sensor values:");
  console.log("Length:", result.transactionBytes.length / 2, "bytes");
  console.log("Hex:", result.transactionBytes);
}

main().catch(console.error);
