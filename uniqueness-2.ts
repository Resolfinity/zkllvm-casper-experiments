const array = [0, 2, 3, 5, 4, 1];

const sortedArray = [0, 1, 2, 3, 4, 5];

const mappingArray = [0, 5, 1, 2, 4, 3]; // sorted -> original

// verify uniqueness of array using other two, without sets
function verifyUniqueness(
  array: number[],
  sortedArray: number[],
  mappingArray: number[]
) {
  for (let i = 0; i < array.length - 1; i++) {
    if (sortedArray[i] >= sortedArray[i + 1]) {
      console.log("Not sorted");
      return false;
    }

    if (sortedArray[i] !== array[mappingArray[i]]) {
      // console.log(
      //   "Not mapped",
      //   i,
      //   mappingArray[mappingArray[i]],
      //   array[i],
      //   sortedArray[i]
      // );
      return false;
    }
  }

  return true;
}

console.log(verifyUniqueness(array, sortedArray, mappingArray));
