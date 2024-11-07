#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

bool Writer::is_closed() const
{
  // Your code here.
  return close_flag_;
}

void Writer::push( string data )
{
  // Your code here.
  //(void)data;
  if ( is_closed() || data.empty() ) {
    return;
  }
  const uint64_t len_to_push = min( available_capacity(), data.size() );
  const std::string bytes_to_push = data.substr( 0, len_to_push );
  bytes_.append( bytes_to_push );
  num_bytes_pushed_ += len_to_push;
  num_bytes_ += len_to_push;
  return;
}

void Writer::close()
{
  // Your code here.
  close_flag_ = true;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - num_bytes_;
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return num_bytes_pushed_;
}

bool Reader::is_finished() const
{
  // Your code here.
  return close_flag_ && bytes_buffered() == 0;
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return num_bytes_popped_;
}

string_view Reader::peek() const
{
  // Your code here.
  // return {};
  return std::string_view( bytes_ );
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  //(void)len;
  if ( bytes_buffered() == 0 ) {
    return;
  }
  const uint64_t len_to_pop = min( len, bytes_buffered() );
  num_bytes_popped_ += len_to_pop;
  num_bytes_ -= len_to_pop;
  bytes_ = bytes_.erase( 0, len_to_pop );
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return num_bytes_;
}
