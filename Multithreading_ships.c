#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_THREADS 5
#define NUMBER_OF_SHIPS 100
#define DELAY 0

typedef enum _SHIP_SIZE { THE_LAST, SMALL , MIDDLE , BIG } SHIP_SIZE;
typedef enum _PROD_TYPE { ALL, BREAD, BANANAS, CLOTHES } PROD_TYPE;

// structure for message queue
typedef struct _MSG {
    long mesg_type;
    SHIP_SIZE ship_size;
    PROD_TYPE prod_type;
} MSG;

int msgid = 0;

void create_ship_send_tosecond_threads(void);
void send_ship_tothreads(void);
void getting_ship_with_bread(void);
void getting_ship_with_bananas(void);
void getting_ship_with_clothes(void);
int convert_ship_size(SHIP_SIZE size);

/*creation of ships with goods and, depending on the goods,
 *  transfer to the 3rd, 4th and 5th flows as well as the counting of goods.
 */
////////////////////////////////////////////////////////////////////////
int main(void)
{
	int i = 0;
	key_t key = 0;
	int ret_msgctl = 0;

	pthread_t threads[MAX_THREADS] = { 0 };
	void* thread_functions[MAX_THREADS] = { create_ship_send_tosecond_threads, send_ship_tothreads,
											getting_ship_with_bread, getting_ship_with_bananas, getting_ship_with_clothes };
	int status = 0;

	/* create queue */
	key = ftok("src/Multithreading_ships.c", 65);
	if(-1 == key)
	{
		printf("Value error key\n");
		return -1;
	}
	msgid = msgget(key, 0666 | IPC_CREAT);
	if(-1 == msgid)
	{
		printf("Value error msgid\n");
		return -1;
	}
	for( i = 0; i< MAX_THREADS; i++)
	{
		status = pthread_create(&threads[i], NULL, thread_functions[i], NULL);
		if(0 != status )
		{
			printf("Function return error  pthread_create()\n");
			return -1;
		}
	}
    for(i = MAX_THREADS-1; i >= 0; i-- )
    {
    	status = pthread_join(threads[i], NULL);
    	if( 0 != status)
    	{
    		printf("Function pthread_join() return error\n");
    		return -1;
    	}
    }
	/* delete queue */
    msgctl(msgid, IPC_RMID, NULL);
    if(-1 == ret_msgctl )
    {
    	printf("Function msgctl() return error\n");
    	return -1;
    }
	return EXIT_SUCCESS;
}
//////////////////////////////////////////////////////////////
/*		create random ship and send information to second thread
*/
void create_ship_send_tosecond_threads(void)
{
	int i = 0;
	int k = 0;
	int count_all = 0;
	MSG message;
	memset(&message, 0, sizeof(message));
	srand(time(NULL));
	message.mesg_type = 10;

	for(i = 0; i < NUMBER_OF_SHIPS; i++ )
	{
		// create random ship and send it to thread 2 over queue 1
		message.prod_type = rand()%3 + 1;
		message.ship_size = rand()%3 + 1;
		count_all += convert_ship_size(message.ship_size);
		k = msgsnd(msgid, &message, sizeof(message), 0);
		printf("k = %d\n", k);
		usleep(DELAY);
	}
	// notify about the last ship
	message.ship_size = THE_LAST;
	message.prod_type = ALL;
	count_all += convert_ship_size(message.ship_size);
	msgsnd(msgid, &message, sizeof(message), 0);
	usleep(DELAY);
	printf("total = %d\n", count_all);
	return;
}
/////////////////////////////////////////////////////////////
/*	receiving the ship and, depending on its goods, transferring it further to 3 4 or 5 threads
 */
void send_ship_tothreads(void)
{
	int i = 0;
	int msg_size = 0;
	MSG message;
	memset(&message, 0, sizeof(message));
    do
    {
    	i++;
		msg_size = msgrcv(msgid, &message, sizeof(message), 10, 0);
		printf("Data Received [%d] is : %d %d %d\n", i,  message.ship_size, message.prod_type, msg_size);

		if (message.prod_type != ALL)
		{
			message.mesg_type = message.prod_type;
			msgsnd(msgid, &message, sizeof(message), 0);
			usleep(DELAY);
		}
		else
		{
			/* broadcast notification about the last ship */
			for(i = BREAD; i <= CLOTHES; i++)
			{
				message.mesg_type = i;
				msgsnd(msgid, &message, sizeof(message), 0);
				usleep(DELAY);
			}
		}
    } while(THE_LAST != message.ship_size);
	return;
}
////////////////////////////////////////////////////////////////
/*  getting a ship with bread
 *
 */
void getting_ship_with_bread(void)
{
	int count_bread = 0;
	MSG message;
	memset(&message, 0, sizeof(message));
	do
	{
		msgrcv(msgid, &message, sizeof(message), BREAD, 0);
		count_bread += convert_ship_size(message.ship_size);
	} while (THE_LAST != message.ship_size);
	printf("count_bread = %d\n", count_bread);
	return;
}
//////////////////////////////////////////////////////////////////
/*  getting a ship with bananas
 */
void getting_ship_with_bananas(void)
{
	int count_bananas = 0;
	MSG message;
	memset(&message, 0, sizeof(message));
	do
	{
		msgrcv(msgid, &message, sizeof(message), BANANAS, 0);
		count_bananas += convert_ship_size(message.ship_size);
	} while (THE_LAST != message.ship_size);
	printf("count_bananas = %d\n", count_bananas);
	return;
}
/////////////////////////////////////////////////////////////
/*	getting a ship with clothes
 */
void getting_ship_with_clothes(void)
{
	int count_clothes = 0;
	MSG message;
	memset(&message, 0, sizeof(message));
	do
	{
		msgrcv(msgid, &message, sizeof(message), CLOTHES, 0);
		count_clothes += convert_ship_size(message.ship_size);
	} while (THE_LAST != message.ship_size);

	printf("count_clothes = %d\n", count_clothes);
	return;
}
///////////////////////////////////////////////////////////////
/* counting the quantity of goods
 */
int convert_ship_size(SHIP_SIZE size)
{
	switch(size)
	{
		case SMALL:  return 10;
		case MIDDLE: return 50;
		case BIG:    return 100;
		default:     return 0; /* 0 for all not supported ship types */
	}
}

