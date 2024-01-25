Since phase1 exps showed to us that attestation proof takes more than 3 minute, and we will have to build proofs for up to 2k attestsations, it will require more than 60 hours of server time to process one epoch.

We want to research another method of attestations proofs, here is a concept:

- make one circuit which verifies many attestations at a bulk, ideal case is to prove all attestations in one circuit
- or to use pragma workers to prove it parallel way

What should we check for that case:

- bls aggregated signature proof over 400 pubkeys and hashed message, including \_\_builtin_assigner_hash_to_curve for message
- 100 bls sigs verification with different messages
- 2000 bls sigs verification

- can we use poseidon with -e bls12381 - NOT YET

- is public inputs diff with same inputs but private in terms of prover/assigner time - NO
- how long can be public inputs - ?
- do public inputs play commitment role to proofe verification - NO
