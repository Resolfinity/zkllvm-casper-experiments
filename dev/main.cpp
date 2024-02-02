#include <nil/crypto3/hash/algorithm/hash.hpp>
#include <nil/crypto3/hash/sha2.hpp>

using namespace nil::crypto3;

using hash_type = hashes::sha2<256>;
using block_type = hash_type::block_type;
using field_type = algebra::curves::pallas::base_field_type::value_type;

[[circuit]] int circuit()
{

  std::array<int, 2> partial_validator_hashes;

#pragma zk_multi_prover 0
  {
    size_t subproblem_idx = 0;

    partial_validator_hashes[subproblem_idx] = 1;
  }

#pragma zk_multi_prover 1
  {
    size_t subproblem_idx = 1;

    partial_validator_hashes[subproblem_idx] = 1;
  }

  int result = 0;

  for (size_t i = 0; i < 2; i++)
  {
    result += partial_validator_hashes[i];
  }

  return result;
}