/*
 * Copyright(C) 2011-2017 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 *
 * This file is part of Nanvix.
 *
 * Nanvix is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Nanvix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
 */
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#include <mppa/osconfig.h>
#include <nanvix/hal.h>
#include <nanvix/mm.h>
#include <nanvix/pm.h>
#include <nanvix/name.h>
#include <nanvix/klib.h>

#ifdef DEBUG
#include <stdio.h>
#endif

#define NAME_SERVER_NODE 0

static pthread_mutex_t lock;

/**
 * @brief Number of registration.
 */
static int nr_registration = 0;

/**
 * @brief Lookup table of process names.
 */
static struct {
	int nodeid;               /**< NoC node ID.  */
	char name[PROC_NAME_MAX]; /**< Process name. */
} names[HAL_NR_NOC_NODES];

/*=======================================================================*
 * _name_lookup()                                                        *
 *=======================================================================*/

/**
 * @brief Converts a name into a NoC node ID.
 *
 * @param name 		Target name.
 *
 * @returns Upon successful completion the NoC node ID whose name is @p
 * name is returned. Upon failure, a negative error code is returned
 * instead.
 */
static int _name_lookup(const char *name)
{
	/* Search for portal name. */
	for (int i = 0; i < HAL_NR_NOC_NODES; i++)
	{
		/* Found. */
		if (!strcmp(name, names[i].name))
			return (names[i].nodeid);
	}

	return (-ENOENT);
}

/*=======================================================================*
 * _name_link()                                                          *
 *=======================================================================*/

/**
 * @brief Register a process name.
 *
 * @param nodeid		NoC node ID of the process to register.
 * @param name			Name of the process to register.
 *
 * @returns Upon successful registration the number of name registered
 * is returned. Upon failure, a negative error code is returned instead.
 */
static int _name_link(int nodeid, char *name)
{
	int index;          /* Index where the process will be stored. */

	/* No entry available. */
	if (nr_registration >= HAL_NR_NOC_NODES)
		return (-EINVAL);

	/* Check that the name is not already used */
	for (int i = 0; i < HAL_NR_NOC_NODES; i++)
	{
		if (strcmp(names[i].name, name) == 0)
			return (-EINVAL);
	}

	/* Find index. */
	for (int i = 0; i < HAL_NR_NOC_NODES; i++)
	{
		/* Found. */
		if (names[i].nodeid == nodeid)
		{
			index = i;
			goto found;
		}
	}

	return (-EINVAL);

found:

	/* Entry not available */
	if (strcmp(names[index].name, ""))
		return (-EINVAL);

#ifdef DEBUG
	printf("writing [nodeid ID:%d name: %s] at index %d.\n",
	                   names[index].nodeid, name, index);
#endif

	strcpy(names[index].name, name);

	return (++nr_registration);
}

/*=======================================================================*
 *_name_unlink()                                                         *
 *=======================================================================*/

/**
 * @brief Remove a name
 *
 * @param name			Name of the process to unlink.
 *
 * @returns Upon successful registration the new number of name registered
 * is returned. Upon failure, a negative error code is returned instead.
 */
static int _name_unlink(char *name)
{
	/* Search for portal name. */
	int i = 0;

	while (i < HAL_NR_NOC_NODES && strcmp(name, names[i].name))
	{
		i++;
	}

	if (i < HAL_NR_NOC_NODES)
	{
		strcpy(names[i].name, "\0");
		return (--nr_registration);
	}

	return (-ENOENT);
}

/*===================================================================*
 * name_server()                                                     *
 *===================================================================*/

/**
 * @brief Handles remote name requests.
 *
 * @param args Server arguments.
 *
 * @returns Always returns NULL.
 */
static void *name_server(void *args)
{
	int inbox;   /* Mailbox for small messages. */
	int source;  /* NoC node ID of the client   */
	int tmp;

	((void) args);

	/* Open server mailbox. */
	pthread_mutex_lock(&lock);
		inbox = hal_mailbox_create(hal_noc_nodes[NAME_SERVER_NODE]);
	pthread_mutex_unlock(&lock);

	while(1)
	{
		struct name_message msg;

		assert(hal_mailbox_read(inbox, &msg, HAL_MAILBOX_MSG_SIZE)
		                                  == HAL_MAILBOX_MSG_SIZE);

		/* Handle name requests. */
		switch (msg.op)
		{
			/* Lookup. */
			case NAME_LOOKUP:
#ifdef DEBUG
				printf("Entering NAME_LOOKUP case... name provided:%s.\n"
						                                     , msg.name);
#endif
				msg.nodeid = _name_lookup(msg.name);

				/* Send response. */
				source = hal_mailbox_open(msg.source);
				assert(source >= 0);
				assert(hal_mailbox_write(source, &msg, HAL_MAILBOX_MSG_SIZE)
				                                   == HAL_MAILBOX_MSG_SIZE);
				assert(hal_mailbox_close(source) == 0);
				break;

			/* Add name. */
			case NAME_LINK:
#ifdef DEBUG
				printf("Entering NAME_LINK case... [nodeid ID: %d, name: %s].\n"
				                                       , msg.nodeid, msg.name);
#endif
				tmp = nr_registration;

				if (_name_link(msg.nodeid, msg.name) == (tmp + 1))
				{
					msg.op = NAME_SUCCESS;
				}
				else
				{
					msg.op = NAME_FAIL;
				}

				/* Send acknowledgement. */
				source = hal_mailbox_open(msg.source);
				assert(source >= 0);
				assert(hal_mailbox_write(source, &msg, HAL_MAILBOX_MSG_SIZE)
				                                   == HAL_MAILBOX_MSG_SIZE);
				assert(hal_mailbox_close(source) == 0);

				break;

			/* Remove name. */
			case NAME_UNLINK:
#ifdef DEBUG
				printf("Entering NAME_UNLINK case... name: %s.\n", msg.name);
#endif
				assert(nr_registration > 0);

				tmp = nr_registration;

				if (_name_unlink(msg.name) == (tmp - 1))
				{
					msg.op = NAME_SUCCESS;
				}
				else
				{
					msg.op = NAME_FAIL;
				}

				/* Send acknowledgement. */
				source = hal_mailbox_open(msg.source);
				assert(source >= 0);
				assert(hal_mailbox_write(source, &msg, HAL_MAILBOX_MSG_SIZE)
				                                   == HAL_MAILBOX_MSG_SIZE);
				assert(hal_mailbox_close(source) == 0);

				break;

			/* Should not happen. */
			default:
				break;
		}
	}

	/* House keeping. */
	pthread_mutex_lock(&lock);
		hal_mailbox_unlink(inbox);
	pthread_mutex_unlock(&lock);

	return (NULL);
}

/*===================================================================*
 * name_init()                                                       *
 *===================================================================*/

/**
 * @brief Initializes the name server.
 */
static void name_init(void)
{
	/* Initialize lookup table. */
	for (int i = 0; i < HAL_NR_NOC_NODES; i++)
	{
		names[i].nodeid = hal_noc_nodes[i];
		strcpy(names[i].name, "");
	}

	strcpy(names[hal_noc_nodes[NAME_SERVER_NODE]].name, "/io0");
}

/*===================================================================*
 * main()                                                            *
 *===================================================================*/

/**
 * @brief Resolves process names.
 */
int main(int argc, char **argv)
{
	int global_barrier; /* System barrier. */
	pthread_t tid;      /* Thread ID.      */

	((void) argc);
	((void) argv);

#ifdef DEBUG
	/* printf("[NAME] booting up server\n"); */
#endif

	/* Spawn name server thread. */
	assert((pthread_create(&tid,
		NULL,
		name_server,
		NULL)) == 0
	);

	name_init();

	/* Release master IO cluster. */
	global_barrier = barrier_open(NR_IOCLUSTER);
	barrier_wait(global_barrier);

#ifdef DEBUG
	/* printf("[NAME] server alive\n"); */
#endif

	/* Wait for name server thread. */
	pthread_join(tid, NULL);

	/* House keeping. */
	barrier_close(global_barrier);

	return (EXIT_SUCCESS);
}
