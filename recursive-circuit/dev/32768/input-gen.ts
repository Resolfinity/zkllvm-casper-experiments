import * as fs from "fs";

const field = { field: "1" };
const vector = { vector: [field, field] };
const int = { int: 1 };

function getArray(content: any) {
  return { array: content };
}

function getArrayOfVectors(size: number): any {
  const array = [] as (typeof vector)[];
  for (let i = 0; i < size; i++) {
    array.push(vector);
  }
  return getArray(array);
}

function getArrayOfInts(size: number): any {
  const array = [] as (typeof int)[];
  for (let i = 0; i < size; i++) {
    array.push(int);
  }
  return getArray(array);
}

// Function to generate the object with 500 fields
function generateInput(validators: number): object {
  const validators_pubkeys = getArrayOfVectors(validators);
  const validators_withdrawal_credentials = getArrayOfVectors(validators);
  const validators_effective_balance = getArrayOfInts(validators);
  const validators_slashed = getArrayOfInts(validators);
  const validators_activation_eligibility_epoch = getArrayOfInts(validators);
  const validators_activation_epoch = getArrayOfInts(validators);
  const validators_exit_epoch = getArrayOfInts(validators);
  const validators_withdrawable_epoch = getArrayOfInts(validators);
  const total_active_balance = int;
  const slot = int;
  const epoch = int;

  const array = [
    validators_pubkeys,
    validators_withdrawal_credentials,
    validators_effective_balance,
    validators_slashed,
    validators_activation_eligibility_epoch,
    validators_activation_epoch,
    validators_exit_epoch,
    validators_withdrawable_epoch,
    total_active_balance,
    slot,
    epoch,
  ];

  return array;
}

// Generate the object
const input = generateInput(32768);

// Convert the object to a JSON string
const jsonString = JSON.stringify(input, null, 2);

// Write the JSON string to a file named 'private-input.json'
fs.writeFileSync("private-input.json", jsonString);

console.log("File written successfully.");
