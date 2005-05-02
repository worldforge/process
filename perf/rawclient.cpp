/* Simple Atlas-C++ Client
 *
 * Part of the Atlas-C++ Tutorial
 *
 * Copyright 2000 Stefanus Du Toit.
 * Copyright 2005 Al Riddoch.
 *
 * This file is covered by the GNU General Public License
 */

// Atlas negotiation
#include <Atlas/Net/Stream.h>
#include <Atlas/Codecs/Packed.h>

// tcp_socket_stream - the iostream-based socket class
#include <skstream/skstream.h>

// cout, cerr
#include <iostream>
#include <sstream>

#include <map>
#include <list>

// sleep()
#include <sys/select.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

class NullBridge : public Atlas::Bridge {
  public:
    void streamBegin() { }
    void streamMessage() { }
    void streamEnd() { }

    void mapMapItem(const std::string& name) { }
    void mapListItem(const std::string& name) { }
    void mapIntItem(const std::string& name, long) { }
    void mapFloatItem(const std::string& name, double) { }
    void mapStringItem(const std::string& name, const std::string&) { }
    void mapEnd() { }

    void listMapItem() { }
    void listListItem() { }
    void listIntItem(long) { }
    void listFloatItem(double) { }
    void listStringItem(const std::string&) { }
    void listEnd() { }
};

char buf[256];
unsigned int in_count = 0,
             out_count = 0;

int poll(int socket, Atlas::Codec & c)
{
    fd_set ifds;
    struct timeval timeout = { 0, 0 };

    FD_ZERO(&ifds);
    FD_SET(socket, &ifds);

    if (select(socket + 1, &ifds, 0, 0, &timeout) > 0) {
#if 0
        c.poll();
#else
        int count;
        do {
            count = read(socket, buf, 256);
            in_count += count;
            if (count == 0) {
                return -1;
            }
        } while (count == 256);
#endif
    }
    return 0;
}

// This sends a very simple message to c
void helloWorld(Atlas::Codec & c)
{
    c.streamMessage();
    c.mapStringItem("objtype", "op");
    c.mapListItem("parents");
    c.listStringItem("get");
    c.listEnd();
    c.mapEnd();
}

int getTime()
{
    struct timeval tv;
    gettimeofday(&tv, 0);

    return tv.tv_usec / 1000 + tv.tv_sec * 1000;
}

int main(int argc, char** argv)
{
    signal(SIGPIPE, SIG_IGN);

    // The socket that connects us to the server
    tcp_socket_stream connection;

    std::cout << "Connecting..." << std::flush;
    
    // Connect to the server
    if(argc>1) {
      connection.open(argv[1], 6767);
    } else {
      connection.open("127.0.0.1", 6767);
    }
    
    // The DebugBridge puts all that comes through the codec on cout
    NullBridge bridge;
    // Do client negotiation with the server
    Atlas::Net::StreamConnect conn("simple_client", connection, bridge);

    std::cout << "Negotiating... " << std::flush;
    // conn.poll() does all the negotiation
    while (conn.getState() == Atlas::Net::StreamConnect::IN_PROGRESS) {
        conn.poll();
    }
    std::cout << "done" << std::endl;

    // Check whether negotiation was successful
    if (conn.getState() == Atlas::Net::StreamConnect::FAILED) {
        std::cerr << "Failed to negotiate" << std::endl;
        return 2;
    }
    // Negotiation was successful

    // Get the codec that negotiation established
    Atlas::Codec * codec = conn.getCodec();

    if (dynamic_cast<Atlas::Codecs::Packed *>(codec) == 0) {
        std::cerr << "We are not talking to the server using Packed."
                  << std::endl << std::flush;
        return 1;
    }

    std::stringstream buffer;
    NullBridge bridge2;
    Atlas::Codecs::Packed packed(buffer, bridge2);
    packed.streamBegin();
    helloWorld(packed);
    std::string message(buffer.str());
    const char * msg_buf = message.c_str();
    size_t msg_len = message.size();

    // This should always be sent at the beginning of a session
    codec->streamBegin();

    int start_time = getTime();
    
    // Say hello to the server
    int i;
    for (i = 0; i < 50000; ++i) {
        if (poll(connection.getSocket(), *codec)) {
            std::cout << "HUPPED" << std::endl << std::flush;
            break;
        }
        // helloWorld(*codec);
        int count = write(connection.getSocket(), msg_buf, msg_len);
        if (count > 0) {
            out_count += count;
        }
        connection << std::flush;
    }

    int end_time = getTime();
    int elapsed = end_time - start_time;
    
    std::cout << "Wrote " << out_count << " bytes to server and got back "
              << in_count << " bytes in " << elapsed << " milliseconds."
              << std::endl << std::flush;

    std::cout << "Server accepted " << (i * 1000) / elapsed
              << " ops per second" << std::endl;

    std::cout << "Server handled " << (out_count * 1000) / elapsed
              << " bytes per second" << std::endl
              << "Server returned " << (in_count * 1000) / elapsed
              << " bytes per second" << std::endl << std::flush;

    // iosockinet::operator bool() returns false if the connection was broken
    if (!connection) {
        std::cout << "Server exited." << std::endl;
    } else {
        // It was not broken by the server, so we'll close ourselves
        std::cout << "Closing connection... " << std::flush;
        // This should always be sent at the end of a session
        codec->streamEnd();
        connection << std::flush;
        // Close the socket
        connection.close();
        std::cout << "done." << std::endl;
    }

    return 0;
}
