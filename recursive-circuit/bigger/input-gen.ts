import * as fs from "fs";
import { resolve } from "path";

const field = { field: "2" };
const vector = { vector: [field, field] };
const int = { int: 1 };

function getValidatorFields(): any {
  return [field, field, field, field, field, field, field, field, field, field];
}

// Function to generate the object with 500 fields
function generateInput(validators: number) {
  const input: any = [];

  const validatorsData = {
    array: Array.from({ length: validators }, () =>
      getValidatorFields()
    ).flat(),
  };

  const changedValidatorsData: any = {
    array: Array.from({ length: 256 }, () => getValidatorFields()).flat(),
  };

  const changedValidatorsProofs = {
    array: Array.from({ length: 256 * 20 }, () => vector),
  };

  const validatorsIndices = {
    array: Array.from({ length: validators }, () => ({
      int: Math.floor(Math.random() * (validators - 100)).toString(),
    })),
  };

  input.push(
    validatorsData,
    changedValidatorsData,
    changedValidatorsProofs,
    validatorsIndices,
    int,
    int,
    int
  );

  return input;
}

// Generate the object
const validatorsCount = 2 ** 20;

const input = generateInput(validatorsCount);

// Convert the object to a JSON string
const jsonString = JSON.stringify(input, null, 2);

// Write the JSON string to a file named 'private-input.json'
fs.writeFileSync(resolve(__dirname, "private-input.json"), jsonString);
fs.writeFileSync(resolve(__dirname, "public-input.json"), "[]");

console.log("File written successfully.");
