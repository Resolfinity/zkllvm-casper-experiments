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
  BoolArray,

  IntArray,
  IntArray,
  IntArray,
  IntArray,

  BlocksArray,
  IntArray,

  BlocksArray,
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

const generateInputs = (
  validators: number,
  updated_validators: number,
  voters: number,
  epoch: number
): Inputs => {
  const validators_pubkeys = generateArrayOfBlocks(validators);
  const balances = generateArrayOfInts(validators);
  const slashed = generateBoolArray(validators, 4); // slashed
  const activation_epoch = generateArrayOfInts(validators, epoch + 3);
  const exit_epoch = generateArrayOfInts(validators, epoch + 3);
  const isValidatorUpdated = generateBoolArray(validators, updated_validators);

  const updated_effective_balance = generateArrayOfInts(updated_validators);
  const updated_slashed = generateBoolArray(updated_validators, 1);
  const updated_activation_epoch = generateArrayOfInts(
    updated_validators,
    epoch + 1
  );
  const updated_exit_epoch = generateArrayOfInts(updated_validators, epoch + 1);

  const active_validators_pubkeys_raw = validators_pubkeys.array.filter(
    (pubkey, index) => {
      return (
        Number(activation_epoch.array[index].int) <= epoch &&
        Number(exit_epoch.array[index].int) > epoch
      );
    }
  );

  const voters_pubkeys_ordered = filterRandomElements(
    active_validators_pubkeys_raw,
    voters
  );

  const voters_pubkeys = {
    array: shuffleArray(voters_pubkeys_ordered).map((pubkey) => {
      return pubkey;
    }),
  };

  const voters_indexes = {
    array: getIndicesOfElements(
      validators_pubkeys.array,
      voters_pubkeys.array
    ).map((index) => ({ int: index.toString() })),
  };

  const voters_sorted_pubkeys = {
    array: voters_pubkeys.array.slice().sort((a, b) => {
      return a.vector[0].field.localeCompare(b.vector[0].field);
    }),
  };

  const sorted_to_voters_mapping = {
    array: getIndicesOfElements(
      voters_pubkeys.array,
      voters_sorted_pubkeys.array
    ).map((index) => ({ int: index.toString() })),
  };

  const input: Inputs = [
    { int: epoch.toString() },
    validators_pubkeys,
    balances,
    slashed,
    activation_epoch,
    exit_epoch,
    isValidatorUpdated,

    updated_effective_balance,
    updated_slashed,
    updated_activation_epoch,
    updated_exit_epoch,

    voters_pubkeys,
    voters_indexes,

    voters_sorted_pubkeys,
    sorted_to_voters_mapping,
  ];

  console.log(
    "voters_sorted_pubkeys.length",
    voters_sorted_pubkeys.array.length
  );

  return input;
};

const validators = 32;
const updated_validators = 3;
const voters = 20;
const epoch = 3;

const inputsJson = generateInputs(
  validators,
  updated_validators,
  voters,
  epoch
);

fs.writeFileSync(
  path.join(__dirname, "private-input.json"),
  JSON.stringify(inputsJson, null, 2)
);
