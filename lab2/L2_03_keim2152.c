#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define MUTEX 1
#define MAX_MATRIX_VAL 10
#define BUFFER_SIZE 32

typedef struct thread_param{
    int index;
    int mx_size;
    struct random_data * random_state;
}thread_param;

int**matrix;
int*seged;

//letrehoz egy N*N-es matrixot
int** createMatrix(int N){
    
    //dinamkusan helyet foglalok a matrixnak
    int** matrix = (int**)calloc(N,sizeof(int*));
    int i;
    for(i = 0; i < N;i++){
        matrix[i] = (int*)calloc(N,sizeof(int));
        
        int j;
        for(j = 0; j < N; j++){
            matrix[i][j] = rand() % MAX_MATRIX_VAL + 5;
        }
    }

    return matrix;
}

//kiiratja a matrixot
void printMx(int** mx, int N, FILE*out){
    int i,j;
    for(i =0 ; i < N;i++){
        for(j=0; j < N; j++){
            fprintf(out,"%d ",mx[i][j]);
        }
        fprintf(out, "\n");
    }
}

//mutex init
pthread_mutex_t mutp = PTHREAD_MUTEX_INITIALIZER;

//Szalfuggveny
void *f(void * arg){
    thread_param *tp =(thread_param*) arg;
    
    while(1){
        int r;
        random_r(tp->random_state,&r);
        usleep(r%10000);

        #ifdef MUTEX
        pthread_mutex_lock(&mutp);
        #endif

        if(seged[tp->index] == 1){
            int rowSum=0;
            int colSum=0;
            for(int i = 0; i < tp->mx_size; i++){
                colSum+=matrix[tp->index][i];
                rowSum+=matrix[i][tp->index];
            }
            printf("%d. thread SOR:%d OSZLOP:%d\n",tp->index,rowSum,colSum);
            seged[tp->index] = 0;
        }

        #ifdef MUTEX
        pthread_mutex_unlock(&mutp);
        #endif
    }
    return NULL;
}

int main(int argc, char* argv[]){

    //parameter ellenorzes
    if(argc != 2){
        perror("Hibas parameterezes!\n");
    }

    int N = atoi(argv[1]);
    if(!N){
        perror("Hibas N parameter!\n");
    }

    //Matrix Letrehozas
    FILE*outFile = fopen("out.dat","a");
    srand(time(NULL));
    matrix = createMatrix(N);
    printMx(matrix,N,outFile);

    //Segedtomb letrehozas
    int i;
    seged = (int*)calloc(N,sizeof(int));
    for(i=0;i<N;i++){
        seged[i] = 1;
    }

    //Szalak elinditasa
    int NTHREADS = N;
    pthread_t *threads = (pthread_t*)calloc(N,sizeof(pthread_t));
    struct random_data* rand_states;
    char* rand_statebufs;
    thread_param * tp = (thread_param*)calloc(N,sizeof(thread_param));

    rand_states = (struct random_data*)calloc(NTHREADS+1, sizeof(struct random_data));
    rand_statebufs = (char*)calloc(NTHREADS+1, BUFFER_SIZE);
    srandom(time(0));
    for(i = 0; i < NTHREADS; i++){
        initstate_r(random(), &rand_statebufs[i*BUFFER_SIZE], BUFFER_SIZE,&rand_states[i]);
        tp[i].index = i;
        tp[i].random_state = &rand_states[i];
        tp[i].mx_size = N;

        if(pthread_create ((pthread_t *) & threads[i], NULL, f,(void *) &tp[i]) != 0){
        perror("pthread_create hiba");
        exit(1);     
        }
    }
    /*foszal*/
    initstate_r(random(), &rand_statebufs[NTHREADS*BUFFER_SIZE], BUFFER_SIZE,&rand_states[NTHREADS]);
    int changes = 0;
    while(changes < 15){
        sleep(1);
        int i,j;        //generalt poziciok
        random_r(&(rand_states[NTHREADS]),&i);
        random_r(&(rand_states[NTHREADS]),&j);
        i = i%N;
        j = j%N;

        //ellenorzi a segedtombot
        if(!seged[i] && !seged[j]){
            int newVal;
            random_r(&(rand_states[NTHREADS]),&newVal);

            #ifdef MUTEX
            pthread_mutex_lock(&mutp);
            #endif  
                //MUTEX
                matrix[i][j] = newVal;
                seged[i] = seged[j] = 1;
            #ifdef MUTEX
            pthread_mutex_unlock(&mutp);
            #endif

            printf("Foszal: Matrix(%d,%d) modositott!\n",i,j);
            printMx(matrix,N,outFile);
            changes++;
        }
        else if(seged[i] || seged[j]){
            printf("Foszal: Matrix(%d,%d) NEM modositott!\n",i,j);
        }
    }

    for(i = 0; i<NTHREADS;i++){
        if(pthread_join (threads[i], NULL) != 0){
         perror("pthread_join hiba");
         exit(1);
        }       
        pthread_cancel(threads[i]);
    }

    return 0;
}