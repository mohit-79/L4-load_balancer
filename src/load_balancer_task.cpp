#include "load_balancer_task.hpp"
#include "metrics.hpp"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <atomic>
#include <backend_pool.h>

// ============================================================================
// CONSTRUCTOR
// ============================================================================

LoadBalancerTask::LoadBalancerTask(int fd)
    : client_fd(fd)
{
}

// ============================================================================
// TASK EXECUTION ROUTINE
// ============================================================================

void LoadBalancerTask::execute(int worker_id)
{
    std::cout << "METRICS VERSION LOADED" << std::endl;
    static std::atomic<int> rr{0}; // rr: Round-robin counter (Thread-safe)

    // Select routing port based on counter parity
    // int backend_port;

    // if (backend1_active.load() <= backend2_active.load())
    // {
    //     backend_port = 8081;
    //     backend1_active++;
    // }
    // else
    // {
    //     backend_port = 8082;
    //     backend2_active++;
    // }
    int index= rr++ % backends.size();
    int backend_port =
    backends[
        index //thread-safe increment
    ];

backend_hits[index]++;
    std::cout
<< "backend_hits[" << index << "] = "
<< backend_hits[index].load()
<< std::endl;
        

    std::cout
        << "[Worker " << worker_id
        << "] Backend port: "
        << backend_port
        << std::endl;
        
    total_requests++; // Increment total global request counter
    
    // SYNTAX: int socket(int domain, int type, int protocol);
    // - domain:   AF_INET     -> IPv4
    // - type:     SOCK_STREAM -> TCP
    // - protocol: 0           -> Default
    int backend_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (backend_fd < 0)
    {
        std::cerr << "Failed to create backend socket\n";
        close(client_fd); // close: Closes descriptor
        return;
    }

    sockaddr_in backend_addr{};
    backend_addr.sin_family = AF_INET; // AF_INET: IPv4-addressing
    backend_addr.sin_port = htons(backend_port); // htons: Host-to-network-byte-order

    // SYNTAX: int inet_pton(int af, const char *src, void *dst);
    // - af:  AF_INET     -> IPv4-parsing
    // - src: "127.0.0.1" -> Localhost-string
    // - dst: &backend_addr.sin_addr -> Destination-buffer
    inet_pton(
        AF_INET,
        "127.0.0.1",
        &backend_addr.sin_addr
    );

    // SYNTAX: int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    // - sockfd:  backend_fd    -> Active-socket
    // - addr:    &backend_addr -> Destination-endpoint
    // - addrlen: sizeof(...)   -> Structure-size
    if (
        connect(
            backend_fd,
            (sockaddr*)&backend_addr,
            sizeof(backend_addr)
        ) < 0
    )
    {
        std::cerr
            << "Connect failed to backend "
            << backend_port
            << std::endl;
        // if (backend_port == 8081)
            // backend1_active--;
        // else
            // backend2_active--;
        close(client_fd); // Cleanup client
        close(backend_fd); // Cleanup backend
        return;
    }
    
    // Track target hit distribution
    // backend_active[index]--; // Increment hit counter for the selected backend
        
   std::cout
<< "Total=" << total_requests;

for(size_t i = 0; i < backends.size(); i++)
{
    std::cout
        << " B"
        << (i + 1)
        << "="
        << backend_hits[i];
}

std::cout << std::endl;

    char buffer[8192]; // Fixed packet buffer size

    // SYNTAX: ssize_t recv(int sockfd, void *buf, size_t len, int flags);
    // - sockfd: client_fd -> Source-descriptor
    // - buf:    buffer    -> Storage-array
    // - len:    sizeof(...) -> Limit-bounds
    // - flags:  0         -> Standard-read
    ssize_t bytes =
        recv(
            client_fd,
            buffer,
            sizeof(buffer),
            0
        );

    std::cout
        << "CLIENT RECV = "
        << bytes
        << " bytes"
        << std::endl;

    if (bytes <= 0)
    {
        close(client_fd);
        close(backend_fd);
        return;
    }

    // SYNTAX: ssize_t send(int sockfd, const void *buf, size_t len, int flags);
    // - sockfd: backend_fd -> Destination-descriptor
    // - buf:    buffer     -> Payload-pointer
    // - len:    bytes      -> Dynamic-count
    // - flags:  0          -> Standard-write
    ssize_t sent =
        send(
            backend_fd,
            buffer,
            bytes,
            0
        );

    std::cout
        << "CLIENT -> BACKEND = "
        << sent
        << " bytes"
        << std::endl;

    int chunk = 0;

    // Stream server response payload loop
    while (true)
    {
        // Intercept data streams returning from backend node
        bytes =
            recv(
                backend_fd,
                buffer,
                sizeof(buffer),
                0
            );

        std::cout
            << "BACKEND RECV = "
            << bytes
            << " bytes"
            << std::endl;

        if (bytes <= 0) // Break loop when data stream terminates
            break;

        chunk++;

        // Return processed backend stream packets to initiating user endpoint
        sent =
            send(
                client_fd,
                buffer,
                bytes,
                0
            );

        std::cout
            << "Chunk "
            << chunk
            << " -> CLIENT = "
            << sent
            << " bytes"
            << std::endl;
    }

    std::cout
        << "Connection closed"
        << std::endl;

    close(client_fd); // Reclaim client resource
    close(backend_fd); // Reclaim backend resource
    if (backend_port == 8081)
        backend1_active--;
    else
        backend2_active--;
}