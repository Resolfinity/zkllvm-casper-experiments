#include <nil/crypto3/hash/algorithm/hash.hpp>
#include <nil/crypto3/hash/sha2.hpp>

using namespace nil::crypto3;

using hash_type = hashes::sha2<256>;
using block_type = hash_type::block_type;
using field_type = algebra::curves::pallas::base_field_type::value_type;
using size_t = std::size_t;

constexpr size_t VALIDATORS_COUNT = 100000;
constexpr size_t UPDATED_VALIDATORS_COUNT = 250;
constexpr size_t VOTERS_COUNT = 22191;

bool is_validator_active(
    size_t activation_epoch,
    size_t exit_epoch,
    size_t epoch)
{
  return activation_epoch <= epoch && epoch < exit_epoch;
}

bool is_same(block_type block0, block_type block1)
{
  return block0[0] == block1[0] && block0[1] == block1[1];
}

bool less_than(block_type block0, block_type block1)
{
  // return block0[0] < block1[0];
  return true;
}

[[circuit]] size_t circuit(
    [[private_input]] size_t epoch,

    [[private_input]] std::array<block_type, VALIDATORS_COUNT> validators_pubkeys,
    [[private_input]] std::array<size_t, VALIDATORS_COUNT> validators_effective_balances,
    [[private_input]] std::array<size_t, VALIDATORS_COUNT> validators_slashed,
    [[private_input]] std::array<size_t, VALIDATORS_COUNT> validators_activation_epoch,
    [[private_input]] std::array<size_t, VALIDATORS_COUNT> validators_exit_epoch,

    [[private_input]] std::array<size_t, UPDATED_VALIDATORS_COUNT> updated_validators_indexes,
    [[private_input]] std::array<size_t, UPDATED_VALIDATORS_COUNT> updated_validators_effective_balances,
    [[private_input]] std::array<size_t, UPDATED_VALIDATORS_COUNT> updated_validators_slashed,
    [[private_input]] std::array<size_t, UPDATED_VALIDATORS_COUNT> updated_validators_activation_epoch,
    [[private_input]] std::array<size_t, UPDATED_VALIDATORS_COUNT> updated_validators_exit_epoch,

    [[private_input]] std::array<block_type, VOTERS_COUNT> voters_pubkeys,
    [[private_input]] std::array<size_t, VOTERS_COUNT> voters_indexes, // indexes of voters in validators list

    [[private_input]] std::array<block_type, VOTERS_COUNT> voters_sorted_indexes)
{

  std::array<size_t, 2> partial_voters_balances;

  int computed_voters_balance = 0;
  for (size_t idx = 0; idx < VOTERS_COUNT - 1; ++idx)
  {
    size_t validator_idx = voters_indexes[idx];

    // // voter are in validators
    __builtin_assigner_exit_check(is_same(voters_pubkeys[idx], validators_pubkeys[validator_idx]));
    // check if voter is active
    if (is_validator_updated[validator_idx])
    {
      __builtin_assigner_exit_check(is_validator_active(updated_validators_activation_epoch[validator_idx], updated_validators_exit_epoch[validator_idx], epoch));
    }
    else
    {
      __builtin_assigner_exit_check(is_validator_active(validators_activation_epoch[validator_idx], validators_exit_epoch[validator_idx], epoch));
    }

    // voters_pubkeys are unique by using sorted_indexes
    __builtin_assigner_exit_check(is_same(voters_sorted_pubkeys[idx], voters_pubkeys[sorted_to_voters_mapping[idx]]));
    __builtin_assigner_exit_check(less_than(voters_sorted_pubkeys[idx], voters_sorted_pubkeys[idx + 1]));

    computed_voters_balance += validators_effective_balances[validator_idx];
  }
  partial_voters_balances[0] = computed_voters_balance;

  return computed_voters_balance;
}