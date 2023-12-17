#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 7
#define BUFFER_SIZE 15

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

int isFull() {
    if((front == rear + 1) || (front == 0 && rear == BUFFER_SIZE - 1)) return 1;
    return 0;
}

int isEmpty() {
    if(front == -1) return 1;
    return 0;
}

void *thread_function(void *arg)
{
    sem_t *mutex = (sem_t *)arg;
    sem_wait(mutex);
    i++;
    printf("Thread %ld\n", pthread_self());
    sem_post(mutex);
    pthread_exit(NULL);
}
void *monitor_function(void *arg)
{
    sem_t *mutex = (sem_t *)arg;
    sem_wait(mutex);
    enqueque(i);
    printf("Monitor %ld\n", pthread_self());
    sem_post(mutex);
    pthread_exit(NULL);
}

void *collector_function(void *arg)
{
    sem_t *mutex = (sem_t *)arg;
    sem_wait(mutex);
    dequque();
    printf("Collector %ld\n", pthread_self());
    sem_post(mutex);
    pthread_exit(NULL);
}

void main()
{
    pthread_t mCounter[NUM_THREADS], mMonitor, mCollector;
    sem_t mutex;
    sem_init(&mutex, 0, 1);
    for (i = 0; i < NUM_THREADS; i++)
        pthread_create(&mCounter[i], NULL, thread_function, &mutex);
    pthread_create(&mMonitor, NULL, monitor_function, &mutex);
    pthread_create(&mCollector, NULL, collector_function, &mutex);
    for (i = 0; i < NUM_THREADS; i++)
        pthread_join(mCounter[i], NULL);
    pthread_join(mMonitor, NULL);
    pthread_join(mCollector, NULL);
    
    sem_destroy(&mutex);
}