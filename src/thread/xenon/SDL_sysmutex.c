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

/* An implementation of mutexes using semaphores */

#include "SDL_thread.h"
#include "SDL_systhread_c.h"

#include <xenon_soc/xenon_power.h>
#include <ppc/atomic.h>
#include <xetypes.h>
#include <ppc/register.h>
#include <ppc/xenonsprs.h>

#include <threads/threads.h>
#include <threads/mutex.h>

struct SDL_mutex {
	MUTEX *lock;
	MUTEX *mutex;
	int recursive;
	Uint32 owner;
};

/* Create a mutex */
SDL_mutex *SDL_CreateMutex(void)
{
	SDL_mutex *mutex;

	/* Allocate mutex memory */
	mutex = (SDL_mutex *)SDL_malloc(sizeof(*mutex));
	mutex->lock = mutex_create(1);
	mutex->mutex = mutex_create(1);
	if ( mutex ) {
		mutex->recursive = 0;
		mutex->owner = -1;
	} else {
		SDL_OutOfMemory();
	}
	return mutex;
}

/* Free the mutex */
void SDL_DestroyMutex(SDL_mutex *mutex)
{
	if ( mutex ) {
		mutex_destroy(mutex->lock);
		mutex_destroy(mutex->mutex);
		SDL_free(mutex);
	}
}

/* Lock the semaphore */
int SDL_mutexP(SDL_mutex *mutex)
{
#if SDL_THREADS_DISABLED
	return  0;
#else
	Uint32 this_thread;

	if ( mutex == NULL ) {
		SDL_SetError("Passed a NULL mutex");
		return -1;
	}

	//mutex_acquire(mutex->lock, -1);

	this_thread = SDL_ThreadID();
	if ( mutex->owner == this_thread ) {
		++mutex->recursive;
	} else {
		/* The order of operations is important.
		   We set the locking thread id after we obtain the lock
		   so unlocks from other threads will fail.
		*/
		mutex_acquire(mutex->mutex, -1);
		mutex->owner = this_thread;
		mutex->recursive = 0;
	}

	//mutex_release(mutex->lock);

	return 0;
#endif /* SDL_THREADS_DISABLED */
}

/* Unlock the mutex */
int SDL_mutexV(SDL_mutex *mutex)
{
#if SDL_THREADS_DISABLED
	return 0;
#else
	if ( mutex == NULL ) {
		SDL_SetError("Passed a NULL mutex");
		return -1;
	}

	//mutex_acquire(mutex->lock, -1);

	/* If we don't own the mutex, we can't unlock it */
	if ( SDL_ThreadID() != mutex->owner ) {
		SDL_SetError("mutex not owned by this thread");
		//mutex_release(mutex->lock);
		return -1;
	}

	if ( mutex->recursive ) {
		--mutex->recursive;
	} else {
		/* The order of operations is important.
		   First reset the owner so another thread doesn't lock
		   the mutex and set the ownership before we reset it,
		   then release the lock semaphore.
		 */
		mutex->owner = -1;
		mutex_release(mutex->mutex);
	}

	//mutex_release(mutex->lock);

	return 0;
#endif /* SDL_THREADS_DISABLED */
}
