#include <nil/crypto3/algebra/fields/bls12/base_field.hpp>
#include <nil/crypto3/algebra/curves/bls12.hpp>
#include <nil/crypto3/algebra/algorithms/pair.hpp>

using namespace nil::crypto3;

constexpr const std::size_t max_attestations_count = 10;
constexpr const std::size_t max_commitee_size = 500;
constexpr const std::size_t validators_amount = attestations_count * max_commitee_size;
constexpr const std::size_t committees_per_epoch = 2048;

std::size_t compute_committee_size(std::size_t n, std::size_t j, std::size_t N)
{
  std::size_t start = n * j / (32 * N);
  std::size_t end = n * (j + 1) / (32 * N);
  return end - start;
}

[[circuit]] bool verify_aggregated_signature(
    __zkllvm_field_bls12381_base finalized_checkpoint_merkelized_hash; // public
    std::size_t total_epoch_balance;                                   // public
    std::size_t active_validators_count;
    std::size_t actual_attestations_count;
    std::array<typename algebra::curves::bls12<381>::template g2_type<>::value_type, validators_amount> pubkeys,
    std::array<std::size_t, validators_amount> balances,
    std::array<std::size_t, validators_amount> shuffled_indexes,
    std::array<typename algebra::fields::bls12_base_field<381>::value_type, attestations_count> hashed_msgs,
    std::array<int, attestations_count> attestation_committee_index,
    std::array<int, attestations_count> attestation_total_balance;
    std::array<typename algebra::curves::bls12<381>::template g1_type<>::value_type, attestations_count> aggregated_signatures)
{

  /*
  def compute_committee_size(n, j, N):
    start = n * j // (32 * N)
    end = n * (j + 1) // (32 * N)
    return end - start
  */

  std::size_t total_voted_balance = 0;

  // verify each attestation
  for (std::size_t i = 0; i < actual_attestations_count; i++)
  {

    // get commitee from inputs
    std::size_t commitee_index = attestation_committee_index[i];
    std::size_t commitee_size = compute_committee_size(active_validators_count, commitee_index, committees_per_epoch);
    std::array<typename algebra::curves::bls12<381>::template g2_type<>::value_type, commitee_size> commitee_pubkeys;
    typename algebra::curves::bls12<381>::template g1_type<>::value_type aggregated_signature = aggregated_signatures[i];

    for (std::size_t j = 0; j < commitee_size; j++)
    {
      std::size_t validator_index = shuffled_indexes[commitee_index * max_commitee_size + j];
      commitee_pubkeys[j] = pubkeys[validator_index];
      total_voted_balance += balances[validator_index];

      // verify aggregated signature
      __builtin_assigner_exit_check(__builtin_assigner_is_in_g1_check(aggregated_signature));

      typename algebra::curves::bls12<381>::template g2_type<>::value_type g2_group_generator = algebra::curves::bls12<381>::template g2_type<>::one();
      typename algebra::curves::bls12<381>::gt_type::value_type pairing1 =
          algebra::pair<algebra::curves::bls12<381>>(aggregated_signature, g2_group_generator);

      typename algebra::fields::bls12_base_field<381>::value_type hashed_msg = hashed_msgs[i];
      typename algebra::curves::bls12<381>::template g1_type<>::value_type msg_point = __builtin_assigner_hash_to_curve(hashed_msg);

      __builtin_assigner_exit_check(__builtin_assigner_is_in_g2_check(commitee_pubkeys[0]));
      typename algebra::curves::bls12<381>::gt_type::value_type pairing2 =
          algebra::pair<algebra::curves::bls12<381>>(msg_point, commitee_pubkeys[0]);

      typename algebra::curves::bls12<381>::gt_type::value_type current_pairing;
      for (std::size_t i = 1; i < commitee_size; i++)
      {
        __builtin_assigner_exit_check(__builtin_assigner_is_in_g2_check(commitee_pubkeys[i]));
        current_pairing = algebra::pair<algebra::curves::bls12<381>>(msg_point, commitee_pubkeys[i]);
        pairing2 = __builtin_assigner_gt_multiplication(pairing2, current_pairing);
      }

      bool are_equal = 1; // change to 0, now skip real verification until we have real inputs data...
      for (std::size_t i = 0; i < 12; i++)
      {
        are_equal = are_equal && (pairing1[i] == pairing2[i]);
      }

      __builtin_assigner_exit_check(are_equal);
    }

    // verify total balance
    __builtin_assigner_exit_check(attestation_total_balance[i] == total_voted_balance);

    /*
      at that point we have verified all attestations signatures, using
    - total validators pubkeys list
    - total validators balances list
    - shuffled validators list, mapping validator index to pubkey index
    - attestations committee indexes list

    what do we have to proof or to have proved:
    - validators pubkeys list is active validators list
    - validators balances correct
    - attestation message includes checkpoint we want to finalize
    - shuffled indexes are unique, hence we don't use same validator twice

    Should we check if attestation message includes the following:
    - attestation epoch is in message
    - attestation slot is in message
    - attestation merkelized hash is correct

    I believe no, since we have a lot of signatures from the majority of validators,
    so we can be sure that the message is correct, and we need only checkpoint, otherwise
    we can fake the proof by using attestations from another checkpoint.
    (this hypothesis should be verified)

    How could we prove that checkpoint is included in message?
    We have merkle root of this message.

    we can prove each message has checkpoint in separate proof, then use this proof with public inputs here
    hence, we need separate circuit
    see attestation-messages.cpp

    */
  }
}