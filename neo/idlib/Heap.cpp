/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012 Robert Beckebans
Copyright (C) 2012 Daniel Gibson
Copyright (C) 2022 Michael Bullington

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

//===============================================================
//
//	memory allocation all in one place
//
//===============================================================

#undef new

#include <sx/lockless.h>
#include "vendor/rpmalloc/rpmalloc/rpmalloc.h"

// This is if you want to track memory tagging at runtime.
// It is not enabled by default because it is a performance hit.
idCVar com_memTag("com_memTag", "0", CVAR_BOOL,
                  "Enables memory tagging for debugging memory leaks. "
                  "This will slow down the game.");

char* memTagNames[TAG_NUM_TAGS] = {
#define MEM_TAG(x) #x,
#include "sys/sys_alloc_tags.h"
#undef MEM_TAG
};

// Make sure the tag array/table are thread safe (and fiber safe) by using
// spinlocks.
//
// We need two so we can disable idStr allocation while we are in the
// middle of recording.
sx_lock_t memTagLock = 0;
sx_lock_t memTagRecordingLock = 0;
// This is used to disable allocation while we are in the middle of adding
// memory tags to the mappings.
//
// Similar in theory to 'memTagRecordingLock', but only affects the current
// thread.
thread_local bool memTagRecursiveCheck = false;

// This is a mapping of memory tags to their total allocated size.
idTempArray<size_t> memTagToByteUsage(MAX_TAGS);
// This is a mapping of pointers to their memory tag.
idHashTable<memTag_t> pointerToMemTag;

//
// START UP AND SHUT DOWN
//

bool rpmallocInitialized = false;
void Mem_Init() {
  rpmalloc_initialize();
  if (rpmallocInitialized) {
    return;
  }

  rpmallocInitialized = true;

  sx_lock_enter(&memTagLock);

  // Reset the memory tagging.
  memTagToByteUsage.Zero();
  pointerToMemTag.Clear();

  sx_lock_exit(&memTagLock);
}

void Mem_Shutdown() { rpmalloc_finalize(); }

void Mem_ThreadLocalInit() { Mem_Init(); }
void Mem_ThreadLocalShutdown() { rpmalloc_thread_finalize(1); }

//
// TAGGING AND ALLOC/FREE
//

CONSOLE_COMMAND(memTagStats, "print memory stats", 0) {
  if (!(com_memTag.IsRegistered() && com_memTag.GetBool())) {
    idLib::Printf("com_memTag is not enabled.");
    return;
  }

  sx_lock_enter(&memTagRecordingLock);
  sx_lock_enter(&memTagLock);

  // Create a table of the memory tags.
  idStr table;
  table += "Memory Tag Stats\n";
  for (short i = 0; i < MAX_TAGS; i++) {
    if (memTagToByteUsage[i] > 0) {
      table += va("%s\t\t\t: %zu\n", memTagNames[i], memTagToByteUsage[i]);
    }
  }

  sx_lock_exit(&memTagLock);
  sx_lock_exit(&memTagRecordingLock);

  // Print the table.
  idLib::Printf("%s", table.c_str());
}

void* Mem_Alloc(const size_t size, const memTag_t tag) {
  if (!size) {
    return NULL;
  }

  // Care was taken to make sure this has many fast path returns.
  // This is because malloc may be called **before** the game has started in
  // some initializers.
  Mem_Init();

  if (!memTagRecursiveCheck && !memTagRecordingLock &&
      (com_memTag.IsRegistered() && com_memTag.GetBool())) {
    memTagRecursiveCheck = true;
    sx_lock_enter(&memTagLock);

    void* ptr = rpmalloc(size);

    // The key only accepts char*, so we're getting a little hacky here.
    // Quick reminder: We can use NULL (\0) to terminate the string.
    void* ptrKey[2] = {ptr, NULL};
    memTag_t mutTag = tag;
    pointerToMemTag.Set((char*)(ptrKey), mutTag);

    // Add the actual size to the dict.
    memTagToByteUsage[tag] += rpmalloc_usable_size(ptr);

    memTagRecursiveCheck = false;
    sx_lock_exit(&memTagLock);
    return ptr;
  } else {
    return rpmalloc(size);
  }
}

void Mem_Free(void* ptr) {
  if (ptr == NULL) {
    return;
  }

  if (!memTagRecursiveCheck && !memTagRecordingLock &&
      (com_memTag.IsRegistered() && com_memTag.GetBool())) {
    memTagRecursiveCheck = true;
    sx_lock_enter(&memTagLock);

    // The key only accepts char*, so we're getting a little hacky here.
    // Quick reminder: We can use NULL (\0) to terminate the string.
    void* ptrKey[2] = {ptr, NULL};

    memTag_t* tag = NULL;
    pointerToMemTag.Get((char*)(ptrKey), &tag);

    if (tag != NULL) {
      // Subtract the actual size from the dict.
      memTagToByteUsage[*tag] -= rpmalloc_usable_size(ptr);
      // Remove from the dictionary now that we've used the ptr address.
      pointerToMemTag.Remove((char*)(ptrKey));
    }

    memTagRecursiveCheck = false;
    sx_lock_exit(&memTagLock);
  }

  rpfree(ptr);
}

void* Mem_ClearedAlloc(const size_t size, const memTag_t tag) {
  void* mem = Mem_Alloc(size, tag);
  SIMDProcessor->Memset(mem, 0, size);
  return mem;
}

char* Mem_CopyString(const char* in) {
  char* out = (char*)Mem_Alloc(strlen(in) + 1, TAG_STRING);
  strcpy(out, in);
  return out;
}
