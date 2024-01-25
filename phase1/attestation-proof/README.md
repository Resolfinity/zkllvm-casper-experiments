we have

- attestation with 400 validators, message, aggregated signature
- internal poseidon tree of validators

we want

- merkelize message
- verify each vaildator is in merkle tree (and is active)
- verify signature

we have to:

1. merkelize message
   we want to sha256 merkelization inside the circuit

2. verify merkle proofs of validators against internal tree root
   to do it, we have to make

- 5 hashes for each validator to get leaf hash
- 21 hashes for each validator to verify merkle proofs of validators

hence, we have to make 26 hashes \* 400 validators = 10400 hashes

3. verify signature

- todo
