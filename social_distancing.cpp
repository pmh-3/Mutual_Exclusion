//social_distancing.cpp
//Peter Heitkemper
//5/5/21

#include <pthread.h>
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <semaphore.h>

using namespace std;


int vaxk_count = 0; //the number of vaccinated kids currently playing at the structure

sem_t mutex;	// Ensures mutual exclusion when vaccinated_kids_count is updated
int ret1 = sem_init(&mutex,0,1);
sem_t play_mutex;
int ret2 = sem_init(&play_mutex,0,1);	// provides mutual exclusion for accessing the play structure


void play(int ID, char *v){

	printf("%s Thread %d playing! \n",v,ID);
	sleep(1);
	printf("%s Thread %d playing done! \n",v,ID);
	
}

//vacinated kid thread
void *vaxed(void *arg_id){
	
	//cast void ptr to int pointer, then dereference
	int id = *(static_cast<int*>(arg_id));
	
	char v[] = "V";
	do{
		sem_wait(&mutex);
		printf("V Thread %d acquired mutex for incrementing the count.\n",id);
		vaxk_count++;
		if(vaxk_count == 1){
			printf("V Thread %d trying to acquire play_mutex.\n",id);
			sem_wait(&play_mutex);
		}
		printf("V Thread %d about to release mutex after incrementing.\n",id);
		sem_post(&mutex);
		play(id,v);
		sem_wait(&mutex);
		printf("V Thread %d acquired mutex for decrementing the count.\n",id);
		
		vaxk_count--;
		if(vaxk_count == 0){
			printf("V Thread %d about to release play_mutex.\n",id);
			sem_post(&play_mutex);
		}
		printf("V Thread %d about to release mutex after decrementing.\n",id);
		sem_post(&mutex);
		sleep(1);
	}while(true);
	
	pthread_exit(0);

}

//Non-vaccinated kid thread
void *nonVaxed(void *arg_id){
	//cast void ptr to int pointer, then dereference
	int id = *(static_cast<int*>(arg_id));
	char v[] = "NV";
	do{
		printf("NV Thread %d trying to acquire play_mutex. \n",id);
		sem_wait(&play_mutex);
		play(id,v);
		sem_post(&play_mutex);
		sleep(.75);
	}while(true);
	
	pthread_exit(0);
}



int main(int argc, char *argv[]) {
	int v_threads;
	int nv_threads;
	if (argc == 3) {
		v_threads = atoi(argv[1]);
		nv_threads = atoi(argv[2]);
	}
	else {
		cout << "ERROR: invalid input" << endl;
		return -1;
	}

	const int NUM_THREADS = v_threads + nv_threads;
	int status;

	pthread_t* ID = new pthread_t[NUM_THREADS];	//array to store tid
	int* arg_arr = new int[NUM_THREADS]; //array to store value of counter
	int t;
	
	//loop to create threads
	//vaccinated kid threads get the first IDs
	for(t = 0; t < v_threads; t++){
		
		arg_arr[t] = t; //store VALUE of t 
		
		status = pthread_create(&ID[t], NULL, vaxed, static_cast<void*>(&arg_arr[t]));
		
		if(status != 0){
			cout<< "ERROR creating thread " << endl; 
			exit(-1);
		}
	}
	for(t = v_threads; t<NUM_THREADS; t++){
		arg_arr[t] = t; //store VALUE of t 

		status = pthread_create(&ID[t], NULL, nonVaxed, static_cast<void*>(&arg_arr[t]));
		
		if(status != 0){
			cout<< "ERROR creating thread " << endl; 
			exit(-1);
		}
	}
	
	for(t = 0; t<NUM_THREADS; t++){
		
		status =  pthread_join(ID[t], NULL);
		if(status){
				cout << "Error: cannot join " << endl;
				exit(-1);
		}
		
	}
	delete ID;
	delete arg_arr;
	return 0;
}

/*
Questions:

dynamically allocate and delete passed arguments?
how to delete threads appropriately?


*/