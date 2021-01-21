#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdint.h>
#include <ctype.h>
#include "queue.h"
#include "gel.h"
#define ARRIVAL 1
#define DEPARTURE 2
#define BUSY 1
#define FREE 0
#define MAX_DATA_LENGTH		1544
#define MAX_T		        100
#define ACK_SIZE		    64
#define WLAN_CAP		    11000000
#define SIFS			    0.05
#define DIFS			    0.1
#define SENSERATE			0.01


struct hosts {
    int isBusy;
    queue_t *hostsQ;
    queue_t *ackQ;
}hosts;

typedef struct hosts* hosts_t;

static int numEvents;
static int numHost;
static int dataSent;
static double dataRate;
static double arrivalRate;
static double totalDelay;
static double clk; 
static dll_t gel;
static hosts_t hostList;
static event_t currentE;

double negative_exponentially_distributed_time (double rate)
{
    double u;
    u = drand48();
    return ((-1/rate)*log(1-u));
}

int randomBackoff(int numBackoff)
{
    int bkf =  (rand() % MAX_T + 1) * (rand() % (int)pow(2,numBackoff) + 1);
    return bkf;
}

int randomDataLength()
{    
    int ans = (int) ((negative_exponentially_distributed_time(1)) * 1544);
    
    if (ans > 1544) return randomDataLength();
    return ans;
}

int randomHost(int i)
{
    int host = rand() % numHost;
    if (host == i) return randomHost(i);
    return host;
}

int arrival_process()
{
    int srcH = currentE->srcHost;
    double timeL;
    int result;
    event_t newA;
    clk = currentE->timeS;
    newA = create_event(randomHost(-1), -1, clk + negative_exponentially_distributed_time(arrivalRate), -1.0, ARRIVAL, -1.0, -1.0, -1);
    result = gel_insert(gel, newA);
    if (result)
    {
         printf("arrival_process(): Insert ARRIVAL event failed, source host id: %d\n", srcH);
         printf("%d, %d, %lf, %lf, %d\n", newA->srcHost, newA->destHost, newA->timeS, newA->timeL, result);
         printf("%lf\n\n", clk);
         return -1;
     }

    timeL = ((randomDataLength() * 8.0) / ((double)WLAN_CAP)) * 1000;
    currentE->destHost = randomHost(srcH);
    currentE->timeL = timeL;

    if(hostList->isBusy == FREE)
    {
        currentE->backoff = DIFS;
    }
    else{
        currentE->attempts++;
        currentE->backoff = DIFS + randomBackoff(currentE->attempts);
    }

    if (queue_enqueue(hostList->hostsQ[srcH], (void*) currentE))
    {
        printf("arrival_process(): enqueue host list failed, source host id: %d\n", srcH);
        return -1;

    }

 
    return 0;
}

int departure_process()
{
    int srcH = currentE->srcHost;

    event_t temp;

    clk = currentE->timeS;
    hostList->isBusy = FREE;
    dataSent += currentE->timeL * WLAN_CAP / 8000;
    totalDelay += currentE->delay + (ACK_SIZE * 8.0) / (WLAN_CAP) * 1000;
    queue_dequeue(hostList->hostsQ[srcH], (void**)&temp);
    queue_dequeue(hostList->ackQ[temp->destHost], (void**)&temp);
    if (temp->destHost != currentE->destHost) printf("BOBOBOBOBOBB\n");
    return 0;
}

int sense_channel()
{
    event_t temp;
    event_t newD;
    event_t ackA;
    int isack = 0;
    int result;
    if(hostList->isBusy == BUSY) return 0;
    for (int i = 0; i < numHost; i++)
    {
        if (queue_length(hostList->ackQ[i]) != 0)
        {
            if (queue_get(hostList->ackQ[i], (void**)&temp))
            {
                printf("sense_channel(): queue_get failed I\n");
                return -1;
            }
            temp->backoff -= SENSERATE;
            if (temp->backoff < -0.02)
            {
                return -1;
            }
            if (temp->backoff < 0.0001)            
            {
                if(hostList->isBusy == BUSY) 
                {
                    queue_dequeue(hostList->ackQ[i], (void**)&temp);
                    //printf("%d: BUSY CHANNEL COLLISION @ %d after %d\n", isack, temp->srcHost, newD->srcHost);
                }
                else
                {
                    newD = create_event(temp->srcHost, temp->destHost, clk+temp->timeL+(ACK_SIZE * 8.0) / (WLAN_CAP) * 1000, temp->timeL, DEPARTURE, temp->backoff, clk+temp->timeL+(ACK_SIZE * 8.0) / (WLAN_CAP) * 1000 - temp->timeS+temp->delay, temp->attempts);
                    if(newD->delay < 0){
                    exit(0);
                    }
                    result = gel_insert(gel, newD);
                    if (result == -1)
                    {
                        printf("sense_channel(): Insert DEPARTURE event failed, source host id: %d\n", temp->srcHost);
                        return -1;
                    }   
                    if (result == -2) // collision
                    {
                        //printf("COLLISION\n");
                        free(newD);
                        queue_dequeue(hostList->ackQ[i], (void**)&temp);
                    
                    }
                    else 
                    {
                        hostList->isBusy = BUSY;
                        isack = 1;

                    }
                }
            }
        }
        if (queue_length(hostList->hostsQ[i]) != 0)
        {
            if (queue_get(hostList->hostsQ[i], (void**)&temp))
            {
                printf("sense_channel(): queue_get failed II\n");
                return -1;
            }
            temp->backoff -= SENSERATE;
            if (temp->backoff < -0.0200)
            {
                return -1;
            }
            if (temp->backoff < 0.0001)            
            {
                if (temp->delay > 0.0001) // collision
                {
                    //printf("Suspected COLLISION @ %d wiht %d atp\n", temp->srcHost, temp->attempts++);
                    temp->attempts++;
                    temp->backoff = DIFS + randomBackoff(temp->attempts);
                    queue_enqueue(hostList->hostsQ[i], temp);
                    queue_dequeue(hostList->hostsQ[i], (void**)&temp);
                }
                else 
                {
                    temp->delay = clk - temp->timeS;
                    temp->backoff = 1.5 * (SIFS + (ACK_SIZE * 8.0) / (WLAN_CAP) * 1000);
                    ackA = create_event(temp->srcHost, temp->destHost, clk, temp->timeL, ARRIVAL, SIFS, temp->delay, temp->attempts);
                    queue_enqueue(hostList->ackQ[temp->destHost], (void*)ackA);
                }
            }  
        }
    }

    return 0;
}

int initialization()
{

    clk = 0;
    dataSent = 0;
    totalDelay = 0;

    printf("\n");
    printf("***********************************************\n");
    printf("                INITIALIZING\n");
    printf("             Getting Parameters\n");
    printf("***********************************************\n");
    printf("\n");

    printf("Arrival rate:\n >");
	scanf("%lf",&arrivalRate);
	printf("Arrival Rate: %.3f \n", arrivalRate);
    if (arrivalRate < 0 || arrivalRate > 1)
    {
        printf("initialization(): invalid arrivalRate\n");
        return -1;
    }
    printf("\n");
    printf("Host number:\n >");
	scanf("%d",&numHost);
	printf("Host number: %d \n", numHost);
    if (numHost < 0)
    {
        printf("initialization(): invalid numHost\n");
        return -1;
    }
    if (numHost < 2)
    {
        printf("initialization(): at least two hosts are needed\n");
        return -1;
    }
    printf("\n");
    printf("Events number:\n >");
	scanf("%d",&numEvents);
	printf("Events number: %d \n", numEvents);
    if (numEvents < 0)
    {
        printf("initialization(): invalid numEvents\n");
        return -1;
    }
    printf("\n");

    srand(time(NULL));
    gel = gel_create();
    hostList = (hosts_t)malloc(sizeof(struct hosts));
    hostList->hostsQ = (queue_t*)malloc(sizeof(queue_t)*numHost);
    hostList->ackQ = (queue_t*)malloc(sizeof(queue_t)*numHost);
    hostList->isBusy = FREE;

    /* 
     * Gernerating the first event
     * For each host in the list
     * Listing in GEL
     */
    for (int i = 0; i < numHost; i++)
    {

        hostList->hostsQ[i] = queue_create();
        hostList->ackQ[i] = queue_create();
        event_t newA = create_event(i, -1, negative_exponentially_distributed_time(arrivalRate), -1.0, ARRIVAL, -1.0, -1.0, -1);
        int result = gel_insert(gel, newA);
        if (result)
        {
            printf("initialization(): First event failed, source host id: %d\n", i);
            return -1;
        }
    }
    return 0;
}

int main()
{
    double throughput;
    double avgDelay;

    printf("\n\n\n\n\n");
    printf("|*************************************************|\n");
    printf("|           ECS 152 / EEC 173A PROJECT II         |\n");
    printf("| Simulation Analysis of IEEE 802.x Based Network |\n");
    printf("|                                                 |\n");
    printf("|                   AUTHORS:                      |\n");
    printf("|                  Kaiqi Jin                      |\n");
    printf("|                  Jiaxin Zhao                    |\n");
    printf("|                  Jiayi Li                       |\n");
    printf("|*************************************************|\n");
    printf("\n\n");
    
    if(initialization())
    {
        printf("int main(): initialization() failed\n");
        return -1;
    }
    printf("\n");
    printf("***********************************************\n");
    printf("            INITIALIZATION SUCCESS\n");
    printf("         Start Transporting Packages\n");
    printf("***********************************************\n");
    printf("\n");

    for (int i = -1; i < numEvents; i++) 
    {

        if (gel_length(gel) == 0)
        {
            printf("gel is empty\n");
            printf("System Clock: %.3f\n", clk);
            break;
        }
        if (gel_remove(gel, &currentE))
        {
            printf("int main(): failed to get the first event in gel\n");
            return -1;
        }

        if (currentE->timeS - clk >= SENSERATE)
        {
            gel_insert(gel, currentE);
            if (sense_channel())
            {
                return -1;
            }
            i--;
            clk += SENSERATE;
            continue;
        }

        if (currentE->type != DEPARTURE) 
        {
            i--;
            arrival_process();
        }
        else
        {
            departure_process();
        }

        
    }
   
   throughput = dataSent / clk * 1000;
   avgDelay = totalDelay / dataSent;

    printf("\n");
    printf("***********************************************\n");
    printf("                FINISH RUNNING\n");
    printf("               Ready for Output\n");
    printf("***********************************************\n");
    printf("\n");
    printf("Number of events:               %d\n",numEvents);
    printf("Hosts Number:                   %d\n", numHost);
    printf("Arrival Rate:                   %.3f\n", arrivalRate);
    printf("Data frame length rate:         %.3f\n", dataRate);
    printf("T value:                        %d\n", MAX_T);
    printf("SIFS:                           %.3f ms\n", SIFS);
    printf("DIFS:                           %.3f ms\n", DIFS);
    printf("-----------------------------------------------\n");
    printf("System Clock:                   %.3f\n", clk);
    printf("Total delay:                    %.3f ms\n", totalDelay);
    printf("Data sent:                      %d\n", dataSent);
    printf("Throughput:                     %.3f kbps\n", throughput / 1000);
    printf("Average network delay:          %.3f ms\n", avgDelay);
    printf("\n");
    printf("***********************************************\n");
    printf("                CLEARING AND EXIT\n");    
    printf("***********************************************\n");
    printf("\n\n\n");

    /*
     *  CLEARING AND EXIT
     */

    
    while (gel_length(gel) != 0) 
    {
        if (gel_remove(gel, &currentE))
        {
            printf("int main(): Fail to remove the first event in gel\n");
            return -1;
        }
        free(currentE);
    }

    gel_destroy(gel);

    return 0;
}