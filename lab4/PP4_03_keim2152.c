/*Kovacs Elek Akos
keim2152
3. Feladat : Egy CFR utazási ügynökségen 5 jegypénztár van (melyek tevékenységét egy-egy szál modellezi), minden
egyes jegypénztárnál váltható jegy bárhová....*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <errno.h>

#define THREAD_SIZE 5
#define BUFFER_SIZE 32
#define TIMEOUT 15

typedef struct param{
    int db_size;
    int index;
    struct random_data*random_state;
} param;


int*database;
pthread_mutex_t mutex_database;

int selled_tickets;
pthread_mutex_t mutex_selled_tickets;

pthread_mutex_t mutex_finished;
pthread_cond_t cond_finished;
bool finished;
bool time_exeeded;


void initDB(int dbSize){
    database = (int*)calloc(dbSize, sizeof(int));
    for(int i = 0; i < dbSize; i++)database[i] = 1;
}

int generateSeatIndex(int size, struct random_data*random_state){
    int tries = 0;
    int index;
    random_r(random_state,&index);
    index = index % size;
    if(index < 0) index = -index;

    //3 random tries
    while(!database[index] && tries < 3){
        random_r(random_state,&index);
        index = index % size;
        if(index < 0) index = -index;
        tries++;
        //printf("generated index: %d", index);
    }

    //if still cant find a seat, linear search
    if(database[index] == 0){
        index = 0;
        while(database[index] == 0){
            index++;
        }
    }
    
    return index;
}

void printDatabase(int size){
    for(int i = 0; i < size; i++)printf("%d ",database[i]);
    printf("\n");
}

void * thread_routine(void * arg){
    param * p = (param*)arg;

    while(!finished){
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);

            
            pthread_mutex_lock(&mutex_database);
            int seatIndex = generateSeatIndex(p->db_size,p->random_state);
            printf("[%d]index %d ",p->index, seatIndex);
            database[seatIndex] = 0;
            printDatabase(p->db_size);
            
        
        pthread_mutex_lock(&mutex_finished);
        selled_tickets++;
            
        if(selled_tickets == p->db_size){
            pthread_cond_signal(&cond_finished);
        }
        pthread_mutex_unlock(&mutex_finished);
        pthread_mutex_unlock(&mutex_database);
        

        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
        sleep(1);
    }

    //pthread_detach(pthread_self());
    return NULL;
}



/*We will have a main thread, that waits for the User level Threads to finish selling tickets */
int main(int argc, char**argv){
    //N and T input parameters, where
    //N is the size of the database (nr of tickets to sell)
    //T is the maximum time the threads work for
    int N = 50;
    selled_tickets = 0;

    finished = false;
    time_exeeded = false;

    /*creates a databes array, fills with ones ex [1,1,1,1]*/
    initDB(N);


    //initializing the mutex and conditonal variable, grandts access to database
    if(pthread_mutex_init(&mutex_selled_tickets,NULL) != 0){perror("Mutex init failed!\n"); exit(1);}
    if(pthread_mutex_init(&mutex_database,NULL) != 0){perror("Mutex init failed!\n"); exit(1);}
    if(pthread_mutex_init(&mutex_finished,NULL) != 0){perror("Mutex init failed!\n"); exit(1);}
    if(pthread_cond_init(&cond_finished, NULL) != 0){perror("Cond init failed!\n"); exit(1);}


    //random state for every thread
    struct random_data*random_state = (struct random_data*)calloc(THREAD_SIZE,sizeof(struct random_data));
    char*rand_statebufs = (char*)calloc(THREAD_SIZE,BUFFER_SIZE);
    srandom(time(NULL));

    /*creating N ticket selling threads*/
    /*creadting parameters for the threads*/
    pthread_t *threads = (pthread_t*)calloc(THREAD_SIZE, sizeof(pthread_t));
    param *p = (param*)calloc(THREAD_SIZE,sizeof(param));
    for(int i = 0; i < THREAD_SIZE; i++){
        initstate_r(random(),&rand_statebufs[i*BUFFER_SIZE],BUFFER_SIZE,&random_state[i]);

        p[i].index = i;
        p[i].db_size = N;
        p[i].random_state = &random_state[i];

        if(pthread_create(&threads[i], NULL, &thread_routine,  &p[i]) != 0){
            perror("Thread create failed!\n");
            exit(1);
        }
    }

    //start timer
    //time_t start = time(NULL);
    //double elapsed_time;

    struct timeval tv;
    struct timespec ts;
    gettimeofday(&tv,NULL);
    ts.tv_sec = tv.tv_sec + TIMEOUT;
    ts.tv_nsec = tv.tv_usec * 1000;

    //clock_gettime(CLOCK_REALTIME, (pthread_timestruc_t*)&to);

    //main thread waiting for other threads to finish
    pthread_mutex_lock(&mutex_finished);
    while(!time_exeeded && selled_tickets < N){
        printf("Waiting for threads | elapsed time\n");
        int err = pthread_cond_timedwait(&cond_finished, &mutex_finished,&ts);
        
        //int err = pthread_cond_timedwait(&c, &m, &ts);
	if (err == ETIMEDOUT) {
            time_exeeded = 1;
        }
    }
    pthread_mutex_unlock(&mutex_finished);
    finished = true;
    
    /*for(int i = 0; i < THREAD_SIZE; i++){
        if(pthread_cancel(threads[i]) != 0){
            perror("Thread cancel failed!\n");
            exit(1);
        }
    }*/

    //waiting for the threads to finish
    for(int i = 0; i < THREAD_SIZE; i++){
        if(pthread_join(threads[i], NULL) != 0){
            printf("%d Thread join failed!\n",i);
            //exit(1);
        }
    }

    if(time_exeeded){
        printf("Nem sikerult eladni minden jegyet idoben, eladott jegyek szama: %d/%d\n",selled_tickets,N);
    }
    else{
        printf("Sikeresen eladtunk minden jegyet!\n");
    }

    free(threads);
    free(p);
    free(database);
    free(rand_statebufs);
    free(random_state);
    pthread_mutex_destroy(&mutex_finished);
    pthread_mutex_destroy(&mutex_database);
    pthread_cond_destroy(&cond_finished);
    return 0;
}
