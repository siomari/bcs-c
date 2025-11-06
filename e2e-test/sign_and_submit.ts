import { Ed25519Keypair } from "@mysten/sui/keypairs/ed25519";
import { SuiClient } from "@mysten/sui/client";
import { Transaction } from "@mysten/sui/transactions";
import dotenv from "dotenv";

dotenv.config();

async function signAndSubmit(transactionKindHex: string) {
  const privateKey = process.env.PRIVATE_KEY;

  if (!privateKey) {
    throw new Error("PRIVATE_KEY must be set in .env");
  }

  // Create keypair
  const keypair = Ed25519Keypair.fromSecretKey(privateKey);
  const sender = keypair.toSuiAddress();

  console.log("Sender:", sender);
  console.log("Transaction kind bytes:", transactionKindHex.substring(0, 32) + "...\n");

  // Create client
  const client = new SuiClient({
    url: "https://fullnode.testnet.sui.io:443",
  });

  // Convert hex to bytes
  const transactionKindBytes = Buffer.from(transactionKindHex, "hex");

  console.log("Building transaction from bytes...");

  // Create Transaction from kind bytes
  const tx = Transaction.fromKind(transactionKindBytes);

  // Set sender
  tx.setSender(sender);

  // Set gas budget
  tx.setGasBudget(100000000); // 0.1 SUI

  console.log("Building and signing transaction...");

  // Sign and execute
  const result = await client.signAndExecuteTransaction({
    transaction: tx,
    signer: keypair,
    options: {
      showEffects: true,
      showObjectChanges: true,
    },
  });

  console.log("\n✓ Transaction executed!");
  console.log("Digest:", result.digest);
  console.log("Status:", result.effects?.status?.status);

  if (result.effects?.status?.status === "failure") {
    console.log("\nError:", result.effects?.status?.error);
  } else {
    console.log("\nGas used:", result.effects?.gasUsed);
  }

  console.log(`\n🔗 https://testnet.suivision.xyz/txblock/${result.digest}`);

  return result;
}

// Main - paste your transaction bytes from C code here
async function main() {
  // Modified transaction from C code with new sensor values
  const transactionBytes = "00060101e3b83e0616d2c47961ca802a5169a94896e65063252ae9f2a3603afea35b42c34e7a3916000000000100022e0900028c190002942700025203000800412a67000000000100b045c6512cef5ad7d20377ec3fe79488b97697a72041b87bb8d7d8c1887de6980673656e736f720e7265636f72645f72656164696e670006010000010100010200010300010400010500";

  await signAndSubmit(transactionBytes);
}

main().catch(console.error);
