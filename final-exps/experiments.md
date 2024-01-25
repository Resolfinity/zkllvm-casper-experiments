below are possible algorithm to do finality proof:

-

## attestation circuit

### inputs

as much as possible attestations data

- list of validators_pubkeys
- list of validators_effective_balances
- list of shuffled_validators_indexes
- list of attestations_signatures
- list of attestations_hashed_messages
- int active_validators_count

### logic

- calculate commitee_size using active_validators_count
- set total_voted_balance = 0
- for each attestation_index from 0 to 2000
  - get commitee indexes from shuffled_validators_indexes (offset by commitee_size\*attestation_index)
  - get commitee_validators_pubkeys from validators_pubkeys
  - get attestation signature from attestations_signatures
  - get attestation hashed message from attestations_hashed_messages
  - verify attestation signature
  - total_voted_balance +=
