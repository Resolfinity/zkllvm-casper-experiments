#include <nil/crypto3/hash/algorithm/hash.hpp>
#include <nil/crypto3/hash/sha2.hpp>

using namespace nil::crypto3;

using hash_type = hashes::sha2<256>;
using block_type = hash_type::block_type;
using field_type = algebra::curves::pallas::base_field_type::value_type;
using size_t = std::size_t;

constexpr size_t VALIDATORS_COUNT = 100;
constexpr size_t VOTERS_COUNT = 80;

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
    [[private_input]] std::array<bool, VALIDATORS_COUNT> validators_slashed,
    [[private_input]] std::array<size_t, VALIDATORS_COUNT> validators_activation_epoch,
    [[private_input]] std::array<size_t, VALIDATORS_COUNT> validators_exit_epoch,

    [[private_input]] std::array<size_t, VOTERS_COUNT> voters_indexes, // indexes of voters in validators list
    [[private_input]] std::array<size_t, VOTERS_COUNT> voters_sorted_indexes)
{

  std::array<size_t, 2> partial_voters_balances;
  std::array<bool, VOTERS_COUNT> duplicates;

  for (size_t idx = 0; idx < VOTERS_COUNT - 1; ++idx)
  {
    duplicates[idx] = false;
  }

  int computed_voters_balance = 0;
  for (size_t idx = 0; idx < VOTERS_COUNT - 1; ++idx)
  {

    size_t voter_sorted_index = voters_sorted_indexes[idx];

    __builtin_assigner_exit_check(duplicates[voter_sorted_index] != true);

    duplicates[voter_sorted_index] = true;

    size_t voter_idx = voters_indexes[voter_sorted_index];

    // check if voter is active
    __builtin_assigner_exit_check(is_validator_active(validators_activation_epoch[voter_idx], validators_exit_epoch[voter_idx], epoch));

    // voters_pubkeys are unique by using voters_sorted_indexes, skip last pubkey check

    if (voter_sorted_index < VOTERS_COUNT - 1)
    {
      block_type voter_pubkey = validators_pubkeys[voter_idx];

      size_t next_sorted_pubkey_idx = voters_sorted_indexes[idx + 1];
      block_type next_voter_pubkey_index = voters_indexes[next_sorted_pubkey_idx];
      block_type next_voter_pubkey = validators_pubkeys[next_sorted_pubkey_idx];

      __builtin_assigner_exit_check(less_than(voter_pubkey, next_voter_pubkey));
    }

    if (validators_slashed[voter_idx] == true)
    {
      computed_voters_balance += validators_effective_balances[voter_idx];
    }
  }
  partial_voters_balances[0] = computed_voters_balance;

  return computed_voters_balance;
}