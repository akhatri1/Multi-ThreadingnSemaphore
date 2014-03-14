#include<pthread.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<semaphore.h>
#include<time.h>


int front= -1, rear= -1; 
sem_t sem1;
sem_t sem2;
int NUM_CUST = 10;
int processed_count = 0;
int served_count = 0;
int s_queue_index = 0;

struct dish
{
	int itemID;
	char description[50];
	float price;
	int prepTime;
};
struct dish dishes[10];

struct order
{
	int clientID;
	struct dish dish1;
	time_t atime;
	time_t ctime;
};
struct order o_queue[100];
struct order s_queue[100];
struct order info_queue[100];

void process_client(struct order ord)
{
	sem_wait(&sem2);

	ord.ctime = time(0);
	s_queue[s_queue_index++] = ord;
	
	sem_post(&sem2);
}

void insert_o_queue(struct order ord)
{
	sem_wait(&sem1);
	printf("\nINSERTED: CustomerID %d in the queue", ord.clientID);
	if(front == -1  && rear == -1)
	{	
		front =0;
		rear = 0;
		o_queue[rear] = ord;
	}
	else if( (rear + 1)% 100 == front)
	{
		printf("\n*** RESTAURENT FULL ***");
	}
	else
	{
		o_queue[++rear] = ord;
	}
	sem_post(&sem1);
}

struct order remove_o_queue()
{
	sem_wait(&sem1);
	struct order ord; 
	if(front == -1 && rear == -1)
	{
		printf("\n*** RESTAURENT EMPTY ***");
		ord = (struct order){-1, {-1,"", 0.0f, 0}, 0, 0};
		
	}
	else if(front == rear)
	{
		ord = o_queue[front];
		front = -1;
		rear = -1;
		
	}
	else
	{
		ord = o_queue[front];
		front = (front + 1) % 100;
		
	}
	sem_post(&sem1);
	return ord;
	
}

void initialize()
{
	dishes[0] = (struct dish){1, "BBQ-Chicken-Salad", 8.95f, 180};
	dishes[1] = (struct dish){2, "Spinach-Power", 9.15f, 180};
	dishes[2] = (struct dish){3, "Garden-Salad", 4.75f, 100};
	dishes[3] = (struct dish){4, "Steak-Blue-Cheese", 7.15f, 220};
	dishes[4] = (struct dish){5, "Slovenian-Salad", 6.75f, 130};
	dishes[5] = (struct dish){6, "Hong-Kong-Chicken-Salad", 9.15f, 150};
	dishes[6] = (struct dish){7, "Mongolian-BBQ-Plate", 9.75f, 210};
	dishes[7] = (struct dish){8, "Club-Sandwitch", 6.35f, 135};
	dishes[8] = (struct dish){9, "Belgian-Cheese-Sub", 10.25f, 150};
	dishes[9] = (struct dish){10, "Chipotle-Beef-Sub", 9.35f, 180};
}

void* server(void * arg)
{
	struct order ord1;
	float totalRevenue = 0;
	time_t AvgtimeWaiting = 0, AvgtimeSpend = 0;
	int counter[10] = {0}, tempCounter = 0;
	char *topDishes[10], *temp;
	FILE *fp = NULL;
	fp = fopen("SCU_Info.txt","a+");
	int i,r,j;
	while(1)
	{
		sleep(1);
		
		sem_wait(&sem2);
		
		for(i = 0; i < s_queue_index; i++)
		{
			struct order ord = s_queue[i];
			
			if( (ord.clientID != -1) && (ord.dish1.prepTime < (time(0) - ord.ctime) )) // multiplying by 10 {(time(0) - ord.ctime)*10 } to avoid waiting during debugging
			{
				s_queue[i] = (struct order){-1, {-1,"", 0.0f, 0}, 0, 0};
				//printf("\n customer served");
				ord.atime = time(0)-ord.atime;
				ord.ctime = time(0)-ord.ctime;
				info_queue[served_count] = ord;
				served_count++;
				r = rand()%10;
				sleep(r); // this is the service time
			
			}
		}
		if(served_count == NUM_CUST)
		{
			printf("\nAll customers are served by the server");
			for(i=0;i<served_count;i++)
			{
				ord1 = info_queue[i];
				totalRevenue = totalRevenue + ord1.dish1.price;
				AvgtimeWaiting = AvgtimeWaiting + ord1.ctime;
				AvgtimeSpend = AvgtimeSpend + ord1.atime;
				counter[ord1.dish1.itemID -1] = counter[ord1.dish1.itemID -1] + 1;
				topDishes[ord1.dish1.itemID -1] = ord1.dish1.description;
				//printf("\n %d %s",ord1.dish1.itemID -1,topDishes[ord1.dish1.itemID -1]);
				fprintf(fp,"\n____________________________________________________\n");
				fprintf(fp,"*** customer ID = %d\n",ord1.clientID);
				fprintf(fp,"*** item ID = %d\n",ord1.dish1.itemID);
				fprintf(fp,"*** description = %s\n",ord1.dish1.description);
				fprintf(fp,"*** price = %f\n",ord1.dish1.price);
				fprintf(fp,"*** total time spent in store = %d\n",ord1.atime);
				fprintf(fp,"*** total time spent in waiting = %d\n",ord1.ctime);
				fprintf(fp,"\n____________________________________________________\n");
				printf("\n____________________________________________________");
				printf("\n*** customer ID = %d ", ord1.clientID);
				printf("\n*** item ID = %d ", ord1.dish1.itemID);
				printf("\n*** description = %s ", ord1.dish1.description);
				printf("\n*** price = %f ", ord1.dish1.price);
				printf("\n*** prepTime = %d ", ord1.dish1.prepTime);
				printf("\n*** total time spent in store = %d ", ord1.atime);
				printf("\n*** total time spent in waiting = %d ",ord1.ctime);
				printf("\n____________________________________________________");
			}
			fprintf(fp,"\n Average time spend in the store = %f",(float)(AvgtimeSpend/served_count));
			fprintf(fp,"\n Average time spend waiting = %f",(float)(AvgtimeWaiting/served_count));
			fprintf(fp,"\n Total Revenue generated = %f",totalRevenue);
			fprintf(fp,"\n Total Number of Customers = %d",served_count);
			printf("\n Average time spend in the store = %f",(float)(AvgtimeSpend/served_count));
			printf("\n Average time spend waiting = %f",(float)(AvgtimeWaiting/served_count));
			printf("\n Total Revenue generated = %f",totalRevenue);
			printf("\n Total Number of Customers = %d",served_count);
			
			
			break;
		}
		sem_post(&sem2);
	}
	fclose(fp);
}

void* cashier1(void * arg)
{
	while(1)
	{
		struct order ord = remove_o_queue();
		if(ord.clientID == -1)
		{
			printf("\ncashier1 : TAKING BREAK 10 seconds");
			sleep(10);  // sleeping for tbreak amount of time
		}
		else
		{
			sleep(5); // service time
			process_client(ord);
			processed_count++;
			printf("\ncashier1 : processed %d ", ord.clientID);
		}
		if(processed_count == NUM_CUST)
		{
			printf("\ncashier1: All customers are processed by cashiers");
			break;
		}
		
		
	}

}
void* cashier2(void * arg)
{
	int i;	
	
	struct order ord1;
	while(1)
	{
		struct order ord = remove_o_queue();
		if(ord.clientID == -1)
		{
			printf("\ncashier2 : TAKING BREAK 10 seconds ");
			sleep(10);  // sleeping for tbreak amount of time
		}
		else
		{
			sleep(5); // service time
			process_client(ord);
			processed_count++;
			printf("\ncashier2 : processed clinet with ID: %d", ord.clientID);
		}
		if(processed_count == NUM_CUST)
		{
			printf("\ncashier2: All customers are processed by cashiers");
			break;
		}
				
	}
	
}

void* client(void * arg)
{
	int i, r, clientID;
	srand(time(NULL));
	

	for(i =0; i< NUM_CUST; i++)
	{
		r = rand()%10;
		sleep(r);
		
		int dish_no = rand() % 10;
		
		time_t atime = time(0);
		clientID = rand() % 999;
		
		struct order ord = (struct order){clientID, dishes[dish_no], atime, 0};

		
		insert_o_queue(ord);
	}
	
}

void main()
{
	int a =0, i, res;
	pthread_t tid[4];

	res = sem_init(&sem1, 0, 1); // third argument : initial value of the semaphore (binary semaphore here
	res = sem_init(&sem2, 0, 1);

	initialize();

	a = pthread_create( &tid[0], NULL, &server, NULL);
	a = pthread_create( &tid[1], NULL, &cashier1, NULL);
	a = pthread_create( &tid[2], NULL, &cashier2, NULL);
	a = pthread_create( &tid[3], NULL, &client, NULL);

	

	pthread_exit(0);

	sem_destroy(&sem1);
	sem_destroy(&sem2);
}
