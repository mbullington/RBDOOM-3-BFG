/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012-2013 Robert Beckebans
Copyright (C) 2013 Daniel Gibson

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition
Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or
modify it under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your option)
any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see
<http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain
additional terms. You should have received a copy of these additional terms
immediately following the terms and conditions of the GNU General Public License
which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a
copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional
terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite
120, Rockville, Maryland 20850 USA.

===========================================================================
*/
#include "precompiled.h"
#pragma hdrstop

#include "../TaskScheduler.h"

#include <sx/allocator.h>
#include <sx/threads.h>
#include <sx/atomic.h>

const sx_alloc* alloc;

/*
========================
Sys_Createthread
========================
*/
threadHandle_t Sys_CreateThread(xthread_t function, void* parms,
                                xthreadPriority priority, const char* name,
                                core_t core, int stackSize, bool suspended) {
  sx_thread* handle = sx_thread_create(alloc, function, parms, stackSize, name);
  // NOTE: Unlike before, we don't set the thread affinity and let the OS deal
  // with scheduling
  return handle;
}

/*
========================
Sys_DestroyThread
========================
*/
void Sys_DestroyThread(threadHandle_t threadHandle) {
  if (threadHandle == 0) {
    return;
  }

  if (sx_thread_destroy(threadHandle, alloc) != 0) {
    common->FatalError("Sys_DestroyThread: failed");
  }
}

/*
========================
Sys_Yield
========================
*/
void Sys_Yield() {
  if (id::taskScheduler->InTask()) {
    sx_relax_cpu();
  }

  sx_thread_yield();
}

/*
================================================================================================

        Signal

================================================================================================
*/

/*
========================
Sys_SignalCreate
========================
*/
void Sys_SignalCreate(signalHandle_t& handle, bool manualReset) {
  handle = new sx_signal;
  sx_signal_init(handle, manualReset);
}

/*
========================
Sys_SignalDestroy
========================
*/
void Sys_SignalDestroy(signalHandle_t& handle) { sx_signal_release(handle); }

/*
========================
Sys_SignalRaise
========================
*/
void Sys_SignalRaise(signalHandle_t& handle) { sx_signal_raise(handle); }

/*
========================
Sys_SignalClear
========================
*/
void Sys_SignalClear(signalHandle_t& handle) {
  sx_signal_reset_if_manual(handle);
}

/*
========================
Sys_SignalWait
========================
*/
bool Sys_SignalWait(signalHandle_t& handle, int timeout) {
  if (id::taskScheduler->InTask()) {
    common->FatalError(
        "Cannot use operating system thread sync within task; use locks "
        "instead.");
  }

  return sx_signal_wait(handle, timeout);
}

/*
================================================================================================

        Mutex

================================================================================================
*/

/*
========================
Sys_MutexCreate
========================
*/
void Sys_MutexCreate(mutexHandle_t& handle) {
  handle = new sx_mutex;
  sx_mutex_init(handle);
}

/*
========================
Sys_MutexDestroy
========================
*/
void Sys_MutexDestroy(mutexHandle_t& handle) { sx_mutex_release(handle); }

/*
========================
Sys_MutexLock
========================
*/
bool Sys_MutexLock(mutexHandle_t& handle, bool blocking) {
  if (id::taskScheduler->InTask()) {
    common->FatalError(
        "Cannot use operating system thread sync within task; use locks "
        "instead.");
  }

  if (sx_mutex_try(handle) != 0) {
    if (!blocking) {
      return false;
    }
    sx_mutex_enter(handle);
  }
  return true;
}

/*
========================
Sys_MutexUnlock
========================
*/
void Sys_MutexUnlock(mutexHandle_t& handle) {
  if (id::taskScheduler->InTask()) {
    common->FatalError(
        "Cannot use operating system thread sync within task; use locks "
        "instead.");
  }

  sx_mutex_exit(handle);
}

/*
================================================================================================

        Interlocked Integer

================================================================================================
*/

/*
========================
Sys_InterlockedIncrement
========================
*/
interlockedInt_t Sys_InterlockedIncrement(interlockedInt_t& value) {
  return __sync_add_and_fetch(&value, 1);
}

/*
========================
Sys_InterlockedDecrement
========================
*/
interlockedInt_t Sys_InterlockedDecrement(interlockedInt_t& value) {
  return __sync_sub_and_fetch(&value, 1);
}

/*
========================
Sys_InterlockedAdd
========================
*/
interlockedInt_t Sys_InterlockedAdd(interlockedInt_t& value,
                                    interlockedInt_t i) {
  return __sync_add_and_fetch(&value, i);
}

/*
========================
Sys_InterlockedSub
========================
*/
interlockedInt_t Sys_InterlockedSub(interlockedInt_t& value,
                                    interlockedInt_t i) {
  return __sync_sub_and_fetch(&value, i);
}

/*
========================
Sys_InterlockedExchange
========================
*/
interlockedInt_t Sys_InterlockedExchange(interlockedInt_t& value,
                                         interlockedInt_t exchange) {
  return __sync_val_compare_and_swap(&value, value, exchange);
}

/*
========================
Sys_InterlockedCompareExchange
========================
*/
interlockedInt_t Sys_InterlockedCompareExchange(interlockedInt_t& value,
                                                interlockedInt_t comparand,
                                                interlockedInt_t exchange) {
  return __sync_val_compare_and_swap(&value, comparand, exchange);
}

/*
================================================================================================

        Interlocked Pointer

================================================================================================
*/

/*
========================
Sys_InterlockedExchangePointer
========================
*/
void* Sys_InterlockedExchangePointer(void*& ptr, void* exchange) {
  return __sync_val_compare_and_swap(&ptr, ptr, exchange);
}

/*
========================
Sys_InterlockedCompareExchangePointer
========================
*/
void* Sys_InterlockedCompareExchangePointer(void*& ptr, void* comparand,
                                            void* exchange) {
  return __sync_val_compare_and_swap(&ptr, comparand, exchange);
}
