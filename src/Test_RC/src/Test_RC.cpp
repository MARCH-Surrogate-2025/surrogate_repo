#include <cstdio>
#include <cstring>
#include <alchemy/task.h>
#include <alchemy/timer.h>
#include "ethercat.h"

char IOmap[4096];
OSAL_THREAD_HANDLE thread1;
int expectedWKC;
volatile int wkc;
boolean inOP = FALSE;
uint8 currentgroup = 0;

RT_TASK ecat_task;

void ecat_loop(void *arg)
{
    rt_task_set_periodic(NULL, TM_NOW, 1000000); // 1ms 주기

    while (1)
    {
        ec_send_processdata();
        wkc = ec_receive_processdata(EC_TIMEOUTRET);

        if (wkc >= expectedWKC)
        {
            uint8_t *ai_data = ec_slave[2].inputs;
            printf("ai_data raw dump: ");
            for (int i = 0; i < 12; i++)
            {
                printf("%02X ", ai_data[i]);
            }
            printf("\n");

            int16_t ch1 = *(int16_t *)(ai_data + 2);
            int16_t ch2 = *(int16_t *)(ai_data + 6);
            int16_t ch3 = *(int16_t *)(ai_data + 10);
            int16_t ch4 = *(int16_t *)(ai_data + 14);
            int16_t ch5 = *(int16_t *)(ai_data + 18);
            int16_t ch6 = *(int16_t *)(ai_data + 22);

            printf("CH1: %d | CH2: %d | CH3: %d\n", ch1, ch2, ch3);
        }

        rt_task_wait_period(NULL);
    }
}

int main(int argc, char *argv[])
{
    printf("SOEM (Simple Open EtherCAT Master) - Xenomai Version\n");

    if (argc > 1)
    {
        if (ec_init(argv[1]))
        {
            printf("ec_init on %s succeeded.\n", argv[1]);

            if (ec_config_init(FALSE) > 0)
            {
                printf("%d slaves found and configured.\n", ec_slavecount);

                ec_config_map(&IOmap);
                ec_configdc();

                expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
                printf("Calculated expectedWKC: %d\n", expectedWKC);

                ec_slave[0].state = EC_STATE_OPERATIONAL;
                ec_send_processdata();
                ec_receive_processdata(EC_TIMEOUTRET);
                ec_writestate(0);

                int chk = 200;
                do
                {
                    ec_send_processdata();
                    ec_receive_processdata(EC_TIMEOUTRET);
                    ec_statecheck(0, EC_STATE_OPERATIONAL, 50000);
                } while (chk-- && (ec_slave[0].state != EC_STATE_OPERATIONAL));

                if (ec_slave[0].state == EC_STATE_OPERATIONAL)
                {
                    printf("All slaves reached OPERATIONAL state.\n");
                    inOP = TRUE;

                    rt_task_create(&ecat_task, "ecat_task", 0, 80, T_JOINABLE);
                    rt_task_start(&ecat_task, &ecat_loop, NULL);

                    rt_task_join(&ecat_task);
                }
                else
                {
                    printf("Not all slaves reached OP state.\n");
                }
            }
            else
            {
                printf("No slaves found!\n");
            }
            ec_close();
        }
        else
        {
            printf("No socket connection on %s\n", argv[1]);
        }
    }
    else
    {
        printf("Usage: xenomai_simple_test [ifname]\n");
    }

    printf("End program\n");
    return 0;
}
