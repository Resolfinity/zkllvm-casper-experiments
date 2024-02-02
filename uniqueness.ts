const array = [0, 1, 3, 2, 4, 5];

const shuffledArray = array.sort(() => Math.random() - 0.5);

console.log(shuffledArray);

const n = array.length;

for (let i = 0; i < n; i++) {
  let index = shuffledArray[i];
  console.log(index);

  if (index >= n) {
    index = index - n;
  }

  if (shuffledArray[index] >= n) {
    console.log("Not unique");
    process.exit(1);
  }

  shuffledArray[index] = shuffledArray[index] + n;
}
