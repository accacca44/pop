#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>
#include <time.h>

#define MAX 500

int getRandValue(unsigned int * seed){
    return rand_r(seed)%MAX;
}

void fillRandValues(int * arr, int n, int numThreads){
    #pragma omp parallel num_threads(numThreads) default(none) shared(arr,n,numThreads)
    {
        int tdCnt = omp_get_thread_num();
        unsigned int seed = tdCnt;

        #pragma omp for
        for(int i = 0; i < n; i++){
            arr[i] = getRandValue(&seed);
        }
    }
}

void printArr(int * arr, int n){
    for(int i = 0; i < n; i++){
        printf("%d ", arr[i]);
    }
    printf("\n");
}

int check(int num){
    while(num > 9){
        int temp = 0;
        while(num > 0){
            temp += (num%10)*(num%10);
            num /= 10;
        }
        num = temp;
        //printf("Num:%d",num);
    }
    return (num == 1);
}

int volt(int * arr, int n, int elem){
    for(int i = 0; i < n; i++){
        if(arr[i] == elem)return 1;
    }
    return 0;
}

void processValues(int m, int * arr, int * eredmeny, int *eredmenyLen, int n, int numThreads){
    #pragma omp parallel num_threads(numThreads) default(none) shared(n,m,arr,eredmeny,eredmenyLen,numThreads)
    {
        int tdCnt = omp_get_thread_num();
        unsigned int seed = tdCnt;

        #pragma omp for
        for(int i = 0; i < n; i++){

            #ifdef DEBUG
            printf("[%d]:",i);
            #endif
            
            for(int j = 0; j < m ;j++){
                if(check(arr[i]) == 1){
                    if(!volt(eredmeny,(*eredmenyLen),arr[i]))
                    {   
                        eredmeny[(*eredmenyLen)] = arr[i];
                        (*eredmenyLen)++;
                    }
                }
                arr[i] = getRandValue(&seed);
                #ifdef DEBUG
                printf("%d ",arr[i]);
                #endif
            }
            #ifdef DEBUG
            printf("\n");
            #endif
        }
    }
}

int main(int argc, char * argv[]){

    //Parameter ellenorzes
    if(argc != 4){
        printf("Nem helyes parameter!\n");
        exit(1);
    }
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    int numThreads = atoi(argv[3]);
    if(!n || !m || !numThreads)printf("Termeszetes szamokat adj meg!\n");

    int *arr = (int*)calloc(n,sizeof(int));
    int *eredmeny = (int*)calloc(MAX,sizeof(int));
    int eredmenyLen = 0;
    

    double start = 0;
    double stop = 0;
    start = omp_get_wtime();

    fillRandValues(arr, n, numThreads);

    processValues(m,arr,eredmeny,&eredmenyLen,n,numThreads);
    
    stop = omp_get_wtime();

    printf("Eredmenyek: ");
    printArr(eredmeny,eredmenyLen);
    printf("Mert ido: %f\n",stop-start);
    return 0;
}