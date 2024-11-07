#include "tcp_sender.hh"
#include "tcp_config.hh"
#include <iostream>

using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  // return {};
  uint64_t res = 0;
  for ( auto msg : unack_buffer_ ) {
    res += msg.sequence_length();
  }
  return res;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  // return {};
  return retransmission_cnt;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  // Your code here.
  // (void)transmit;
  uint64_t remain_space = ( windows_size_ == 0 ) ? 1 : windows_size_;
  if ( remain_space <= sequence_numbers_in_flight() ) {
    return;
  }
  remain_space -= sequence_numbers_in_flight();
  while ( remain_space > 0 ) {
    TCPSenderMessage message = make_empty_message();
    if ( !syn_sent_ ) {
      message.SYN = true;
      syn_sent_ = true;
      remain_space--;
    }
    message.seqno = Wrap32::wrap( first_unacked_, isn_ );
    read( input_.reader(), min( TCPConfig::MAX_PAYLOAD_SIZE, remain_space ), message.payload );
    remain_space -= message.payload.size();
    if ( !fin_sent_ && reader().is_finished() && remain_space > 0 ) {
      message.FIN = true;
      fin_sent_ = true;
      remain_space--;
    }
    if ( message.sequence_length() == 0 ) {
      break;
    }
    unack_buffer_.push_back( message );
    transmit( message );
    first_unacked_ += message.sequence_length();
  }
  if ( !timer_start_ && sequence_numbers_in_flight() != 0 ) {
    current_time_ = 0;
    current_RTO_ = initial_RTO_ms_;
    timer_start_ = true;
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  // Your code here.
  // return {};
  TCPSenderMessage message;
  message.seqno = Wrap32::wrap( first_unacked_, isn_ );
  message.SYN = false;
  message.FIN = false;
  message.RST = input_.has_error();
  return message;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  // (void)msg;
  if ( msg.RST ) {
    input_.set_error();
  }
  windows_size_ = msg.window_size;
  if ( !msg.ackno.has_value() ) {
    return;
  }
  uint64_t received_ack = msg.ackno.value().unwrap( isn_, first_unacked_ );
  if ( received_ack > first_unacked_ ) {
    return;
  }
  bool fully_acked = false;
  for ( auto it = unack_buffer_.begin(); it != unack_buffer_.end(); it++ ) {
    if ( it->seqno.unwrap( isn_, first_unacked_ ) + it->sequence_length() <= received_ack ) {
      it = unack_buffer_.erase( it );
      it--;
      fully_acked = true;
    } else {
      break;
    }
  }
  if ( fully_acked ) {
    retransmission_cnt = 0;
    current_time_ = 0;
    timer_start_ = false;
    current_RTO_ = initial_RTO_ms_;
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  // Your code here.
  // (void)ms_since_last_tick;
  // (void)transmit;
  // (void)initial_RTO_ms_;
  current_time_ += ms_since_last_tick;
  if ( timer_start_ && current_time_ >= current_RTO_ ) {
    transmit( unack_buffer_.front() );
    if ( windows_size_ > 0 ) {
      retransmission_cnt++;
      current_RTO_ = current_RTO_ * 2;
    }
    current_time_ = 0;
    timer_start_ = true;
  }
}