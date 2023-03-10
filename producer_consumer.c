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
struct task_struct *consumer_thread;


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
	int i = 0;
	for (i = 0; i < buffSize; i++)
	{
		buffer[in] = i;
		in = (in + 1) % buffSize;
	}
	return 0;
}

int consumer(void *data)
{
	int i, value;
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
	struct task_struct *consumer_thread;

	//Create the buffer with size buffSize. 
	//Otherwise return Out of Memory Error Code
	buffer = kmalloc(buffSize * sizeof(int), GFP_KERNEL);
	if(!buffer)
	{
		return -ENOMEM;
	}

	producer_thread = kthread_run(producer, NULL, "Producer");
	consumer_thread = kthread_run(consumer, NULL, "Consumer");












	//semaphore test
	//struct semaphore empty;
	//sema_init(&empty, 5);

	//struct task_struct* p;
	//size_t process_counter = 0;
	//for_each_process(p)
	//{
	//	printk("Individual Process Ran\n");
	//	++process_counter;
	//}

	//printk("Processes: %d", process_counter);
	//Display();
	//printk("Hello Kernel\n");
	return 0;
}


static void ModuleExit(void)
{

	//Stop both the producer and consumer thread
	kthread_stop(producer_thread);
	kthread_stop(consumer_thread);

	//Free up the buffer memory
	kfree(buffer);



	//id funciton exists but cant remember
	//printk("The total elapsed time of all processes for UID <UID of the user> is ");
	//kthread_stop(kthread_3);
}

module_init(ModuleInit);
module_exit(ModuleExit);
MODULE_LICENSE("GPL");
