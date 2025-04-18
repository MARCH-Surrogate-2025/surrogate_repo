//
// Created by surrogate on 24. 7. 26.
//

#include "hello_xenomai.h"

void demo(void *arg)
{
    RT_TASK_INFO curtaskinfo;
    rt_task_inquire(&demo_task, &curtaskinfo);
    printf("Task name : %s \n", curtaskinfo.name);
}

int main(int argc, char* argv[])
{
    char *str = "demo";

    printf("start task\n");

    rt_task_create(&demo_task, str, 0, 50, 0);
    rt_task_start(&demo_task, &demo, 0);

    sleep(1);
}