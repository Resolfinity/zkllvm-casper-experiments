#include <cstdint>
#include <nil/crypto3/algebra/curves/pallas.hpp>
#include <cstdlib>

using namespace nil::crypto3;

using field_type = algebra::curves::pallas::base_field_type;

constexpr std::size_t VALIDATORS_COUNT = 1000000;

typename field_type::value_type pow(typename field_type::value_type a, int n)
{
  if (n == 0)
    return 1;

  typename field_type::value_type res = 1;
  for (int i = 0; i < n; ++i)
  {
    res *= a;
  }
  return res;
}

[[circuit]] bool is_unique(
    [[private_input]] std::array<int, VALIDATORS_COUNT> shuffled_validators_indexes)
{
  int n = VALIDATORS_COUNT;
  field_type::value_type x = 2; // You can choose a different value for x if needed
  field_type::value_type polynomialValue = 0;
  field_type::value_type expectedValue = 0;

  // Calculate the polynomial value based on array elements
  for (int e : shuffled_validators_indexes)
  {
    polynomialValue += pow(x, e);
  }

  // Calculate the expected value for a unique set
  for (int i = 0; i < n; i++)
  {
    expectedValue += pow(x, i);
  }

  // Compare the calculated polynomial value with the expected value
  return polynomialValue == expectedValue;
}