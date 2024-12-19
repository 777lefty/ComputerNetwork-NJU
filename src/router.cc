#include "router.hh"

#include <iostream>
#include <limits>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  // Your code here.
  route_table_.push_back( { route_prefix, prefix_length, next_hop, interface_num } );
}

// Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
void Router::route()
{
  // Your code here.
  for ( auto& interface : _interfaces ) {
    while ( !interface->datagrams_received().empty() ) {
      int8_t max_prefix_length = -1;
      auto interface_num = -1;
      optional<Address> next_hop;
      auto dgram = interface->datagrams_received().front();
      interface->datagrams_received().pop();
      auto dst = dgram.header.dst;
      if ( dgram.header.ttl == 0 ) {
        std::cerr << "TTL is 0, dropping packet\n";
        continue;
      }
      dgram.header.ttl--;
      if ( dgram.header.ttl == 0 ) {
        std::cerr << "TTL is 0, dropping packet\n";
        continue;
      }
      dgram.header.compute_checksum();
      for ( auto& route : route_table_ ) {
        if ( route.prefix_length < max_prefix_length )
          continue;
        int i = -1;
        for ( i = 0; i < route.prefix_length; i++ ) {
          if ( ( dst & ( 1 << ( 31 - i ) ) ) != ( route.route_prefix & ( 1 << ( 31 - i ) ) ) )
            break;
        }
        if ( i == route.prefix_length && route.prefix_length >= max_prefix_length ) {
          max_prefix_length = route.prefix_length;
          next_hop = route.next_hop;
          interface_num = route.interface_num;
        }
      }
      if ( max_prefix_length == -1 ) {
        std::cerr << "No route found, dropping packet\n";
        continue;
      }
      if ( next_hop.has_value() ) {
        _interfaces[interface_num]->send_datagram( dgram, next_hop.value() );
      } else {
        _interfaces[interface_num]->send_datagram( dgram, Address::from_ipv4_numeric( dgram.header.dst ) );
      }
    }
  }
}