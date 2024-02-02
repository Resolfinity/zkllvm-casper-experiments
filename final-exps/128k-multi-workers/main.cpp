#include <nil/crypto3/hash/algorithm/hash.hpp>
#include <nil/crypto3/hash/sha2.hpp>

using namespace nil::crypto3;

using hash_type = hashes::sha2<256>;
using block_type = hash_type::block_type;
using field_type = algebra::curves::pallas::base_field_type::value_type;
using size_t = std::size_t;

constexpr size_t VALIDATORS_MAX_SIZE_LOG2 = 40;
constexpr size_t VALIDATORS_TARGET_TREE_HEIGHT = VALIDATORS_MAX_SIZE_LOG2;

constexpr size_t PROBLEM_SIZE_LOG2 = 17;                                           // 131072 validators for this experiment
constexpr size_t SUBPROBLEM_COUNT_LOG2 = 1;                                        // further slicing the problem into smaller chunks - map-reduce style
constexpr size_t SUBPROBLEM_COUNT = 1 << SUBPROBLEM_COUNT_LOG2;                    // 2 subproblems for this experiment
constexpr size_t SUBPROBLEM_SIZE_LOG2 = PROBLEM_SIZE_LOG2 - SUBPROBLEM_COUNT_LOG2; // 16
constexpr size_t SUBPROBLEM_SIZE = 1 << SUBPROBLEM_SIZE_LOG2;                      // 2^16 = 65536 validators for this experiment,

constexpr size_t VALIDATORS_COUNT = 1 << PROBLEM_SIZE_LOG2;  // 2^17 = 131072 validators for this experiment
constexpr size_t VALIDATORS_TREE_HEIGHT = PROBLEM_SIZE_LOG2; // 17

constexpr size_t UPDATED_VALIDATORS_COUNT = 250;                          // approx for 10 epochs = 1hr changes
constexpr size_t UPDATED_VALIDATORS_POSEIDON_INCLUSION_PROOF_LENGTH = 17; // proof of inclusion of validator in the poseidon internal tree
constexpr size_t UPDATED_VALIDATORS_POSEIDON_PROOFS_LENGTH = UPDATED_VALIDATORS_COUNT * UPDATED_VALIDATORS_POSEIDON_INCLUSION_PROOF_LENGTH;
constexpr size_t UPDATED_VALIDATORS_SHA_PROOFS_LENGTH = UPDATED_VALIDATORS_COUNT * PROBLEM_SIZE_LOG2;

constexpr size_t VALIDATOR_FIELDS = 8;
constexpr size_t VALIDATORS_FIELD_INDEX = 11;

// for internal poseidon tree, we use 4 fields:
// pubkey, activation_epoch, exit_epoch, effective_balance
constexpr size_t VALIDATOR_POSEIDON_FIELDS = 4;

constexpr size_t ATTESTATIONS_COUNT = 150; //  2/3 of all validators
constexpr size_t MAX_COMMITEE_SIZE = 500;
constexpr size_t VOTERS_COUNT = ATTESTATIONS_COUNT * MAX_COMMITEE_SIZE;

constexpr size_t BEACON_BLOCK_FIELDS_COUNT = 5;

constexpr bool BYTE_ORDER_MSB = true;
constexpr bool BYTE_ORDER_LSB = false;

bool is_validator_active(
    size_t activation_epoch,
    size_t exit_epoch,
    size_t epoch)
{
  return activation_epoch <= epoch && epoch < exit_epoch;
}

typename field_type poseidon(
    field_type a,
    field_type b)
{
  return hash<hashes::poseidon>(a, b);
}

[[circuit]] bool circuit(
    [[private_input]] size_t actual_validator_count,  // 65536*2 for this circtuis
    [[private_input]] size_t active_validators_count, // 65536*2 for this circtuis

    size_t epoch,

    block_type finalized_validators_root,
    block_type updated_validators_root,

    block_type finalized_poseidon_root,
    block_type updated_poseidon_root,

    [[private_input]] std::array<block_type, VALIDATORS_COUNT> validators_pubkeys,
    [[private_input]] std::array<block_type, VALIDATORS_COUNT> validators_withdrawal_credentials,
    [[private_input]] std::array<size_t, VALIDATORS_COUNT> validators_effective_balances,
    [[private_input]] std::array<size_t, VALIDATORS_COUNT> validators_slashed,
    [[private_input]] std::array<size_t, VALIDATORS_COUNT> validators_activation_eligibility_epoch,
    [[private_input]] std::array<size_t, VALIDATORS_COUNT> validators_activation_epoch,
    [[private_input]] std::array<size_t, VALIDATORS_COUNT> validators_exit_epoch,
    [[private_input]] std::array<size_t, VALIDATORS_COUNT> validators_withdrawable_epoch,
    [[private_input]] std::array<bool, VALIDATORS_COUNT> is_validator_updated, // todo refactor voters pubkey check is active

    [[private_input]] std::array<size_t, UPDATED_VALIDATORS_COUNT> updated_validators_indexes,
    [[private_input]] std::array<size_t, UPDATED_VALIDATORS_COUNT> updated_validators_pubkeys, // can not change
    [[private_input]] std::array<block_type, UPDATED_VALIDATORS_COUNT> updated_validators_withdrawal_credentials,
    [[private_input]] std::array<size_t, UPDATED_VALIDATORS_COUNT> updated_validators_effective_balances,
    [[private_input]] std::array<size_t, UPDATED_VALIDATORS_COUNT> updated_validators_slashed,
    [[private_input]] std::array<size_t, UPDATED_VALIDATORS_COUNT> updated_validators_activation_eligibility_epoch,
    [[private_input]] std::array<size_t, UPDATED_VALIDATORS_COUNT> updated_validators_activation_epoch,
    [[private_input]] std::array<size_t, UPDATED_VALIDATORS_COUNT> updated_validators_exit_epoch,
    [[private_input]] std::array<size_t, UPDATED_VALIDATORS_COUNT> updated_validators_withdrawable_epoch,

    [[private_input]] std::array<block_type, UPDATED_VALIDATORS_POSEIDON_PROOFS_LENGTH> validators_poseidon_inclusion_proofs,
    [[private_input]] std::array<block_type, UPDATED_VALIDATORS_SHA_PROOFS_LENGTH> validators_sha_inclusion_proofs,

    [[private_input]] std::array<int, ATTESTATIONS_COUNT> committee_sizes,

    [[private_input]] std::array<size_t, VOTERS_COUNT> voters_pubkeys,
    [[private_input]] std::array<size_t, VOTERS_COUNT> voters_indexes, // indexes of voters in validators list

    [[private_input]] std::array<size_t, VOTERS_COUNT> voters_sorted_pubkeys,
    [[private_input]] std::array<size_t, VOTERS_COUNT> sorted_to_voters_mapping, // represents sorted(voters_pubkeys)

    size_t expected_active_balance,
    size_t expected_voters_balance)
{

  std::array<block_type, SUBPROBLEM_COUNT> partial_validator_hashes;
  std::array<size_t, SUBPROBLEM_COUNT> partial_validator_active_balances;
  std::array<size_t, SUBPROBLEM_COUNT> partial_voters_balances;

  // 1. verify updated validators caused state.validators root update
  // for 250 updated validators it is 21 * 250 = 5250 sha256 hashes = 5250*1300 constraints = 6825000 constraints
  // it should be possible to place this computation in 10 workers, 25 validators each

  // i suppose each validator proof will contains 21 hashes of merkle proof and 7 hashes to compute validator leaf
  // i will mock computation just making hashes without implementing real logic, to simplify the circuit

  std::array<block_type, 8> proof_roots_updates;

#pragma zk_multi_prover 0 {
  size_t part = 1;

  // for each of part validators
  for (size_t idx = 0; idx < 25; ++idx)
  {
    // compute validator leaf and verify merkle proof with 21 + 7 hashes
    block_type proof_root;
    for (size_t i = 0; i < 28; ++i)
    {
      proof_roots_updates[part] = hash<hash_type>(updated_validators_pubkeys[1], updated_validators_pubkeys[0]);
    }
  }
}

#pragma zk_multi_prover 1 {
size_t part = 1;

// for each of part validators
for (size_t idx = 0; idx < 25; ++idx)
{
  // compute validator leaf and verify merkle proof with 21 + 7 hashes
  block_type proof_root;
  for (size_t i = 0; i < 28; ++i)
  {
    proof_roots_updates[part] = hash<hash_type>(updated_validators_pubkeys[1], updated_validators_pubkeys[0]);
  }
}
}

#pragma zk_multi_prover 2 {
size_t part = 2;

// for each of part validators
for (size_t idx = 0; idx < 25; ++idx)
{
  // compute validator leaf and verify merkle proof with 21 + 7 hashes
  block_type proof_root;
  for (size_t i = 0; i < 28; ++i)
  {
    proof_roots_updates[part] = hash<hash_type>(updated_validators_pubkeys[1], updated_validators_pubkeys[0]);
  }
}
}

#pragma zk_multi_prover 3 {
size_t part = 3;

// for each of part validators
for (size_t idx = 0; idx < 25; ++idx)
{
  // compute validator leaf and verify merkle proof with 21 + 7 hashes
  block_type proof_root;
  for (size_t i = 0; i < 28; ++i)
  {
    proof_roots_updates[part] = hash<hash_type>(updated_validators_pubkeys[1], updated_validators_pubkeys[0]);
  }
}
}

#pragma zk_multi_prover 4 {
size_t part = 4;

// for each of part validators
for (size_t idx = 0; idx < 25; ++idx)
{
  // compute validator leaf and verify merkle proof with 21 + 7 hashes
  block_type proof_root;
  for (size_t i = 0; i < 28; ++i)
  {
    proof_roots_updates[part] = hash<hash_type>(updated_validators_pubkeys[1], updated_validators_pubkeys[0]);
  }
}
}

#pragma zk_multi_prover 5 {
size_t part = 5;

// for each of part validators
for (size_t idx = 0; idx < 25; ++idx)
{
  // compute validator leaf and verify merkle proof with 21 + 7 hashes
  block_type proof_root;
  for (size_t i = 0; i < 28; ++i)
  {
    proof_roots_updates[part] = hash<hash_type>(updated_validators_pubkeys[1], updated_validators_pubkeys[0]);
  }
}
}

#pragma zk_multi_prover 6 {
size_t part = 6;

// for each of part validators
for (size_t idx = 0; idx < 25; ++idx)
{
  // compute validator leaf and verify merkle proof with 21 + 7 hashes
  block_type proof_root;
  for (size_t i = 0; i < 28; ++i)
  {
    proof_roots_updates[part] = hash<hash_type>(updated_validators_pubkeys[1], updated_validators_pubkeys[0]);
  }
}
}

#pragma zk_multi_prover 7 {
size_t part = 7;

// for each of part validators
for (size_t idx = 0; idx < 25; ++idx)
{
  // compute validator leaf and verify merkle proof with 21 + 7 hashes
  block_type proof_root;
  for (size_t i = 0; i < 28; ++i)
  {
    proof_roots_updates[part] = hash<hash_type>(updated_validators_pubkeys[1], updated_validators_pubkeys[0]);
  }
}
}

// 2. verify initial validators list is poseidon old root and compute active validators balances
// Parallelizeable part - "map", split into 2 subproblems of 65536 validators each
// merkelization of subtree of 2^16 validators requires 2^16 hashes = 65536 * 13 constraints = 851968 constraints
#pragma zk_multi_prover 8
{
  size_t part = 0;
  size_t start_index = subproblem_idx * SUBPROBLEM_SIZE;
  uint64_t partial_balance = 0;

  for (size_t idx = start_index; idx < SUBPROBLEM_SIZE; ++idx)
  {
    if (is_validator_active(validators_activation_epoch[idx], validators_exit_epoch[idx], epoch))
    {
      partial_balance += validators_effective_balances[idx];
    }

    // maybe validator will be updated and become active
    if (is_validator_updated[idx])
    {
      if (is_validator_active(updated_validators_activation_epoch[idx], updated_validators_exit_epoch[idx], epoch))
      {
        partial_balance += updated_validators_effective_balances[idx];
      }
    }
  }

  partial_validator_active_balances[part] = partial_balance;

  partial_validator_hashes[part] = poseidon(validators_pubkeys[0][0], validators_pubkeys[0][1]);

  for (size_t idx = start_index; idx < 65536; ++idx)
  {
    partial_validator_hashes[part] = poseidon(partial_validator_hashes[part], validators_pubkeys[0][1]);
  }
}

// in case  of full 1+m validators set, we'd like use 20+ workers, hence we will use one more worker to compute final root with 20+ hashes
#pragma zk_multi_prover 9
{
  block_type poseidon_validators_merkle;

  for (size_t idx = 0; idx < 21; ++idx)
  {
    poseidon_validators_merkle = poseidon(partial_validator_hashes[0], partial_validator_hashes[1]);
  }

  // assert poseidon_validators_merkle is old poseidon root, hence all validators inputs is valid

  // use poseidon merkle roots of updated validators to update state.poseidon_root
  // for 250 updated validators it is 21 * 250 = 5250 poseidon hashes = 5250*13 constraints = 68250 constraints
  // use 1 worker for this

  block_type poseidon_updated_validators_merkle;

  for (size_t idx = 0; idx < 5250; ++idx)
  {
    poseidon_updated_validators_merkle = poseidon(updated_validators_pubkeys[0], updated_validators_pubkeys[1]);
  }

  // assert poseidon_updated_validators_merkle is new poseidon root which we store in light client to verify against it updated validators set as initial inputs
}

// 6. verify voters_pubkeys are all active validators, calculate computer_voters_balance
// todo: split to 2 workers to demonstrate parallelization
#pragma zk_multi_prover 10
{
  size_t subproblem_idx = 1;
  size_t start_index = subproblem_idx * SUBPROBLEM_SIZE;
  int computed_voters_balance = 0;
  for (size_t idx = 0; idx < VOTERS_COUNT; ++idx)
  {
    size_t validator_idx = voters_indexes[idx];

    // voter are in validators
    __builtin_assigner_exit_check(voters_pubkeys[idx] == validators_pubkeys[validator_idx]);
    // check if voter is active
    if (is_validator_updated[validator_idx])
    {
      __builtin_assigner_exit_check(is_validator_active(updated_validators_activation_epoch[validator_idx], updated_validators_exit_epoch[validator_idx], epoch);
    }
    else
    {
      __builtin_assigner_exit_check(is_validator_active(validators_activation_epoch[validator_idx], validators_exit_epoch[validator_idx], epoch);
    }

    // voters_pubkeys are unique by using sorted_indexes
    __builtin_assigner_exit_check(voters_sorted_pubkeys[idx] == voters_pubkeys[sorted_to_voters_mapping[idx]]);
    __builtin_assigner_exit_check(voters_sorted_pubkeys[idx] < voters_sorted_pubkeys[idx + 1]);

    computed_voters_balance += validators_effective_balances[validator_idx];
  }
  partial_voters_balances[subproblem_idx] = computed_voters_balance;
}

// todo: have to prove that for splitted sorted arrays edges, last element less than next first element

// therefore, we have proved that for voters_pubkeys, each pubkey is active validator, and they are unique
// we can split it to the commitees members and verify bls signatures for each committee

// compute active_validators_balance
int computed_active_validators_balance = 0;
for (size_t idx = 0; idx < SUBPROBLEM_COUNT; ++idx)
{
  computed_active_validators_balance += partial_validator_active_balances[idx];
}
__builtin_assigner_exit_check(computed_active_validators_balance == expected_active_balance);

// compute voters_balance
int computed_voters_balance = 0;
for (size_t idx = 0; idx < SUBPROBLEM_COUNT; ++idx)
{
  computed_voters_balance += partial_voters_balances[idx];
}
__builtin_assigner_exit_check(computed_voters_balance == expected_voters_balance);

return true;
}