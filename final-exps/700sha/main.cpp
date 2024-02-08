#include <nil/crypto3/hash/algorithm/hash.hpp>
#include <nil/crypto3/hash/sha2.hpp>

using namespace nil::crypto3;

bool is_same(typename hashes::sha2<256>::block_type block0,
             typename hashes::sha2<256>::block_type block1)
{

  return block0[0] == block1[0] && block0[1] == block1[1];
}

constexpr std::size_t HASHES = 700;

[[circuit]] bool
validate_path([[private_input]] std::array<typename hashes::sha2<256>::block_type, HASHES> merkle_path,
              [[private_input]] typename hashes::sha2<256>::block_type leave,
              [[private_input]] typename hashes::sha2<256>::block_type root)
{

  typename hashes::sha2<256>::block_type subroot = leave;

  for (int i = 0; i < HASHES; i++)
  {
    subroot = hash<hashes::sha2<256>>(subroot, merkle_path[i]);
  }

  if (is_same(subroot, root))
  {
    return true;

        // __builtin_assigner_exit_check(is_same(subroot, root));
  }

  return false;
}