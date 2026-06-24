// ============================================================================
// HEADER INCLUDES
// ============================================================================

// Custom components
#include "notifier.hpp"           // For REACTOR::EventNotifier (monitors socket events)
#include "thread_pool.hpp"        // For EXECUTOR::ThreadPool (manages worker threads)
#include "load_balancer_task.hpp" // For LoadBalancerTask (encapsulates client request logic)
#include "metrics.hpp"            // For tracking global metrics (total_requests, backend hits)

// Standard library headers
#include <thread>                 // For std::thread and pausing execution via sleep_for
#include <chrono>                 // For defining time intervals (e.g., std::chrono::milliseconds)
#include <string>                 // For using dynamic string objects (std::string)
#include <iostream>               // For standard I/O streams (std::cout, std::cerr, std::endl)

// System and Networking headers
#include <arpa/inet.h>            // For IP address conversion utilities (inet_pton, htons)
#include <sys/socket.h>           // For core socket APIs (socket, bind, listen, accept, sendto)
#include <unistd.h>               // For low-level system operations (close)

// ============================================================================
// TELEMETRY BACKGROUND WORKER
// ============================================================================

void telemetry_thread()
{
    std::cout << "Telemetry thread started\n";
    
    /*
     * SYNTAX: int socket(int domain, int type, int protocol);
     * OPTIONS & PARAMETERS:
     * - domain:   AF_INET     -> Specifies IPv4 Internet protocols
     * AF_INET6    -> Specifies IPv6 Internet protocols
     * AF_UNIX     -> Specifies Local communication (Unix domain sockets)
     * - type:     SOCK_DGRAM  -> Supports datagrams (connectionless, unreliable UDP messages)
     * SOCK_STREAM -> Supports sequenced, reliable, connection-based byte streams (TCP)
     * - protocol: 0           -> Automatically selects the default protocol matching the domain and type (UDP here)
     */
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);

    // Configure the target address details (localhost:5001)
    sockaddr_in addr{};
    
    // We select AF_INET here because our socket was initialized to use IPv4 protocol families.
    addr.sin_family = AF_INET;
    
    // Convert port number 5001 from host byte order to network byte order (big-endian)
    addr.sin_port = htons(5001);
    
    /*
     * SYNTAX: int inet_pton(int af, const char *src, void *dst);
     * OPTIONS & PARAMETERS:
     * - af:  AF_INET      -> Parse an IPv4 dotted-decimal string address
     * AF_INET6     -> Parse an IPv6 hexadecimal string address
     * - src: "127.0.0.1"  -> The presentation string format IP to be converted
     * - dst: &addr.sin_addr -> Pointer to the network bytes structure where result is saved
     */
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    // Periodically broadcast metrics JSON
    while (true)
    {
        // Construct JSON payload using atomic metric counters
        std::string json =
            "{\"total\":"
            + std::to_string(total_requests.load())
            + ",\"backend1\":"
            + std::to_string(backend1_hits.load())
            + ",\"backend2\":"
            + std::to_string(backend2_hits.load())
            +",\"active1\":"
            + std::to_string(backend1_active.load())
            +",\"active2\":"
            + std::to_string(backend2_active.load())
            + "}";
            
        std::cout << "UDP SEND: " << json << std::endl;
        
        /*
         * SYNTAX: ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
         * PARAMETERS:
         * - sockfd:    udp_sock         -> Descriptor of the sending UDP socket
         * - buf:       json.c_str()     -> Pointer to the raw byte buffer to send
         * - len:       json.size()      -> Number of bytes to send from the buffer
         * - flags:     0                -> Message flags (0 means standard default transmission)
         * - dest_addr: (sockaddr*)&addr -> Target destination endpoint configuration 
         * - addrlen:   sizeof(addr)     -> Size in bytes of the destination address structure
         */
        sendto(
            udp_sock,
            json.c_str(),
            json.size(),
            0,
            (sockaddr*)&addr,
            sizeof(addr)
        );

        // Throttle telemetry updates to every 500ms
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

// ============================================================================
// MAIN APPLICATION ROUTINE
// ============================================================================

int main()
{
    // Initialize the Thread Pool based on system hardware concurrency
    EXECUTOR::ThreadPool pool(std::thread::hardware_concurrency());
    
    // Spawn and detach the background telemetry thread
    std::thread telemetry(telemetry_thread);
    telemetry.detach();
    
    // Initialize the Reactor pattern event notifier
    REACTOR::EventNotifier notifier;

    // ------------------------------------------------------------------------
    // SERVER SOCKET SETUP & CONFIGURATION
    // ------------------------------------------------------------------------
    
    /*
     * SYNTAX: int socket(int domain, int type, int protocol);
     * OPTIONS & PARAMETERS:
     * - domain:   AF_INET     -> Specifies IPv4 Internet protocols
     * - type:     SOCK_STREAM -> Supports sequenced, reliable, two-way connection-based byte streams (TCP)
     * - protocol: 0           -> Automatically selects the default protocol matching domain/type (TCP here)
     */
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        std::cerr << "socket failed\n";
        return 1;
    }

    // Set SO_REUSEADDR to allow immediate reuse of the port after restart
    int opt = 1;
    
    /*
     * SYNTAX: int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
     * PARAMETERS:
     * - sockfd:  server_fd    -> Socket file descriptor to modify
     * - level:   SOL_SOCKET   -> Interpret option at the core socket API layer
     * - optname: SO_REUSEADDR -> Allow local address reuse (bypasses TIME_WAIT state)
     * - optval:  &opt         -> Pointer to the setting value (1 = Enabled)
     * - optlen:  sizeof(opt)  -> Size of the variable holding the option state
     */
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind socket to all available interfaces on Port 9000
    sockaddr_in addr{};
    
    // We select AF_INET here because our listening socket expects IPv4 address families.
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9000);
    addr.sin_addr.s_addr = INADDR_ANY;

    /*
     * SYNTAX: int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
     * PARAMETERS:
     * - sockfd:  server_fd       -> The socket to assign the address configuration to
     * - addr:    (sockaddr*)&addr-> Pointer to address configuration structure containing IP/Port
     * - addrlen: sizeof(addr)    -> Size in bytes of the address structure passed
     */
    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0)
    {
        std::cerr << "bind failed\n";
        return 1;
    }

    /*
     * SYNTAX: int listen(int sockfd, int backlog);
     * PARAMETERS:
     * - sockfd:  server_fd -> Socket file descriptor to mark as passive listener
     * - backlog: 128       -> Maximum length of the queue of pending unaccepted connections
     */
    listen(server_fd, 128);

    // Register the server socket to monitor for read/connection events
    notifier.add_fd(server_fd);

    std::cout << "Load balancer listening on :9000\n" << std::thread::hardware_concurrency();

    // ------------------------------------------------------------------------
    // MAIN EVENT LOOP
    // ------------------------------------------------------------------------
    
    while (true)
    {
        // Block and wait until network events occur
        auto events = notifier.wait_for_events();

        for (auto& ev : events)
        {
            // Only process events related to our main server listening socket
            if (ev.fd != server_fd)
                continue;

            // Accept the incoming connection
            sockaddr_in client_addr{};
            socklen_t len = sizeof(client_addr);
            
            /*
             * SYNTAX: int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
             * PARAMETERS:
             * - sockfd:  server_fd          -> Passive listening socket descriptor
             * - addr:    (sockaddr*)&client_addr -> Pointer to buffer that receives connected client's identity
             * - addrlen: &len               -> Input/Output parameter; passes buffer size, returns exact address size
             */
            int client_fd = accept(server_fd, (sockaddr*)&client_addr, &len);

            if (client_fd < 0)
                continue;

            // Package the socket connection into a Task
            auto task = std::make_unique<LoadBalancerTask>(client_fd);

            // Hand off the connection task to the Thread Pool workers
            pool.enqueue_task(std::move(task));
        }
    }

    return 0;
}