#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/sched/signal.h>
#include <linux/semaphore.h>
#include <linux/time.h>
#include <linux/slab.h>

//Two Thread Pointers
struct task_struct *producer_thread;
struct task_struct **consumer_threads;


//Buffer and its size
static int buffSize = 10;
int *buffer;
static int in;
static int out;

int prod = 0;
int cons = 0;
int uuid = 0;

//Define variables as readable and rritasble
module_param(buffSize, int, S_IRUSR | S_IWUSR);
module_param(prod, int, S_IRUSR | S_IWUSR);
module_param(cons, int, S_IRUSR | S_IWUSR);
module_param(uuid, int, S_IRUSR | S_IWUSR);

void Display(void)
{
	printk("Test: buffSize = %d", buffSize);
	printk("Test: prod = %d", prod);
	printk("Test: cons = %d", cons);
	printk("Test: uuid = %d", uuid);
}


int producer(void *data)
{

	struct task_struct *process;

	//TEMPORARY: PRINT TEST
	for_each_process(process)
	{
		printk("Process: %d, UUID: %d\n", process->pid, process->cred->uid.val);
	}




	//TEMPORARY: OLD TEST CODE
	//int i = 0;
	//for (i = 0; i < buffSize; i++)
	//{
	//	buffer[in] = i;
	//	in = (in + 1) % buffSize;
	//}
	return 0;
}

int consumer(void *data)
{

	int* values = (int*)data;
	int i, value;
	printk("Value is %d\n", *values);

	for(i = 0; i < buffSize; i++)
	{
		value = buffer[out];
		out = (out + 1) % buffSize;
	}
	return 2;
}



static int ModuleInit(void)
{

	//Create the producer and consumer threads
	struct task_struct *producer_thread;
	struct task_struct **consumer_threads;

	int index = 0; 


	//TMEPORARY: Display Varaibles
	Display();

	//Create the buffer with size buffSize. 
	//Otherwise return Out of Memory Error Code
	buffer = kmalloc(buffSize * sizeof(int), GFP_KERNEL);
	if(!buffer)
	{
		return -ENOMEM;
	}

	//Create producer thread
	if(prod == 1)
	{
		producer_thread = kthread_run(producer, NULL, "Producer");
	}

	//Create specified # of Consumer threads
	if(cons > 0)
	{
		consumer_threads = kmalloc(sizeof(struct task_struct*), GFP_KERNEL);
		
		for(index = 0; index < cons; index++)
		{
			int* threadID = kmalloc(sizeof(int), GFP_KERNEL);
			*threadID = index;
			consumer_threads[index] = kthread_run(consumer, (void*)threadID, "Consumer");
		}
	}

	//semaphore test
	//struct semaphore empty;
	//sema_init(&empty, 5);
	
	return 0;
}


static void ModuleExit(void)
{
	int index;

	//Stop producer thread if exists
	if(prod == 1 && producer_thread != NULL)
	{
		//Currently an error with rmmod and below line
		//kthread_stop(producer_thread);
	}

	//Stop Consumer Threads if exists
	if(cons > 0)
	{
		for(index = 0; index < cons; index++)
		{
			if(consumer_threads[index] != NULL)
			{

				//Currently an error with rmmod and below line
				//kthread_stop(consumer_threads[index]);
			}
		}
	}

	//Free up the buffer memory
	kfree(buffer);
	//printk("The total elapsed time of all processes for UID <UID of the user> is ");
}

module_init(ModuleInit);
module_exit(ModuleExit);
MODULE_LICENSE("GPL");
