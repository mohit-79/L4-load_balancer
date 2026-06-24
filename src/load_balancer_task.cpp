#include "load_balancer_task.hpp"
#include "metrics.hpp"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <atomic>

LoadBalancerTask::LoadBalancerTask(int fd)
    : client_fd(fd)
{
}

void LoadBalancerTask::execute(int worker_id)
{
    std::cout << "METRICS VERSION LOADED" << std::endl;
    static std::atomic<int> rr{0};

    int backend_port =
        (rr.fetch_add(1) % 2 == 0)
        ? 8081
        : 8082;

    std::cout
        << "[Worker " << worker_id
        << "] Backend port: "
        << backend_port
        << std::endl;
    total_requests++;
    int backend_fd =
        socket(AF_INET, SOCK_STREAM, 0);

    if (backend_fd < 0)
    {
        std::cerr << "Failed to create backend socket\n";
        close(client_fd);
        return;
    }

    sockaddr_in backend_addr{};
    backend_addr.sin_family = AF_INET;
    backend_addr.sin_port = htons(backend_port);

    inet_pton(
        AF_INET,
        "127.0.0.1",
        &backend_addr.sin_addr
    );

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

        close(client_fd);
        close(backend_fd);
        return;
    }
    if (backend_port == 8081)
    backend1_hits++;
else
    backend2_hits++;
    std::cout
    << "Total=" << total_requests
    << " B1=" << backend1_hits
    << " B2=" << backend2_hits
    << std::endl;

    char buffer[8192];

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

    while (true)
    {
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

        if (bytes <= 0)
            break;

        chunk++;

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

    close(client_fd);
    close(backend_fd);
}

