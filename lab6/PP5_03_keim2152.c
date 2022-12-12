//Kovacs Elek Akos
//522/2
//keim2152
//Feladat 3 : Szamoljuk ki a diakok atlagat, illetve javitsuk a jegyeket, majd adjuk meg a top 3at

//Szekvencialis kod

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <omp.h>
#include <unistd.h>


#define BUFFER_SIZE 32

double sec_time = 0.0;
double par_time = 0.0;

typedef struct stud{
	char studID [6];
	int grades[5];
	double avg;

}stud;

int getNewGrade(struct random_data random_state, int i, int tdCnt){
	int r = 0;
	random_r(&random_state, &r);
	return (r%(11-4))+4;
}

void printAll(double * avg, stud*s, int numStud){
	//print students grades
	for(int i = 0; i < numStud; i++){
		printf("%s ",s[i].studID);
		for(int j = 0; j < 5;j++){
			printf("%d\t",s[i].grades[j]);
		}
		printf("%.2f\n",s[i].avg);
	}

	//print cvcourse avgs
	for(int j = 0; j < 5;j++){
		printf("%.3f\t",avg[j]);
	}
	printf("\n");

}

stud * generateData_parallel(int numStuds, int numThreads){
	stud * students = (stud*)calloc(numStuds,sizeof(stud));

	#pragma omp parallel num_threads(numThreads) default(none) shared(numStuds,students)
	{
		int tdCnt = omp_get_thread_num();
		unsigned int seed = tdCnt;
		#pragma omp for 
		for(int i = 0; i < numStuds; i++){
			//legeneralom az ID-t //A-Z 65-90 //48-57 0-9
			char fn = (rand_r(&seed)%('z'-'a' + 1))+'a';
			char ln = (rand_r(&seed)%('z'-'a' + 1))+'a';
			char nums[4];

			sprintf(nums,"%d",(rand_r(&seed)%(9999-1000))+1000);
			sprintf(students[i].studID,"%c%c%s",fn,ln,nums);

			//legeneralok minden targybol 1 jegyet
			for(int j = 0; j < 5;j++){
				students[i].grades[j] = (rand_r(&seed)%(11-4))+4;
			}
		}
	}
	return students;
}
stud * generateData_secvential(int numStuds){
	stud * students = (stud*)calloc(numStuds,sizeof(stud));

	for(int i = 0; i < numStuds; i++){
		//legeneralom az ID-t //A-Z 65-90 //48-57 0-9
		char fn = (rand()%('z'-'a' + 1))+'a';
		char ln = (rand()%('z'-'a' + 1))+'a';
		char nums[4];

		sprintf(nums,"%d",(rand()%(9999-1000))+1000);
		sprintf(students[i].studID,"%c%c%s",fn,ln,nums);

		//legeneralok minden targybol 1 jegyet
		for(int j = 0; j < 5;j++){
			students[i].grades[j] = (rand()%(11-4))+4;
		}
	}
	return students;
}
void calcAvgStud_parallel(stud * students, int numStuds, int numThreads){
	//minden diakra szamolok egy atlagot
	#pragma omp parallel for num_threads(numThreads) default(none) shared(students, numStuds, numThreads)
	for(int i = 0; i < numStuds; i++){
		double tempSum = 0.0;
		for(int j = 0; j < 5; j++){
			tempSum += students[i].grades[j];
		}
		students[i].avg = tempSum/5;
	}
}
void calcAvgStud_secvential(stud * students, int numStuds){
	//minden diakra szamolok egy atlagot
	for(int i = 0; i < numStuds; i++){
		double tempSum = 0.0;
		for(int j = 0; j < 5; j++){
			tempSum += students[i].grades[j];
		}
		students[i].avg = tempSum/5;
	}
}

double * calcAvgCourse_parallel(stud * students, int numStuds, int numThreads){
	double * avg = (double*)calloc(5,sizeof(double));
	double avg1 = 0;
	double avg2 = 0;
	double avg3 = 0;
	double avg4 = 0;
	double avg5 = 0;
	
	#pragma omp parallel for num_threads(numThreads) default(none) shared(students, numStuds, numThreads)  reduction(+: avg1) reduction(+: avg2) reduction(+: avg3) reduction(+: avg4) reduction(+: avg5)

	for(int i = 0; i < numStuds; i++){
		for(int j = 0; j < 5; j++){
			if(j == 0)avg1 += students[i].grades[j];
			if(j == 1)avg2 += students[i].grades[j];
			if(j == 2)avg3 += students[i].grades[j];
			if(j == 3)avg4 += students[i].grades[j];
			if(j == 4)avg5 += students[i].grades[j];
		}
	}

	avg[0] = avg1;
	avg[1] = avg2;
	avg[2] = avg3;
	avg[3] = avg4;
	avg[4] = avg5;
	for(int j = 0; j < 5; j++){
		avg[j] /= numStuds;
	}
	return avg;
}
double * calcAvgCourse_secvential(stud * students, int numStuds){
	double * avg = (double*)calloc(5,sizeof(double));
	for(int i = 0; i < numStuds; i++){
		for(int j = 0; j < 5; j++){
			avg[j] += students[i].grades[j];
		}
	}

	for(int j = 0; j < 5; j++){
		avg[j] /= numStuds;
	}
	return avg;
}

void reGrade_parallel(double * avg, stud * students, int numStuds, int numThreads, unsigned int * seeds){
	//ellenorzom minden diak minden jegyet, ha ez kissebb mint az atlag kap egy uj jegyet

	#pragma omp parallel num_threads(numThreads) default(none) shared(avg, students, numStuds, numThreads,seeds) 
	{
		//struct random_data local_random_state = random_states[tdCnt];
		unsigned int threadCount = omp_get_thread_num();

		#pragma omp for
		for(int diak = 0; diak < numStuds; diak++){
			bool changed = false;
			for(int jegy = 0; jegy < 5; jegy++){
				//ha atlag alatti jegye van kap egy uj eselyt
				if(avg[jegy] - students[diak].grades[jegy] >= 1){
					//int newGrade = getNewGrade(local_random_state,diak,tdCnt);
					int newGrade = (rand_r(&seeds[threadCount*64])%(11-4))+4;
					//usleep(1);
					//ha jobb jegyet kapott akkor lecsereljuk
					if(students[diak].grades[jegy] < newGrade){
						students[diak].grades[jegy] = newGrade;
						changed = true;
					}
				}
			}
			//ha sikerult javitson, ujra szamoljuk a diak atlagat
				if(changed){
					double tempSum = 0.0;
					for(int j = 0; j < 5; j++){
						tempSum += students[diak].grades[j];
					}
					students[diak].avg = tempSum/5;
			}
			
		}
	}
}
void reGrade_secvential(double * avg, stud * students, int numStuds){
	//ellenorzom minden diak minden jegyet, ha ez kissebb mint az atlag kap egy uj jegyet
	for(int diak = 0; diak < numStuds; diak++){
		bool changed = false;
		for(int jegy = 0; jegy < 5; jegy++){
			//ha atlag alatti jegye van kap egy uj eselyt
			if(avg[jegy] - students[diak].grades[jegy] >= 1){
				int newGrade = (rand()%(11-4))+4;
				//usleep(1);
				//ha jobb jegyet kapott akkor lecsereljuk
				if(students[diak].grades[jegy] < newGrade){
					students[diak].grades[jegy] = newGrade;
					changed = true;
				}
			}
		}

		//ha sikerult javitson, ujra szamoljuk a diak atlagat
		if(changed){
			double tempSum = 0.0;
			for(int j = 0; j < 5; j++){
				tempSum += students[diak].grades[j];
			}
			students[diak].avg = tempSum/5;
		}
		
	}
}

int * topStud(stud*students, int numStuds){
	//helyet foglalok a 3 legjobb tanulo poziciojanak
	int * topStuds = (int*)calloc(3,sizeof(int));

	//a top 3 diak indexe es jegye a students-tombben
	int p1 = -1,p2=-3,p3=-5;
	double j1 = 0, j2 = 0, j3 = 0;

	//maximumot keresek
	for(int i = 0; i < numStuds; i++){
		if(students[i].avg > j1){
			j3 = j2;
			j2 = j1;
			j1 = students[i].avg;

			p3 = p2;
			p2 = p1;
			p1 = i;
		}
		else if(students[i].avg > j2){
			j3 = j2;
			j2 = students[i].avg;

			p3 = p2;
			p2 = i;
		}
		else if(students[i].avg > j3){
			j3 = i;
			p3 = i;
		}
	}
	topStuds[0] = p1;
	topStuds[1] = p2;
	topStuds[2] = p3;

	return topStuds;
}
int * topStud_parallel(stud*students, int numStuds, int numThreads){
	//helyet foglalok a 3 legjobb tanulo poziciojanak
	int * topStuds = (int*)calloc(3,sizeof(int));

	//a top 3 diak indexe es jegye a students-tombben
	int p1 = -1,p2=-3,p3=-5;
	double j1 = 0, j2 = 0, j3 = 0;

	//maximumot keresek
	#pragma omp parallel for num_threads(numThreads) shared(students, numThreads,topStuds,p1,p2,p3,j1,j2,j3)
	for(int i = 0; i < numStuds; i++){
		if(students[i].avg > j1){
			j3 = j2;
			j2 = j1;
			j1 = students[i].avg;

			p3 = p2;
			p2 = p1;
			p1 = i;
		}
		else if(students[i].avg > j2){
			j3 = j2;
			j2 = students[i].avg;

			p3 = p2;
			p2 = i;
		}
		else if(students[i].avg > j3){
			j3 = i;
			p3 = i;
		}
	}
	topStuds[0] = p1;
	topStuds[1] = p2;
	topStuds[2] = p3;

	return topStuds;
}

void szekvencialis(int numStuds){
	printf("Szekvencialis:\n");

	srand(time(0));

	//omp idomeres feladatokhoz
	double start;
	double end;
	double time;

	start = omp_get_wtime();
	stud * students = generateData_secvential(numStuds);
	end = omp_get_wtime();
	time = end-start;
	printf("%f sec\n",time);
	sec_time += (end-start);

	start = omp_get_wtime();
	calcAvgStud_secvential(students,numStuds);
	end = omp_get_wtime();
	printf("%f sec\n",end-start);
	sec_time += (end-start);

	start = omp_get_wtime();
	double * avgCourse = calcAvgCourse_secvential(students,numStuds);
	end = omp_get_wtime();
	printf("%f sec\n",end-start);
	sec_time += (end-start);
	
	#ifdef DEBUG
	printAll(avgCourse,students,numStuds);
	#endif 

	start = omp_get_wtime();
	reGrade_secvential(avgCourse,students,numStuds);
	end = omp_get_wtime();
	printf("%f sec\n",end-start);
	sec_time += (end-start);
	
	
	start = omp_get_wtime();
	int * topStuds = topStud(students, numStuds);
	end = omp_get_wtime();
	printf("%f sec\n",end-start);
	sec_time += (end-start);

	#ifdef DEBUG
	printAll(avgCourse,students,numStuds);
	printf("Top diakok: %d %d %d\n",topStuds[0],topStuds[1],topStuds[2]);
	#endif

	free(students);
	free(avgCourse);
	free(topStuds);

}

void parallel(int numStuds, int numThreads){
	printf("Parhuzamos:\n");

	double start;
	double end;

	start = omp_get_wtime();
	stud * students = generateData_parallel(numStuds,numThreads);
	end = omp_get_wtime();
	printf("%f sec\n",end-start);
	par_time += (end-start);

	start = omp_get_wtime();
	calcAvgStud_parallel(students,numStuds,numThreads);
	end = omp_get_wtime();
	printf("%f sec\n",end-start);
	par_time += (end-start);
	
	start = omp_get_wtime();
	double * avgCourse = calcAvgCourse_parallel(students,numStuds,numThreads);
	end = omp_get_wtime();
	printf("%f sec\n",end-start);
	par_time += (end-start);

	#ifdef DEBUG
	printAll(avgCourse,students,numStuds);
	#endif

	//minden diakra megprobal javitani
	unsigned int * seeds = (unsigned int*)calloc(64*numThreads, sizeof(unsigned int));
	for(int i = 0; i < numThreads; i++)seeds[i*64] = i;

	start = omp_get_wtime();
	reGrade_parallel(avgCourse, students, numStuds,numThreads,seeds);
	end = omp_get_wtime();
	printf("%f sec\n",end-start);
	par_time += (end-start);
	
	start = omp_get_wtime();
	int * topStuds = topStud_parallel(students, numStuds,numThreads);
	end = omp_get_wtime();
	printf("%f sec\n",end-start);
	par_time += (end-start);

	#ifdef DEBUG
	printAll(avgCourse,students,numStuds);
	printf("Top diakok: %d %d %d\n",topStuds[0],topStuds[1],topStuds[2]);
	#endif

	free(students);
	free(avgCourse);	
	free(topStuds);
	free(seeds);
}

int main(int argc, char** argv){
	int numStuds;
	int numThreads;

	//parameterEllenorzes
	if(argc != 3)perror("Nem helyes parameter!");
	
	numStuds = atoi(argv[1]);
	numThreads = atoi(argv[2]);
	if(!numStuds || !numThreads)perror("Hibas parameter!");

	szekvencialis(numStuds);
	printf("\n");
	parallel(numStuds, numThreads);

	double gyorsitas = sec_time/par_time;
	double hatekonysag = gyorsitas/numThreads;

	printf("Parhuzamos: %f, Szekvencialis: %f\n",par_time,sec_time);
	printf("Gyors:%f Hat:%f\n",gyorsitas,hatekonysag);


	return 0;
}
