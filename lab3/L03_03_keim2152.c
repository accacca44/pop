#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <stdbool.h>
#include <string.h>

int **matrix;
int **seged;
int finished = 0;
sem_t nrActWorker_semaphore;
sem_t mod_semaphore;
sem_t s1;
sem_t* s2;

typedef struct neighbr{
    int live;
    int dead;
}neighbr;

typedef struct thread_parameters{
    int index;
    int size;   //matrix merete;
    int *_actWorker;
    int *_mod;
}thread_parameters;

//Letrehoz egy nxn-es matrixot, 0/1 ertekekkel feltoltve
int** createMatrix(int n){
    int** tempMx = (int**)calloc(n,sizeof(int*));
    for(int i = 0; i < n; i++){
    tempMx[i] = (int*)calloc(n, sizeof(int));
    for(int j = 0; j < n; j++){
            tempMx[i][j] = rand()%2;
        }    
    }
    return tempMx;
}

//letrehoz egy matrixot az adott matrix elemeivel
int **createAndCopyMatrix(int**ptr,int n){
    int** tempMx = (int**)calloc(n,sizeof(int*));
    for(int i = 0; i < n; i++){
        tempMx[i] = (int*)calloc(n, sizeof(int));
        for(int j = 0; j < n; j++){
            tempMx[i][j] = ptr[i][j];
        }    
    }
    return tempMx;
}

//Kiirat egy adott matrixot a kepernyore
void printMatrix(int**ptr, int n){
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            if(ptr[i][j] == 1)printf("X");
            else printf(" ");
            //printf("%d ",ptr[i][j]);
        }
        printf("\n");
    }
}

neighbr checkNeighbours(int i, int j, int n){
    neighbr tmp;
    tmp.live = 0;
    tmp.dead = 0;

    int posX[8] = {1,-1,0,0, -1,1,1,-1};
    int posY[8] = {0,0,1,-1, -1,1,-1,1};
    for(int k = 0; k < 8;k++){
        int nextX = i + posX[k];
        int nextY = j + posY[k];
        if((nextX >= 0 && nextX < n) && (nextY >= 0 && nextY < n)){
            //printf("%d",matrix[nextX][nextY]);
            if(matrix[nextX][nextY] == 1)
                tmp.live++;
            else 
                tmp.dead++;
        }
    }
    return tmp;
}

void edit(int** ptr, int i, int j, int val){
    ptr[i][j] = val;
}

void copy(int **p1, int **p2, int n){
    for(int i = 0; i < n; i++){
        for(int j =0; j < n; j++){
            p1[i][j] = p2[i][j];
        }
    }
}

void printThreadInfo(int i, int n, int step, bool has_changed){
    char buff1[250];
    char buff2[250];
    char buff3[250];
    sprintf(buff1, "[%d] szal, %d. lepes, [",i,step);
    for(int k = 0; k < n;k++){
        char c[1];
        c[0] = seged[i][k] + 48;
        strcat(buff2,c);
    }
    
    sprintf(buff3, " Modositva: %d",(int)has_changed);
    //printf("%s\n",buff1);
    //printf("%s\n",buff2);
    printf("%s%s\n",buff1,buff3);
}

//void printMainInfo();


//Felhasznaloio szalak routine-ja
void*start_routine(void * arg){
    thread_parameters* tp = (thread_parameters*) arg;
    int step = 0;
    
    while(!finished){
        step++;
        sem_wait(&s2[tp->index]);

        
        bool has_changed = false;
        //Bejarja a matrix i. sorat es megszamlalja az 1es szomszedokat
        for(int col = 0; col < tp->size; col++){
            neighbr nb = checkNeighbours(tp->index,col,tp->size);
            //printf("Live : %d | Dead: %d\n",nb.live, nb.dead);
            if(nb.live >= 5 && matrix[tp->index][col] == 0){
                edit(seged, tp->index, col, 1);
                has_changed = true;
            }
            if(nb.dead >= 5 && matrix[tp->index][col] == 1){
                edit(seged, tp->index, col, 0);
                has_changed = true;
            }
        }

        //printThreadInfo(tp->index,tp->size,step,has_changed);

        if(has_changed){
            sem_wait(&mod_semaphore);
            *(tp->_mod) = 1;
            sem_post(&mod_semaphore);
        }

        //utolso szal
        sem_wait(&nrActWorker_semaphore);
        *(tp->_actWorker) = *(tp->_actWorker) - 1;
        if(*(tp->_actWorker) == 0){

            sem_wait(&mod_semaphore);

            if(*(tp->_mod) == 1){
                copy(matrix,seged,tp->size);
            }
            else{
                *(tp->_mod) = -1;
            }

            sem_post(&mod_semaphore);
            sem_post(&s1);
        }
        sem_post(&nrActWorker_semaphore);
    }
    return NULL;
}

int main(int argc, char* argv[]){
    //Ellenorzom hogy csak 1 parametert kapott a program
    if(argc != 2){
        perror("Csak 1 szam parametert adj meg!\n");
        return 1;
    }

    //Ellenorzom, hogy a megadott parameter szam e
    int N = atoi(argv[1]);
    if(!N){
        perror("A megadott parameter nem valodi szam, adj meg egy 0nal nagyobb szamot!\n");
        return 1;
    }

    //Modositasokat jelzo valtozok
    int nrActWorker = N;
    int mod = 0;

    //Letrehozom a matrixot es a segedmatrixot
    srand(time(NULL));
    matrix = createMatrix(N);
    seged = createAndCopyMatrix(matrix,N);

    printMatrix(matrix,N);
    //printMatrix(seged,N);

    //Inicializalom a semaphorokat
    sem_init(&nrActWorker_semaphore, 0, 1);
    sem_init(&mod_semaphore, 0, 1);
    sem_init(&s1, 0, 1);
    s2 = (sem_t*)calloc(N,sizeof(sem_t));
    for(int i = 0; i < N; i++){
        sem_init(&s2[i], 0 ,0);
    }




    //Elinditok N szalat
    pthread_t*threads = (pthread_t*)calloc(N,sizeof(pthread_t));
    thread_parameters*tp = (thread_parameters*)calloc(N,sizeof(thread_parameters));
    for(int i = 0; i < N; i++){
        tp[i].index = i;
        tp[i].size = N;
        tp[i]._actWorker=&nrActWorker;
        tp[i]._mod = &mod;
        if((pthread_create((pthread_t*)&threads[i], NULL, start_routine, &tp[i])) < 0){
            perror("Nem sikerult letrehozni a szalat!\n");
            return 1;
        }
    }

    //Foszal
    while(!finished){
        sem_wait(&s1);

        //leallasi feltetel
        sem_wait(&mod_semaphore);
        if(mod == -1){
            finished = true;
            sem_post(&mod_semaphore);
            break;
        }
        sem_post(&mod_semaphore);

        //todo
        sleep(0.5);
        system("cls");
        printMatrix(matrix,N);
        sem_wait(&mod_semaphore);
        sem_wait(&nrActWorker_semaphore);
        mod = 0;
        nrActWorker = N;
        sem_post(&nrActWorker_semaphore);
        sem_post(&mod_semaphore);
        
        //wait
        for(int i = 0; i < N; i++){
            sem_post(&s2[i]);
        }
    }
    if(finished){
        printMatrix(matrix,N);
        
        for(int i = 0; i < N; i++){
            sem_post(&s2[i]);
        }
    }

    for(int i = 0; i < N; i++){
        pthread_join(threads[i],NULL);
    }
    return 0;
}