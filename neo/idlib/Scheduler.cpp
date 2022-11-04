/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2020 Michael Bullington

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

#include <sx/allocator.h>
#include <sx/jobs.h>
#include <sx/os.h>  // sx_os_numcores

#include "Scheduler.h"

namespace id {

Scheduler* scheduler = NULL;

void SxThreadInit(sx_job_context* context, int thread_index,
                  unsigned int thread_id, void* user) {
  // Intialize the thread's allocator.
  Mem_ThreadLocalInit();

  int64 threads = (int64)user;

  bool combinedRenderTags = threads <= 4;
  if (combinedRenderTags) {
    if (thread_index == threads - 1) {
      sx_job_set_current_thread_tags(
          context, TAG_RENDERER_FRONTEND | TAG_RENDERER_BACKEND);
    }
  } else {
    if (thread_index == threads - 1) {
      sx_job_set_current_thread_tags(context, TAG_RENDERER_BACKEND);
    }

    if (thread_index == threads - 2) {
      sx_job_set_current_thread_tags(context, TAG_RENDERER_FRONTEND);
    }
  }
}

struct sxTaskData_t {
  int workLen;
  jobFn_t fn;
  void* data;
};

sxTaskData_t* CreateSxTaskData(jobFn_t fn, int workLen, void* data) {
  auto taskData = new sxTaskData_t;
  taskData->fn = fn;
  taskData->workLen = workLen;
  taskData->data = data;
  return taskData;
}

void SxJobFunc(int range_start, int range_end, int thread_index, void* user) {
  sxTaskData_t* taskData = (sxTaskData_t*)user;

  for (int i = range_start; i < range_end; i++) {
    taskData->fn(i, taskData->workLen, taskData->data);
  }

  // Free task data
  delete taskData;
}

Scheduler::Scheduler(int stackSizeBytes) {
  // Per https://fabiensanglard.net/doom3_bfg/threading.php, there are three
  // "serialized" threads:
  //
  // 1. The main thread / render backend
  // 2. Game logic / render frontend
  // 3. High frequency joystick input
  //
  // Over time, it would be nice to "not to have a main thread," but this may
  // require a lot of refactoring.
  //
  // The Minimum-Viable-Product right now is constructing backend Vulkan command
  // buffers using the task scheduler--and replacing id's ParallelJobList. This
  // is a good first step.
  int threads = Max(sx_os_numcores() - 3, 2);

  sx_job_context_desc desc = {
      .num_threads = threads,
      .max_fibers = MAX_FIBERS,
      .fiber_stack_sz = stackSizeBytes,
      .thread_init_cb = SxThreadInit,
      .thread_user_data = (void*)static_cast<int64>(threads),
  };
  this->context = sx_job_create_context(alloc, &desc);
}

Scheduler::~Scheduler() { sx_job_destroy_context(context, alloc); }

jobListHandle_t Scheduler::Submit(taskTags_t tag, jobFn_t fn, int workLen,
                                  void* data) {
  auto taskData = CreateSxTaskData(fn, workLen, data);
  auto job = sx_job_dispatch(context, workLen, SxJobFunc, taskData,
                             SX_JOB_PRIORITY_NORMAL, tag);
  return {
      .job = job,
      .deleted = false,
  };
}

void Scheduler::Await(jobListHandle_t handle) {
  if (handle.deleted) {
    return;
  }

  sx_job_wait_and_del(context, handle.job);
  handle.deleted = true;
}

bool Scheduler::IsCompleted(jobListHandle_t handle) {
  if (handle.deleted) {
    return true;
  }

  bool ret = sx_job_test_and_del(context, handle.job);
  if (ret) {
    handle.deleted = true;
  }

  return ret;
}

bool Scheduler::InTask() { return sx_job_in_job(context); }

}  // namespace id