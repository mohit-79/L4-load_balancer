#pragma once

#include "task.hpp"

class LoadBalancerTask : public EXECUTOR::Task
{
public:
    explicit LoadBalancerTask(int client_fd);

    void execute(int worker_id) override;

private:
    int client_fd;
};
