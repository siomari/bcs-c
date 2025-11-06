import { Ed25519Keypair } from "@mysten/sui/keypairs/ed25519";
import { SuiClient } from "@mysten/sui/client";
import { Transaction } from "@mysten/sui/transactions";
import dotenv from "dotenv";

dotenv.config();

async function signAndSubmit(transactionHex: string) {
  const privateKey = process.env.PRIVATE_KEY;

  if (!privateKey) {
    throw new Error("PRIVATE_KEY must be set in .env");
  }

  // Create keypair
  const keypair = Ed25519Keypair.fromSecretKey(privateKey);
  const sender = keypair.toSuiAddress();

  console.log("Sender:", sender);
  console.log("Transaction bytes:", transactionHex.substring(0, 32) + "...\n");

  // Create client
  const client = new SuiClient({
    url: "https://fullnode.testnet.sui.io:443",
  });

  // Convert hex to bytes
  const transactionBytes = Buffer.from(transactionHex, "hex");

  console.log("Deserializing transaction...");

  // Deserialize the full transaction (includes gas payment, sender, budget)
  const tx = Transaction.from(transactionBytes);

  console.log("Signing and executing transaction...");

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
  // Modified full transaction from C code (includes gas payment)
  const transactionBytes = "0000060101e3b83e0616d2c47961ca802a5169a94896e65063252ae9f2a3603afea35b42c34e7a3916000000000100022e0900028c190002942700025203000800412a67000000000100b045c6512cef5ad7d20377ec3fe79488b97697a72041b87bb8d7d8c1887de6980673656e736f720e7265636f72645f72656164696e6700060100000101000102000103000104000105006c389948ab60dc14b7c18e0e4e3dd08bf2ced62f7a34c1f7ceaeb24d967db7c701052c8d054356c92a3b62cb4dd75339c8f465eac608393cd1f7180cd2ea0c1a4a587a391600000000206f3a7c4925d994bf8cab59fb842e4659f492f0ecf2ae5d39c24cdfa6d29336cb6c389948ab60dc14b7c18e0e4e3dd08bf2ced62f7a34c1f7ceaeb24d967db7c7e80300000000000000e1f5050000000000";

  await signAndSubmit(transactionBytes);
}

main().catch(console.error);
