// #include "notifier.hpp"
// #include "thread_pool.hpp"
// #include "load_balancer_task.hpp"

// #include <arpa/inet.h>
// #include <fcntl.h>
// #include <sys/socket.h>
// #include <unistd.h>

// int main()
// {
//     EXECUTOR::ThreadPool pool(4);

//     REACTOR::EventNotifier notifier;

//     int server_fd =
//     socket(AF_INET,
//            SOCK_STREAM,
//            0);

//     sockaddr_in addr{};
//     addr.sin_family = AF_INET;
//     addr.sin_port = htons(9000);
//     addr.sin_addr.s_addr = INADDR_ANY;

//     bind(server_fd,
//          (sockaddr*)&addr,
//          sizeof(addr));

//     listen(server_fd, 128);

//     notifier.add_fd(server_fd);

//     while (true)
//     {
//         auto events =
//         notifier.wait_for_events();

//         for (auto &ev : events)
//         {
//             if (ev.fd == server_fd)
//             {
//                 sockaddr_in caddr{};
//                 socklen_t len =
//                 sizeof(caddr);

//                 int client_fd =
//                 accept(server_fd,
//                        (sockaddr*)&caddr,
//                        &len);

//                 auto task =
//                 std::make_unique<
//                 LoadBalancerTask>(
//                     client_fd);

//                 pool.enqueue_task(
//                     std::move(task));
//             }
//         }
//     }
// }
#include "notifier.hpp"
#include "thread_pool.hpp"
#include "load_balancer_task.hpp"
#include "metrics.hpp"

#include <thread>
#include <chrono>
#include <string>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
void telemetry_thread()
{
    std::cout << "Telemetry thread started\n";
    int udp_sock =
        socket(AF_INET,
               SOCK_DGRAM,
               0);

    sockaddr_in addr{};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(5001);

    inet_pton(
        AF_INET,
        "127.0.0.1",
        &addr.sin_addr
    );

    while (true)
    {
        std::string json =
            "{\"total\":"
            + std::to_string(total_requests.load())
            + ",\"backend1\":"
            + std::to_string(backend1_hits.load())
            + ",\"backend2\":"
            + std::to_string(backend2_hits.load())
            + "}";
        std::cout
    << "UDP SEND: "
    << json
    << std::endl;
        sendto(
            udp_sock,
            json.c_str(),
            json.size(),
            0,
            (sockaddr*)&addr,
            sizeof(addr)
        );

        std::this_thread::sleep_for(
            std::chrono::milliseconds(500)
        );
    }
}
int main()
{
    EXECUTOR::ThreadPool pool( std::thread::hardware_concurrency()
    );
    std::thread telemetry(
    telemetry_thread
);

telemetry.detach();
    REACTOR::EventNotifier notifier;

    int server_fd =
        socket(
            AF_INET,
            SOCK_STREAM,
            0
        );

    if (server_fd < 0)
    {
        std::cerr << "socket failed\n";
        return 1;
    }

    int opt = 1;

    setsockopt(
        server_fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        &opt,
        sizeof(opt)
    );

    sockaddr_in addr{};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(9000);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (
        bind(
            server_fd,
            (sockaddr*)&addr,
            sizeof(addr)
        ) < 0
    )
    {
        std::cerr << "bind failed\n";
        return 1;
    }

    listen(server_fd, 128);

    notifier.add_fd(server_fd);

    std::cout
        << "Load balancer listening on :9000\n"<< std::thread::hardware_concurrency();

    while (true)
    {
        auto events =
            notifier.wait_for_events();

        for (auto& ev : events)
        {
            if (ev.fd != server_fd)
                continue;

            sockaddr_in client_addr{};
            socklen_t len =
                sizeof(client_addr);

            int client_fd =
                accept(
                    server_fd,
                    (sockaddr*)&client_addr,
                    &len
                );

            if (client_fd < 0)
                continue;

            auto task =
                std::make_unique<
                    LoadBalancerTask
                >(client_fd);

            pool.enqueue_task(
                std::move(task)
            );
        }
    }

    return 0;
}