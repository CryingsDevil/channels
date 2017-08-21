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

// void *working(int *id){
// 	int cid = *id;
// 	int locked = 0;
// 	while(1){
// 		puts("Channels trying to lock recv");
// 		while(!locked)locked =	pthread_mutex_trylock(&channels[cid]->recv);
// 		locked = 0;
// 		puts("Channels lokced recv");
// 		// while(channels[cid]->hadMessage == 0){
// 		// 	puts("Calling send");
// 		// 	pthread_cond_broadcast(&channels[cid]->condS);
// 		// 	usleep(1000);
// 		// }
// 		pthread_cond_broadcast(&channels[cid]->condS);
// 		puts("Channles broadcasted send");
// 		// pthread_mutex_lock(&channels[cid]->recv);
// 		while(channels[cid]->hadMessage == 0)pthread_cond_wait(&channels[cid]->condCS,&channels[cid]->recv);
// 		puts("Channls get message");
// 		// while(channels[cid]->hadMessage == 1){
// 		// 	pthread_cond_broadcast(&channels[cid]->condR);
// 		// 	usleep(1000);
// 		// }
// 		pthread_mutex_unlock(&channels[cid]->recv);

// 		while(!locked)locked = pthread_mutex_trylock(&channels[cid]->send);
// 		puts("Channles called recv");
// 		locked = 0;
// 		// pthread_mutex_lock(&channels[cid]->message);
// 		// while(channels[cid]->hadMessage == 1){
// 		// 	puts("Calling recv");
// 		// 	pthread_cond_broadcast(&channels[cid]->condR);
// 		// 	usleep(1000);
// 		// }
// 		pthread_cond_broadcast(&channels[cid]->condR);
// 		puts("Channels waiting for recv finished");
// 		while(channels[cid]->hadMessage == 1)pthread_cond_wait(&channels[cid]->condCR,&channels[cid]->send);
// 		puts("Channles' message had been received");
// 		pthread_mutex_unlock(&channels[cid]->send);


		
// 		// while(!locked)locked =	pthread_mutex_trylock(&channels[cid]->recv);
// 		// locked = 0;
// 		// while(!channels[cid]->hadMessage);
// 		// pthread_mutex_unlock(&channels[cid]->recv);
// 		// while(!locked)locked = pthread_mutex_trylock(&channels[cid]->message);
// 		// locked = 0;
// 		// while(channels[cid]->hadMessage);
// 		// pthread_mutex_unlock(&channels[cid]->message);
// 	}
// }

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
			// pthread_cond_init(&channels[i]->condCS , NULL);
			// pthread_cond_init(&channels[i]->condCR , NULL);
			ret += pthread_mutex_init(&channels[i]->send , NULL) + 1;
			ret += pthread_mutex_init(&channels[i]->recv , NULL) + 1;
			// ret = pthread_create(&channels[i]->pid , NULL , (void *)working, &channels[i]->id);   
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
// void *producer(void *p)  
// {  
//     struct msg *mp;  
//     while(1){  
//        mp =  malloc(sizeof(struct msg));  
//        mp->num = rand() % 1000 + 1;  
//        printf("produce %d\n", mp->num);  
       
//        pthread_mutex_lock(&lock);  
//        mp->next = head;  
//        head = mp;         
//        pthread_mutex_unlock(&lock);  
//        pthread_cond_signal(&has_product);         
//        sleep(2);  
//     }  
// }  
  
// void *consumer(void *p)  
// {  
//     while(1){  
//         pthread_cond_wait(&has_product, &lock);  
      
//         struct msg *tmp = head;  
//         while(tmp != NULL) {  
//             printf("%d ", tmp->num);  
//                 tmp = tmp->next;  
//         }  
//         printf("\n");  
//     }  
// }  

int ch_send(int channel, void* msg){
	if(channel >= 8){
		puts("Error: Channel number is larger than 7!");
		return -1;
	}else{
		// usleep(1000);
		// int locked = 0;
		// while(!locked)locked = pthread_mutex_trylock(&channels[channel]->send);
			// puts("Sends called");
			pthread_mutex_lock(&channels[channel]->send);
			while(channels[channel]->hadMessage == 1)pthread_cond_wait(&channels[channel]->condS,&channels[channel]->send);
			// puts("Sends get singal");
			// pthread_cond_wait(&channels[channel]->condS,&channels[channel]->message);
			channels[channel]->item = (void*)malloc(sizeof((void*) msg));
			channels[channel]->item = msg;
			msg = NULL;
			free(msg);
			channels[channel]->hadMessage = 1;
			pthread_cond_broadcast(&channels[channel]->condR);
			pthread_mutex_unlock(&channels[channel]->send);
			// puts("Sends finished!");
					// int locked = 0;
		// while(!locked){
		// 	locked = pthread_mutex_trylock(&channels[channel]->message);
		// 	if(locked){
		// 		if(channels[channel]->hadMessage == 0){
		// 			channels[channel]->item = (void*)malloc(sizeof((void*) msg));
		// 			channels[channel]->item = msg;
		// 			msg = NULL;
		// 			free(msg);
		// 			channels[channel]->hadMessage = 1;
		// 			usleep(1000);
		// 		}else{
		// 			locked = 0;
		// 		}
		// 		pthread_mutex_unlock(&channels[channel]->message);
				
		// 	}
			// usleep(1000);
		
		return 0;
	}
}

int ch_recv(int channel, void** dest){
	if(channel >= 8){
		puts("Error: Channel number is larger than 7!");
		return -1;
	}else{
		// usleep(1000);
		// printf("RECV called on channel %d!!\n", channel);
		// int locked = 0;
		// while(!locked)locked = pthread_mutex_trylock(&channels[channel]->recv);
		pthread_mutex_lock(&channels[channel]->recv);
		
		while(channels[channel]->hadMessage == 0)pthread_cond_wait(&channels[channel]->condR,&channels[channel]->recv);
		// puts("RECV get singal");
		// pthread_cond_wait(&channels[channel]->condR,&channels[channel]->recv);
		*dest = (void*)malloc(sizeof((void*) channels[channel]->item));
		*dest = channels[channel]->item;
		channels[channel]->item = NULL;
		free(channels[channel]->item);
		channels[channel]->hadMessage = 0;
		pthread_cond_broadcast(&channels[channel]->condS);
		pthread_mutex_unlock(&channels[channel]->recv);
		// puts("RECV finished");
				// int locked = 0;
		// while(!locked){

		// 	locked = pthread_mutex_trylock(&channels[channel]->recv);
		// 	if(locked){
		// 		if(channels[channel]->hadMessage == 1){
		// 			*dest = (void*)malloc(sizeof((void*) channels[channel]->item));
		// 			*dest = channels[channel]->item;
		// 			channels[channel]->item = NULL;
		// 			free(channels[channel]->item);
		// 			channels[channel]->hadMessage = 0;
		// 			usleep(1000);
		// 		}else{
		// 			locked = 0;
		// 		}
		// 		pthread_mutex_unlock(&channels[channel]->recv);

		// 	}
			// usleep(1000);
		
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
