#include "wrapping_integers.hh"
#include <iostream>

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  // (void)n;
  // (void)zero_point;
  return Wrap32 { (uint32_t)( zero_point.raw_value_ + n ) };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  // (void)zero_point;
  // (void)checkpoint;
  // return {};
  int64_t diff = this->raw_value_ - zero_point.raw_value_;
  if ( diff < 0 ) {
    diff += UINT32_MAX + 1ULL;
  }
  uint64_t udiff = (uint64_t)diff;
  uint64_t factor = checkpoint / ( UINT32_MAX + 1ULL );
  uint64_t ans = factor * ( UINT32_MAX + 1ULL ) + udiff;
  if ( ans > checkpoint ) {
    if ( ans - checkpoint > ( UINT32_MAX + 1ULL ) / 2 && ans > UINT32_MAX ) {
      ans -= ( UINT32_MAX + 1ULL );
    }
  } else {
    if ( checkpoint - ans > ( UINT32_MAX + 1ULL ) / 2 && ans < UINT64_MAX - UINT32_MAX ) {
      ans += ( UINT32_MAX + 1ULL );
    }
  }
  return ans;
}
