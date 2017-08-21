#include <malloc.h>  
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "channels.h"

typedef struct{
	int id;
	int hadMessage;
	pthread_cond_t condS; 
	pthread_cond_t condR;
	pthread_cond_t condCS; 
	pthread_cond_t condCR;
	pthread_mutex_t send;
	pthread_mutex_t recv;
	void* item;
	pthread_t pid;
} channelL;

static channelL *channels[8];


int ch_setup(){
	int ret = 0;
	int success = 0;
	for(int i = 0; i <= 7; i++){
		channels[i] = malloc(sizeof(channelL));
		if(channels[i] != NULL){
			channels[i]->id = i;
			channels[i]->item = NULL;
			channels[i]->hadMessage = 0;
			ret += pthread_cond_init(&channels[i]->condS , NULL) + 1;
			ret += pthread_cond_init(&channels[i]->condR , NULL) + 1;
			ret += pthread_mutex_init(&channels[i]->send , NULL) + 1;
			ret += pthread_mutex_init(&channels[i]->recv , NULL) + 1;
			usleep(1000);
			if(ret == 4	){
				ret = 0;
				success++;		
			}else{
				puts("Create struct error");
			}
		
		
		}else{
			puts("No enough memory!");
		}
		
	}
	if(success == 8){
		return 0;
	}else{
		return -1;
	}
}


int ch_send(int channel, void* msg){
	if(channel >= 8){
		puts("Error: Channel number is larger than 7!");
		return -1;
	}else{
		pthread_mutex_lock(&channels[channel]->send);
		while(channels[channel]->hadMessage == 1)pthread_cond_wait(&channels[channel]->condS,&channels[channel]->send);
		channels[channel]->item = (void*)malloc(sizeof((void*) msg));
		channels[channel]->item = msg;
		msg = NULL;
		free(msg);
		channels[channel]->hadMessage = 1;
		pthread_cond_broadcast(&channels[channel]->condR);
		pthread_mutex_unlock(&channels[channel]->send);
		return 0;
	}
}

int ch_recv(int channel, void** dest){
	if(channel >= 8){
		puts("Error: Channel number is larger than 7!");
		return -1;
	}else{
		pthread_mutex_lock(&channels[channel]->recv);
		while(channels[channel]->hadMessage == 0)pthread_cond_wait(&channels[channel]->condR,&channels[channel]->recv);
		*dest = (void*)malloc(sizeof((void*) channels[channel]->item));
		*dest = channels[channel]->item;
		channels[channel]->item = NULL;
		free(channels[channel]->item);
		channels[channel]->hadMessage = 0;
		pthread_cond_broadcast(&channels[channel]->condS);
		pthread_mutex_unlock(&channels[channel]->recv);
		return 0;
	}
}


int ch_tryrecv(int channel, void** dest){
	if(channel >= 8){
		puts("Error: Channel number is larger than 7!");
		return -1;
	}else{
		puts("Why use me???");
		if(channels[channel]->hadMessage == 1){
			if(pthread_mutex_trylock(&channels[channel]->recv) == 0){
				*dest = (void*)malloc(sizeof((void*) channels[channel]->item));
				*dest = channels[channel]->item;
				channels[channel]->item = NULL;
				free(channels[channel]->item);
				channels[channel]->hadMessage = 0;
				pthread_mutex_unlock(&channels[channel]->recv);
				return 1;
			}else{
				dest = NULL;
				return 0;
			}
		}else{
			dest = NULL;
			return 0;
		}
	}
}


int ch_peek(int channel){
	return channels[channel]->hadMessage;
}
