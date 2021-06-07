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
#define MAX 10
#define SHIPS 3
#define SUCCESS 0
#define NUMBER_OF_SHIPS 10
#define DELAY 10

typedef enum _SHIP_SIZE { THE_LAST, SMALL , MIDDLE , BIG } SHIP_SIZE;
typedef enum _PROD_TYPE { ALL, BREAD, BANANAS, CLOTHES } PROD_TYPE;

// structure for message queue
typedef struct _MSG {
    long mesg_type;
    SHIP_SIZE ship_size;
    PROD_TYPE prod_type;
} MSG;

int msgid = 0;

void hello_world1();
void hello_world2();
void hello_world3();
void hello_world4();
void hello_world5();
int convert_ship_size(SHIP_SIZE size);

/////////////////////////////////////////
int main(void)
{
	int i = 0;
	key_t key = 0;

	pthread_t threads[MAX_THREADS] = { 0 };
	void* thread_functions[MAX_THREADS] = { hello_world1, hello_world2, hello_world3, hello_world4, hello_world5};
	int status[MAX_THREADS] = { 0 };
	int status_addr[MAX_THREADS] = { 0 };

	/* create queue */
	key = ftok("src/Multithreading_ships.c", 65);
	msgid = msgget(key, 0666 | IPC_CREAT);

	for( i = 0; i< MAX_THREADS; i++)
	{
		status[i] = pthread_create(&threads[i], NULL, thread_functions[i], NULL);
	}

    for(i = MAX_THREADS-1; i >= 0; i-- )
    {
    	status[i] = pthread_join(threads[i], NULL);
    }
	/* delete queue */
    msgctl(msgid, IPC_RMID, NULL);

	return EXIT_SUCCESS;
}

void hello_world1()
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

void hello_world2()
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

void hello_world3()
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

void hello_world4()
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
void hello_world5()
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

