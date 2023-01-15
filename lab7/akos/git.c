#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include <math.h>
#include <string.h>

#define MAX_ROUND 20
#define MAX_TIP 10
#define TIP_SIZE 8

int dub(int x){
	return 2*x;
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

char * randUser(){
	FILE*fp = NULL;
	char cmd[80];
	sprintf(cmd,"last -f wtmp | cut -c 1-8 | head -n 20 | sort | uniq | shuf | head -n 1");
	
	fp = popen(cmd, "r");
	if (fp == NULL)
		printf("Popen Error!");

	char*name = (char*)calloc(9,sizeof(char));
	fgets(name,9*sizeof(char),fp);
	return name;
}

char * generateTips(){
	char * tips = (char*)calloc(MAX_TIP*9,sizeof(char));
	sprintf(tips,"%s%s%s%s%s%s%s%s%s%s",randUser(),randUser(),randUser(),randUser(),randUser(),randUser(),randUser(),randUser(),randUser(),randUser());
	return tips;
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
	

int main(void)
{
	int comm_sz;
	int my_rank;
	unsigned int my_seed;
	int korokSzama;


	

	MPI_Init(NULL,NULL);
	MPI_Comm_size(MPI_COMM_WORLD,&comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);

	int * newTotal = (int*)calloc(comm_sz,sizeof(int));
	my_seed = time(NULL) + my_rank*comm_sz;

	char*tipp = (char*)calloc(MAX_TIP * TIP_SIZE,sizeof(char));						//10*9 = 90 chars
    char*allTipp = (char*)calloc(MAX_TIP * TIP_SIZE * comm_sz,sizeof(char));		//90 * n

	if(my_rank != 0){
		int player;
		char megoldas[TIP_SIZE];

		//koronkenti jatek
		MPI_Bcast(&korokSzama,1,MPI_INT,0,MPI_COMM_WORLD);

		int allPointsLen = comm_sz * (korokSzama + (comm_sz - korokSzama % comm_sz));
		if(korokSzama%comm_sz == 0)allPointsLen = korokSzama*comm_sz;
		//printf(" Kerekites %d kor\n",allPointsLen/comm_sz );
		int * total  = (int*)calloc(comm_sz, sizeof(int));
		int * allPoints = (int*)calloc(allPointsLen,sizeof(int));
		int * subarr = (int*)calloc(allPointsLen / comm_sz, sizeof(int));

		for(int i = 0; i < korokSzama;i++){
			MPI_Recv(&player,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			
			if(player == my_rank){
				MPI_Recv(megoldas,TIP_SIZE,MPI_CHAR,0,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

				//bevarom a tippeket
				int *points = (int*)calloc(comm_sz,sizeof(int));
				tipp = generateTips();
				printf("[%d]%s\n",my_rank,tipp);

				//osszegyujti egyben a tippeket		
				MPI_Gather(tipp,MAX_TIP*TIP_SIZE,MPI_CHAR,allTipp,MAX_TIP*TIP_SIZE,MPI_CHAR,player,MPI_COMM_WORLD);

				grade(player,points,allTipp,megoldas,comm_sz);
				
				//elkudlom a 0.folzamatnak osszesiteskent
				MPI_Send(points,comm_sz,MPI_INT,0,0,MPI_COMM_WORLD);

				//kiiratom a pontokat
				//for(int i = 0;i<comm_sz-1;i++)printf("%d",points[i]);
				//printf("\n");
			}	
			else{
				//generalom a tippeket
				tipp = generateTips();
				printf("[%d]%s\n",my_rank,tipp);

				//printf("Round : %d | %d:[%s]\n",i+1,my_rank,tipp);
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
		//generalom a korok szamat
		korokSzama = rand_r(&my_seed)%MAX_ROUND+1;
		korokSzama = 3;
		int pos = 0;

		int allPointsLen = comm_sz * (korokSzama + (comm_sz - korokSzama % comm_sz));
		if(korokSzama%comm_sz == 0)allPointsLen = comm_sz*korokSzama;
		//printf(" Kerekites %d kor\n",allPointsLen/comm_sz );
		int * total  = (int*)calloc(comm_sz, sizeof(int));
		int * allPoints = (int*)calloc(allPointsLen,sizeof(int));
		int * subarr = (int*)calloc(allPointsLen / comm_sz, sizeof(int));

		//szimulalom a koroket
		for(int i = 0; i < korokSzama;i++){
			fflush (stdout);
			printf("\nUj Jatek %d\n\n",korokSzama);


			//generalom a kitalalando szot
			//char megfejtes = (char)rand_r(&my_seed)%('9'-'0')+'0'+1;
			char megfejtes[9];
			sprintf(megfejtes,"%s",randUser());

			//generalom a jatekost, nem lehet a 0.as
			int jatekos = rand_r(&my_seed)%(comm_sz-1)+1;
			fflush (stdout);
			printf("JatekMester generalta: %d jatekos mutogatja: %s\n",jatekos,megfejtes);

			MPI_Bcast(&korokSzama,1,MPI_INT,0,MPI_COMM_WORLD);

			//elkuldom mindenkinek, hogy ki mutogat
			for(int p = 1; p < comm_sz;p++){
				MPI_Send(&jatekos,1,MPI_INT,p,0,MPI_COMM_WORLD);
			}

			//a mutogatonak elkuldom a szot is
			MPI_Send(&megfejtes,TIP_SIZE,MPI_CHAR,jatekos,1,MPI_COMM_WORLD);
			
			//a 0. folyamat is elkuldi a tippet amit nem vesznek figyelembe
			tipp = generateTips();
			//printf("\n0. tipp : %s\n",tipp);
			printf("[%d]%s\n",my_rank,tipp);

			MPI_Gather(tipp,MAX_TIP*TIP_SIZE,MPI_CHAR,allTipp,MAX_TIP*TIP_SIZE,MPI_CHAR,jatekos,MPI_COMM_WORLD);

			int*currentPoints = (int*)calloc(comm_sz,sizeof(int));
			MPI_Recv(currentPoints,comm_sz,MPI_INT,jatekos,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			
			for(int j = 0; j < comm_sz;j++){
				allPoints[pos] = currentPoints[j];
				pos++;
			}
			
			for(int i = 0; i < allPointsLen;i++)printf("%d",allPoints[i]);
			printf("\n");

			MPI_Barrier(MPI_COMM_WORLD);
		}

		//Legvegen kiiratom a pontokat
		printf("Jatek Vege\n");
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
	}

	MPI_Finalize();

	

	return 0;
}