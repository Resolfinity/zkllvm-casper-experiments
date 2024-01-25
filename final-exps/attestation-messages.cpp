/*
todo:
args:
- attestations messaged hashed
- finalized ckeckpoint
- merkle proofs of checkpoint inclusion into hashed messages

according to the https://eth2book.info/capella/part2/building_blocks/merkleization/#the-full-picture
each SOURCE merkle proof consists of 2 hashes
*/

#include <nil/crypto3/hash/algorithm/hash.hpp>
#include <nil/crypto3/hash/sha2.hpp>

using namespace nil::crypto3;

bool is_same(typename hashes::sha2<256>::block_type block0,
             typename hashes::sha2<256>::block_type block1)
{

  return block0[0] == block1[0] && block0[1] == block1[1];
}

constexpr std::size_t attestations_count = 10;
constexpr std::size_t hashes_in_attestation_merkle_path = 2;
constexpr std::size_t merkle_paths_array_length = attestations_count * hashes_in_attestation_merkle_path;

[[circuit]] bool validate_messages(
    std::array<typename hashes::sha2<256>::block_type, attestations_count> hashed_msgs,
    [[private_input]] std::array<typename hashes::sha2<256>::block_type, merkle_paths_array_length> merkle_paths,
    typename hashes::sha2<256>::block_type finalized_checkpoint)
{

  // hash the finalized_checkpoint
  typename hashes::sha2<256>::block_type empty_block = {0, 0};
  typename hashes::sha2<256>::block_type hashed_checkpoint = hash<hashes::sha2<256>>(subroot, empty_block);

  // verify each checkpoint is included into the message
  for (std::size_t i = 0; i < attestations_count; i++)
  {
    typename hashes::sha2<256>::block_type msg_root = hashed_msgs[i];

    typename hashes::sha2<256>::block_type subroot = hash<hashes::sha2<256>>(hashed_checkpoint, merkle_paths[i * hashes_in_attestation_merkle_path]);
    subroot = hash<hashes::sha2<256>>(subroot, merkle_paths[i * hashes_in_attestation_merkle_path + 1]);

    __builtin_assigner_exit_check(is_same(subroot, msg_root));
  }
}