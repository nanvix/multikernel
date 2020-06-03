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

#include <nanvix/servers/message.h>
#include <nanvix/runtime/stdikc.h>
#include <nanvix/sys/noc.h>
#include <posix/stdint.h>
#include <nanvix/ulib.h>

/**
 * The message_header_sprint() function prints the message header
 * pointed to by @p h in the string pointed to by @p str.
 */
void message_header_sprint(char *str, message_header *h)
{
	const char *fmt = "source=%d mailbox_port=%d portal_port=%d opcode=%d";

	uassert(h != NULL);

	usprintf(str, fmt,
		h->source,
		h->mailbox_port,
		h->portal_port,
		h->opcode
	);
}

/**
 * The message_header_build2() function builds a message header pointed
 * to by @p h. The opcode of the message is set to @p opcode.
 */
void message_header_build2(
	message_header *h,
	uint8_t opcode,
	uint8_t portal_port
)
{
	uassert(h != NULL);

	h->source = knode_get_num();
	h->opcode = opcode;
	h->mailbox_port = stdinbox_get_port();
	h->portal_port = portal_port;
}

/**
 * @see message_header_build2().
 */
void message_header_build(message_header *h, uint8_t opcode)
{
	message_header_build2(h, opcode, stdinportal_get_port());
}
