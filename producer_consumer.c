#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/sched/signal.h>
#include <linux/semaphore.h>
#include <linux/ktime.h>
#include <linux/slab.h>

//Two Thread Pointers
struct task_struct *producer_thread;
struct task_struct **consumer_threads;

//Buffer and its size
static int buffSize = 10;
struct task_struct **buffer;

int prod = 0;
int cons = 0;
int uuid = 0;

//Total Elapsed time for the program
u64 total_elapsed_time;

//Define Semaphore
static struct semaphore empty;
static struct semaphore full;

//Counters
int producer_count = 0;
int consumer_count = 0;

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
	int index = 0;

	for_each_process(process)
	{
		if(uuid == process->cred->uid.val)
		{
			if(down_interruptible(&empty)) {break;}
			index = (producer_count) % buffSize;
			producer_count++;
			buffer[index] = process;
			printk("[kProducer-1] Produce-Item:%d at buffer index: %d for PID:%d\n", producer_count, index, process->pid);
			up(&full);
		}
	}

	return 0;
}

int consumer(void *data)
{
	struct task_struct *process;
	int index = 0;
	int* threadID = (int*)data;
	u64  current_time, elapsed_time;
	int seconds, minutes, hours;

	while(!kthread_should_stop())
	{
		if(down_interruptible(&full)) {break;}
		index = consumer_count % buffSize;
		consumer_count++;
		process = buffer[index];

		//Get current time
		current_time = ktime_get_ns();
		elapsed_time = current_time - process->start_time;
		total_elapsed_time += elapsed_time;

		seconds = (int)(elapsed_time/ 1000000000);
		minutes = seconds / 60;
		hours = minutes/60;
		seconds = seconds % 60;
		minutes = minutes %60;

		printk("[kConsumer-%d] consumed-Item:%d at buffer index: %d for PID:%di\tElapsed Time-%d:%d:%d\n", *threadID, consumer_count, index,  process->pid, hours, minutes, seconds);
		up(&empty);
	}
	
	return 0;
}



static int ModuleInit(void)
{
	//Create the producer and consumer threads
	struct task_struct *producer_thread;
	struct task_struct **consumer_threads;

	int index = 0; 

	sema_init(&empty, buffSize);
	sema_init(&full, 0);

	printk("CSE330 POroject-2 Kernel Module Inserted\n");
	printk("Kernel module received the following inputs: UID:%d, Buffer-Size:%d No of Producer:%d No of Consumer:%d\n", uuid, buffSize, prod, cons);

	//Create the buffer with size buffSize. 
	//Otherwise return Out of Memory Error Code
	buffer = kmalloc(buffSize * sizeof(struct task_struct*), GFP_KERNEL);
	if(!buffer)
	{
		return -ENOMEM;
	}

	//Create producer thread
	if(prod == 1)
	{
		producer_thread = kthread_run(producer, NULL, "Producer");

		//Check to see if producer thread was created successfully
		if(!IS_ERR(producer_thread))
		{
			printk("[kProducer-1] kthread Producer Created Successfully");
		}
	}

	//Create specified # of Consumer threads
	if(cons > 0)
	{
		consumer_threads = kmalloc(cons * sizeof(struct task_struct*), GFP_KERNEL);
		
		for(index = 0; index < cons; index++)
		{
			int* threadID = kmalloc(sizeof(int), GFP_KERNEL);
			*threadID = index;
			consumer_threads[index] = kthread_run(consumer, (void*)threadID, "Consumer");
			if(!IS_ERR(consumer_threads[index]))
			{
				printk("[kConsumer-%d] kthread Consumer Created Successfully\n", index);
			}
		}
	}

	return 0;
}


static void ModuleExit(void)
{
	int index;

	//Stop producer thread if exists
	if(prod == 1 && producer_thread != NULL)
	{
		//Currently an error with rrmod
		//kthread_stop(producer_thread);	
		//printk("[kProducer-1] Producer Thread stopped.\n");
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
				//printk("[kConsumer-%d] Consumer Thread stopped.\n", index);
			}
		}
	}

	printk("Total number of items produced: %d\n", producer_count);
	printk("Total number of items consumed: %d\n", consumer_count);
	//printk("The total elapsed time of all processes for UID %d is %d:%d:%d\n", uid);
	printk("CSE330 POroject-2 Kernel Module Removed\n");

	//Free up the buffer memory
	kfree(buffer);
}

module_init(ModuleInit);
module_exit(ModuleExit);
MODULE_LICENSE("GPL");
