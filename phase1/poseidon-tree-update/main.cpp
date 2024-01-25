
#include <nil/crypto3/hash/algorithm/hash.hpp>
#include <nil/crypto3/hash/poseidon.hpp>

using namespace nil::crypto3;
using namespace nil::crypto3::algebra::curves;

[[circuit]] pallas::base_field_type::value_type merkle_tree_poseidon(
    [[private_input]] std::array<typename pallas::base_field_type::value_type, 0x20> layer_0_leaves)
{

  typename pallas::base_field_type::value_type result;

  for (int i = 0; i < 675; i++)
  {
    result =
        hash<hashes::poseidon>(layer_0_leaves[0], layer_0_leaves[1]);
  }

  return result;
}