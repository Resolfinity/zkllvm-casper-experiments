import sha256CryptoJS from "crypto-js/sha256";

function sha256(data: string): string {
  return sha256CryptoJS(data).toString();
}

function createMerkleTree(leaves: number[]): string[][] {
  let nodes = leaves.map((leaf) => sha256(leaf.toString()));
  let tree: string[][] = [nodes.slice()];

  while (nodes.length > 1) {
    let tempNodes: string[] = [];
    for (let i = 0; i < nodes.length; i += 2) {
      let combinedHash =
        i + 1 === nodes.length
          ? sha256(nodes[i] + nodes[i])
          : sha256(nodes[i] + nodes[i + 1]);
      tempNodes.push(combinedHash);
    }
    nodes = tempNodes;
    tree.push(nodes);
  }

  return tree;
}

function createMerkleMultiproof(
  tree: string[][],
  leafIndices: number[]
): string[] {
  let proof: string[] = [];
  let indices = Array.from(new Set(leafIndices)).sort((a, b) => a - b);
  let levelLength = tree[0].length;

  for (let level of tree.slice(0, -1)) {
    let nextIndices: number[] = [];
    let nextLevelLength = Math.ceil(levelLength / 2);

    for (let index of indices) {
      let pairIndex = index ^ 1;

      if (pairIndex < levelLength && !indices.includes(pairIndex)) {
        proof.push(level[pairIndex]);
      }

      nextIndices.push(Math.floor(index / 2));
    }

    indices = Array.from(new Set(nextIndices)).sort((a, b) => a - b);
    levelLength = nextLevelLength;
  }

  return proof;
}

function updateLeafAndRecalculateRoot(
  tree: string[][],
  leafIndex: number,
  newLeafValue: number
): string {
  let newLeafHash = sha256(newLeafValue.toString());
  tree[0][leafIndex] = newLeafHash;

  for (let level = 1; level < tree.length; level++) {
    let index = Math.floor(leafIndex / 2);
    let leftChild = tree[level - 1][index * 2];
    let rightChild =
      index * 2 + 1 < tree[level - 1].length
        ? tree[level - 1][index * 2 + 1]
        : leftChild;
    tree[level][index] = sha256(leftChild + rightChild);
    leafIndex = index;
  }

  return tree[tree.length - 1][0];
}

function calculateRootFromMultiproof(
  leafIndices: number[],
  updatedLeafHashes: string[],
  proof: string[],
  originalRoot: string,
  levelLength: number
): string {
  let combined: { [key: number]: string } = {};
  leafIndices.forEach(
    (index, idx) => (combined[index] = updatedLeafHashes[idx])
  );
  let proofIndex = 0;

  for (let level = 0; level < tree.length - 1; level++) {
    let nextCombined: { [key: number]: string } = {};
    for (let index of Object.keys(combined)
      .map(Number)
      .sort((a, b) => a - b)) {
      let siblingIndex = index % 2 === 0 ? index + 1 : index - 1;

      if (siblingIndex < levelLength) {
        let siblingHash =
          siblingIndex in combined
            ? combined[siblingIndex]
            : proof[proofIndex++];
        let combinedHash =
          index % 2 === 0
            ? sha256(combined[index] + siblingHash)
            : sha256(siblingHash + combined[index]);
        nextCombined[Math.floor(index / 2)] = combinedHash;
      } else {
        nextCombined[Math.floor(index / 2)] = combined[index];
      }
    }
    combined = nextCombined;
    levelLength = Math.ceil(levelLength / 2);
  }

  return Object.values(combined)[0];
}

// Example usage
let leaves = Array.from({ length: 16 }, (_, i) => i);
let tree = createMerkleTree(leaves);
let originalRoot = tree[tree.length - 1][0];
let leafIndices = [2, 7];
let leafHashes = leafIndices.map((leaf) => sha256(leaf.toString()));
let merkleMultiproof = createMerkleMultiproof(tree, leafIndices);

let updatedLeaves = { 2: 20, 7: 21 };
Object.entries(updatedLeaves).forEach(([index, value]) => {
  updateLeafAndRecalculateRoot(tree, parseInt(index), value);
});

let updatedLeafHashes = leafIndices.map((leaf) =>
  sha256(updatedLeaves[leaf].toString())
);
let newRootFromProof = calculateRootFromMultiproof(
  leafIndices,
  updatedLeafHashes,
  merkleMultiproof,
  originalRoot,
  tree[0].length
);

console.log("Roots are equal:", newRootFromProof === tree[tree.length - 1][0]);
