#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#define NUM_THREADS 7
#define BUFFER_SIZE 5
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"

sem_t mutex1, mutex2;
sem_t empty, full;

int queue[BUFFER_SIZE];
int front = -1, rear =-1;
int i = 0;

void enqueue(int value) {
    if(front == -1) front = 0;
    rear = (rear + 1) % BUFFER_SIZE;
    queue[rear] = value;
}
int dequeue() {
    int value;
        value = queue[front];
        if(front == rear){
            front = -1;
            rear = -1;
        }//reseting front and rear if queue is empty
        else {
            front = (front + 1) % BUFFER_SIZE;
        }
}


void signalHandler(int sig_num)
{
    //set color back to normal color
    printf("%sExit\n",KNRM);
    //destroy semaphores
    sem_destroy(&mutex1);
    sem_destroy(&mutex2);
    sem_destroy(&empty);
    sem_destroy(&full);
    exit(0);
}

void *counter_function(void *arg)
{
    while(1){
    int thread_no = *(int *)arg;
    printf("\n%sThread %d is trying to Enter critical section..\n",KGRN,thread_no); 
    sem_wait(&mutex1);
    //critical section
    printf("\n%sThread %d Entered critical section..\n",KGRN,thread_no); 
    i++;
    sleep(1);
    //signal and exit critical section
    printf("\n%sThread %d Just Exiting critical section...\n",KGRN,thread_no); 
    sem_post(&mutex1);
    //sleep with random number between 1 and 10
    sleep(rand() % 10) + 1;
    }
}
void *monitor_function(void *arg)
{
    while(1){
    printf("\n%sMonitor is trying to Enter critical section..\n",KYEL);
    sem_wait(&mutex1);
    //critical section
    printf("\n%sMonitor Entered critical section..\n",KYEL);
    int number = i;
    i = 0;
    sleep(1);
    //signal and exit critical section
    printf("\n%sMonitor Just Exiting critical section...\n",KYEL);
    sem_post(&mutex1);
   
    sem_wait(&empty);
    printf("\n%sMonitor is trying to Enter critical section..\n",KRED);
    sem_wait(&mutex2);
    //critical section
    printf("\n%sMonitor Entered critical section..\n",KRED);
    enqueue(number);
    sleep(1);
    //signal and exit critical section
    printf("\n%sMonitor Just Exiting critical section...\n",KRED);
    sem_post(&mutex2);
    sem_post(&full);
    //sleep with random number between 10 and 20
    sleep(rand() % 11) + 10;
    }
}

void *collector_function(void *arg)
{
    while(1){
    sem_wait(&full);
    printf("%sCollector Trying to Enter critical section..\n",KBLU); 
    sem_wait(&mutex2);
    //critical section
    printf("\n%sCollector Entered critical section..\n",KBLU);
    dequeue();
    sleep(1);
    //signal and exit critical section
    printf("\n%sCollector Just Exiting critical section...\n",KBLU);
    sem_post(&mutex2);
    sem_post(&empty);
    //sleep with random number between 20 and 30
    sleep(rand() % 11) + 20;
    }

}

int main()
{
    pthread_t mCounter[NUM_THREADS], mMonitor, mCollector;


    sem_init(&mutex1, 0, 1);
    sem_init(&mutex2, 0, 1);

    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
    
    for (int j = 0; j < NUM_THREADS; j++)
        pthread_create(&mCounter[j], NULL, counter_function,(void *) &j);    
    pthread_create(&mMonitor, NULL, monitor_function, NULL);
    pthread_create(&mCollector, NULL, collector_function, NULL);

    //handling signal
    signal(SIGINT, signalHandler);
    
    for (int j = 0; j < NUM_THREADS; j++)
        pthread_join(mCounter[j], NULL);
    pthread_join(mMonitor, NULL);
    pthread_join(mCollector, NULL);
    
    sem_destroy(&mutex1);
    sem_destroy(&mutex2);
    sem_destroy(&empty);
    sem_destroy(&full);

    printf("%sExit\n", KNRM);
    return 0;
}

//check printings.
//check thread garbage values.