# Circuit 1: Validators set updates & voters list

## Given

- finalized point
- validators state root
- internal validators merkle tree root
- validators list from state of finalized point
- new point has to be finalized
- validators list from the state of new point updated_validators_list
- list of accounted attestations, finalizing new point

## Pre-compute

- updated validators
- added validators
- changed validators multi-proof for finalized state
- ordered list of participators of accounted attestations pubkeys/balances ordered_attesters_list
- list of mapping between ordered_attesters_list and updated_validators_list
- sorted_ordered_attesters_list
- list of mapping between sorted_ordered_attesters_list and ordered_attesters_list

## Output

- proved list of attestation participants
- sum of attestations balances
- total active validators balance

## Recursion

This circuit used parallel workers to split computations of:

### Merkelization of poseidon tree

Given 1.5kk validators in tree of height 21, using precomputed zeroes hashes for a range of levels,

- 13 rows of assignment table for each poseidon hash operation
- 1m rows in a single table
- 1.5kk \* 13 = 20m rows

Therefore, we have to split this procedure to more than 20 workers.

1.5kk validators / 20 workers = 75000 validators

Lower power of 2 is 16 = 65,536 validators in each worker = 1.5kk/65536 = 22 workers.

### Validation of attesters list uniqueness

We use sorted array check algorithm for around 1kk list.
We split this algorithm to 4 workers.

## Final recursive proofs verification

Given 15 minutes to compute two proofs addition.

We have 22+4 = 28 proofs.

To combine it, after proofs generation, we have to run:

- 14 pairs of proofs in parallel = 15 minutes -> 7 proofs of level 2
- 4 pairs of proofs in parallel = 15 minutes -> 2 pairs of level 2
- 2 pairs of proofs in parallel = 15 minutes -> final proof

Therefore, to compute whole circuit, we have to measure proof building times for all of the proofs and sum it with the recurcive procedures (1 hr approx).

## Code

We are not ready to run full tests for millions of validators, hence we've prepared few circtuits with some smaller validators set. We want to make sure that the computation time is a multiple of the size of the input sheets.
