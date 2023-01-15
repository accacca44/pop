//szukseges konyvtarak
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include <math.h>
#include <string.h>

//definialasok
#define MAX_ROUND 20 //maximalis korok szama
#define MAX_TIP 10   //maximalis tippek szama
#define TIP_SIZE 8   //tippek hossza

//randUser fuggveny kivalaszt egy random felhasznalonevet a szerverre bejelentkezett felhasznalok kozul
char * randUser(){
	FILE*fp = NULL;
	char cmd[80];
	sprintf(cmd,"last -f wtmp | cut -c 1-8 | head -n 20 | sort | uniq | shuf | head -n 1");
	
	fp = popen(cmd, "r");
	if (fp == NULL)
		printf("Popen Error!");

	char*name = (char*)calloc(TIP_SIZE+1,sizeof(char));
	fgets(name,(TIP_SIZE+1)*sizeof(char),fp);
	return name;
}

//generateTipp, general 10 tippet
char * generateTips(){
	char * tips = (char*)calloc(MAX_TIP*9,sizeof(char));
	sprintf(tips,"%s%s%s%s%s%s%s%s%s%s",randUser(),randUser(),randUser(),randUser(),randUser(),randUser(),randUser(),randUser(),randUser(),randUser());
	return tips;
}

void printTime(double start ,double finish){
	printf("Osszegzeshez hasznalt ido: %fsec.\n",finish-start);
}

void grade(int player, int * points, char* tipps,char* megoldas, int comm_sz){
	int i = 0;
	int current = 0;
	int correct = 0;

	while(i < MAX_TIP*TIP_SIZE*comm_sz){
		int j = 0;
		while(j < MAX_TIP*TIP_SIZE){

            #ifdef DEBUG
			char * tempName = tipps+i+j;
            
			for(int x = 0; x < TIP_SIZE;x++){
				printf("%c",tempName[x]);
			}
			printf(" ");
			for(int x = 0; x < TIP_SIZE;x++){
				printf("%c",megoldas[x]);
			}
			if(strncmp(tempName,megoldas,TIP_SIZE) == 0)printf("  <-");

			printf("\n");
            #endif

			if(current != player && strncmp(tipps+i+j,megoldas,TIP_SIZE) == 0){
				points[current] = MAX_TIP-j/TIP_SIZE;
				correct++;
				break;
			}
			j+=TIP_SIZE;
		}
		current++;
		i+=MAX_TIP*TIP_SIZE;
	}   
	points[0] = 0;
	points[player] = correct;
}

void osszegez(int * total, int len, int*newTotal, int comm_sz){
	int i = 0;
	while(i < len){
		for(int p = 0; p < comm_sz; p++){
			newTotal[p] += total[i+p];
		}
		i+=comm_sz;
	}
}

void szekvencialisOsszegzes(int *allPoint, int *total, int comm_sz, int korokSzama){
	double start;
	double finish;

	start = MPI_Wtime();
	for(int player = 0; player < comm_sz; player++){
		for(int kor = 0; kor < korokSzama; kor++){
			total[player] += allPoint[comm_sz*kor + player];
		}
	}
	finish = MPI_Wtime();
	printTime(start,finish);
}

void parhuzamosOsszegzes(int *allPoints, int *total, int comm_sz, int korokSzama, int*subarr, int subSize, int* newTotal){
	MPI_Barrier(MPI_COMM_WORLD);
	double start;
	double finish;
	start = MPI_Wtime();

	MPI_Scatter(allPoints,subSize,MPI_INT,subarr,subSize,MPI_INT,0,MPI_COMM_WORLD);

	osszegez(subarr,subSize,newTotal,comm_sz);

	MPI_Reduce(newTotal,total,comm_sz,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
	finish = MPI_Wtime();
	printTime(start,finish);
}

int main(void){
    //minden szalhoz tarozo atributumok
    int comm_sz;            //osszes folyamat szama
	int my_rank;            //sajat folyamat szama
	unsigned int my_seed;   //sajat random seed
	int korokSzama;         //jatekban szereplo korok szama
	int player;             //mutogato jatekos sorszama

    //folyamatok elinditasanak inicializalasa
    MPI_Init(NULL,NULL);
	MPI_Comm_size(MPI_COMM_WORLD,&comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);

    //minden folyamatnak egyedi random seedje van
    my_seed = time(NULL) + my_rank*comm_sz;

    char*tipp = (char*)calloc(MAX_TIP * TIP_SIZE,sizeof(char));						//osszesen 10 tip, 8 karakter hosszu
    char*allTipp = (char*)calloc(MAX_TIP * TIP_SIZE * comm_sz,sizeof(char));		//minden foltamat tippjeit tartalmazza
    int * newTotal = (int*)calloc(comm_sz,sizeof(int));

    //minden folyamat jatszik kiveve a 0. folyamat
    if(my_rank != 0){
        //minden folyamat kiolvassa a korok szamat
        MPI_Bcast(&korokSzama,1,MPI_INT,0,MPI_COMM_WORLD);

        int allPointsLen = comm_sz * (korokSzama + (comm_sz - korokSzama % comm_sz));
        if(korokSzama%comm_sz == 0)allPointsLen = korokSzama*comm_sz;
        int*total  = (int*)calloc(comm_sz, sizeof(int));
        int*allPoints = (int*)calloc(allPointsLen,sizeof(int));
        int*subarr = (int*)calloc(allPointsLen / comm_sz, sizeof(int));

        for(int i = 0; i < korokSzama;i++){
            MPI_Bcast(&player,1,MPI_INT,0,MPI_COMM_WORLD);

            if(my_rank == player){
                char megfejtes[9];
                MPI_Recv(megfejtes,9,MPI_CHAR,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                #ifdef DEBUG
                printf("%d. jatekos mutogatja: %s\n",my_rank,megfejtes);
                #endif

                //a jatekos azert general tippeket, hogy az MPI_Gatherben, neki is legyen memoria, de ez figyelmen kivul hagyja az osszesitesnel
                tipp = generateTips();
				#ifdef DEBUG
                printf("Round : %d | %d:[%s]\n",i+1,my_rank,tipp);
                #endif
                //osszegyujti mindenkitol a tippeket
				MPI_Gather(tipp,MAX_TIP*TIP_SIZE,MPI_CHAR,allTipp,MAX_TIP*TIP_SIZE,MPI_CHAR,player,MPI_COMM_WORLD);

                //Pontozni fogja a tippeket
                int *points = (int*)calloc(comm_sz,sizeof(int));
                grade(player,points,allTipp,megfejtes,comm_sz);
 
                //elkudlom a 0.folzamatnak osszesiteskent
                MPI_Send(points,comm_sz,MPI_INT,0,1,MPI_COMM_WORLD);
                
                free(points);
            }
            else{
                //mindenki mas, aki jatszik, tippeket ad
                tipp = generateTips();
				#ifdef DEBUG
                printf("Round : %d | %d:[%s]\n",i+1,my_rank,tipp);
                #endif
                //elkuldom a tippet a jatekosnak
				MPI_Gather(tipp,MAX_TIP*TIP_SIZE,MPI_CHAR,allTipp,MAX_TIP*TIP_SIZE,MPI_CHAR,player,MPI_COMM_WORLD);
            }
			MPI_Barrier(MPI_COMM_WORLD);
        }

        int subSize = allPointsLen/comm_sz;
		if(korokSzama > 10){
			MPI_Barrier(MPI_COMM_WORLD);
			MPI_Scatter(allPoints,subSize,MPI_INT,subarr,subSize,MPI_INT,0,MPI_COMM_WORLD);
			osszegez(subarr,subSize,newTotal,comm_sz);
			MPI_Reduce(newTotal,total,comm_sz,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
		}
    }
    else{
        //a 0. folyamat a jatekot iranyito folyamat

        //generaljuk a korok szamat
        korokSzama = rand_r(&my_seed)%MAX_ROUND+1;
		//korokSzama = 3;
		MPI_Bcast(&korokSzama,1,MPI_INT,0,MPI_COMM_WORLD);

		int allPointsLen = comm_sz * (korokSzama + (comm_sz - korokSzama % comm_sz));
		if(korokSzama%comm_sz == 0)allPointsLen = comm_sz*korokSzama;
		int * total  = (int*)calloc(comm_sz, sizeof(int));
		int * allPoints = (int*)calloc(allPointsLen,sizeof(int));
		int * subarr = (int*)calloc(allPointsLen / comm_sz, sizeof(int));

        //szimulalok egy jatekbeli kort
        for(int i = 0; i < korokSzama;i++){
            #ifdef DEBUG
			printf("\nUj Kor %d of %d\n\n",i+1,korokSzama);
            #endif

            //generalom a kitalalando szot
            char megfejtes[9];
            sprintf(megfejtes,"%s",randUser()); 

            //generalom a jatekostm akinek el kell mutogatni a kitalalando szot, ez nem lehet a 0. jatekos
            player = rand_r(&my_seed)%(comm_sz-1)+1;

            //Elkuldjuk az generalt informaciokat
            printf("0. generalta: %d jatekos mutogatja: %s\n",player,megfejtes);
            MPI_Bcast(&player,1,MPI_INT,0,MPI_COMM_WORLD);
            MPI_Send(megfejtes,9,MPI_CHAR,player,0,MPI_COMM_WORLD);

            //a korben a 0. folyamat is elkuldi a tippeit
            tipp = generateTips();
            #ifdef DEBUG
            printf("Round : %d | %d:[%s]\n",i+1,my_rank,tipp);
			#endif
            
            MPI_Gather(tipp,MAX_TIP*TIP_SIZE,MPI_CHAR,allTipp,MAX_TIP*TIP_SIZE,MPI_CHAR,player,MPI_COMM_WORLD);
            //bevarom a pontokat
            int *currentPoints = allPoints + i*comm_sz;
            MPI_Recv(currentPoints,comm_sz,MPI_INT,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            
            #ifdef DEBUG
            for(int i = 0; i < allPointsLen;i++)printf("%d ",allPoints[i]);
			printf("\n");
            #endif

			MPI_Barrier(MPI_COMM_WORLD);
        }
        //Legvegen kiiratom a pontokat
        #ifdef DEBUG
		printf("Jatek Vege\n");
        #endif
		printf("Osszesites:\n");

		if(korokSzama > 10){
			parhuzamosOsszegzes(allPoints,total,comm_sz,korokSzama,subarr,allPointsLen/comm_sz,newTotal);
		}
		else{
			szekvencialisOsszegzes(allPoints,total,comm_sz,korokSzama);
		}

		for(int i =0; i < comm_sz; i++){
			printf("%d ",total[i]);
		}
        printf("\n");

        free(allPoints);
        free(total);
        free(subarr);
    }

    free(allTipp);
    free(tipp);

    MPI_Finalize();
    return 0;
}