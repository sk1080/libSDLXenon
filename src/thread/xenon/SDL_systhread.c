/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2009 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include "SDL_config.h"

/* Thread management routines for SDL */

#include <xenon_soc/xenon_power.h>
#include <ppc/atomic.h>
#include <xetypes.h>
#include <ppc/register.h>
#include <ppc/xenonsprs.h>
#include <threads/threads.h>

#include "SDL_thread.h"
#include "SDL_systhread_c.h"
#include "../SDL_thread_c.h"
#include "../SDL_systhread.h"

void RunThread(void *data)
{	
	SDL_RunThread(data);
	return;
}


int SDL_SYS_CreateThread(SDL_Thread *thread, void *args)
{
	static int core = 2;

	PTHREAD thr = thread_create(RunThread, 0, args, THREAD_FLAG_CREATE_SUSPENDED);
	thread_set_processor(thr, core);
	core++;
	if(core > 5)
		core = 1;
	thread_resume(thr);

	thread->handle = thr;

	return 0;
}

void SDL_SYS_SetupThread(void)
{
	return;
}

Uint32 SDL_ThreadID(void)
{
	return thread_get_current()->ThreadId;
}

void SDL_SYS_WaitThread(SDL_Thread *thread)
{
	PTHREAD pthr = (PTHREAD)thread->handle;
	while(pthr->ThreadTerminated == 0);
	return;
}

void SDL_SYS_KillThread(SDL_Thread *thread)
{
	printf("KillThread: Not Yet Implemented\n");
}


