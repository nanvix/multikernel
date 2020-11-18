/*
 * MIT License
 *
 * Copyright(c) 2011-2020 The Maintainers of Nanvix
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Must come first. */
#define __NEED_NAME_SERVICE
#define __NEED_RPC_SERVER
#define __NEED_RPC_SERVICE

#include <nanvix/sys/task.h>
#include <nanvix/servers/rpc.h>
#include <nanvix/runtime/pm.h>
#include <nanvix/config.h>
#include <nanvix/ulib.h>
#include <posix/errno.h>
#include <posix/stdbool.h>

#if __NANVIX_USE_TASKS

/*============================================================================*
 * Structures                                                                 *
 *============================================================================*/

/**
 * @brief Period of the main task.
 */
#define RPC_PERIOD 128

/**
 * @brief Periodic RPC task.
 */
static struct task rpc_dispatcher;

/**
 * @brief RPC structures.
 */
static struct rpc rpcs[RPC_MAX];

/**
 * @brief Lock.
 */
static spinlock_t rpc_lock;

/**
 * @brief Is the name service initialized ?
 */
static bool initialized = false;

/*============================================================================*
 * Internal Functions                                                         *
 *============================================================================*/

/*============================================================================*
 * __nanvix_rpc_config()                                                      *
 *============================================================================*/

static inline void nanvix_rpc_config(int i, int rid, rpc_fn request, rpc_fn response)
{
	/* Configures structure. */
	rpcs[i].rid      = rid;
	rpcs[i].request  = request;
	rpcs[i].response = response;
}

/*============================================================================*
 * nanvix_rpc_handler()                                                       *
 *============================================================================*/

static inline int nanvix_rpc_handler(struct task_args * args)
{
	int ret;
	int source;
	int portal_port;
	int mailbox_port;
	rpc_fn fn;

	spinlock_lock(&rpc_lock);

		/* Gets RPC index. */
		ret = (int) args->arg0;

		/* Gets function and arguments. */
		fn           = rpcs[ret].request;
		source       = rpcs[ret].msg.header.source;
		mailbox_port = rpcs[ret].msg.header.mailbox_port;
		portal_port  = rpcs[ret].msg.header.portal_port;
		umemcpy(args, &rpcs[ret].msg.args, sizeof(struct task_args));

	spinlock_unlock(&rpc_lock);

	/* Execute RPC. */
	ret = fn(
		source,
		mailbox_port,
		portal_port,
		args->arg0,
		args->arg1,
		args->arg2,
		args->arg3,
		args->arg4,
		args->arg5
	);

	/* Error? */
	if (ret < 0)
		return (TASK_RET_ERROR);

	/* Success. */
	return (TASK_RET_SUCCESS);
}

/*============================================================================*
 * __nanvix_rpc_dispatcher()                                                  *
 *============================================================================*/

static int nanvix_rpc_dispatcher(struct task_args * args)
{
	int ret;
	int inbox;
	struct task * rpc;
	struct rpc_message req;

	UNUSED(args);

	uassert(inbox = stdinbox_get() >= 1);

//	if ((ret = kmailbox_ioctl(inbox, KMAILBOX_IOCTL_SET_REMOTE, MAILBOX_ANY_SOURCE, RPC_MAILBOX_PORT)) < 0)
//		return (TASK_RET_AGAIN);

	/* Message not consumed. */
	if ((ret = kmailbox_aread(inbox, &req, RPC_MESSAGE_SIZE)) < 0)
		return (TASK_RET_AGAIN);

	/* Message is not for this port. */
	if ((ret = kmailbox_wait(inbox)) != 0)
		return (TASK_RET_AGAIN);

	spinlock_lock(&rpc_lock);

		/* Searchs for the requested RPC. */
		rpc = NULL;
		for (int i = 0; i < RPC_MAX; ++i)
		{
			if (rpcs[i].rid == req.rid)
			{
				umemcpy(&rpcs[i].msg, &req, sizeof(struct rpc_message));
				rpc            = &rpcs[i].task;
				rpc->args.arg0 = (word_t) i;
				break;
			}
		}

		/* Found. */
		if (rpc)
		{
#if 0
			/* We must verify if the RPC are alredy waiting for execution. */
#endif
			/* Dispatchs task of the requested RPC. */
			uassert(ktask_create(rpc, nanvix_rpc_handler, &rpc->args, 0) == 0);
			uassert(ktask_dispatch(rpc) == 0);
		}

	spinlock_unlock(&rpc_lock);

	/* Reschedule this task to periodic read new messages. */
	return (TASK_RET_AGAIN);
}

/*============================================================================*
 * Exported Functions                                                         *
 *============================================================================*/

/*============================================================================*
 * nanvix_rpc_create()                                                        *
 *============================================================================*/

/**
 * @brief Create an RPC.
 *
 * @param rid      RPC identification.
 * @param request  Resqueted function.
 * @param response Gets response function.
 *
 * @returns Upon successful completion 0 is returned.
 * Upon failure, a negative error code is returned instead.
 */
int nanvix_rpc_create(int rid, rpc_fn request, rpc_fn response)
{
	int newid = (-EINVAL);

	/* Invalid ID. */
	if (rid < 0)
		return (-EINVAL);

	/* Invalid function. */
	if (!request)
		return (-EINVAL);

	spinlock_lock(&rpc_lock);

		/* Tries to alloc an RPC structure. */
		for (int i = 0; i < RPC_MAX; ++i)
		{
			/* ID already exists. */
			if (rpcs[i].rid == rid)
			{
				newid = (-EINVAL);
				break;
			}

			newid = (newid < 0) && (rpcs[i].rid < 0) ? i : newid;
		}

		/* Found. */
		if (newid >= 0)
		{
			nanvix_rpc_config(newid, rid, request, response);
			newid = (0);
		}

	spinlock_unlock(&rpc_lock);

	return (newid);
}

/*============================================================================*
 * nanvix_rpc_unlink()                                                        *
 *============================================================================*/

/**
 * @brief Unlink an RPC.
 *
 * @param rid RPC identification.
 *
 * @returns Upon successful completion 0 is returned.
 * Upon failure, a negative error code is returned instead.
 */
int nanvix_rpc_unlink(int rid)
{
	int ret = (-EINVAL);

	/* Invalid RPC. */
	if (rid < 0)
		return (-EINVAL);

	spinlock_lock(&rpc_lock);

		/* Searchs for the target RPC. */
		for (int i = 0; i < RPC_MAX; ++i)
		{
			/* Found. */
			if (rpcs[i].rid == rid)
			{
				nanvix_rpc_config(i, -1, NULL, NULL);
				ret = 0;
				break;
			}
		}

	spinlock_unlock(&rpc_lock);

	return (ret);
}

/*============================================================================*
 * nanvix_rpc_request()                                                       *
 *============================================================================*/

/**
 * @brief Sends a RPC request to another processor.
 *
 * @param ... Args.
 *
 * @returns Upon successful 0. Upon failure, a negative error code is
 * returned instead.
 */
int nanvix_rpc_request(
	const char *name,
	int rid,
	int mode,
	word_t arg0,
	word_t arg1,
	word_t arg2,
	word_t arg3,
	word_t arg4,
	word_t arg5
)
{
	int fd;                 /* NoC connector.   */
	int nodenum;            /* NoC node.        */
	struct rpc_message req; /* Request message. */

	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Resolves name, */
	if ((nodenum = nanvix_name_lookup(name)) < 0)
		return (-EAGAIN);

	/* Opens underlying HW channel (port 0). */
	if ((fd = kmailbox_open(nodenum, RPC_MAILBOX_PORT)) < 0)
		return (-EAGAIN);

		/*Configures request. */
		message_header_build(&req.header, RPC_ONE_WAY);
		req.rid       = rid;
		req.args.arg0 = arg0;
		req.args.arg1 = arg1;
		req.args.arg2 = arg2;
		req.args.arg3 = arg3;
		req.args.arg4 = arg4;
		req.args.arg5 = arg5;

		/* Writes the message. */
		if (kmailbox_write(fd, &req, RPC_MESSAGE_SIZE) < 0)
			return (-EINVAL);

		/* Not supported yet. */
		if (mode == RPC_NORMAL)
		{
			/* Read from a mailbox to receive the acceptance message. */
			UNUSED(mode);
		}

	/* Closes underlying HW channel. */
	if (kmailbox_close(fd) < 0)
		return (-EAGAIN);

	/* Success. */
	return (0);
}

/*============================================================================*
 * nanvix_rpc_response()                                                      *
 *============================================================================*/

/**
 * @brief Sends a RPC request to another processor.
 *
 * @param ... Args.
 *
 * @returns Upon successful 0. Upon failure, a negative error code is
 * returned instead.
 */
int nanvix_rpc_response(
	const char *name,
	int rid,
	word_t arg0,
	word_t arg1,
	word_t arg2,
	word_t arg3,
	word_t arg4,
	word_t arg5
)
{
	int target;

	rpc_fn response; /* Response function. */

	/* Invalid name. */
	if (name == NULL)
		return (-EINVAL);

	/* Resolves name, */
	if ((target = nanvix_name_lookup(name)) < 0)
		return (-EAGAIN);


	response = NULL;

	spinlock_lock(&rpc_lock);

		/* Tries to alloc an RPC structure. */
		for (int i = 0; !response && i < RPC_MAX; ++i)
		{
			/* ID already exists. */
			if (rpcs[i].rid == rid)
				response = rpcs[i].response;
		}

	spinlock_unlock(&rpc_lock);

	/* No response function configured. */
	if (!response)
		return (-EINVAL);

	return (
		response(
			target,
			RPC_MAILBOX_PORT,
			RPC_PORTAL_PORT,
			arg0,
			arg1,
			arg2,
			arg3,
			arg4,
			arg5
		)
	);
}

/*============================================================================*
 * Setup Functions                                                            *
 *============================================================================*/

/*============================================================================*
 * __nanvix_rpc_setup()                                                       *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
static int ___nanvix_rpc_setup(struct task_args * args)
{
	UNUSED(args);

	/* Initializes lock. */
	spinlock_init(&rpc_lock);

	/* Initializes stdmailbox. */
	uassert(__stdmailbox_setup() == 0);

	/* Open connection with Name Server. */
	for (int i = 0; i < RPC_MAX; ++i)
		nanvix_rpc_config(i, -1, NULL, NULL);

	/* Notify success to requester. */
	args->ret = 0;

	/* Notify success to dispatcher. */
	return (TASK_RET_SUCCESS);
}

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __nanvix_rpc_setup(void)
{
	struct task task;

	/* Nothing to do. */
	if (initialized)
		return (0);

	/**
	 * We need to move the setup to the dispatcher because it need perform the
	 * __stdmailbox_setup call.
	 */
	uassert(ktask_create(&task, ___nanvix_rpc_setup, NULL, 0) == 0);
	uassert(ktask_dispatch(&task) == 0);
	uassert(ktask_wait(&task) == 0);
	uassert(task.args.ret == 0);

	/* Dispatch the main task. */
	uassert(ktask_create(&rpc_dispatcher, nanvix_rpc_dispatcher, NULL, RPC_PERIOD) == 0);
	uassert(ktask_dispatch(&rpc_dispatcher) == 0);

	initialized = true;

	return (0);
}

/*============================================================================*
 * nanvix_rpc_cleanup()                                                       *
 *============================================================================*/

/**
 * @todo TODO: provide a detailed description for this function.
 */
static int ___nanvix_rpc_cleanup(struct task_args * args)
{
	/* Nothing to do. */
	if (!initialized)
		return (0);

	/* Initializes stdmailbox. */
	uassert(__stdmailbox_cleanup() == 0);

	/* Open connection with Name Server. */
	for (int i = 0; i < RPC_MAX; ++i)
		nanvix_rpc_config(i, -1, NULL, NULL);

	initialized = false;

	args->ret = 0;

	return (0);
}

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __nanvix_rpc_cleanup(void)
{
	struct task task;

	/* Nothing to do. */
	if (!initialized)
		return (0);

	/**
	 * We need to move the setup to the dispatcher because it need perform the
	 * __stdmailbox_setup call.
	 */
	uassert(ktask_create(&task, ___nanvix_rpc_cleanup, NULL, 0) == 0);
	uassert(ktask_dispatch(&task) == 0);
	uassert(ktask_wait(&task) == 0);
	uassert(task.args.ret == 0);

	initialized = false;

	return (0);
}

#endif /* __NANVIX_USE_TASKS */
