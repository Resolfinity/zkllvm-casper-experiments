import * as fs from "fs";

// Function to generate the object with 500 fields
function generateObject(): object {
  const array = [] as any;
  for (let i = 1; i <= 500; i++) {
    array.push({ field: i });
  }
  return [{ array }];
}

// Generate the object
const myObject = generateObject();

// Convert the object to a JSON string
const jsonString = JSON.stringify(myObject, null, 2);

// Write the JSON string to a file named 'private-input.json'
fs.writeFileSync("private-input.json", jsonString);

console.log("File written successfully.");
