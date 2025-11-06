import { fromBase58 } from "@mysten/sui/utils";

const digest = "EdGjwQ2A18BMpzc4GRfRi2RiJvgtLwsB5HHYjNJWQjG3";
const bytes = fromBase58(digest);
console.log("Digest hex:", Buffer.from(bytes).toString("hex"));
