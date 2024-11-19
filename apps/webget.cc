#include "socket.hh"

#include <cstdlib>
#include <iostream>
#include <span>
#include <string>

#include "tcp_minnow_socket.hh"

using namespace std;

void get_URL( const string& host, const string& path )
{
  // host: cs144.keithw.org
  // path: /hello
  CS144TCPSocket socket;
  const Address HostAdress = Address( host, "http" ); // resolve the host name
  socket.connect( HostAdress );                       // open the byte stream to the host server

  // now write to the stream as is done in the terminal in the previous parts
  std::string data;
  socket.write( "GET " + path + " HTTP/1.1\r\n" );
  // if ( socket.eof() ) {
  //   cerr << "EOF reached\n";
  // } else {
  //   socket.read( data );
  //   cerr << "Data read: " << data << "\n";
  // }
  socket.write( "Host: " + host + "\r\n" );
  // if ( socket.eof() ) {
  //   cerr << "EOF reached\n";
  // } else {
  //   socket.read( data );
  //   cerr << "Data read: " << data << "\n";
  // }
  socket.write( "Connection: close\r\n" );
  // if ( socket.eof() ) {
  //   cerr << "EOF reached\n";
  // } else {
  //   socket.read( data );
  //   cerr << "Data read: " << data << "\n";
  // }
  socket.write( "\r\n" );
  while ( !socket.eof() ) {
    socket.read( data );
    cout << data;
  }
  socket.close();
  // cerr << "Function called: get_URL(" << host << ", " << path << ")\n";
  // cerr << "Warning: get_URL() has not been implemented yet.\n";
}

int main( int argc, char* argv[] )
{
  try {
    if ( argc <= 0 ) {
      abort(); // For sticklers: don't try to access argv[0] if argc <= 0.
    }

    auto args = span( argv, argc );

    // The program takes two command-line arguments: the hostname and "path" part of the URL.
    // Print the usage message unless there are these two arguments (plus the program name
    // itself, so arg count = 3 in total).
    if ( argc != 3 ) {
      cerr << "Usage: " << args.front() << " HOST PATH\n";
      cerr << "\tExample: " << args.front() << " stanford.edu /class/cs144\n";
      return EXIT_FAILURE;
    }

    // Get the command-line arguments.
    const string host { args[1] };
    const string path { args[2] };

    // Call the student-written function.
    get_URL( host, path );
  } catch ( const exception& e ) {
    cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
