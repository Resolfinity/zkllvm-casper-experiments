import fs from "fs";
import * as path from "path";

// Define the structure of your vector object
interface Block {
  vector: [{ field: string }, { field: string }];
}

interface BlocksArray {
  array: Block[];
}

interface Int {
  int: string;
}

interface IntArray {
  array: Int[];
}

interface Bool {
  int: "1" | "0";
}

interface BoolArray {
  array: Bool[];
}

type Inputs = [
  Int,
  BlocksArray,
  IntArray,
  BoolArray,
  IntArray,
  IntArray,

  IntArray,
  IntArray
];

const generateRandomBlock = (): Block => {
  const getRandomField = (): string =>
    Math.floor(Math.random() * 1e10).toString();

  return { vector: [{ field: getRandomField() }, { field: getRandomField() }] };
};

const generateArrayOfBlocks = (size: number): BlocksArray => {
  const blocks: Block[] = [];
  for (let i = 0; i < size; i++) {
    blocks.push(generateRandomBlock());
  }

  return { array: blocks };
};

const generateRandomInt = (maxValue?: number): Int => {
  const getRandomInt = maxValue
    ? (): number => Math.floor(Math.random() * maxValue)
    : (): number => Math.floor(Math.random() * 1e10);

  return { int: getRandomInt().toString() };
};

const generateArrayOfInts = (size: number, maxValue?: number): IntArray => {
  const ints: Int[] = [];
  for (let i = 0; i < size; i++) {
    ints.push(generateRandomInt(maxValue));
  }

  return { array: ints };
};

function generateBoolArray(length: number, trues: number): BoolArray {
  // Create an array with 'trues' number of true values and the rest false
  let array: boolean[] = new Array<boolean>(length).fill(false);
  for (let i = 0; i < trues; i++) {
    array[i] = true;
  }

  // Shuffle the array to distribute the true values randomly
  for (let i = array.length - 1; i > 0; i--) {
    const j = Math.floor(Math.random() * (i + 1));
    [array[i], array[j]] = [array[j], array[i]];
  }

  return { array: array.map((bool) => ({ int: bool ? "1" : "0" })) };
}

function filterRandomElements<T>(array: T[], desiredLength: number): T[] {
  if (desiredLength >= array.length) {
    return array.slice(); // Return a copy of the original array if desired length is equal or greater
  }

  // Generate indices to remove
  let indicesToRemove = new Set<number>();
  while (indicesToRemove.size < array.length - desiredLength) {
    let randomIndex = Math.floor(Math.random() * array.length);
    indicesToRemove.add(randomIndex);
  }

  // Filter the array based on these indices
  return array.filter((_, index) => !indicesToRemove.has(index));
}

function shuffleArray<T>(array: T[]): T[] {
  for (let i = array.length - 1; i > 0; i--) {
    const j = Math.floor(Math.random() * (i + 1));
    [array[i], array[j]] = [array[j], array[i]]; // Swap elements
  }
  return array;
}

function getIndicesOfElements<T>(sourceArray: T[], targetArray: T[]): number[] {
  return targetArray.map((element) => sourceArray.indexOf(element));
}

function selectRandomElements<T>(inputArray: T[], n: number): T[] {
  // Validate input
  if (n > inputArray.length) {
    throw new Error("n cannot be greater than the input array length.");
  }
  if (n < 0) {
    throw new Error("n cannot be negative.");
  }

  // Generate an array of indices from the input array
  const indices = Array.from(inputArray.keys());

  // Shuffle the indices array
  for (let i = indices.length - 1; i > 0; i--) {
    const j = Math.floor(Math.random() * (i + 1));
    [indices[i], indices[j]] = [indices[j], indices[i]];
  }

  // Select the first n indices and sort them to maintain original order
  const selectedIndices = indices.slice(0, n).sort((a, b) => a - b);

  // Create the output array using the selected indices
  const outputArray = selectedIndices.map((index) => inputArray[index]);

  return outputArray;
}

const generateInputs = (
  validators: number,
  voters: number,
  epoch: number
): Inputs => {
  const validators_pubkeys = generateArrayOfBlocks(validators);
  const balances = generateArrayOfInts(validators);
  const slashed = generateBoolArray(validators, 4); // slashed
  const activation_epoch = generateArrayOfInts(validators, 1);
  const exit_epoch = generateArrayOfInts(validators, epoch + 233);

  // activation_epoch.array.map((epoch, index) => {
  //   console.log(epoch.int, exit_epoch.array[index].int);
  // });

  const activeValidators = validators_pubkeys.array.filter((_, index) => {
    return (
      Number(activation_epoch.array[index].int) <= epoch &&
      Number(exit_epoch.array[index].int) > epoch
    );
  });

  console.log("active validators", activeValidators.length);

  const voters_pubkeys_ordered = selectRandomElements(activeValidators, voters);
  const voters_pubkeys_shuffled = shuffleArray(voters_pubkeys_ordered);

  // console.log(
  //   "shuffled pubkeys",
  //   voters_pubkeys_shuffled.map((pubkey) => pubkey.vector[0].field)
  // );

  const voters_pubkeys_sorted = voters_pubkeys_shuffled.slice().sort((a, b) => {
    return a.vector[0].field.localeCompare(b.vector[0].field);
  });

  // console.log(
  //   "sorted pubkeys",
  //   voters_pubkeys_sorted.map((pubkey) => pubkey.vector[0].field)
  // );

  const voters_indexes = getIndicesOfElements(
    validators_pubkeys.array,
    voters_pubkeys_shuffled
  ).map((index) => ({ int: index.toString() }));

  const voters_sorted_indexes = getIndicesOfElements(
    voters_pubkeys_sorted,
    voters_pubkeys_shuffled
  ).map((index) => ({ int: index.toString() }));

  // console.log(
  //   "sorted indexes",
  //   voters_sorted_indexes.map((index) => index.int)
  // );

  const input: Inputs = [
    { int: epoch.toString() },
    validators_pubkeys,
    balances,
    slashed,
    activation_epoch,
    exit_epoch,

    // { array: voters_pubkeys_shuffled },
    { array: voters_indexes },
    { array: voters_sorted_indexes },
  ];

  console.log("voters length", voters_sorted_indexes.length);

  return input;
};

const validators = 1;
const voters = 0;
const epoch = 3;

const inputsJson = generateInputs(validators, voters, epoch);

fs.writeFileSync(
  path.join(__dirname, "private-input.json"),
  JSON.stringify(inputsJson, null, 2)
);
