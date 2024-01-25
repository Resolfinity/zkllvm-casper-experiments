#include <cstdint>
#include <nil/crypto3/algebra/curves/pallas.hpp>
#include <cstdlib>

using namespace nil::crypto3;

using field_type = algebra::curves::pallas::base_field_type;

constexpr std::size_t VALIDATORS_COUNT = 1000;

[[circuit]] bool is_unique(
    [[private_input]] std::array<int, VALIDATORS_COUNT> shuffled_validators_indexes)
{
    int n = VALIDATORS_COUNT;

    for (int i = 0; i < n; ++i)
    {
        // Using modulo to get the original value if it has been marked

        int index = shuffled_validators_indexes[i];

        if (index >= n)
            index = index - n;

        // Check if the value at this index has been marked
        if (shuffled_validators_indexes[index] >= n)
        {
            return false; // Duplicate found
        }

        // Mark this value by adding n to it
        shuffled_validators_indexes[index] += n;
    }

    return true; // No duplicates
}