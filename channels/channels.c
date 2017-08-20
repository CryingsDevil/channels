#include <malloc.h>  
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include "channels.h"

typedef struct{
	int id;
	int hadMessage;
	pthread_mutex_t message;
	pthread_mutex_t recv;
	void* item;
	pthread_t pid;
} channelL;

static channelL *channels[8];

void *working(int *id){
	int cid = *id;
	int locked = 0;
	while(1){
		while(!locked)locked =	pthread_mutex_trylock(&channels[cid]->recv);
		locked = 0;		
		while(!channels[cid]->hadMessage);
		pthread_mutex_unlock(&channels[cid]->recv);
		while(!locked)locked = pthread_mutex_trylock(&channels[cid]->message);
		locked = 0;
		while(channels[cid]->hadMessage);
		pthread_mutex_unlock(&channels[cid]->message);
	}
}

int ch_setup(){
	int ret = -1;
	int success = 0;
	for(int i = 0; i <= 7; i++){
		channels[i] = malloc(sizeof(channelL));
		if(channels[i] != NULL){
			channels[i]->id = i;
			channels[i]->item = NULL;
			channels[i]->hadMessage = 0;
			pthread_mutex_init(&channels[i]->message, NULL);
			pthread_mutex_init(&channels[i]->recv, NULL);
			ret = pthread_create(&channels[i]->pid,NULL,(void *)working, &channels[i]->id);   

			if(ret == 0){
				ret = -1;
				success++;		
			}else{
				puts("Create struct error");
			}
		
		}
		
	}
	if(success == 8){
		return 0;
	}else{
		return -1;
	}
}

int ch_send(int channel, void* msg){
	int locked = 0;
	while(!locked){
		locked = pthread_mutex_trylock(&channels[channel]->message);
		if(locked){
			channels[channel]->item = (void*)malloc(sizeof((void*) msg));
			channels[channel]->item = msg;
			msg = NULL;
			free(msg);		
			channels[channel]->hadMessage = 1;
			pthread_mutex_unlock(&channels[channel]->message);
		}
	}
	return 0;

}

int ch_recv(int channel, void** dest){
	int locked = 0;
	while(!locked){
		locked = pthread_mutex_trylock(&channels[channel]->recv);
		if(locked){
			if(channels[channel]->hadMessage == 1){
				*dest = (void*)malloc(sizeof((void*) channels[channel]->item)); 
				*dest = channels[channel]->item;
				channels[channel]->item = NULL;
				free(channels[channel]->item;
				channels[channel]->hadMessage = 0;
			}else{
				locked = 0;
			}
			pthread_mutex_unlock(&channels[channel]->recv);

		}
	}
	return 0;

}


int ch_tryrecv(int channel, void** dest){
	if(channels[channel]->hadMessage == 1){
		if(pthread_mutex_trylock(&channels[channel]->recv) == 0){
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


int ch_peek(int channel){
	return channels[channel]->hadMessage;
}
