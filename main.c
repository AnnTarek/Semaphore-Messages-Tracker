#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#define NUM_THREADS 7
#define BUFFER_SIZE 3
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
    int thread_no = *(int *)arg;
    while(1){
    //sleep with random number between 1 and 10
    sleep(rand() % 10) + 1;
    printf("\n%sCounter thread %d: received a message\n", KGRN, thread_no);

    printf("\n%sCounter thread %d: waiting to write\n",KGRN, thread_no); 
    sem_wait(&mutex1);
    //critical section
    i++;
    printf("\n%sCounter thread %d: now adding to counter, counter value=%d\n",KGRN, thread_no, i); 
    sleep(1);
    //signal and exit critical section
    sem_post(&mutex1);
    
    }
}
void *monitor_function(void *arg)
{
    while(1){
    //sleep with random number between 5 and 12
    sleep(rand() % 11) + 2;
    printf("\n%sMonitor thread: waiting to read counter\n",KYEL);

    sem_wait(&mutex1);
    //critical section
    int number = i;
    printf("\n%sMonitor thread: reading a count value of %d\n",KYEL,number);
    i = 0;
    sleep(1);
    //signal and exit critical section
    sem_post(&mutex1);
    
    int y;
    sem_getvalue(&empty, &y);
    if(y == 0){
        printf("\n%sMonitor thread: Buffer full!\n", KRED);
    }
    sem_wait(&empty);
    sem_wait(&mutex2);
    //critical section
    enqueue(number);
    printf("\n%sMonitor thread: writing to buffer at position %d\n",KRED, rear);
    sleep(1);
    //signal and exit critical section
    sem_post(&mutex2);
    sem_post(&full);
    
    }
}

void *collector_function(void *arg)
{
    while(1){
    //sleep with random number between 25 and 35
    sleep(rand() % 11) + 25;
    
    int x;
    sem_getvalue(&full, &x);
    if(x == 0){
        printf("\n%sCollector thread: nothing is in the buffer!\n", KBLU);
    }
    sem_wait(&full);
    sem_wait(&mutex2);
    //critical section
    dequeue();
    printf("\n%sCollector thread: reading from buffer at position %d\n",KBLU,front);
    sleep(1);
    //signal and exit critical section
    sem_post(&mutex2);
    sem_post(&empty);
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
