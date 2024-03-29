/*
 ----------------- COMP 310/ECSE 427 Winter 2018 -----------------
 Dimitri Gallos
 Assignment 2 skeleton
 
 -----------------------------------------------------------------
 I declare that the awesomeness below is a genuine piece of work
 and falls under the McGill code of conduct, to the best of my knowledge.
 -----------------------------------------------------------------
 */

//Please enter your name and McGill ID below
//Name: Xi Meng Huang
//McGill ID: 260608596

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <semaphore.h>

int BUFFER_SIZE = 100; //size of queue
sem_t full;
sem_t empty;
pthread_mutex_t mutex;

// A structure to represent a queue
struct Queue
{
    int front, rear, size;
    unsigned capacity;
    int *array;
};

// function to create a queue of given capacity.
// It initializes size of queue as 0
struct Queue *createQueue(unsigned capacity)
{
    struct Queue *queue = (struct Queue *)malloc(sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1; // This is important, see the enqueue
    queue->array = (int *)malloc(queue->capacity * sizeof(int));
    return queue;
}

// Queue is full when size becomes equal to the capacity
int isFull(struct Queue *queue)
{
    return ((queue->size) >= queue->capacity);
}

// Queue is empty when size is 0
int isEmpty(struct Queue *queue)
{
    return (queue->size == 0);
}

// Function to add an item to the queue.
// It changes rear and size
void enqueue(struct Queue *queue, int item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
    // printf("%d enqueued to queue\n", item);
}

// Function to remove an item from queue.
// It changes front and size
int dequeue(struct Queue *queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

// Function to get front of queue
int front(struct Queue *queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->front];
}

// Function to get rear of queue
int rear(struct Queue *queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->rear];
}

void print(struct Queue *queue)
{
    if (queue->size == 0)
    {
        return;
    }

    for (int i = queue->front; i < queue->front + queue->size; i++)
    {

        printf(" Element at position %d is %d \n ", i % (queue->capacity), queue->array[i % (queue->capacity)]);
    }
}

struct Queue *queue;

/*Producer Function: Simulates an Airplane arriving and dumping 5-10 passengers to the taxi platform */
void *FnAirplane(void *cl_id)
{
    int index;
    index = *((int*)cl_id);

    while(1){
        //make passenger
        int num_passengers = rand() % (10 + 1 - 5) + 5;
        
        printf("Airplane %i arrives with %i passengers\n",index,num_passengers);

        if(isFull(queue)){
            printf("Platform is full: Rest of passengers of plane %i take the bus\n",index);
            //sleep for 1 hour
            sleep(1);
            continue;
        }


        char str[7];

        //critical section
        for(int p=0;p<num_passengers;p++){

            //down empty
            sem_wait(&empty);
            //down mutex
            pthread_mutex_lock(&mutex);
        
            sprintf(str,"1%.3d%.3d",index,p);
            int passenger_id = atoi(str);
            printf("Passenger 1%.3d%.3d of airplane %i arrives to platform\n",index,p,index);

            
            enqueue(queue,passenger_id);

            
            //up mutex
            pthread_mutex_unlock(&mutex);

            //up full
            sem_post(&full);
        }

        

        //sleep for 1 hour
        sleep(1);
    
    }
}

/* Consumer Function: simulates a taxi that takes n time to take a passenger home and come back to the airport */
void *FnTaxi(void *pr_id)
{
    int index;
    index = *((int*)pr_id);
    while(1){
        printf("Taxi driver %i arrives\n",index);

        int pickup;

        //down full
        sem_wait(&full);
        //down mutex
        pthread_mutex_lock(&mutex);


        //critical section
        if(isEmpty(queue)){
            printf("Taxi driver %i waits for passengers to enter the platform\n",index);
            continue;
        } else {
            pickup = dequeue(queue);
        }

        //up mutex
        pthread_mutex_unlock(&mutex);

        //up empty
        sem_post(&empty);

        printf("Taxi driver %i picks up client %d from the platform\n",index,pickup);


        //sleep for 10 to 30 minutes
        int taxi_interval = rand() % (30 + 1 - 10) + 10;
        // printf("%d\n",(double)taxi_interval/60.0 * 1000000);
        usleep((double)taxi_interval/60.0 * 1000000);
    }
    
}

int main(int argc, char *argv[])
{

    int num_airplanes;
    int num_taxis;

    num_airplanes = atoi(argv[1]);
    num_taxis = atoi(argv[2]);


    printf("You entered: %d airplanes per hour\n", num_airplanes);
    printf("You entered: %d taxis\n", num_taxis);

    //initialize queue
    queue = createQueue(BUFFER_SIZE);

    //declare arrays of threads and initialize semaphore(s)
    pthread_t a_threads[num_airplanes];
    pthread_t t_threads[num_taxis];

    sem_init(&full, 0, 0);
    sem_init(&empty, 0, BUFFER_SIZE);
    pthread_mutex_init(&mutex,NULL);

    //create arrays of integer pointers to ids for taxi / airplane threads
    int *taxi_ids[num_taxis];
    int *airplane_ids[num_airplanes];

    //create threads for airplanes

    for (int i = 0; i < num_airplanes; i++)
    {
        airplane_ids[i] = malloc(sizeof(int));
        *airplane_ids[i] = i;

        if (pthread_create(&a_threads[i], NULL, FnAirplane, airplane_ids[i]) == 0)
        {
            printf("Creating airplane thread %i\n", i);
        } else {
            printf("Error with airplane thread %i\n", i);

        }
    }

    //create threads for taxis
    for (int i = 0; i < num_taxis; i++)
    {
        taxi_ids[i] = malloc(sizeof(int));
        *taxi_ids[i] = i;

        if (pthread_create(&t_threads[i], NULL, FnTaxi, taxi_ids[i]) == 0)
        {
            printf("Creating taxi thread %i\n", i);
        } else {
            printf("Error with taxi thread %i\n", i);
        }
    }

    pthread_exit(NULL);
}
