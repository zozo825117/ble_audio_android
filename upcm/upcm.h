/*
* ---------------------------------------------------------------------------
* FILE: upcm.h
*
* PURPOSE:

* Copyright (C) 2012 by Cambridge Silicon Radio Ltd.
*
* Refer to LICENSE.txt included with this source code for details on
* the license terms.
*
* ---------------------------------------------------------------------------
*/

#ifndef _UPCM_H_
#define _UPCM_H_

#include <linux/types.h>

enum upcm_event_type {
	// Input event
	UPCM_CREATE,
	UPCM_DESTROY,
	UPCM_INPUT,

	// Output event
	UPCM_TRIGGER_START,
	UPCM_TRIGGER_STOP,
	UPCM_OUTPUT,	
};

#define UPCM_DATA_MAX 256

struct upcm_input_req {
	__u8 data[UPCM_DATA_MAX];
	__u16 size;
} __attribute__((packed));

struct upcm_output_req {
	__u8 data[UPCM_DATA_MAX];
	__u16 size;
} __attribute__((packed));

struct upcm_event {
	__u32 type;

	union {
		struct upcm_input_req input;
		struct upcm_output_req output;
	} u;
} __attribute__((packed));

#endif
