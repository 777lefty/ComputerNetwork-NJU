#include <iostream>

#include "arp_message.hh"
#include "exception.hh"
#include "network_interface.hh"

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( string_view name,
                                    shared_ptr<OutputPort> port,
                                    const EthernetAddress& ethernet_address,
                                    const Address& ip_address )
  : name_( name )
  , port_( notnull( "OutputPort", move( port ) ) )
  , ethernet_address_( ethernet_address )
  , ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address ) << " and IP address "
       << ip_address.ip() << "\n";
}

EthernetFrame NetworkInterface::create_frame( const EthernetAddress& src_ethernet_address,
                                              const EthernetAddress& dest_ethernet_address,
                                              const EthernetType type,
                                              std::vector<std::string> data )
{
  EthernetFrame frame;
  frame.header.src = src_ethernet_address;
  frame.header.dst = dest_ethernet_address;
  frame.header.type = type;
  frame.payload = std::move( data );
  return frame;
}

ARPMessage NetworkInterface::create_arp_request( const uint16_t opcode,
                                                 const EthernetAddress sender_ethernet_address,
                                                 const uint32_t sender_ip_address,
                                                 const EthernetAddress target_ethernet_address,
                                                 const uint32_t target_ip_address )
{
  ARPMessage arp;
  arp.opcode = opcode;
  arp.sender_ethernet_address = sender_ethernet_address;
  arp.sender_ip_address = sender_ip_address;
  arp.target_ethernet_address = target_ethernet_address;
  arp.target_ip_address = target_ip_address;
  return arp;
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but
//! may also be another host if directly connected to the same network as the destination) Note: the Address type
//! can be converted to a uint32_t (raw 32-bit IP address) by using the Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  // Your code here.
  // (void)dgram;
  // (void)next_hop;
  auto next_hop_ip_address = next_hop.ipv4_numeric();
  auto it = arp_table_.find( next_hop_ip_address );
  if ( it != arp_table_.end() ) {
    transmit( create_frame(
      ethernet_address_, it->second.ethernet_address, EthernetHeader::TYPE_IPv4, serialize( dgram ) ) );
  } else {
    auto wait_it = waiting_datagrams_.find( next_hop_ip_address );
    if ( wait_it == waiting_datagrams_.end() ) {
      waiting_datagrams_[next_hop_ip_address] = queue<InternetDatagram>();
    }
    waiting_datagrams_[next_hop_ip_address].push( dgram );
    if ( arp_requests_sent_.contains( next_hop_ip_address ) ) {
      return;
    }
    auto arp = create_arp_request(
      ARPMessage::OPCODE_REQUEST, ethernet_address_, ip_address_.ipv4_numeric(), {}, next_hop_ip_address );
    transmit( create_frame( ethernet_address_, ETHERNET_BROADCAST, EthernetHeader::TYPE_ARP, serialize( arp ) ) );
    arp_requests_sent_.insert( next_hop_ip_address );
    arp_request_expire_time_[next_hop_ip_address] = timer_ + 5000;
  }
}

//! \param[in] frame the incoming Ethernet frame
void NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  // Your code here.
  // (void)frame;
  if ( frame.header.dst != ethernet_address_ && frame.header.dst != ETHERNET_BROADCAST ) {
    return;
  }
  if ( frame.header.type == EthernetHeader::TYPE_IPv4 ) {
    InternetDatagram dgram;
    auto res = parse( dgram, frame.payload );
    if ( res ) {
      datagrams_received_.push( dgram );
    }
  } else if ( frame.header.type == EthernetHeader::TYPE_ARP ) {
    ARPMessage arpmsg;
    auto res = parse( arpmsg, frame.payload );
    if ( !res ) {
      return;
    }
    arp_table_[arpmsg.sender_ip_address] = { arpmsg.sender_ethernet_address, timer_ + 30000 };
    if ( arpmsg.opcode == ARPMessage::OPCODE_REQUEST && arpmsg.target_ip_address == ip_address_.ipv4_numeric() ) {
      auto arp = create_arp_request( ARPMessage::OPCODE_REPLY,
                                     ethernet_address_,
                                     ip_address_.ipv4_numeric(),
                                     arpmsg.sender_ethernet_address,
                                     arpmsg.sender_ip_address );
      transmit( create_frame(
        ethernet_address_, arpmsg.sender_ethernet_address, EthernetHeader::TYPE_ARP, serialize( arp ) ) );
    }
    if ( waiting_datagrams_.contains( arpmsg.sender_ip_address ) ) {
      auto& q = waiting_datagrams_[arpmsg.sender_ip_address];
      while ( !q.empty() ) {
        auto datagram = q.front();
        transmit( create_frame(
          ethernet_address_, arpmsg.sender_ethernet_address, EthernetHeader::TYPE_IPv4, serialize( datagram ) ) );
        q.pop();
      }
      waiting_datagrams_.erase( arpmsg.sender_ip_address );
    }
  }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  // (void)ms_since_last_tick;
  timer_ += ms_since_last_tick;
  for ( auto it = arp_table_.begin(); it != arp_table_.end(); ) {
    if ( it->second.expire_time <= timer_ ) {
      it = arp_table_.erase( it );
    } else {
      ++it;
    }
  }
  for ( auto it = arp_request_expire_time_.begin(); it != arp_request_expire_time_.end(); ) {
    if ( it->second <= timer_ ) {
      arp_requests_sent_.erase( it->first );
      it = arp_request_expire_time_.erase( it );
    } else {
      ++it;
    }
  }
}