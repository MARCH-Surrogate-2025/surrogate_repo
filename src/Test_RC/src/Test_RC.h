#ifndef TEST_RC_H
#define TEST_RC_H

#include "ethercat.h"
#include "EL3068.h"
#include <alchemy/task.h>

class Test_RC
{
public:
    Test_RC();
    void run(const char* ifname);
    void loop();

private:
    static void rt_task_entry(void *arg);
    EL3068 el3068;
};

#endif // TEST_RC_H
