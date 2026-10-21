/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_vulkandisplay.h
 * @author rdb
 * @date 2016-02-16
 */

#ifndef CONFIG_VULKANDISPLAY_H
#define CONFIG_VULKANDISPLAY_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "configVariableInt64.h"
#include "configVariableFilename.h"
#include "configVariableString.h"
#include "configVariableInt.h"

NotifyCategoryDecl(vulkandisplay, EXPCL_VULKANDISPLAY, EXPTP_VULKANDISPLAY);

extern ConfigVariableInt64 vulkan_memory_page_size;
extern ConfigVariableInt64 vulkan_global_uniform_buffer_size;
extern ConfigVariableInt64 vulkan_staging_buffer_size;
extern ConfigVariableBool vulkan_support_custom_border_color;
extern ConfigVariableFilename vulkan_pipeline_cache_file;
extern ConfigVariableBool vulkan_persist_pipeline_cache;
extern ConfigVariableBool vulkan_validation;
extern ConfigVariableString vulkan_present_mode;
extern ConfigVariableInt vulkan_swapchain_images;
extern ConfigVariableInt vulkan_frame_stats;
extern ConfigVariableBool vulkan_recycle_semaphores;
extern ConfigVariableBool vulkan_keep_last_frame;

// Lightweight per-interval counters, printed every vulkan-frame-stats frames.
struct VulkanFrameStats {
  uint64_t vb_prepare = 0, vb_update = 0, vb_inuse = 0, vb_bytes = 0;
  uint64_t ib_prepare = 0, ib_update = 0;
  uint64_t alloc_mem = 0, new_page = 0, create_buffer = 0, dset_alloc = 0;
  double wait_ms = 0, wait_max = 0;
  double acquire_ms = 0, acquire_max = 0;
  double present_ms = 0, present_max = 0;
  uint64_t acquire_slow = 0, present_slow = 0;
  uint64_t sem_create = 0; double sem_ms = 0, sem_max = 0;
  uint64_t frames = 0;
};
extern VulkanFrameStats vulkan_stats;

extern EXPCL_VULKANDISPLAY void init_libvulkandisplay();
extern "C" EXPCL_VULKANDISPLAY int get_pipe_type_p3vulkandisplay();

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR

#elif defined(HAVE_COCOA)
#define VK_USE_PLATFORM_METAL_EXT

#elif defined(HAVE_X11)
// Make sure Eigen is included first to avoid conflict
#include "lsimpleMatrix.h"
#include "pre_x11_include.h"
#define VK_USE_PLATFORM_XLIB_KHR
#endif

// Necessary for VK_KHR_portability_subset (MoltenVK)
#define VK_ENABLE_BETA_EXTENSIONS

#include <vulkan/vulkan.h>

#ifdef HAVE_X11
#include "post_x11_include.h"
#endif

void vulkan_error(VkResult result, const char *message);

#endif
