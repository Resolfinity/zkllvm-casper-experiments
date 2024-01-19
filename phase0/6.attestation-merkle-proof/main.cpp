#include <nil/crypto3/hash/algorithm/hash.hpp>
#include <nil/crypto3/hash/sha2.hpp>
#include <cstdint>

using namespace nil::crypto3;

using hash_type = hashes::sha2<256>;
using block_type = hash_type::block_type;
using field_type = algebra::curves::pallas::base_field_type;

constexpr std::size_t VALIDATORS_MAX_SIZE_LOG2 = 40;

constexpr std::size_t VALIDATORS_COUNT = 400;

constexpr std::size_t BEACON_STATE_FIELD_INCLUSION_PROOF_LENGTH = 5;
constexpr std::size_t MULTIPROOF_SIZE = BEACON_STATE_FIELD_INCLUSION_PROOF_LENGTH * VALIDATORS_COUNT;

constexpr std::size_t VALIDATOR_FIELDS = 8;
constexpr std::size_t VALIDATORS_FIELD_INDEX = 11;

constexpr bool BYTE_ORDER_MSB = true;
constexpr bool BYTE_ORDER_LSB = false;

const std::array<block_type, 1> precomputed_zero_hashes = {{{0x00000000000000000000000000000000_cppui255, 0x00000000000000000000000000000000_cppui255}}};

bool is_same(block_type block0, block_type block1)
{
  return block0[0] == block1[0] && block0[1] == block1[1];
}

template <std::size_t LayerSize>
block_type hash_layer(std::array<block_type, LayerSize> input, size_t layer)
{
  constexpr size_t NextLayerSize = (LayerSize % 2 == 0) ? LayerSize / 2 : (LayerSize / 2 + 1);
  std::array<block_type, NextLayerSize> next_layer;

  for (std::size_t leaf_index = 0; leaf_index < LayerSize / 2; leaf_index++)
  {
    next_layer[leaf_index] = hash<hash_type>(input[2 * leaf_index], input[2 * leaf_index + 1]);
  }
  if (LayerSize % 2 != 0)
  {
    next_layer[NextLayerSize - 1] = hash<hash_type>(LayerSize - 1, precomputed_zero_hashes[0]);
  }
  if (LayerSize == 2)
    return next_layer[0];
  else
    return hash_layer<NextLayerSize>(next_layer, layer + 1);
}

template <std::size_t LayerSize>
block_type hash_tree(std::array<block_type, LayerSize> input)
{
  return hash_layer(input, 0);
}

block_type lift_uint64(uint64_t value)
{
  std::array<typename field_type::value_type, 128> decomposed_block;
  __builtin_assigner_bit_decomposition(decomposed_block.data(), 64, value, BYTE_ORDER_LSB);
  __builtin_assigner_bit_decomposition(decomposed_block.data() + 64, 64, uint64_t(0), BYTE_ORDER_LSB);
  return {
      __builtin_assigner_bit_composition(decomposed_block.data(), 128, BYTE_ORDER_LSB),
      0};
}

template <size_t ProofSize>
bool verify_inclusion_proof(size_t field_index, block_type field_hash, block_type merkle_root, std::array<block_type, ProofSize> inclusion_proof)
{
  size_t cur_index = field_index;
  block_type current_hash = field_hash;
  block_type return_block = field_hash;

  current_hash = hash<hash_type>(current_hash, inclusion_proof[0]);
  current_hash = hash<hash_type>(inclusion_proof[1], current_hash);
  current_hash = hash<hash_type>(current_hash, inclusion_proof[2]);
  current_hash = hash<hash_type>(inclusion_proof[3], current_hash);
  current_hash = hash<hash_type>(current_hash, inclusion_proof[4]);

  return is_same(current_hash, merkle_root);
}

[[circuit]] bool multiproof(
    [[private_input]] std::array<block_type, VALIDATORS_COUNT> validators_pubkeys,
    [[private_input]] std::array<block_type, VALIDATORS_COUNT> validators_withdrawal_credentials,
    [[private_input]] std::array<uint64_t, VALIDATORS_COUNT> validators_effective_balances,
    [[private_input]] std::array<uint64_t, VALIDATORS_COUNT> validators_slashed,
    [[private_input]] std::array<uint64_t, VALIDATORS_COUNT> validators_activation_eligibility_epoch,
    [[private_input]] std::array<uint64_t, VALIDATORS_COUNT> validators_activation_epoch,
    [[private_input]] std::array<uint64_t, VALIDATORS_COUNT> validators_exit_epoch,
    [[private_input]] std::array<uint64_t, VALIDATORS_COUNT> validators_withdrawable_epoch,
    block_type beacon_state_hash,
    [[private_input]] std::array<block_type, MULTIPROOF_SIZE> validators_multi_proof)
{

  for (size_t validator_idx = 0; validator_idx < VALIDATORS_COUNT; ++validator_idx)
  {

    // merkelize validator
    block_type validator_hash = hash_tree<8>({validators_pubkeys[validator_idx],
                                              validators_withdrawal_credentials[validator_idx],
                                              lift_uint64(validators_effective_balances[validator_idx]),
                                              lift_uint64(validators_slashed[validator_idx]),
                                              lift_uint64(validators_activation_eligibility_epoch[validator_idx]),
                                              lift_uint64(validators_activation_epoch[validator_idx]),
                                              lift_uint64(validators_exit_epoch[validator_idx]),
                                              lift_uint64(validators_withdrawable_epoch[validator_idx])});

    // verify inclusion proof
    // from validators_multi_proof array, get subarray of 400 elements starting from validator_idx *  400

    std::array<block_type, BEACON_STATE_FIELD_INCLUSION_PROOF_LENGTH> inclusion_proof;
    for (size_t i = 0; i < BEACON_STATE_FIELD_INCLUSION_PROOF_LENGTH; ++i)
    {
      inclusion_proof[i] = validators_multi_proof[validator_idx * BEACON_STATE_FIELD_INCLUSION_PROOF_LENGTH + i];
    }

    bool valid = verify_inclusion_proof<BEACON_STATE_FIELD_INCLUSION_PROOF_LENGTH>(VALIDATORS_FIELD_INDEX, validator_hash, beacon_state_hash, inclusion_proof);
  }

  return true;
}