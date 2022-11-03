//Kovacs Elek Akos
//Labor 2, Feladat 3
//Azonosito: keim2152

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define MAX_MATRIX_VAL 10   //matrix maximalis ertekei
#define BUFFER_SIZE 32

typedef struct thread_param{
    int index;
    int matrix_size;
    struct random_data*random_state;
    pthread_mutex_t mut;
}thread_param;


int**matrix;
int*seged;
pthread_mutex_t*mutexek;

//letrehoz egy N*N-es matrixot random ertekkel
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

void printMx(int** mx, int N, FILE*out){
    int i,j;
    for(i =0 ; i < N;i++){
        for(j=0; j < N; j++){
            fprintf(out,"%d ",mx[i][j]);
        }
        fprintf(out, "\n");
    }
    fprintf(out,"\n");
}

void printSeged(int n){
    for(int i = 0; i < n;i++){
        printf("%d ",seged[i]);
    }
    printf("\n");
}

void * routine(void * arg){
    thread_param*tp = (thread_param*)(arg);

    while(1){
        //random ideig alszik
        int sleepTime;
        random_r(tp->random_state,&sleepTime);
        usleep(sleepTime%10000);

        //mutex lock
        pthread_mutex_lock(&mutexek[tp->index]);

        int i = tp->index;
        if(seged[i] == 1){
            printf("----\n");
            printSeged(tp->matrix_size);
            int rowSum=0;
            int colSum=0;
            for(int k = 0; k < tp->matrix_size; k++){
                colSum+=matrix[i][k];
                rowSum+=matrix[k][i];
            }
            printf("%d. thread SOR:%d OSZLOP:%d\n",i,rowSum,colSum);
            seged[i] = 0;
            printSeged(tp->matrix_size);
            printf("----\n");
        }


        //mutex unlock
        pthread_mutex_unlock(&mutexek[tp->index]);
    }

    return NULL;
}

int main(int argc, char* argv[]){

    //PARAMETER
    if(argc != 2){perror("Hibas parameterezes!\n");return 1;}
    int N = atoi(argv[1]);
    if(!N){perror("Hibas N parameter!\n");return 1;}

    //Matrix, segedtomb letrehozas
    srand(time(NULL));                          //random generator a matrix ertekeinek
    FILE*outFile = fopen("out.dat","a");        //output file
    matrix = createMatrix(N);                   
    seged = (int*)calloc(N,sizeof(int));
    for(int i=0;i<N;i++)seged[i] = 1;           //segedtomb kezdeti ertekei 1-esek
    printMx(matrix,N,outFile);

    //Mutexek dinamikus letrehozasa
    mutexek = (pthread_mutex_t*)calloc(N,sizeof(pthread_mutex_t));
    for(int i = 0;i < N;i++)pthread_mutex_init(&(mutexek[i]),NULL);

    //ULT 
    pthread_t*threads = (pthread_t*)calloc(N,sizeof(pthread_t));
    struct random_data*random_state = (struct random_data*)calloc(N+1,sizeof(struct random_data));
    char*rand_statebufs = (char*)calloc(N+1,BUFFER_SIZE);
    thread_param*tp = (thread_param*)calloc(N,sizeof(thread_param));
    srandom(time(NULL));
    for(int i = 0; i < N;i++){
        //atadom a szalak rutinjainak a szukseges parametereket
        initstate_r(random(),&rand_statebufs[i*BUFFER_SIZE],BUFFER_SIZE,&random_state[i]);
        tp[i].index = i;
        tp[i].matrix_size = N;
        tp[i].random_state = &random_state[i];
        tp[i].mut = mutexek[i];

        if(pthread_create ((pthread_t *) & threads[i], NULL, routine,(void *) &(tp[i])) != 0){
            perror("pthread_create hiba");
            exit(1);     
        }
    }

    //MAINT 
    initstate_r(random(),&rand_statebufs[N*BUFFER_SIZE],BUFFER_SIZE,&random_state[N]);
    
    //A foszal 15 valtoztatas utan leall
    int changes = 0;
    while(changes < 15){
        sleep(1);

        //gerenal random poziciokat
        int i,j;
        random_r(&(random_state[N]),&i);
        random_r(&(random_state[N]),&j);
        i = i%N;
        j = j%N;

        //uj eretket ad a tombnek
        int newVal;
        random_r(&(random_state[N]),&newVal);

        //mutex foglalas helyes olvasashoz
        //pthread_mutex_lock(&(mutexek[i]));
        //pthread_mutex_lock(&(mutexek[j]));
        
        //segedtomb ellenorzes
        if(seged[i] == 0 && seged[j] == 0){
            matrix[i][j] = newVal%MAX_MATRIX_VAL;
            seged[i] = 1;
            seged[j] = 1;

            printf("Foszal: Matrix(%d,%d) modositott!\n",i,j);
            printMx(matrix,N,outFile);
            changes++;
        }
        else{
            printf("Foszal: Matrix(%d,%d) NEM modositott!\n",i,j);
        }
        //mutex felszabaditas
        //pthread_mutex_unlock(&(mutexek[i]));
        //pthread_mutex_unlock(&(mutexek[j]));
    }

    for(int i = 0; i<N;i++)pthread_cancel(threads[i]);
    /*for(int i = 0; i<N;i++){
        if(pthread_join (threads[i], NULL) != 0){
         perror("pthread_join hiba");
         exit(1);
        }       
    }*/
    

    return 0;
}