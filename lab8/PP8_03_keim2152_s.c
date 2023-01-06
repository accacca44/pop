#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>
#include <time.h>

#define MAX 500

int getRandValue(){
    return rand()%MAX;
}

void fillRandValues(int * arr, int n){
    for(int i = 0; i < n; i++){
        arr[i] = getRandValue();
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

int main(int argc, char * argv[]){

    //Parameter ellenorzes
    if(argc != 3){
        printf("Nem helyes parameter!\n");
        exit(1);
    }
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    if(!n || !m)printf("Termeszetes szamokat adj meg!\n");

    printf("n=%d m=%d\n",n,m);

    int *arr = (int*)calloc(n,sizeof(int));
    int *eredmeny = (int*)calloc(MAX,sizeof(int));
    int eredmenyLen = 0;
    srand(time(NULL));

    double start = 0;
    double stop = 0;
    start = omp_get_wtime();

    fillRandValues(arr, n);

    for(int i = 0; i < n; i++){
        #ifdef DEBUG
        printf("[%d]:",i);
        #endif

        for(int j = 0; j < m ;j++){
            if(check(arr[i]) == 1){
                if(!volt(eredmeny,eredmenyLen,arr[i]))
                {   
                    eredmeny[eredmenyLen] = arr[i];
                    eredmenyLen++;
                }
            }
            arr[i] = getRandValue();

            #ifdef DEBUG
            printf("%d ",arr[i]);
            #endif
        }
        #ifdef DEBUG
        printf("\n");
        #endif
    }
    stop = omp_get_wtime();

    printf("Eredmenyek: ");
    printArr(eredmeny,eredmenyLen);
    printf("Mert ido: %f\n",stop-start);
    return 0;
}