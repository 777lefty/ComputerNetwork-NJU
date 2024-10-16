#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  // Your code here.
  // (void)message;
  if ( message.RST ) {
    reader().set_error();
  }
  if ( ( !message.SYN && !syn_ ) || ( message.SYN && syn_ ) ) {
    return;
  }
  if ( message.SYN && !syn_ ) {
    syn_ = true;
    seqno_ = message.seqno;
  }
  uint64_t absolute_seqno = message.seqno.unwrap( seqno_, writer().bytes_pushed() );
  uint64_t index = absolute_seqno - !message.SYN;
  reassembler_.insert( index, message.payload, message.FIN );
}

TCPReceiverMessage TCPReceiver::send() const
{
  // Your code here.
  TCPReceiverMessage message;
  if ( reader().has_error() ) {
    message.RST = true;
  }
  uint64_t window_size = writer().available_capacity();
  message.window_size = window_size <= UINT16_MAX ? window_size : UINT16_MAX;
  uint64_t first_unassembled = writer().bytes_pushed();
  if ( !syn_ ) {
    message.ackno = {};
  } else {
    uint64_t absolute_seqno = first_unassembled + 1 + writer().is_closed();
    message.ackno = Wrap32::wrap( absolute_seqno, seqno_ );
  }
  return message;
}
