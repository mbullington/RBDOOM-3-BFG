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

#include "vendor/rpmalloc/rpmalloc/rpmalloc.h"

// This is if you want to track memory tagging at runtime.
//
// It is not enabled by default because it is a performance hit.
thread_local bool memTagging = false;

// This is a mapping of memory tags to their total allocated size.
thread_local size_t memTagDict[256];
// This is a mapping of pointers to their memory tag.
thread_local idHashTable<memTag_t> pointerToMemTagTable;

//
// START UP AND SHUT DOWN
//

thread_local bool rpmallocInitialized = false;

void Mem_Init() {
  if (rpmallocInitialized) {
    return;
  }

  rpmallocInitialized = true;
  rpmalloc_initialize();

  // Reset the memory tagging.
  for (short i = 0; i < MAX_TAGS; i++) {
    memTagDict[i] = 0;
  }
  pointerToMemTagTable.Clear();
}

void Mem_Shutdown() { rpmalloc_finalize(); }

void Mem_ThreadLocalInit() { Mem_Init(); }
void Mem_ThreadLocalShutdown() { rpmalloc_thread_finalize(1); }

//
// TAGGING AND ALLOC/FREE
//

void Mem_EnableTagging(bool enable) { memTagging = enable; }

size_t* Mem_GetThreadLocalTagStats() { return memTagDict; }

void* Mem_Alloc16(const size_t size, const memTag_t tag) {
  if (!size) {
    return NULL;
  }

  // Care was taken to make sure this has many fast path returns.
  // This is because malloc may be called **before** the game has started in
  // some initializers.
  Mem_Init();

  if (memTagging) {
    // Turn off tagging so we don't recurse.
    memTagging = false;

    void* ptr = rpmalloc(size);

    // The key only accepts char*, so we're getting a little hacky here.
    // Quick reminder: We can use NULL (\0) to terminate the string.
    void* ptrKey[2] = {ptr, NULL};
    memTag_t mutTag = tag;
    pointerToMemTagTable.Set((char*)(ptrKey), mutTag);

    // Add the actual size to the dict.
    memTagDict[tag] += rpmalloc_usable_size(ptr);

    // Turn tagging back on.
    memTagging = true;
    return ptr;
  } else {
    return rpmalloc(size);
  }
}

void Mem_Free16(void* ptr) {
  if (ptr == NULL) {
    return;
  }

  if (memTagging) {
    // Turn off tagging so we don't recurse.
    memTagging = false;

    // The key only accepts char*, so we're getting a little hacky here.
    // Quick reminder: We can use NULL (\0) to terminate the string.
    void* ptrKey[2] = {ptr, NULL};

    memTag_t* tag = NULL;
    pointerToMemTagTable.Get((char*)(ptrKey), &tag);

    if (tag != NULL) {
      // Subtract the actual size from the dict.
      memTagDict[*tag] -= rpmalloc_usable_size(ptr);
      // Remove from the dictionary now that we've used the ptr address.
      pointerToMemTagTable.Remove((char*)(ptrKey));
    }

    // Turn tagging back on.
    memTagging = true;
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
