#include <nanvix/pm.h>
#include <limits.h>
#include <pthread.h>
//#include <timer.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "master.h"

/* Number of buckets. */
#define NUM_BUCKETS 256
#define NUM_IO_CORES 4

/*
 * Thread's data.
 */
static struct tdata
{
	/* Thread's ID. */
	pthread_t tid;

	/* Arguments. */
	struct
	{
		int i0;               /* Start bucket.        */
		int in;               /* End bucket.          */
		int j0;               /* Start array's index. */
		struct bucket **done; /* Buckets.             */
		int *array;           /* Array.               */
	} args;
} tdata[NUM_IO_CORES];

/*
 * Thread's main.
 */
static void *thread_main(void *args)
{
	int i, j;        /* Loop indexes.  */
	struct tdata *t; /* Thread's data. */

	t = args;
	/* Rebuild array. */
	j = t->args.j0;
	for (i = t->args.i0; i < t->args.in; i++)
	{
		bucket_merge(t->args.done[i], &t->args.array[j]);
		j += bucket_size(t->args.done[i]);
	}

	pthread_exit(NULL);
	return (NULL);
}

/*
 * Rebuilds array.
 */
static void rebuild_array(struct bucket **done, int *array)
{
	int j;    /* array[] offset. */
	int i, k; /* Loop index.     */

#define BUCKETS_PER_CORE (NUM_BUCKETS/NUM_IO_CORES)

	/* Spawn threads. */
	j = 0;
	for (i = 0; i < NUM_IO_CORES; i++)
	{
		tdata[i].args.i0 = i*BUCKETS_PER_CORE;
		tdata[i].args.in = (i + 1)*BUCKETS_PER_CORE;
		tdata[i].args.done = done;
		tdata[i].args.array = array;
		pthread_create(&tdata[i].tid, NULL, thread_main, (void *)&tdata[i]);

		for (k = i*BUCKETS_PER_CORE; k < (i + 1)*BUCKETS_PER_CORE; k++)
			j += bucket_size(done[k]);
	}

	/* Join threads. */
	for (i = 0; i < NUM_IO_CORES; i++)
		pthread_join(tdata[i].tid, NULL);
}

/*
 * Bucket-sort algorithm.
 */
extern void bucketsort(int *array, int n)
{
	int max;                  /* Maximum number.      */
	int i, j;                 /* Loop indexes.        */
	int range;                /* Bucket range.        */
	//int inbox;
	//int outbox[NR_CCLUSTER];
	struct minibucket *minib; /* Working mini-bucket. */
	struct message *msg;      /* Working message.     */
	struct bucket **todo;     /* Todo buckets.        */
	struct bucket **done;     /* Done buckets.        */
	long start, end;      /* Timers.              */
	k1_timer_init();
	/* Setup slaves. */
	open_noc_connectors();
	spawn_slaves();

	todo = smalloc(NUM_BUCKETS*sizeof(struct bucket *));
	done = smalloc(NUM_BUCKETS*sizeof(struct bucket *));
	for (i = 0; i < NUM_BUCKETS; i++)
	{
		done[i] = bucket_create();
		todo[i] = bucket_create();
	}

	/* Find max
	 * number in the
	 * array. */
	start = k1_timer_get();
	max = INT_MIN;
	for (i = 0; i < n; i++)
	{
		/* Found. */
		if (array[i] > max)
			max = array[i];
	}

	/* Distribute
	 * numbers.
	 */
	range = max/NUM_BUCKETS;
	for (i = 0; i < n; i++)
	{
		j = array[i]/range;
		if (j >= NUM_BUCKETS)
			j = NUM_BUCKETS - 1;

		bucket_insert(&todo[j], array[i]);
	}
	end = k1_timer_get();
	master += k1_timer_diff(start, end);

	/* Sort
	 * buckets.
	 */
	j = 0;
	for (i = 0; i < NUM_BUCKETS; i++)
	{
		while (bucket_size(todo[i]) > 0)
		{
			minib = bucket_pop(todo[i]);
			/* Send
			 * message.
			 */

			msg = message_create(SORTWORK, i, minib->size);
			data_send(outfd[j], msg, sizeof(struct message));
			message_destroy(msg);

			/* Send
			 * data.
			 */

			data_send(outfd[j], minib->elements, minib->size*sizeof(int));
			minibucket_destroy(minib);
			j++;

			/*
			 * Slave processes are busy.
			 * So let's wait for results.
			 */
			if (j == nclusters)
			{

				/* Receive
				 * results.
				 */
				for (/* NOOP */ ; j > 0; j--)
				{
					/* Receive
					 * message.
					 */
					//mailbox_read(inbox, &msg);
					msg = message_create(DIE);
					data_receive(infd, nclusters-j, msg, sizeof(struct message));
					assert(msg->type == SORTRESULT);
					/* Receive
					 * mini-bucket.
					 */
					minib = minibucket_create();
					minib->size = msg->u.sortresult.size;
					data_receive(infd,nclusters-j, minib->elements,
							minib->size*sizeof(int));
					
					bucket_push(done[msg->u.sortresult.id], minib);

				}
			}
		}
	}

	/* Receive
	 * results.
	 */
	msg = message_create(DIE);
	for (/* NOOP */ ; j > 0; j--)
	{
		/* Receive
		 * message.
		 */
		data_receive(infd, j, msg, sizeof(struct message));
		/* Receive
		 * bucket.
		 */
		minib = minibucket_create(); 
		minib->size = msg->u.sortresult.size;

		data_receive(infd, j-1, minib->elements, minib->size*sizeof(int));

		bucket_push(done[msg->u.sortresult.id], minib);

	}

	message_destroy(msg);
	start = k1_timer_get();
	rebuild_array(done, array);
	end = k1_timer_get();
	master += k1_timer_diff(start, end);

	/* House
	 * keeping.
	 */
	msg = message_create(DIE);
	for (i = 0; i < nclusters; i++) {
		data_send(outfd[i], msg, sizeof(struct message));
	}

	for (i = 0; i < NUM_BUCKETS; i++)
	{
		bucket_destroy(todo[i]);
		bucket_destroy(done[i]);
	}
	free(done);
	free(todo);
	join_slaves();
	close_noc_connectors();
}
