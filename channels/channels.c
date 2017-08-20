#include <malloc.h>  
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

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
		// printf("channel%d: waiting for message\n", cid);
		while(!channels[cid]->hadMessage);
		pthread_mutex_unlock(&channels[cid]->recv);
		// printf("channel%d: recved message waiting for recving\n", cid);
		// if(channels[cid]->item == NULL)puts("We get an empty message");
		while(!locked)locked = pthread_mutex_trylock(&channels[cid]->message);
		// printf("channel%d: recved message and message is sending\n",cid);
		locked = 0;
		while(channels[cid]->hadMessage);
		// printf("channel%d: recved messages and finished sending, waiting for message again\n",cid);
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
			usleep(1000);
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
	// puts("ch_send working");
	if(channel >= 8){
		puts("Error: Channel number is larger than 7!");
		return -1;
	}else{
		int locked = 0;
		while(!locked){
			locked = pthread_mutex_trylock(&channels[channel]->message);
			if(locked){
				if(channels[channel]->hadMessage == 0){
					// puts("ch_send locked");
			
					// channels[channel]->item = msg;
					channels[channel]->item = (void*)malloc(sizeof((void*) msg));
					// channels[channel]->item = (void*)malloc(msg);

					channels[channel]->item = msg;
					msg = NULL;
					free(msg);
				
					channels[channel]->hadMessage = 1;
					usleep(1000);
				}else{
					locked = 0;
				}
				// puts("ch_send finished");
				pthread_mutex_unlock(&channels[channel]->message);
				
			}
			usleep(1000);
		}
		return 0;
	}
}

int ch_recv(int channel, void** dest){
	// puts("ch_recv working");
	if(channel >= 8){
		puts("Error: Channel number is larger than 7!");
		return -1;
	}else{
		int locked = 0;
		while(!locked){

			locked = pthread_mutex_trylock(&channels[channel]->recv);
			if(locked){
				// if(channel == 5)puts("222222");
				if(channels[channel]->hadMessage == 1){
					// printfuts("111");
					// puts("locked");
					// void* instant = channels[channel]->item;
					*dest = (void*)malloc(sizeof((void*) channels[channel]->item)); 
					// *dest = (void*)malloc((void*) channels[channel]->item); 
					// dest = malloc(sizeof(*channels[channel]->item));
					// puts("get message");
					// dest = instant;
					// if(channels[channel]->item == NULL)puts("ch_recv: We get an empty message");
					*dest = channels[channel]->item;
					// if(dest != NULL)puts("dest had been set");
					channels[channel]->item = NULL;
					free(channels[channel]->item);
					// if(dest == NULL)puts("channels had been free");
					channels[channel]->hadMessage = 0;
					usleep(1000);
					// puts("recv finshed");
				}else{
					locked = 0;
				}
				pthread_mutex_unlock(&channels[channel]->recv);

			}
			usleep(1000);
		}
		return 0;
	}
}


int ch_tryrecv(int channel, void** dest){
	// puts("ch_tryrecv working");
	if(channel >= 8){
		puts("Error: Channel number is larger than 7!");
		return -1;
	}else{
		if(channels[channel]->hadMessage == 1){
			if(pthread_mutex_trylock(&channels[channel]->recv) == 0){
				// dest = malloc(sizeof(channels[channel]->item));
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

void printAll(){
	for(int i = 0; i <= 7; i++){
		printf("Channels %d status: %d ", i, channels[i]->hadMessage);
	}
}