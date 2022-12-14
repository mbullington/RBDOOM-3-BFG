/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
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
#pragma once

#include <sx/jobs.h>
#include <functional>

namespace id {

const int MAX_FIBERS = 2048;
const int STACK_SIZE_BYTES = 1048576;  // 1MB stack size

template <typename T>
using jobFnGeneric_t =
    std::function<void(int work_idx, int work_len, T* user_data)>;
using jobFn_t = jobFnGeneric_t<void>;

typedef sx_job_context* jobContextHandle_t;

struct jobListHandle_t {
  sx_job_t job;
  bool deleted;
};

enum taskTags_t {
  TAG_NONE = 0,

  TAG_RENDERER_FRONTEND = BIT(1),
  TAG_RENDERER_BACKEND = BIT(2),
};

// ================================================
// Scheduler is largely a wrapper around the "sx" library,
// which has a nested job system based on a 2015 GDC talk from Christian Gyrling
// (at Naughty Dog):
//
// https://gdcvault.com/play/1022186/Parallelizing-the-Naughty-Dog-Engine
//
// Dependencies are declaratively defined and use fibers, which can pause jobs
// and resume them later mid-execution.
//
// We'll define "lanes" for special tags, such as frontend/backend, to make sure
// in a worst-case scenario that utility jobs don't starve the renderer.
//
// If we have < 4 cores, we'll use a single lane for both frontend and backend.
// Given hyperthreading & 4+ cores now on most CPUs (even mobile), this should
// be a reasonable default.
// ================================================
struct Scheduler {
 public:
  Scheduler(int stackSizeBytes = STACK_SIZE_BYTES);  // 2MB
  ~Scheduler();

  // Submit the jobs in this list.
  jobListHandle_t Submit(taskTags_t tag, jobFn_t fn, int workLen = 1,
                         void* data = NULL);

  inline jobListHandle_t Submit(jobFn_t fn, int workLen = 1,
                                void* data = NULL) {
    return Submit(TAG_NONE, fn, workLen, data);
  }

  template <typename T>
  inline jobListHandle_t Submit(taskTags_t tag, jobFnGeneric_t<T> fn,
                                int workLen = 1, T* data = NULL) {
    return Submit(tag, (jobFn_t)fn, workLen, (void*)data);
  }

  template <typename T>
  inline jobListHandle_t Submit(jobFnGeneric_t<T> fn, int workLen = 1,
                                T* data = NULL) {
    return Submit(TAG_NONE, (jobFn_t)fn, workLen, (void*)data);
  }

  // Await for the current job to finish.
  void Await(jobListHandle_t handle);
  // Returns true if the job is done, false otherwise.
  bool IsCompleted(jobListHandle_t handle);

  bool InTask();

 private:
  jobContextHandle_t context;
};

extern Scheduler* scheduler;

}  // namespace id