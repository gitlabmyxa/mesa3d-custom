/* Copyright © 2023 Intel Corporation
 * SPDX-License-Identifier: MIT
 */

#ifndef _LIBANV_SHADERS_H_
#define _LIBANV_SHADERS_H_

/* Define stdint types compatible between the CPU and GPU for shared headers */
#ifndef __OPENCL_VERSION__
#include <stdint.h>

#include <vulkan/vulkan_core.h>

#include "util/macros.h"

#else

#define _MESA_LIBCL_ASSERT_IGNORE 1
#include "libcl_vk.h"

#include "genxml/gen_macros.h"
#include "genxml/genX_cl_pack.h"

#define PRAGMA_POISON(param)
#endif

/**
 * Flags for generated_draws.cl
 */
enum anv_generated_draw_flags {
   ANV_GENERATED_FLAG_INDEXED        = BITFIELD_BIT(0),
   ANV_GENERATED_FLAG_PREDICATED     = BITFIELD_BIT(1),
   /* Only used on Gfx9, means the pipeline is using gl_DrawID */
   ANV_GENERATED_FLAG_DRAWID         = BITFIELD_BIT(2),
   /* Only used on Gfx9, means the pipeline is using gl_BaseVertex or
    * gl_BaseInstance
    */
   ANV_GENERATED_FLAG_BASE           = BITFIELD_BIT(3),
   /* Whether the count is indirect  */
   ANV_GENERATED_FLAG_COUNT          = BITFIELD_BIT(4),
   /* Whether the generation shader writes to the ring buffer */
   ANV_GENERATED_FLAG_RING_MODE      = BITFIELD_BIT(5),
   /* Whether TBIMR tile-based rendering shall be enabled. */
   ANV_GENERATED_FLAG_TBIMR          = BITFIELD_BIT(6),
   /* Wa_16011107343 */
   ANV_GENERATED_FLAG_WA_16011107343 = BITFIELD_BIT(7),
   /* Wa_22018402687 */
   ANV_GENERATED_FLAG_WA_22018402687 = BITFIELD_BIT(8),
};

/**
 * Flags for query_copy.cl
 */
#define ANV_COPY_QUERY_FLAG_RESULT64  BITFIELD_BIT(0)
#define ANV_COPY_QUERY_FLAG_AVAILABLE BITFIELD_BIT(1)
#define ANV_COPY_QUERY_FLAG_DELTA     BITFIELD_BIT(2)
#define ANV_COPY_QUERY_FLAG_PARTIAL   BITFIELD_BIT(3)

#ifdef __OPENCL_VERSION__

void genX(write_3DSTATE_VERTEX_BUFFERS)(global void *dst_ptr,
                                        uint32_t buffer_count);

void genX(write_VERTEX_BUFFER_STATE)(global void *dst_ptr,
                                     uint32_t mocs,
                                     uint32_t buffer_idx,
                                     uint64_t address,
                                     uint32_t size,
                                     uint32_t stride);

void genX(write_3DPRIMITIVE)(global void *dst_ptr,
                             bool is_predicated,
                             bool is_indexed,
                             bool use_tbimr,
                             uint32_t vertex_count_per_instance,
                             uint32_t start_vertex_location,
                             uint32_t instance_count,
                             uint32_t start_instance_location,
                             uint32_t base_vertex_location);

#if GFX_VER >= 11
void genX(write_3DPRIMITIVE_EXTENDED)(global void *dst_ptr,
                                      bool is_predicated,
                                      bool is_indexed,
                                      bool use_tbimr,
                                      uint32_t vertex_count_per_instance,
                                      uint32_t start_vertex_location,
                                      uint32_t instance_count,
                                      uint32_t start_instance_location,
                                      uint32_t base_vertex_location,
                                      uint32_t param_base_vertex,
                                      uint32_t param_base_instance,
                                      uint32_t param_draw_id);
#endif

void genX(write_MI_BATCH_BUFFER_START)(global void *dst_ptr, uint64_t addr);

void genX(write_draw)(global uint32_t *dst_ptr,
                      global void *indirect_ptr,
                      global uint32_t *draw_id_ptr,
                      uint32_t draw_id,
                      uint32_t instance_multiplier,
                      bool is_indexed,
                      bool is_predicated,
                      bool uses_tbimr,
                      bool uses_base,
                      bool uses_draw_id,
                      uint32_t mocs);


void genX(copy_data)(global void *dst_ptr,
                     global void *src_ptr,
                     uint32_t size);

#endif /* __OPENCL_VERSION__ */

#endif /* _LIBANV_SHADERS_H_ */
