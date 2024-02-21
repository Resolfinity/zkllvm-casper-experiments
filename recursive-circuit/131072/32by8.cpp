#include <nil/crypto3/hash/algorithm/hash.hpp>
#include <nil/crypto3/hash/sha2.hpp>
#include <nil/crypto3/hash/poseidon.hpp>

using namespace nil::crypto3;
using namespace nil::crypto3::algebra::curves;

using size_t = std::size_t;
using sha_hash = hashes::sha2<256>;
using poseidon_hash = hashes::poseidon;
using block_type = sha_hash::block_type;
using field_type = pallas::base_field_type::value_type;

using uint64_t_le = uint64_t;
using uint64_t_be = uint64_t;

field_type uint_to_field(uint64_t_le lower)
{
  std::array<field_type, 128> decomposed_block;
  __builtin_assigner_bit_decomposition(decomposed_block.data(), 64, 0, true);
  __builtin_assigner_bit_decomposition(decomposed_block.data() + 64, 64, lower, true);
  return __builtin_assigner_bit_composition(decomposed_block.data(), 128, true);
}

field_type zero_poseidon = hash<hashes::poseidon>(uint_to_field(0), uint_to_field(0));

template <std::size_t size>
field_type evaluate_root(
    typename std::array<field_type, size>::iterator begin,
    typename std::array<field_type, size>::iterator end,
    std::size_t distance)
{
  std::size_t stride = 1;

  while (stride < distance)
  {
    for (auto i = begin; i != end; i += 2 * stride)
    {
      *i = hash<hashes::poseidon>(*i, *(i + stride));
    }
    stride *= 2;
  }
  return *begin;
}

bool is_same(block_type block0,
             block_type block1)
{
  return block0[0] == block1[0] && block0[1] == block1[1];
}

struct EvaluateRootResult
{
  field_type root;
  std::size_t sum_balance;
};

template <size_t size>
EvaluateRootResult evaluate_root_layer0(
    typename std::array<block_type, size>::iterator pubkeys_begin,
    typename std::array<uint64_t_be, size>::iterator effective_balances_begin,
    typename std::array<uint64_t_be, size>::iterator slashed_begin,
    typename std::array<uint64_t_be, size>::iterator activation_epoch_begin,
    typename std::array<uint64_t_be, size>::iterator exit_epoch_begin,
    uint64_t_be epoch,
    std::size_t distance,
    std::size_t offset,
    typename std::array<field_type, size>::iterator layer0_begin)
{
  std::size_t stride = 1;
  std::size_t sum_balance = 0;

  while (stride < distance)
  {
    for (size_t i = 0; i != distance; i += 2 * stride)
    {

      typename std::array<field_type, size>::iterator layer0_current = layer0_begin + i + offset;

      typename std::array<block_type, size>::iterator pubkeys_current = pubkeys_begin + i + offset;
      typename std::array<uint64_t_be, size>::iterator effective_balances_current = effective_balances_begin + i + offset;
      typename std::array<uint64_t_be, size>::iterator slashed_current = slashed_begin + i + offset;
      typename std::array<uint64_t_be, size>::iterator activation_epoch_current = activation_epoch_begin + i + offset;
      typename std::array<uint64_t_be, size>::iterator exit_epoch_current = exit_epoch_begin + i + offset;

      block_type pubkey = *pubkeys_current;
      field_type validator_leaf_0 = hash<hashes::poseidon>(pubkey[0], pubkey[1]);
      field_type validator_leaf_1 = hash<hashes::poseidon>(uint_to_field(*effective_balances_current), uint_to_field(*slashed_current));
      field_type validator_leaf_2 = hash<hashes::poseidon>(uint_to_field(*activation_epoch_current), uint_to_field(*exit_epoch_current));
      field_type validator_leaf_3 = zero_poseidon;
      field_type validator_leaf_01 = hash<hashes::poseidon>(validator_leaf_0, validator_leaf_1);
      field_type validator_leaf_23 = hash<hashes::poseidon>(validator_leaf_2, validator_leaf_3);
      field_type validator_leaf = hash<hashes::poseidon>(validator_leaf_01, validator_leaf_23);

      typename std::array<block_type, size>::iterator next_pubkeys_current = pubkeys_begin + i + stride + offset;
      typename std::array<uint64_t_be, size>::iterator next_effective_balances_current = effective_balances_begin + i + stride + offset;
      typename std::array<uint64_t_be, size>::iterator next_slashed_current = slashed_begin + i + stride + offset;
      typename std::array<uint64_t_be, size>::iterator next_activation_epoch_current = activation_epoch_begin + i + stride + offset;
      typename std::array<uint64_t_be, size>::iterator next_exit_epoch_current = exit_epoch_begin + i + stride + offset;

      block_type next_pubkey = *next_pubkeys_current;
      field_type next_validator_leaf_0 = hash<hashes::poseidon>(next_pubkey[0], next_pubkey[1]);
      field_type next_validator_leaf_1 = hash<hashes::poseidon>(uint_to_field(*effective_balances_current), uint_to_field(*slashed_current));
      field_type next_validator_leaf_2 = hash<hashes::poseidon>(uint_to_field(*activation_epoch_current), uint_to_field(*exit_epoch_current));
      field_type next_validator_leaf_3 = zero_poseidon;
      field_type next_validator_leaf_01 = hash<hashes::poseidon>(next_validator_leaf_0, next_validator_leaf_1);
      field_type next_validator_leaf_23 = hash<hashes::poseidon>(next_validator_leaf_2, next_validator_leaf_3);
      field_type next_validator_leaf = hash<hashes::poseidon>(next_validator_leaf_01, next_validator_leaf_23);

      field_type paired_validarors = hash<hashes::poseidon>(validator_leaf, next_validator_leaf);

      // store nodes in pubkeys array
      *layer0_current = paired_validarors;

      // calculate sum_balance for this piece
      if (stride == 1)
      {
        if (*activation_epoch_begin <= epoch && *exit_epoch_begin > epoch)
          sum_balance += *effective_balances_begin;
        if (*next_activation_epoch_current <= epoch && *next_exit_epoch_current > epoch)
          sum_balance += *next_effective_balances_current;
      }
    }
    stride *= 2;
  }
  return {*layer0_begin, sum_balance};
}

constexpr size_t VALIDATORS_TREE_HEIGHT_BEACON = 5;
constexpr size_t VALIDATORS_MAX_COUNT_BEACON = 1 << VALIDATORS_TREE_HEIGHT_BEACON;

constexpr size_t VALIDATORS_TREE_HEIGHT_POSEIDON = 5;
constexpr size_t VALIDATORS_MAX_COUNT_POSEIDON = 1 << VALIDATORS_TREE_HEIGHT_POSEIDON;

constexpr size_t VALIDATOR_FIELDS_BEACON = 8;
constexpr size_t VALIDATOR_FIELDS_POSEIDON = 4;

constexpr size_t MAX_VALIDATORS_CHANGED = 4;

[[circuit]] field_type recursive_circuit(
    // [[private_input]] size_t actual_validator_count,
    // [[private_input]] block_type old_finalized_hash,
    // [[private_input]] block_type new_finalized_hash,
    // [[private_input]] std::array<block_type, VALIDATORS_TREE_HEIGHT_BEACON * MAX_VALIDATORS_CHANGED> changed_validators_beacon_proofs,

    // [[private_input]] field_type old_poseidon_root,
    // [[private_input]] field_type new_poseidon_root,
    // [[private_input]] std::array<value_type, VALIDATORS_MAX_COUNT_POSEIDON> validators_leaves,
    // [[private_input]] std::array<field_type, VALIDATORS_TREE_HEIGHT_POSEIDON * MAX_VALIDATORS_CHANGED> changed_validators_poseidon_proofs,

    [[private_input]] std::array<block_type, VALIDATORS_MAX_COUNT_BEACON> validators_pubkeys, // stored in poseidon
    [[private_input]] std::array<block_type, VALIDATORS_MAX_COUNT_BEACON> validators_withdrawal_credentials,
    [[private_input]] std::array<uint64_t_be, VALIDATORS_MAX_COUNT_BEACON> validators_effective_balances, // stored in poseidon
    [[private_input]] std::array<uint64_t_be, VALIDATORS_MAX_COUNT_BEACON> validators_slashed,            // stored in poseidon
    [[private_input]] std::array<uint64_t_be, VALIDATORS_MAX_COUNT_BEACON> validators_activation_eligibility_epoch,
    [[private_input]] std::array<uint64_t_be, VALIDATORS_MAX_COUNT_BEACON> validators_activation_epoch, // stored in poseidon
    [[private_input]] std::array<uint64_t_be, VALIDATORS_MAX_COUNT_BEACON> validators_exit_epoch,       // stored in poseidon
    [[private_input]] std::array<uint64_t_be, VALIDATORS_MAX_COUNT_BEACON> validators_withdrawable_epoch,
    // [[private_input]] std::array<uint64_t_be, VALIDATORS_MAX_COUNT_BEACON> validators_indices,

    // [[private_input]] size_t actual_changed_validator_count,
    // [[private_input]] std::array<block_type, MAX_VALIDATORS_CHANGED> changed_validators_pubkeys,
    // [[private_input]] std::array<block_type, MAX_VALIDATORS_CHANGED> changed_validators_withdrawal_credentials,
    // [[private_input]] std::array<uint64_t_be, MAX_VALIDATORS_CHANGED> changed_validators_effective_balances,
    // [[private_input]] std::array<uint64_t_be, MAX_VALIDATORS_CHANGED> changed_validators_slashed,
    // [[private_input]] std::array<uint64_t_be, MAX_VALIDATORS_CHANGED> changed_validators_activation_eligibility_epoch,
    // [[private_input]] std::array<uint64_t_be, MAX_VALIDATORS_CHANGED> changed_validators_activation_epoch,
    // [[private_input]] std::array<uint64_t_be, MAX_VALIDATORS_CHANGED> changed_validators_exit_epoch,
    // [[private_input]] std::array<uint64_t_be, MAX_VALIDATORS_CHANGED> changed_validators_withdrawable_epoch,
    // [[private_input]] std::array<uint64_t_be, MAX_VALIDATORS_CHANGED> changed_validators_index,

    [[private_input]] uint64_t_be total_active_balance,
    [[private_input]] uint64_t_be slot,
    [[private_input]] uint64_t_be epoch)
{
  /*
  Main algorithm:
  1. prove updated validators:
  verify proof of beacon updated validators, one by one.
  suppose we have old root, and 1st validator merkle proof.
  we take this merkle proof siblings, recompute validator leaf, and update root, using new leaf and siblings.
  hence we have root, updated by one changed validator.
  although this root is virtual, it does not exist in the beacon state.
  we pre-compute 2nd changed validator merkle-proof using this intermediate merkle tree with updated 1st validator.
  we use this merkle proof to update the root, and so on.
  finally, we have to compute all changed validators merkle-proofs, and compare the final root with the new_finalized_hash.

  2. update internal poseidon tree:
  verify all (old trusted) validators from circuit input, computing poseidon tree root, compare with old_poseidon_root.
  hence we prove that all validators are trusted from previous epoch.
  then,
  as in 1., we update the root, adding changed validators one by one, and getting a new_poseidon_root.

  3. at that point, we have proved set of validators, and their balances, and we can compute new total_active_balance
  which is a sum of all effective balances of all active validators.

  About inputs:
  Also we want to split all the active validators by comittees, starting from the first slot of the epoch and up to the last slot of the epoch.
  We pre-compute all the committees, and their shuffling.
  We send validators lists already shuffled.
  And we send additional validators_indices list which maps validators indixes to their real beacon chain indices,
  which are used to compute poseidon tree.

  Poseidon merkle tree:
  We exclude some fields, keeping only used to check whether the validator is active or not.
  - pubkey as primary key
  - effective_balance to compute total_active_balance
  - activation_epoch to check whether the validator is active or not
  - exit_epoch to check whether the validator is active or not
  That's why VALIDATOR_FIELDS_POSEIDON is 4.
  */

  // recompute old poseidon root
  std::array<field_type, 32> layer0;
  std::array<field_type, 4> layer1;
  std::array<uint64_t_be, 4> balances_subtotals;

  auto layer0_begin = layer0.begin();
  auto pubkeys_begin = validators_pubkeys.begin();
  auto effective_balances_begin = validators_effective_balances.begin();
  auto slashed_begin = validators_slashed.begin();
  auto activation_epoch_begin = validators_activation_epoch.begin();
  auto exit_epoch_begin = validators_exit_epoch.begin();

#pragma zk_multi_prover 0
  {
    auto result =
        evaluate_root_layer0<32>(
            pubkeys_begin,
            effective_balances_begin,
            slashed_begin,
            activation_epoch_begin,
            exit_epoch_begin,
            epoch,
            8,
            0,
            layer0_begin);

    layer1[0] = result.root;
    balances_subtotals[0] = result.sum_balance;
  }

#pragma zk_multi_prover 1
  {
    auto result =
        evaluate_root_layer0<32>(
            pubkeys_begin,
            effective_balances_begin,
            slashed_begin,
            activation_epoch_begin,
            exit_epoch_begin,
            epoch,
            8,
            8,
            layer0_begin);

    layer1[1] = result.root;
    balances_subtotals[1] = result.sum_balance;
  }

#pragma zk_multi_prover 2
  {
    auto result =
        evaluate_root_layer0<32>(
            pubkeys_begin,
            effective_balances_begin,
            slashed_begin,
            activation_epoch_begin,
            exit_epoch_begin,
            epoch,
            8,
            16,
            layer0_begin);

    layer1[2] = result.root;
    balances_subtotals[2] = result.sum_balance;
  }

#pragma zk_multi_prover 3
  {
    auto result =
        evaluate_root_layer0<32>(
            pubkeys_begin,
            effective_balances_begin,
            slashed_begin,
            activation_epoch_begin,
            exit_epoch_begin,
            epoch,
            8,
            24,
            layer0_begin);

    layer1[3] = result.root;
    balances_subtotals[3] = result.sum_balance;
  }

  field_type result;
#pragma zk_multi_prover 4
  {
    auto begin = layer1.begin();
    result = evaluate_root<2>(begin, begin + 2, 2);
  }

  return result;
}