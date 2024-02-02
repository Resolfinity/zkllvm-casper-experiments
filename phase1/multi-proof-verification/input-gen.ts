import { createWriteStream } from "fs";
import * as path from "path";

// Define the structure of your vector object
interface VectorObject {
  vector: Array<{ field: string }>;
}

// Function to generate a single vector object
const generateVectorObject = (): VectorObject => {
  const getRandomField = (): string =>
    Math.floor(Math.random() * 1e20).toString();

  return {
    vector: [{ field: getRandomField() }, { field: getRandomField() }],
  };
};

// Function to generate the first element with N vector objects
const generateFirstElement = (n: number): VectorObject[] => {
  const array: VectorObject[] = [];
  for (let i = 0; i < n; i++) {
    array.push(generateVectorObject());
  }
  return array;
};

// Function to write the JSON to a file using streams
const writeToJSONFile = (n: number) => {
  const filePath = path.resolve(__dirname, "private-input.json");
  const writeStream = createWriteStream(filePath, { flags: "w" });

  writeStream.write('[\n  { "array": ');

  // Write the first element with N vector objects
  const firstElement = generateFirstElement(n);
  writeStream.write(JSON.stringify(firstElement));

  // Write the static parts of your input
  writeStream.write("},\n  ");
  writeStream.write(
    JSON.stringify({
      vector: [
        { field: "166209954665300884822717121800188038799" },
        { field: "144783094885247872016525034474730739405" },
      ],
    })
  );
  writeStream.write(",\n  ");
  writeStream.write(
    JSON.stringify({
      vector: [
        { field: "35174607401651764532592651957486212894" },
        { field: "140712014106150384775276850496842098004" },
      ],
    })
  );

  writeStream.write("\n]\n");
  writeStream.end();
};

// Example usage
const N = 700; // Replace with the desired number
writeToJSONFile(N);
