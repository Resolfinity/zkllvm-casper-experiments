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
      *i = hash<poseidon_hash>(*i, *(i + stride));
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
  field_type sum_balance;
};

template <size_t size>
void compute_validator_leaves(
    typename std::array<field_type, size>::iterator validators_begin,
    uint64_t_be epoch,
    std::size_t distance,
    typename std::array<field_type, size>::iterator layer0_begin,
    typename std::array<field_type, size>::iterator balances_subtotals_begin,
    typename std::array<uint64_t_be, size>::iterator validators_indices_begin,
    std::size_t part)
{

  field_type field_balance = uint_to_field(0);
  size_t offset = part * distance;

  for (size_t i = 0; i != distance; i += 1)
  {

    // index of validator in input array
    size_t index = 0; // *(validators_indices_begin + i + offset);

    // 10 fields per validator in input array
    typename std::array<field_type, size>::iterator layer0_current = layer0_begin + i + offset;
    typename std::array<field_type, size>::iterator validators_current = validators_begin + index;

    field_type pubkey_0 = *(validators_current + 0);
    field_type pubkey_1 = *(validators_current + 1);
    field_type effective_balance = *(validators_current + 4);
    field_type slashed = *(validators_current + 5);
    field_type activation_epoch = *(validators_current + 7);
    field_type exit_epoch = *(validators_current + 8);

    field_type validator_leaf_0 = hash<poseidon_hash>(pubkey_0, pubkey_1);
    field_type validator_leaf_1 = hash<poseidon_hash>(effective_balance, slashed);
    field_type validator_leaf_2 = hash<poseidon_hash>(activation_epoch, exit_epoch);
    field_type validator_leaf_01 = hash<poseidon_hash>(validator_leaf_0, validator_leaf_1);
    field_type validator_leaf_23 = hash<poseidon_hash>(validator_leaf_2, zero_poseidon);
    field_type validator_leaf = hash<poseidon_hash>(validator_leaf_01, validator_leaf_23);

    field_balance = field_balance + effective_balance;

    *layer0_current = validator_leaf;
    *(balances_subtotals_begin + part) = field_balance;
  }
}

template <size_t changed_fields_size,
          size_t leaves_size>
void compute_beacon_validators_leaves(
    typename std::array<field_type, changed_fields_size>::iterator changed_validators_fields_begin,
    std::size_t distance,
    std::size_t offset,
    typename std::array<block_type, leaves_size>::iterator intermediate_merkle_roots_begin,
    typename std::array<block_type, leaves_size * 20>::iterator changed_validators_beacon_proofs_begin)
{

  for (size_t i = 0; i != distance; i += 1)
  {
    // 10 fields per validator in input array
    typename std::array<field_type, changed_fields_size>::iterator changed_validators_current = changed_validators_fields_begin + i * 10 + offset;

    field_type pubkey_0 = *(changed_validators_current + 0);
    field_type pubkey_1 = *(changed_validators_current + 1);
    field_type withdrawal_credentials_0 = *(changed_validators_current + 2);
    field_type withdrawal_credentials_1 = *(changed_validators_current + 3);
    field_type effective_balance = *(changed_validators_current + 4);
    field_type slashed = *(changed_validators_current + 5);
    field_type activation_eligibility_epoch = *(changed_validators_current + 6);
    field_type activation_epoch = *(changed_validators_current + 7);
    field_type exit_epoch = *(changed_validators_current + 8);
    field_type withdrawable_epoch = *(changed_validators_current + 9);

    block_type pubkey = {pubkey_0, pubkey_1};
    block_type withdrawal_credentials = {withdrawal_credentials_0, withdrawal_credentials_1};

    block_type effective_balance_block = {effective_balance, 0};
    block_type slashed_block = {slashed, 0};
    block_type activation_eligibility_epoch_block = {activation_eligibility_epoch, 0};
    block_type activation_epoch_block = {activation_epoch, 0};
    block_type exit_epoch_block = {exit_epoch, 0};
    block_type withdrawable_epoch_block = {withdrawable_epoch, 0};

    block_type validator_leaf_0 = hash<sha_hash>(pubkey, withdrawal_credentials);
    block_type validator_leaf_1 = hash<sha_hash>(effective_balance_block, slashed_block);
    block_type validator_leaf_2 = hash<sha_hash>(activation_eligibility_epoch_block, activation_epoch_block);
    block_type validator_leaf_3 = hash<sha_hash>(exit_epoch_block, withdrawable_epoch_block);

    block_type validator_leaf_01 = hash<sha_hash>(validator_leaf_0, validator_leaf_1);
    block_type validator_leaf_23 = hash<sha_hash>(validator_leaf_2, validator_leaf_3);

    block_type validator_leaf = hash<sha_hash>(validator_leaf_01, validator_leaf_23);

    // recompute intermediate merkle root, using updated validator leaf
    // here we use simplified calculation to get approx rows for given hash operations
    block_type current_hash = validator_leaf;
    // first merkle proof of validator should be made for old validators root
    // each next merkle proof made for the previous root after one validator updated
    for (size_t j = 0; j != 20; j += 1)
    {
      block_type sibling = *(changed_validators_beacon_proofs_begin + i * 20 + j);
      block_type current_hash = hash<sha_hash>(validator_leaf, sibling);
    }

    // here we assert that updated validator caused intermediate root change and next validator merkle proof is made against this change
    block_type prev_root = *(intermediate_merkle_roots_begin + i); // just to make row in table, will be  thinking on it later
    // we store intermediate merkle root in intermediate_merkle_roots array
    *(intermediate_merkle_roots_begin + i) = current_hash;
  }
}

constexpr size_t VALIDATORS_TREE_HEIGHT_BEACON = 21;
constexpr size_t VALIDATORS_MAX_COUNT_BEACON = 1 << VALIDATORS_TREE_HEIGHT_BEACON;

constexpr size_t VALIDATORS_TREE_HEIGHT_POSEIDON = VALIDATORS_TREE_HEIGHT_BEACON;
constexpr size_t VALIDATORS_MAX_COUNT_POSEIDON = 1 << VALIDATORS_TREE_HEIGHT_POSEIDON;

constexpr size_t VALIDATORS_CHUNKS_LOG2 = 0; // split validators into 4 chunks
constexpr size_t VALIDATORS_CHUNK_POSEIDON = 1 << (VALIDATORS_TREE_HEIGHT_POSEIDON - VALIDATORS_CHUNKS_LOG2);

constexpr size_t VALIDATOR_FIELDS_BEACON = 8;
constexpr size_t VALIDATOR_FIELDS_POSEIDON = 4;

constexpr size_t MAX_VALIDATORS_CHANGED = 1024;
constexpr size_t VALIDATORS_FIELDS = 10; // we store validators in consequent fields, decomposing blocks
constexpr size_t CHANGED_VALIDATORS_ARRAY_SIZE = MAX_VALIDATORS_CHANGED * VALIDATORS_FIELDS;

[[circuit]] field_type recursive_circuit(
    // [[private_input]] size_t actual_validator_count,
    // [[private_input]] block_type old_finalized_hash,
    // [[private_input]] block_type new_finalized_hash,

    // [[private_input]] field_type old_poseidon_root,
    // [[private_input]] field_type new_poseidon_root,

    // [[private_input]] std::array<field_type, VALIDATORS_TREE_HEIGHT_POSEIDON * MAX_VALIDATORS_CHANGED> changed_validators_poseidon_proofs,

    [[private_input]] std::array<field_type, 20971520> validators_fields,
    [[private_input]] std::array<field_type, 10240> changed_validators_fields,
    [[private_input]] std::array<block_type, VALIDATORS_TREE_HEIGHT_BEACON * MAX_VALIDATORS_CHANGED> changed_validators_beacon_proofs,

    // as we have validators placed in validator_fields shuffled, here is mapping between input validators and beacon chain indices
    [[private_input]] std::array<uint64_t_be, 2097152> validators_indices,

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
  std::array<field_type, 2097152> layer0; // validators computed leaves
  std::array<field_type, 256> layer1;     // subroots
  std::array<field_type, 256> balances_subtotals;

  auto layer0_begin = layer0.begin(); // in this layer we store validator leaves
  auto layer1_begin = layer1.begin();
  auto validators_begin = validators_fields.begin();
  auto balances_subtotals_begin = balances_subtotals.begin();
  auto validators_indices_begin = validators_indices.begin();

  auto changed_validators_fields_begin = changed_validators_fields.begin();
  std::array<block_type, MAX_VALIDATORS_CHANGED> intermediate_merkle_roots;
  auto intermediate_merkle_roots_begin = intermediate_merkle_roots.begin();
  auto changed_validators_beacon_proofs_begin = changed_validators_beacon_proofs.begin();

  field_type result;

  // #pragma zk_multi_prover 0
  //   {
  //     compute_validator_leaves<2097152>(
  //         validators_begin,
  //         epoch,
  //         8192,
  //         layer0_begin,
  //         balances_subtotals_begin,
  //         validators_indices_begin,
  //         0);

  //     layer1[0] = evaluate_root<8192>(layer0_begin, layer0_begin + 8192, 8192);
  //   }

  // //   // here we have layer1 with validators subtrees roots
  // //   // each subtree is made of 8192 validators, hence for 1_500_000 validators we have 184 subtrees
  // //   // we can compute the root of the whole tree in one worker
  // #pragma zk_multi_prover 2
  //   {
  //     result = evaluate_root<256>(layer0_begin, layer0_begin + 256, 256);
  //   }

  // here we compare computed root with the old_poseidon_root
  // if they are the same, we can trust all the validators were in prev epoch
  // now we have to validate updated validators are the only ones who changed validators root
  // first we have to  merkelize validators_leaves with sha256

#pragma zk_multi_prover 0
  {
    compute_beacon_validators_leaves<CHANGED_VALIDATORS_ARRAY_SIZE, MAX_VALIDATORS_CHANGED>(
        changed_validators_fields_begin,
        16,
        0,
        intermediate_merkle_roots_begin,
        changed_validators_beacon_proofs_begin);
  }

#pragma zk_multi_prover 1
  {
    compute_beacon_validators_leaves<CHANGED_VALIDATORS_ARRAY_SIZE, MAX_VALIDATORS_CHANGED>(
        changed_validators_fields_begin,
        16,
        16,
        intermediate_merkle_roots_begin,
        changed_validators_beacon_proofs_begin);
  }

#pragma zk_multi_prover 2
  {
    compute_beacon_validators_leaves<CHANGED_VALIDATORS_ARRAY_SIZE, MAX_VALIDATORS_CHANGED>(
        changed_validators_fields_begin,
        16,
        16 * 2,
        intermediate_merkle_roots_begin,
        changed_validators_beacon_proofs_begin);
  }

#pragma zk_multi_prover 3
  {
    compute_beacon_validators_leaves<CHANGED_VALIDATORS_ARRAY_SIZE, MAX_VALIDATORS_CHANGED>(
        changed_validators_fields_begin,
        16,
        16 * 3,
        intermediate_merkle_roots_begin,
        changed_validators_beacon_proofs_begin);
  }

#pragma zk_multi_prover 4
  {
    compute_beacon_validators_leaves<CHANGED_VALIDATORS_ARRAY_SIZE, MAX_VALIDATORS_CHANGED>(
        changed_validators_fields_begin,
        16,
        16 * 4,
        intermediate_merkle_roots_begin,
        changed_validators_beacon_proofs_begin);
  }

#pragma zk_multi_prover 5
  {
    compute_beacon_validators_leaves<CHANGED_VALIDATORS_ARRAY_SIZE, MAX_VALIDATORS_CHANGED>(
        changed_validators_fields_begin,
        16,
        16 * 5,
        intermediate_merkle_roots_begin,
        changed_validators_beacon_proofs_begin);
  }

#pragma zk_multi_prover 6
  {
    compute_beacon_validators_leaves<CHANGED_VALIDATORS_ARRAY_SIZE, MAX_VALIDATORS_CHANGED>(
        changed_validators_fields_begin,
        16,
        16 * 6,
        intermediate_merkle_roots_begin,
        changed_validators_beacon_proofs_begin);
  }

#pragma zk_multi_prover 7
  {
    compute_beacon_validators_leaves<CHANGED_VALIDATORS_ARRAY_SIZE, MAX_VALIDATORS_CHANGED>(
        changed_validators_fields_begin,
        16,
        16 * 7,
        intermediate_merkle_roots_begin,
        changed_validators_beacon_proofs_begin);
  }

  // at that point we have verified all the updated validators
  // next step is to update poseidon tree with new validators
  // we have pre-computed merkle proofs for each updated validator
  // we have to do 250*20 = 5000 poseidon hashes, it is possible in one worker

  // todo: write worker logic to update poseidon tree with new validators and compute new root

  // at that point we have verified all the updated validators and new poseidon root
  // we can use input pubkeys to verify aggregated signatures

  return result;
}