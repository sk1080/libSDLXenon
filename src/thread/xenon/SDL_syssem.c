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

/* An implementation of semaphores using mutexes and condition variables */

#include "SDL_timer.h"
#include "SDL_thread.h"
#include "SDL_systhread_c.h"

#include <threads/mutex.h>
 
struct SDL_semaphore
{
	MUTEX* mut;
};

SDL_sem *SDL_CreateSemaphore(Uint32 initial_value)
{
	SDL_sem *sem;

	sem = (SDL_sem *)SDL_malloc(sizeof(*sem));
	if ( ! sem ) {
		SDL_OutOfMemory();
		return NULL;
	}
	sem->mut = mutex_create(1000);//GHETTO
	lock(&sem->mut->Lock);
	sem->mut->CurrentLockCount = 1000 - initial_value;
	unlock(&sem->mut->Lock);
	//mutex_setlockcount(&sem->mut, initial_value);
	return sem;
}

/* WARNING:
   You cannot call this function when another thread is using the semaphore.
*/
void SDL_DestroySemaphore(SDL_sem *sem)
{
	if ( sem ) {
		mutex_destroy(sem->mut);
		free(sem);
		sem = NULL;
	}
}

int SDL_SemTryWait(SDL_sem *sem)
{
	return mutex_acquire(sem->mut, 0);
	return 0;
}

int SDL_SemWaitTimeout(SDL_sem *sem, Uint32 timeout)
{
	return mutex_acquire(sem->mut, timeout);
}

int SDL_SemWait(SDL_sem *sem)
{
	return mutex_acquire(sem->mut, -1);
	return 0;
}

Uint32 SDL_SemValue(SDL_sem *sem)
{
	lock(sem->mut->Lock);
	Uint32 val = sem->mut->CurrentLockCount;
	unlock(sem->mut->Lock);
	return val;
}

int SDL_SemPost(SDL_sem *sem)
{
	if(sem == NULL)
	{
		printf("SDL_SemPost called with null sem\n");
		return -1;
	}
	mutex_release(sem->mut);
	return 0;
}
 
