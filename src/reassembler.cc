#include "reassembler.hh"

using namespace std;

#define first_unassembled_index output_.writer().bytes_pushed()
#define first_unacceptable_index ( output_.writer().bytes_pushed() + output_.writer().available_capacity() )

std::string merge_strings( std::string a, uint64_t a_idx, std::string b, uint64_t b_idx )
{
  if ( a_idx + a.size() == b_idx ) {
    return a + b;
  } else if ( a_idx + a.size() > b_idx + b.size() ) {
    return a;
  } else {
    return a + b.substr( a_idx + a.size() - b_idx );
  }
}

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  // put the substring into the buffer
  uint64_t last_index = first_index + data.size();
  if ( is_last_substring ) {
    tail_index = last_index;
    if ( tail_index == first_unassembled_index ) {
      output_.writer().close();
      return;
    }
  }
  if ( last_index <= first_unassembled_index ) {
    return;
  } else if ( first_index < first_unassembled_index ) {
    std::string data_to_buffer
      = data.substr( first_unassembled_index - first_index, first_unacceptable_index - first_unassembled_index );
    if ( buf_.find( first_unassembled_index ) == buf_.end() ) {
      buf_[first_unassembled_index] = data_to_buffer;
    } else {
      if ( data_to_buffer.size() > buf_[first_unassembled_index].size() ) {
        buf_[first_unassembled_index] = data_to_buffer;
      }
    }
  } else if ( first_index >= first_unassembled_index && first_index < first_unacceptable_index ) {
    std::string data_to_buffer = data.substr( 0, first_unacceptable_index - first_index );
    if ( buf_.find( first_index ) == buf_.end() ) {
      buf_[first_index] = data_to_buffer;
    } else {
      if ( data_to_buffer.size() > buf_[first_index].size() ) {
        buf_[first_index] = data_to_buffer;
      }
    }
  } else {
    return;
  }
  // merge and maintain the buffer
  if ( !buf_.empty() ) {
    auto it = buf_.begin();
    while ( it != buf_.end() ) {
      auto it2 = it;
      it2++;
      while ( it2 != buf_.end() ) {
        if ( it->first + it->second.size() >= it2->first ) {
          it->second = merge_strings( it->second, it->first, it2->second, it2->first );
          it2 = buf_.erase( it2 );
        } else
          it2++;
      }
      it++;
    }
  }
  // printf( "Below is the first chunk in the buffer: " );
  // printf( "Index: %lu, Data size: %lu\n", buf_.begin()->first, buf_.begin()->second.size() );
  // see whether we can write to the output
  if ( !buf_.empty() ) {
    auto first_buf = buf_.begin();
    if ( first_buf->first == first_unassembled_index ) {
      output_.writer().push( first_buf->second );
      buf_.erase( first_buf );
    }
  }
  // printf( "Below is the buffer:\n" );
  // for ( auto iit = buf_.begin(); iit != buf_.end(); iit++ ) {
  //   printf( "Index: %lu, Data size: %lu\n", iit->first, iit->second.size() );
  // }
  // printf( "bytes_pushed is:\n" );
  // printf( "%lu\n", output_.writer().bytes_pushed() );
  if ( buf_.empty() && tail_index == first_unassembled_index ) {
    output_.writer().close();
  }
  // Your code here.
  // (void)first_index;
  // (void)data;
  // (void)is_last_substring;
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  // return {};
  uint64_t res = 0;
  for ( auto s : buf_ ) {
    res += s.second.size();
  }
  return res;
}
