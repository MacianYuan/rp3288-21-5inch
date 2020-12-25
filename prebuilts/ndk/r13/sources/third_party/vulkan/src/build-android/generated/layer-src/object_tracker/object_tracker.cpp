/* THIS FILE IS GENERATED.  DO NOT EDIT. */

/*
 * Copyright (c) 2015-2016 The Khronos Group Inc.
 * Copyright (c) 2015-2016 Valve Corporation
 * Copyright (c) 2015-2016 LunarG, Inc.
 * Copyright (c) 2015-2016 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Mike Stroyan <stroyan@google.com>
 * Author: Tony Barbour <tony@LunarG.com>
 */

// CODEGEN : file ../vk-layer-generate.py line #722
#include "vk_loader_platform.h"
#include "vulkan/vulkan.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cinttypes>

#include <unordered_map>

#include "vulkan/vk_layer.h"
#include "vk_layer_config.h"
#include "vk_layer_table.h"
#include "vk_layer_data.h"
#include "vk_layer_logging.h"

#include "object_tracker.h"


namespace object_tracker {

std::unordered_map<uint64_t, OBJTRACK_NODE*> VkInstanceMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkPhysicalDeviceMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkDeviceMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkQueueMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkCommandBufferMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkCommandPoolMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkFenceMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkDeviceMemoryMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkBufferMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkImageMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkSemaphoreMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkEventMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkQueryPoolMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkBufferViewMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkImageViewMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkShaderModuleMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkPipelineCacheMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkPipelineLayoutMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkPipelineMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkDescriptorSetLayoutMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkSamplerMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkDescriptorPoolMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkDescriptorSetMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkRenderPassMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkFramebufferMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkSwapchainKHRMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkSurfaceKHRMap;
std::unordered_map<uint64_t, OBJTRACK_NODE*> VkDebugReportCallbackEXTMap;

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_instance(VkInstance dispatchable_object, VkInstance vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkInstanceMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_instance(VkInstance dispatchable_object, VkInstance object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkInstanceMap.find(object_handle);
    if (it != VkInstanceMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkInstanceMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_physical_device(VkPhysicalDevice dispatchable_object, VkPhysicalDevice vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkPhysicalDeviceMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_physical_device(VkPhysicalDevice dispatchable_object, VkPhysicalDevice object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkPhysicalDeviceMap.find(object_handle);
    if (it != VkPhysicalDeviceMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkPhysicalDeviceMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_device(VkDevice dispatchable_object, VkDevice vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkDeviceMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_device(VkDevice dispatchable_object, VkDevice object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkDeviceMap.find(object_handle);
    if (it != VkDeviceMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkDeviceMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_queue(VkQueue dispatchable_object, VkQueue vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkQueueMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_queue(VkQueue dispatchable_object, VkQueue object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkQueueMap.find(object_handle);
    if (it != VkQueueMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkQueueMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_command_buffer(VkCommandBuffer dispatchable_object, VkCommandBuffer vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkCommandBufferMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_command_buffer(VkCommandBuffer dispatchable_object, VkCommandBuffer object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkCommandBufferMap.find(object_handle);
    if (it != VkCommandBufferMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkCommandBufferMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_command_pool(VkDevice dispatchable_object, VkCommandPool vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkCommandPoolMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_command_pool(VkDevice dispatchable_object, VkCommandPool object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkCommandPoolMap.find(object_handle);
    if (it != VkCommandPoolMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkCommandPoolMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_fence(VkDevice dispatchable_object, VkFence vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkFenceMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_fence(VkDevice dispatchable_object, VkFence object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkFenceMap.find(object_handle);
    if (it != VkFenceMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkFenceMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_device_memory(VkDevice dispatchable_object, VkDeviceMemory vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkDeviceMemoryMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_device_memory(VkDevice dispatchable_object, VkDeviceMemory object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkDeviceMemoryMap.find(object_handle);
    if (it != VkDeviceMemoryMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkDeviceMemoryMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_buffer(VkDevice dispatchable_object, VkBuffer vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkBufferMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_buffer(VkDevice dispatchable_object, VkBuffer object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkBufferMap.find(object_handle);
    if (it != VkBufferMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkBufferMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_image(VkDevice dispatchable_object, VkImage vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkImageMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_image(VkDevice dispatchable_object, VkImage object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkImageMap.find(object_handle);
    if (it != VkImageMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkImageMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_semaphore(VkDevice dispatchable_object, VkSemaphore vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkSemaphoreMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_semaphore(VkDevice dispatchable_object, VkSemaphore object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkSemaphoreMap.find(object_handle);
    if (it != VkSemaphoreMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkSemaphoreMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_event(VkDevice dispatchable_object, VkEvent vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkEventMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_event(VkDevice dispatchable_object, VkEvent object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkEventMap.find(object_handle);
    if (it != VkEventMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkEventMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_query_pool(VkDevice dispatchable_object, VkQueryPool vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkQueryPoolMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_query_pool(VkDevice dispatchable_object, VkQueryPool object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkQueryPoolMap.find(object_handle);
    if (it != VkQueryPoolMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkQueryPoolMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_buffer_view(VkDevice dispatchable_object, VkBufferView vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkBufferViewMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_buffer_view(VkDevice dispatchable_object, VkBufferView object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkBufferViewMap.find(object_handle);
    if (it != VkBufferViewMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkBufferViewMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_image_view(VkDevice dispatchable_object, VkImageView vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkImageViewMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_image_view(VkDevice dispatchable_object, VkImageView object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkImageViewMap.find(object_handle);
    if (it != VkImageViewMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkImageViewMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_shader_module(VkDevice dispatchable_object, VkShaderModule vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkShaderModuleMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_shader_module(VkDevice dispatchable_object, VkShaderModule object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkShaderModuleMap.find(object_handle);
    if (it != VkShaderModuleMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkShaderModuleMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_pipeline_cache(VkDevice dispatchable_object, VkPipelineCache vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkPipelineCacheMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_pipeline_cache(VkDevice dispatchable_object, VkPipelineCache object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkPipelineCacheMap.find(object_handle);
    if (it != VkPipelineCacheMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkPipelineCacheMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_pipeline_layout(VkDevice dispatchable_object, VkPipelineLayout vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkPipelineLayoutMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_pipeline_layout(VkDevice dispatchable_object, VkPipelineLayout object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkPipelineLayoutMap.find(object_handle);
    if (it != VkPipelineLayoutMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkPipelineLayoutMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_pipeline(VkDevice dispatchable_object, VkPipeline vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkPipelineMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_pipeline(VkDevice dispatchable_object, VkPipeline object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkPipelineMap.find(object_handle);
    if (it != VkPipelineMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkPipelineMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_descriptor_set_layout(VkDevice dispatchable_object, VkDescriptorSetLayout vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkDescriptorSetLayoutMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_descriptor_set_layout(VkDevice dispatchable_object, VkDescriptorSetLayout object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkDescriptorSetLayoutMap.find(object_handle);
    if (it != VkDescriptorSetLayoutMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkDescriptorSetLayoutMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_sampler(VkDevice dispatchable_object, VkSampler vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkSamplerMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_sampler(VkDevice dispatchable_object, VkSampler object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkSamplerMap.find(object_handle);
    if (it != VkSamplerMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkSamplerMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_descriptor_pool(VkDevice dispatchable_object, VkDescriptorPool vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkDescriptorPoolMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_descriptor_pool(VkDevice dispatchable_object, VkDescriptorPool object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkDescriptorPoolMap.find(object_handle);
    if (it != VkDescriptorPoolMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkDescriptorPoolMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_descriptor_set(VkDevice dispatchable_object, VkDescriptorSet vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkDescriptorSetMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_descriptor_set(VkDevice dispatchable_object, VkDescriptorSet object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkDescriptorSetMap.find(object_handle);
    if (it != VkDescriptorSetMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkDescriptorSetMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_render_pass(VkDevice dispatchable_object, VkRenderPass vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkRenderPassMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_render_pass(VkDevice dispatchable_object, VkRenderPass object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkRenderPassMap.find(object_handle);
    if (it != VkRenderPassMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkRenderPassMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_framebuffer(VkDevice dispatchable_object, VkFramebuffer vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkFramebufferMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_framebuffer(VkDevice dispatchable_object, VkFramebuffer object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkFramebufferMap.find(object_handle);
    if (it != VkFramebufferMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkFramebufferMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_swapchain_khr(VkDevice dispatchable_object, VkSwapchainKHR vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkSwapchainKHRMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_swapchain_khr(VkDevice dispatchable_object, VkSwapchainKHR object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkSwapchainKHRMap.find(object_handle);
    if (it != VkSwapchainKHRMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkSwapchainKHRMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_surface_khr(VkDevice dispatchable_object, VkSurfaceKHR vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkSurfaceKHRMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_surface_khr(VkDevice dispatchable_object, VkSurfaceKHR object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkSurfaceKHRMap.find(object_handle);
    if (it != VkSurfaceKHRMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkSurfaceKHRMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

// CODEGEN : file ../vk-layer-generate.py line #781
static void create_debug_report_callback_ext(VkDevice dispatchable_object, VkDebugReportCallbackEXT vkObj, VkDebugReportObjectTypeEXT objType)
{
    log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, objType,(uint64_t)(vkObj), __LINE__, OBJTRACK_NONE, "OBJTRACK",
        "OBJ[%llu] : CREATE %s object 0x%" PRIxLEAST64 , object_track_index++, string_VkDebugReportObjectTypeEXT(objType),
        (uint64_t)(vkObj));

    OBJTRACK_NODE* pNewObjNode = new OBJTRACK_NODE;
    pNewObjNode->belongsTo = (uint64_t)dispatchable_object;
    pNewObjNode->objType = objType;
    pNewObjNode->status  = OBJSTATUS_NONE;
    pNewObjNode->vkObj  = (uint64_t)(vkObj);
    VkDebugReportCallbackEXTMap[(uint64_t)vkObj] = pNewObjNode;
    uint32_t objIndex = objTypeToIndex(objType);
    numObjs[objIndex]++;
    numTotalObjs++;
}

// CODEGEN : file ../vk-layer-generate.py line #804
static void destroy_debug_report_callback_ext(VkDevice dispatchable_object, VkDebugReportCallbackEXT object)
{
    uint64_t object_handle = (uint64_t)(object);
    auto it = VkDebugReportCallbackEXTMap.find(object_handle);
    if (it != VkDebugReportCallbackEXTMap.end()) {
        OBJTRACK_NODE* pNode = it->second;
        uint32_t objIndex = objTypeToIndex(pNode->objType);
        assert(numTotalObjs > 0);
        numTotalObjs--;
        assert(numObjs[objIndex] > 0);
        numObjs[objIndex]--;
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pNode->objType, object_handle, __LINE__, OBJTRACK_NONE, "OBJTRACK",
           "OBJ_STAT Destroy %s obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " %s objs).",
            string_VkDebugReportObjectTypeEXT(pNode->objType), (uint64_t)(object), numTotalObjs, numObjs[objIndex],
            string_VkDebugReportObjectTypeEXT(pNode->objType));
        delete pNode;
        VkDebugReportCallbackEXTMap.erase(it);
    } else {
        log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, (VkDebugReportObjectTypeEXT ) 0,
            object_handle, __LINE__, OBJTRACK_UNKNOWN_OBJECT, "OBJTRACK",
            "Unable to remove obj 0x%" PRIxLEAST64 ". Was it created? Has it already been destroyed?",
            object_handle);
    }
}

//['VkCommandBuffer', 'VkDevice', 'VkInstance', 'VkPhysicalDevice', 'VkQueue']
// CODEGEN : file ../vk-layer-generate.py line #842
static bool validate_command_buffer(VkCommandBuffer dispatchable_object, VkCommandBuffer object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkCommandBufferMap.find((uint64_t)object) == VkCommandBufferMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkCommandBuffer Object 0x%" PRIx64 ,(uint64_t)(object));
    }
    return false;
}

// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_buffer(VkCommandBuffer dispatchable_object, VkBuffer object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkBufferMap.find((uint64_t)object) == VkBufferMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkBuffer Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_descriptor_set(VkCommandBuffer dispatchable_object, VkDescriptorSet object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkDescriptorSetMap.find((uint64_t)object) == VkDescriptorSetMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkDescriptorSet Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_event(VkCommandBuffer dispatchable_object, VkEvent object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkEventMap.find((uint64_t)object) == VkEventMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkEvent Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_framebuffer(VkCommandBuffer dispatchable_object, VkFramebuffer object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkFramebufferMap.find((uint64_t)object) == VkFramebufferMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkFramebuffer Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_image(VkCommandBuffer dispatchable_object, VkImage object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    // We need to validate normal image objects and those from the swapchain
    if ((VkImageMap.find((uint64_t)object) == VkImageMap.end()) &&
        (swapchainImageMap.find((uint64_t)object) == swapchainImageMap.end())) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkImage Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_pipeline(VkCommandBuffer dispatchable_object, VkPipeline object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkPipelineMap.find((uint64_t)object) == VkPipelineMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkPipeline Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_pipeline_layout(VkCommandBuffer dispatchable_object, VkPipelineLayout object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkPipelineLayoutMap.find((uint64_t)object) == VkPipelineLayoutMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkPipelineLayout Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_query_pool(VkCommandBuffer dispatchable_object, VkQueryPool object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkQueryPoolMap.find((uint64_t)object) == VkQueryPoolMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkQueryPool Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_render_pass(VkCommandBuffer dispatchable_object, VkRenderPass object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkRenderPassMap.find((uint64_t)object) == VkRenderPassMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkRenderPass Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}

// CODEGEN : file ../vk-layer-generate.py line #842
static bool validate_device(VkDevice dispatchable_object, VkDevice object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkDeviceMap.find((uint64_t)object) == VkDeviceMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkDevice Object 0x%" PRIx64 ,(uint64_t)(object));
    }
    return false;
}

// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_buffer(VkDevice dispatchable_object, VkBuffer object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkBufferMap.find((uint64_t)object) == VkBufferMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkBuffer Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_buffer_view(VkDevice dispatchable_object, VkBufferView object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkBufferViewMap.find((uint64_t)object) == VkBufferViewMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkBufferView Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_command_buffer(VkDevice dispatchable_object, VkCommandBuffer object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkCommandBufferMap.find((uint64_t)object) == VkCommandBufferMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkCommandBuffer Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_command_pool(VkDevice dispatchable_object, VkCommandPool object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkCommandPoolMap.find((uint64_t)object) == VkCommandPoolMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkCommandPool Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_descriptor_pool(VkDevice dispatchable_object, VkDescriptorPool object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkDescriptorPoolMap.find((uint64_t)object) == VkDescriptorPoolMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkDescriptorPool Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_descriptor_set(VkDevice dispatchable_object, VkDescriptorSet object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkDescriptorSetMap.find((uint64_t)object) == VkDescriptorSetMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkDescriptorSet Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_descriptor_set_layout(VkDevice dispatchable_object, VkDescriptorSetLayout object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkDescriptorSetLayoutMap.find((uint64_t)object) == VkDescriptorSetLayoutMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkDescriptorSetLayout Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_device_memory(VkDevice dispatchable_object, VkDeviceMemory object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkDeviceMemoryMap.find((uint64_t)object) == VkDeviceMemoryMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkDeviceMemory Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_event(VkDevice dispatchable_object, VkEvent object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkEventMap.find((uint64_t)object) == VkEventMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkEvent Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_fence(VkDevice dispatchable_object, VkFence object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkFenceMap.find((uint64_t)object) == VkFenceMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkFence Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_framebuffer(VkDevice dispatchable_object, VkFramebuffer object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkFramebufferMap.find((uint64_t)object) == VkFramebufferMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkFramebuffer Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_image(VkDevice dispatchable_object, VkImage object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    // We need to validate normal image objects and those from the swapchain
    if ((VkImageMap.find((uint64_t)object) == VkImageMap.end()) &&
        (swapchainImageMap.find((uint64_t)object) == swapchainImageMap.end())) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkImage Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_image_view(VkDevice dispatchable_object, VkImageView object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkImageViewMap.find((uint64_t)object) == VkImageViewMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkImageView Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_pipeline(VkDevice dispatchable_object, VkPipeline object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkPipelineMap.find((uint64_t)object) == VkPipelineMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkPipeline Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_pipeline_cache(VkDevice dispatchable_object, VkPipelineCache object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkPipelineCacheMap.find((uint64_t)object) == VkPipelineCacheMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkPipelineCache Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_pipeline_layout(VkDevice dispatchable_object, VkPipelineLayout object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkPipelineLayoutMap.find((uint64_t)object) == VkPipelineLayoutMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkPipelineLayout Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_query_pool(VkDevice dispatchable_object, VkQueryPool object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkQueryPoolMap.find((uint64_t)object) == VkQueryPoolMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkQueryPool Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_queue(VkDevice dispatchable_object, VkQueue object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkQueueMap.find((uint64_t)object) == VkQueueMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkQueue Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_render_pass(VkDevice dispatchable_object, VkRenderPass object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkRenderPassMap.find((uint64_t)object) == VkRenderPassMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkRenderPass Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_sampler(VkDevice dispatchable_object, VkSampler object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkSamplerMap.find((uint64_t)object) == VkSamplerMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkSampler Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_semaphore(VkDevice dispatchable_object, VkSemaphore object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkSemaphoreMap.find((uint64_t)object) == VkSemaphoreMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkSemaphore Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_shader_module(VkDevice dispatchable_object, VkShaderModule object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkShaderModuleMap.find((uint64_t)object) == VkShaderModuleMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkShaderModule Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_surface_khr(VkDevice dispatchable_object, VkSurfaceKHR object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkSurfaceKHRMap.find((uint64_t)object) == VkSurfaceKHRMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkSurfaceKHR Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_swapchain_khr(VkDevice dispatchable_object, VkSwapchainKHR object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkSwapchainKHRMap.find((uint64_t)object) == VkSwapchainKHRMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkSwapchainKHR Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}

// CODEGEN : file ../vk-layer-generate.py line #842
static bool validate_instance(VkInstance dispatchable_object, VkInstance object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkInstanceMap.find((uint64_t)object) == VkInstanceMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkInstance Object 0x%" PRIx64 ,(uint64_t)(object));
    }
    return false;
}

// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_physical_device(VkInstance dispatchable_object, VkPhysicalDevice object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkPhysicalDeviceMap.find((uint64_t)object) == VkPhysicalDeviceMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkPhysicalDevice Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_surface_khr(VkInstance dispatchable_object, VkSurfaceKHR object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkSurfaceKHRMap.find((uint64_t)object) == VkSurfaceKHRMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkSurfaceKHR Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}

// CODEGEN : file ../vk-layer-generate.py line #842
static bool validate_physical_device(VkPhysicalDevice dispatchable_object, VkPhysicalDevice object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkPhysicalDeviceMap.find((uint64_t)object) == VkPhysicalDeviceMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkPhysicalDevice Object 0x%" PRIx64 ,(uint64_t)(object));
    }
    return false;
}

// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_device(VkPhysicalDevice dispatchable_object, VkDevice object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkDeviceMap.find((uint64_t)object) == VkDeviceMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkDevice Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_surface_khr(VkPhysicalDevice dispatchable_object, VkSurfaceKHR object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkSurfaceKHRMap.find((uint64_t)object) == VkSurfaceKHRMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkSurfaceKHR Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}

// CODEGEN : file ../vk-layer-generate.py line #842
static bool validate_queue(VkQueue dispatchable_object, VkQueue object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkQueueMap.find((uint64_t)object) == VkQueueMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkQueue Object 0x%" PRIx64 ,(uint64_t)(object));
    }
    return false;
}

// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_buffer(VkQueue dispatchable_object, VkBuffer object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkBufferMap.find((uint64_t)object) == VkBufferMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkBuffer Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_command_buffer(VkQueue dispatchable_object, VkCommandBuffer object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkCommandBufferMap.find((uint64_t)object) == VkCommandBufferMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkCommandBuffer Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_device_memory(VkQueue dispatchable_object, VkDeviceMemory object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkDeviceMemoryMap.find((uint64_t)object) == VkDeviceMemoryMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkDeviceMemory Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_fence(VkQueue dispatchable_object, VkFence object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkFenceMap.find((uint64_t)object) == VkFenceMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkFence Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_image(VkQueue dispatchable_object, VkImage object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    // We need to validate normal image objects and those from the swapchain
    if ((VkImageMap.find((uint64_t)object) == VkImageMap.end()) &&
        (swapchainImageMap.find((uint64_t)object) == swapchainImageMap.end())) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkImage Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_semaphore(VkQueue dispatchable_object, VkSemaphore object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkSemaphoreMap.find((uint64_t)object) == VkSemaphoreMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkSemaphore Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}
// CODEGEN : file ../vk-layer-generate.py line #859
static bool validate_swapchain_khr(VkQueue dispatchable_object, VkSwapchainKHR object, VkDebugReportObjectTypeEXT objType, bool null_allowed)
{
    if (null_allowed && (object == VK_NULL_HANDLE))
        return false;
    if (VkSwapchainKHRMap.find((uint64_t)object) == VkSwapchainKHRMap.end()) {
        return log_msg(mdd(dispatchable_object), VK_DEBUG_REPORT_ERROR_BIT_EXT, objType, (uint64_t)(object), __LINE__, OBJTRACK_INVALID_OBJECT, "OBJTRACK",
            "Invalid VkSwapchainKHR Object 0x%" PRIx64, (uint64_t)(object));
    }
    return false;
}



// CODEGEN : file ../vk-layer-generate.py line #881
VKAPI_ATTR void VKAPI_CALL DestroyInstance(
VkInstance instance,
const VkAllocationCallbacks* pAllocator)
{
    std::unique_lock<std::mutex> lock(global_lock);

    dispatch_key key = get_dispatch_key(instance);
    layer_data *my_data = get_my_data_ptr(key, layer_data_map);

    // Enable the temporary callback(s) here to catch cleanup issues:
    bool callback_setup = false;
    if (my_data->num_tmp_callbacks > 0) {
        if (!layer_enable_tmp_callbacks(my_data->report_data,
                                        my_data->num_tmp_callbacks,
                                        my_data->tmp_dbg_create_infos,
                                        my_data->tmp_callbacks)) {
            callback_setup = true;
        }
    }

    validate_instance(instance, instance, VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT, false);

    destroy_instance(instance, instance);
    // Report any remaining objects in LL

    for (auto iit = VkDeviceMap.begin(); iit != VkDeviceMap.end();) {
        OBJTRACK_NODE* pNode = iit->second;
        if (pNode->belongsTo == (uint64_t)instance) {
            log_msg(mid(instance), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                    "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                    pNode->vkObj);
            for (auto idt = VkSemaphoreMap.begin(); idt != VkSemaphoreMap.end();) {
                OBJTRACK_NODE* pNode = idt->second;
                if (pNode->belongsTo == iit->first) {
                    log_msg(mid(instance), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                            "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                            pNode->vkObj);
                    VkSemaphoreMap.erase(idt++);
                } else {
                    ++idt;
                }
            }
            for (auto idt = VkCommandBufferMap.begin(); idt != VkCommandBufferMap.end();) {
                OBJTRACK_NODE* pNode = idt->second;
                if (pNode->belongsTo == iit->first) {
                    log_msg(mid(instance), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                            "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                            pNode->vkObj);
                    VkCommandBufferMap.erase(idt++);
                } else {
                    ++idt;
                }
            }
            for (auto idt = VkFenceMap.begin(); idt != VkFenceMap.end();) {
                OBJTRACK_NODE* pNode = idt->second;
                if (pNode->belongsTo == iit->first) {
                    log_msg(mid(instance), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                            "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                            pNode->vkObj);
                    VkFenceMap.erase(idt++);
                } else {
                    ++idt;
                }
            }
            for (auto idt = VkDeviceMemoryMap.begin(); idt != VkDeviceMemoryMap.end();) {
                OBJTRACK_NODE* pNode = idt->second;
                if (pNode->belongsTo == iit->first) {
                    log_msg(mid(instance), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                            "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                            pNode->vkObj);
                    VkDeviceMemoryMap.erase(idt++);
                } else {
                    ++idt;
                }
            }
            for (auto idt = VkBufferMap.begin(); idt != VkBufferMap.end();) {
                OBJTRACK_NODE* pNode = idt->second;
                if (pNode->belongsTo == iit->first) {
                    log_msg(mid(instance), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                            "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                            pNode->vkObj);
                    VkBufferMap.erase(idt++);
                } else {
                    ++idt;
                }
            }
            for (auto idt = VkImageMap.begin(); idt != VkImageMap.end();) {
                OBJTRACK_NODE* pNode = idt->second;
                if (pNode->belongsTo == iit->first) {
                    log_msg(mid(instance), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                            "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                            pNode->vkObj);
                    VkImageMap.erase(idt++);
                } else {
                    ++idt;
                }
            }
            for (auto idt = VkEventMap.begin(); idt != VkEventMap.end();) {
                OBJTRACK_NODE* pNode = idt->second;
                if (pNode->belongsTo == iit->first) {
                    log_msg(mid(instance), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                            "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                            pNode->vkObj);
                    VkEventMap.erase(idt++);
                } else {
                    ++idt;
                }
            }
            for (auto idt = VkQueryPoolMap.begin(); idt != VkQueryPoolMap.end();) {
                OBJTRACK_NODE* pNode = idt->second;
                if (pNode->belongsTo == iit->first) {
                    log_msg(mid(instance), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                            "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                            pNode->vkObj);
                    VkQueryPoolMap.erase(idt++);
                } else {
                    ++idt;
                }
            }
            for (auto idt = VkBufferViewMap.begin(); idt != VkBufferViewMap.end();) {
                OBJTRACK_NODE* pNode = idt->second;
                if (pNode->belongsTo == iit->first) {
                    log_msg(mid(instance), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                            "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                            pNode->vkObj);
                    VkBufferViewMap.erase(idt++);
                } else {
                    ++idt;
                }
            }
            for (auto idt = VkImageViewMap.begin(); idt != VkImageViewMap.end();) {
                OBJTRACK_NODE* pNode = idt->second;
                if (pNode->belongsTo == iit->first) {
                    log_msg(mid(instance), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                            "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                            pNode->vkObj);
                    VkImageViewMap.erase(idt++);
                } else {
                    ++idt;
                }
            }
            for (auto idt = VkShaderModuleMap.begin(); idt != VkShaderModuleMap.end();) {
                OBJTRACK_NODE* pNode = idt->second;
                if (pNode->belongsTo == iit->first) {
                    log_msg(mid(instance), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                            "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                            pNode->vkObj);
                    VkShaderModuleMap.erase(idt++);
                } else {
                    ++idt;
                }
            }
            for (auto idt = VkPipelineCacheMap.begin(); idt != VkPipelineCacheMap.end();) {
                OBJTRACK_NODE* pNode = idt->second;
                if (pNode->belongsTo == iit->first) {
                    log_msg(mid(instance), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                            "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                            pNode->vkObj);
                    VkPipelineCacheMap.erase(idt++);
                } else {
                    ++idt;
                }
            }
            for (auto idt = VkPipelineLayoutMap.begin(); idt != VkPipelineLayoutMap.end();) {
                OBJTRACK_NODE* pNode = idt->second;
                if (pNode->belongsTo == iit->first) {
                    log_msg(mid(instance), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                            "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                            pNode->vkObj);
                    VkPipelineLayoutMap.erase(idt++);
                } else {
                    ++idt;
                }
            }
            for (auto idt = VkRenderPassMap.begin(); idt != VkRenderPassMap.end();) {
                OBJTRACK_NODE* pNode = idt->second;
                if (pNode->belongsTo == iit->first) {
                    log_msg(mid(instance), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                            "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                            pNode->vkObj);
                    VkRenderPassMap.erase(idt++);
                } else {
                    ++idt;
                }
            }
            for (auto idt = VkPipelineMap.begin(); idt != VkPipelineMap.end();) {
                OBJTRACK_NODE* pNode = idt->second;
                if (pNode->belongsTo == iit->first) {
                    log_msg(mid(instance), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                            "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                            pNode->vkObj);
                    VkPipelineMap.erase(idt++);
                } else {
                    ++idt;
                }
            }
            for (auto idt = VkDescriptorSetLayoutMap.begin(); idt != VkDescriptorSetLayoutMap.end();) {
                OBJTRACK_NODE* pNode = idt->second;
                if (pNode->belongsTo == iit->first) {
                    log_msg(mid(instance), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                            "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                            pNode->vkObj);
                    VkDescriptorSetLayoutMap.erase(idt++);
                } else {
                    ++idt;
                }
            }
            for (auto idt = VkSamplerMap.begin(); idt != VkSamplerMap.end();) {
                OBJTRACK_NODE* pNode = idt->second;
                if (pNode->belongsTo == iit->first) {
                    log_msg(mid(instance), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                            "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                            pNode->vkObj);
                    VkSamplerMap.erase(idt++);
                } else {
                    ++idt;
                }
            }
            for (auto idt = VkDescriptorPoolMap.begin(); idt != VkDescriptorPoolMap.end();) {
                OBJTRACK_NODE* pNode = idt->second;
                if (pNode->belongsTo == iit->first) {
                    log_msg(mid(instance), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                            "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                            pNode->vkObj);
                    VkDescriptorPoolMap.erase(idt++);
                } else {
                    ++idt;
                }
            }
            for (auto idt = VkDescriptorSetMap.begin(); idt != VkDescriptorSetMap.end();) {
                OBJTRACK_NODE* pNode = idt->second;
                if (pNode->belongsTo == iit->first) {
                    log_msg(mid(instance), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                            "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                            pNode->vkObj);
                    VkDescriptorSetMap.erase(idt++);
                } else {
                    ++idt;
                }
            }
            for (auto idt = VkFramebufferMap.begin(); idt != VkFramebufferMap.end();) {
                OBJTRACK_NODE* pNode = idt->second;
                if (pNode->belongsTo == iit->first) {
                    log_msg(mid(instance), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                            "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                            pNode->vkObj);
                    VkFramebufferMap.erase(idt++);
                } else {
                    ++idt;
                }
            }
            for (auto idt = VkCommandPoolMap.begin(); idt != VkCommandPoolMap.end();) {
                OBJTRACK_NODE* pNode = idt->second;
                if (pNode->belongsTo == iit->first) {
                    log_msg(mid(instance), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                            "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                            pNode->vkObj);
                    VkCommandPoolMap.erase(idt++);
                } else {
                    ++idt;
                }
            }
            VkDeviceMap.erase(iit++);
        } else {
            ++iit;
        }
    }

    VkLayerInstanceDispatchTable *pInstanceTable = get_dispatch_table(object_tracker_instance_table_map, instance);
    pInstanceTable->DestroyInstance(instance, pAllocator);

    // Disable and cleanup the temporary callback(s):
    if (callback_setup) {
        layer_disable_tmp_callbacks(my_data->report_data,
                                    my_data->num_tmp_callbacks,
                                    my_data->tmp_callbacks);
    }
    if (my_data->num_tmp_callbacks > 0) {
        layer_free_tmp_callbacks(my_data->tmp_dbg_create_infos,
                                 my_data->tmp_callbacks);
        my_data->num_tmp_callbacks = 0;
    }

    // Clean up logging callback, if any
    while (my_data->logging_callback.size() > 0) {
        VkDebugReportCallbackEXT callback = my_data->logging_callback.back();
        layer_destroy_msg_callback(my_data->report_data, callback, pAllocator);
        my_data->logging_callback.pop_back();
    }

    layer_debug_report_destroy_instance(mid(instance));
    layer_data_map.erase(key);

    instanceExtMap.erase(pInstanceTable);
    lock.unlock();
    object_tracker_instance_table_map.erase(key);
}


// CODEGEN : file ../vk-layer-generate.py line #968
VKAPI_ATTR void VKAPI_CALL DestroyDevice(
VkDevice device,
const VkAllocationCallbacks* pAllocator)
{
    std::unique_lock<std::mutex> lock(global_lock);
    validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);

    destroy_device(device, device);
    // Report any remaining objects associated with this VkDevice object in LL
    for (auto it = VkSemaphoreMap.begin(); it != VkSemaphoreMap.end();) {
        OBJTRACK_NODE* pNode = it->second;
        if (pNode->belongsTo == (uint64_t)device) {
            log_msg(mdd(device), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                    "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                    pNode->vkObj);
            VkSemaphoreMap.erase(it++);
        } else {
            ++it;
        }
    }

    for (auto it = VkFenceMap.begin(); it != VkFenceMap.end();) {
        OBJTRACK_NODE* pNode = it->second;
        if (pNode->belongsTo == (uint64_t)device) {
            log_msg(mdd(device), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                    "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                    pNode->vkObj);
            VkFenceMap.erase(it++);
        } else {
            ++it;
        }
    }

    for (auto it = VkDeviceMemoryMap.begin(); it != VkDeviceMemoryMap.end();) {
        OBJTRACK_NODE* pNode = it->second;
        if (pNode->belongsTo == (uint64_t)device) {
            log_msg(mdd(device), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                    "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                    pNode->vkObj);
            VkDeviceMemoryMap.erase(it++);
        } else {
            ++it;
        }
    }

    for (auto it = VkBufferMap.begin(); it != VkBufferMap.end();) {
        OBJTRACK_NODE* pNode = it->second;
        if (pNode->belongsTo == (uint64_t)device) {
            log_msg(mdd(device), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                    "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                    pNode->vkObj);
            VkBufferMap.erase(it++);
        } else {
            ++it;
        }
    }

    for (auto it = VkImageMap.begin(); it != VkImageMap.end();) {
        OBJTRACK_NODE* pNode = it->second;
        if (pNode->belongsTo == (uint64_t)device) {
            log_msg(mdd(device), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                    "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                    pNode->vkObj);
            VkImageMap.erase(it++);
        } else {
            ++it;
        }
    }

    for (auto it = VkEventMap.begin(); it != VkEventMap.end();) {
        OBJTRACK_NODE* pNode = it->second;
        if (pNode->belongsTo == (uint64_t)device) {
            log_msg(mdd(device), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                    "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                    pNode->vkObj);
            VkEventMap.erase(it++);
        } else {
            ++it;
        }
    }

    for (auto it = VkQueryPoolMap.begin(); it != VkQueryPoolMap.end();) {
        OBJTRACK_NODE* pNode = it->second;
        if (pNode->belongsTo == (uint64_t)device) {
            log_msg(mdd(device), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                    "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                    pNode->vkObj);
            VkQueryPoolMap.erase(it++);
        } else {
            ++it;
        }
    }

    for (auto it = VkBufferViewMap.begin(); it != VkBufferViewMap.end();) {
        OBJTRACK_NODE* pNode = it->second;
        if (pNode->belongsTo == (uint64_t)device) {
            log_msg(mdd(device), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                    "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                    pNode->vkObj);
            VkBufferViewMap.erase(it++);
        } else {
            ++it;
        }
    }

    for (auto it = VkImageViewMap.begin(); it != VkImageViewMap.end();) {
        OBJTRACK_NODE* pNode = it->second;
        if (pNode->belongsTo == (uint64_t)device) {
            log_msg(mdd(device), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                    "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                    pNode->vkObj);
            VkImageViewMap.erase(it++);
        } else {
            ++it;
        }
    }

    for (auto it = VkShaderModuleMap.begin(); it != VkShaderModuleMap.end();) {
        OBJTRACK_NODE* pNode = it->second;
        if (pNode->belongsTo == (uint64_t)device) {
            log_msg(mdd(device), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                    "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                    pNode->vkObj);
            VkShaderModuleMap.erase(it++);
        } else {
            ++it;
        }
    }

    for (auto it = VkPipelineCacheMap.begin(); it != VkPipelineCacheMap.end();) {
        OBJTRACK_NODE* pNode = it->second;
        if (pNode->belongsTo == (uint64_t)device) {
            log_msg(mdd(device), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                    "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                    pNode->vkObj);
            VkPipelineCacheMap.erase(it++);
        } else {
            ++it;
        }
    }

    for (auto it = VkPipelineLayoutMap.begin(); it != VkPipelineLayoutMap.end();) {
        OBJTRACK_NODE* pNode = it->second;
        if (pNode->belongsTo == (uint64_t)device) {
            log_msg(mdd(device), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                    "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                    pNode->vkObj);
            VkPipelineLayoutMap.erase(it++);
        } else {
            ++it;
        }
    }

    for (auto it = VkRenderPassMap.begin(); it != VkRenderPassMap.end();) {
        OBJTRACK_NODE* pNode = it->second;
        if (pNode->belongsTo == (uint64_t)device) {
            log_msg(mdd(device), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                    "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                    pNode->vkObj);
            VkRenderPassMap.erase(it++);
        } else {
            ++it;
        }
    }

    for (auto it = VkPipelineMap.begin(); it != VkPipelineMap.end();) {
        OBJTRACK_NODE* pNode = it->second;
        if (pNode->belongsTo == (uint64_t)device) {
            log_msg(mdd(device), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                    "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                    pNode->vkObj);
            VkPipelineMap.erase(it++);
        } else {
            ++it;
        }
    }

    for (auto it = VkDescriptorSetLayoutMap.begin(); it != VkDescriptorSetLayoutMap.end();) {
        OBJTRACK_NODE* pNode = it->second;
        if (pNode->belongsTo == (uint64_t)device) {
            log_msg(mdd(device), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                    "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                    pNode->vkObj);
            VkDescriptorSetLayoutMap.erase(it++);
        } else {
            ++it;
        }
    }

    for (auto it = VkSamplerMap.begin(); it != VkSamplerMap.end();) {
        OBJTRACK_NODE* pNode = it->second;
        if (pNode->belongsTo == (uint64_t)device) {
            log_msg(mdd(device), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                    "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                    pNode->vkObj);
            VkSamplerMap.erase(it++);
        } else {
            ++it;
        }
    }

    for (auto it = VkDescriptorPoolMap.begin(); it != VkDescriptorPoolMap.end();) {
        OBJTRACK_NODE* pNode = it->second;
        if (pNode->belongsTo == (uint64_t)device) {
            log_msg(mdd(device), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                    "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                    pNode->vkObj);
            VkDescriptorPoolMap.erase(it++);
        } else {
            ++it;
        }
    }

    for (auto it = VkFramebufferMap.begin(); it != VkFramebufferMap.end();) {
        OBJTRACK_NODE* pNode = it->second;
        if (pNode->belongsTo == (uint64_t)device) {
            log_msg(mdd(device), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                    "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                    pNode->vkObj);
            VkFramebufferMap.erase(it++);
        } else {
            ++it;
        }
    }

    for (auto it = VkCommandPoolMap.begin(); it != VkCommandPoolMap.end();) {
        OBJTRACK_NODE* pNode = it->second;
        if (pNode->belongsTo == (uint64_t)device) {
            log_msg(mdd(device), VK_DEBUG_REPORT_ERROR_BIT_EXT, pNode->objType, pNode->vkObj, __LINE__, OBJTRACK_OBJECT_LEAK, "OBJTRACK",
                    "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.", string_VkDebugReportObjectTypeEXT(pNode->objType),
                    pNode->vkObj);
            VkCommandPoolMap.erase(it++);
        } else {
            ++it;
        }
    }

    // Clean up Queue's MemRef Linked Lists
    destroyQueueMemRefLists();

    lock.unlock();

    dispatch_key key = get_dispatch_key(device);
    VkLayerDispatchTable *pDisp = get_dispatch_table(object_tracker_device_table_map, device);
    pDisp->DestroyDevice(device, pAllocator);
    object_tracker_device_table_map.erase(key);

}


// CODEGEN : file ../vk-layer-generate.py line #1216
VKAPI_ATTR VkResult VKAPI_CALL CreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance)
{
    return explicit_CreateInstance(pCreateInfo, pAllocator, pInstance);
}



// CODEGEN : file ../vk-layer-generate.py line #1216
VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices)
{
    return explicit_EnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['physicalDevice']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_physical_device(physicalDevice, physicalDevice, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_instance_table_map, physicalDevice)->GetPhysicalDeviceFeatures(physicalDevice, pFeatures);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['physicalDevice']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_physical_device(physicalDevice, physicalDevice, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_instance_table_map, physicalDevice)->GetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties* pImageFormatProperties)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['physicalDevice']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_physical_device(physicalDevice, physicalDevice, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_instance_table_map, physicalDevice)->GetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['physicalDevice']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_physical_device(physicalDevice, physicalDevice, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_instance_table_map, physicalDevice)->GetPhysicalDeviceProperties(physicalDevice, pProperties);
}

// CODEGEN : file ../vk-layer-generate.py line #1216
VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties)
{
    return explicit_GetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['physicalDevice']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_physical_device(physicalDevice, physicalDevice, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_instance_table_map, physicalDevice)->GetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetInstanceProcAddr(VkInstance instance, const char* pName);

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetDeviceProcAddr(VkDevice device, const char* pName);

// CODEGEN : file ../vk-layer-generate.py line #1216
VKAPI_ATTR VkResult VKAPI_CALL CreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
{
    return explicit_CreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
}



// CODEGEN : file ../vk-layer-generate.py line #1216
VKAPI_ATTR void VKAPI_CALL GetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue)
{
    return explicit_GetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL QueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['fence', 'pSubmits[submitCount]', 'queue']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_fence(queue, fence, VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, true);
        if (pSubmits) {
// CODEGEN : file ../vk-layer-generate.py line #1055
            for (uint32_t idx0=0; idx0<submitCount; ++idx0) {
                if (pSubmits[idx0].pCommandBuffers) {
                    for (uint32_t idx1=0; idx1<pSubmits[idx0].commandBufferCount; ++idx1) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                        skipCall |= validate_command_buffer(queue, pSubmits[idx0].pCommandBuffers[idx1], VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
                    }
                }
                if (pSubmits[idx0].pSignalSemaphores) {
                    for (uint32_t idx2=0; idx2<pSubmits[idx0].signalSemaphoreCount; ++idx2) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                        skipCall |= validate_semaphore(queue, pSubmits[idx0].pSignalSemaphores[idx2], VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, false);
                    }
                }
                if (pSubmits[idx0].pWaitSemaphores) {
                    for (uint32_t idx3=0; idx3<pSubmits[idx0].waitSemaphoreCount; ++idx3) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                        skipCall |= validate_semaphore(queue, pSubmits[idx0].pWaitSemaphores[idx3], VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, false);
                    }
                }
            }
        }
        if (queue) {
// CODEGEN : file ../vk-layer-generate.py line #1091
            skipCall |= validate_queue(queue, queue, VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT, false);
        }
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, queue)->QueueSubmit(queue, submitCount, pSubmits, fence);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL QueueWaitIdle(VkQueue queue)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['queue']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_queue(queue, queue, VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, queue)->QueueWaitIdle(queue);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL DeviceWaitIdle(VkDevice device)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->DeviceWaitIdle(device);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL AllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->AllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        if (result == VK_SUCCESS) {
            create_device_memory(device, *pMemory, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT);
        }
    }
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216
VKAPI_ATTR void VKAPI_CALL FreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator)
{
    return explicit_FreeMemory(device, memory, pAllocator);
}

// CODEGEN : file ../vk-layer-generate.py line #1216
VKAPI_ATTR VkResult VKAPI_CALL MapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData)
{
    return explicit_MapMemory(device, memory, offset, size, flags, ppData);
}

// CODEGEN : file ../vk-layer-generate.py line #1216
VKAPI_ATTR void VKAPI_CALL UnmapMemory(VkDevice device, VkDeviceMemory memory)
{
    return explicit_UnmapMemory(device, memory);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL FlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'pMemoryRanges[memoryRangeCount]']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
        if (pMemoryRanges) {
// CODEGEN : file ../vk-layer-generate.py line #1055
            for (uint32_t idx0=0; idx0<memoryRangeCount; ++idx0) {
                if (pMemoryRanges[idx0].memory) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                    skipCall |= validate_device_memory(device, pMemoryRanges[idx0].memory, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT, false);
                }
            }
        }
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->FlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL InvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'pMemoryRanges[memoryRangeCount]']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
        if (pMemoryRanges) {
// CODEGEN : file ../vk-layer-generate.py line #1055
            for (uint32_t idx0=0; idx0<memoryRangeCount; ++idx0) {
                if (pMemoryRanges[idx0].memory) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                    skipCall |= validate_device_memory(device, pMemoryRanges[idx0].memory, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT, false);
                }
            }
        }
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->InvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL GetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'memory']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device_memory(device, memory, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, device)->GetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL BindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['buffer', 'device', 'memory']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_buffer(device, buffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device_memory(device, memory, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->BindBufferMemory(device, buffer, memory, memoryOffset);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL BindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'image', 'memory']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_image(device, image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device_memory(device, memory, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->BindImageMemory(device, image, memory, memoryOffset);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['buffer', 'device']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_buffer(device, buffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, device)->GetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL GetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'image']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_image(device, image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, device)->GetImageMemoryRequirements(device, image, pMemoryRequirements);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL GetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements* pSparseMemoryRequirements)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'image']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_image(device, image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, device)->GetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['physicalDevice']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_physical_device(physicalDevice, physicalDevice, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_instance_table_map, physicalDevice)->GetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
}

// CODEGEN : file ../vk-layer-generate.py line #1216
VKAPI_ATTR VkResult VKAPI_CALL QueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence)
{
    return explicit_QueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL CreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->CreateFence(device, pCreateInfo, pAllocator, pFence);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        if (result == VK_SUCCESS) {
            create_fence(device, *pFence, VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT);
        }
    }
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


// CODEGEN : file ../vk-layer-generate.py line #1251


VKAPI_ATTR void VKAPI_CALL DestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'fence']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_fence(device, fence, VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, false);
    }
    if (skipCall)
        return;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        destroy_fence(device, fence);
    }
    get_dispatch_table(object_tracker_device_table_map, device)->DestroyFence(device, fence, pAllocator);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL ResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'pFences[fenceCount]']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
        if (pFences) {
            for (uint32_t idx0=0; idx0<fenceCount; ++idx0) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                skipCall |= validate_fence(device, pFences[idx0], VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, false);
            }
        }
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->ResetFences(device, fenceCount, pFences);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL GetFenceStatus(VkDevice device, VkFence fence)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'fence']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_fence(device, fence, VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->GetFenceStatus(device, fence);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL WaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'pFences[fenceCount]']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
        if (pFences) {
            for (uint32_t idx0=0; idx0<fenceCount; ++idx0) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                skipCall |= validate_fence(device, pFences[idx0], VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, false);
            }
        }
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->WaitForFences(device, fenceCount, pFences, waitAll, timeout);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL CreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->CreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        if (result == VK_SUCCESS) {
            create_semaphore(device, *pSemaphore, VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT);
        }
    }
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


// CODEGEN : file ../vk-layer-generate.py line #1251


VKAPI_ATTR void VKAPI_CALL DestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'semaphore']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_semaphore(device, semaphore, VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, false);
    }
    if (skipCall)
        return;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        destroy_semaphore(device, semaphore);
    }
    get_dispatch_table(object_tracker_device_table_map, device)->DestroySemaphore(device, semaphore, pAllocator);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL CreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkEvent* pEvent)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->CreateEvent(device, pCreateInfo, pAllocator, pEvent);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        if (result == VK_SUCCESS) {
            create_event(device, *pEvent, VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT);
        }
    }
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


// CODEGEN : file ../vk-layer-generate.py line #1251


VKAPI_ATTR void VKAPI_CALL DestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'event']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_event(device, event, VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT, false);
    }
    if (skipCall)
        return;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        destroy_event(device, event);
    }
    get_dispatch_table(object_tracker_device_table_map, device)->DestroyEvent(device, event, pAllocator);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL GetEventStatus(VkDevice device, VkEvent event)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'event']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_event(device, event, VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->GetEventStatus(device, event);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL SetEvent(VkDevice device, VkEvent event)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'event']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_event(device, event, VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->SetEvent(device, event);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL ResetEvent(VkDevice device, VkEvent event)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'event']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_event(device, event, VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->ResetEvent(device, event);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL CreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->CreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        if (result == VK_SUCCESS) {
            create_query_pool(device, *pQueryPool, VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT);
        }
    }
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


// CODEGEN : file ../vk-layer-generate.py line #1251


VKAPI_ATTR void VKAPI_CALL DestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'queryPool']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_query_pool(device, queryPool, VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT, false);
    }
    if (skipCall)
        return;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        destroy_query_pool(device, queryPool);
    }
    get_dispatch_table(object_tracker_device_table_map, device)->DestroyQueryPool(device, queryPool, pAllocator);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL GetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'queryPool']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_query_pool(device, queryPool, VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->GetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL CreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->CreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        if (result == VK_SUCCESS) {
            create_buffer(device, *pBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT);
        }
    }
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


// CODEGEN : file ../vk-layer-generate.py line #1251


VKAPI_ATTR void VKAPI_CALL DestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['buffer', 'device']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_buffer(device, buffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
    }
    if (skipCall)
        return;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        destroy_buffer(device, buffer);
    }
    get_dispatch_table(object_tracker_device_table_map, device)->DestroyBuffer(device, buffer, pAllocator);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL CreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'pCreateInfo']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
        if (pCreateInfo) {
// CODEGEN : file ../vk-layer-generate.py line #1099
            skipCall |= validate_buffer(device, pCreateInfo->buffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, false);
        }
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->CreateBufferView(device, pCreateInfo, pAllocator, pView);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        if (result == VK_SUCCESS) {
            create_buffer_view(device, *pView, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT);
        }
    }
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


// CODEGEN : file ../vk-layer-generate.py line #1251


VKAPI_ATTR void VKAPI_CALL DestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['bufferView', 'device']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_buffer_view(device, bufferView, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
    }
    if (skipCall)
        return;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        destroy_buffer_view(device, bufferView);
    }
    get_dispatch_table(object_tracker_device_table_map, device)->DestroyBufferView(device, bufferView, pAllocator);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL CreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->CreateImage(device, pCreateInfo, pAllocator, pImage);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        if (result == VK_SUCCESS) {
            create_image(device, *pImage, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT);
        }
    }
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


// CODEGEN : file ../vk-layer-generate.py line #1251


VKAPI_ATTR void VKAPI_CALL DestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'image']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_image(device, image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, false);
    }
    if (skipCall)
        return;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        destroy_image(device, image);
    }
    get_dispatch_table(object_tracker_device_table_map, device)->DestroyImage(device, image, pAllocator);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL GetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'image']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_image(device, image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, device)->GetImageSubresourceLayout(device, image, pSubresource, pLayout);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL CreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'pCreateInfo']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
        if (pCreateInfo) {
// CODEGEN : file ../vk-layer-generate.py line #1099
            skipCall |= validate_image(device, pCreateInfo->image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, false);
        }
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->CreateImageView(device, pCreateInfo, pAllocator, pView);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        if (result == VK_SUCCESS) {
            create_image_view(device, *pView, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT);
        }
    }
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


// CODEGEN : file ../vk-layer-generate.py line #1251


VKAPI_ATTR void VKAPI_CALL DestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'imageView']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_image_view(device, imageView, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, false);
    }
    if (skipCall)
        return;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        destroy_image_view(device, imageView);
    }
    get_dispatch_table(object_tracker_device_table_map, device)->DestroyImageView(device, imageView, pAllocator);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL CreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->CreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        if (result == VK_SUCCESS) {
            create_shader_module(device, *pShaderModule, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT);
        }
    }
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


// CODEGEN : file ../vk-layer-generate.py line #1251


VKAPI_ATTR void VKAPI_CALL DestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'shaderModule']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_shader_module(device, shaderModule, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, false);
    }
    if (skipCall)
        return;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        destroy_shader_module(device, shaderModule);
    }
    get_dispatch_table(object_tracker_device_table_map, device)->DestroyShaderModule(device, shaderModule, pAllocator);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL CreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->CreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        if (result == VK_SUCCESS) {
            create_pipeline_cache(device, *pPipelineCache, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT);
        }
    }
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


// CODEGEN : file ../vk-layer-generate.py line #1251


VKAPI_ATTR void VKAPI_CALL DestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'pipelineCache']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_pipeline_cache(device, pipelineCache, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT, false);
    }
    if (skipCall)
        return;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        destroy_pipeline_cache(device, pipelineCache);
    }
    get_dispatch_table(object_tracker_device_table_map, device)->DestroyPipelineCache(device, pipelineCache, pAllocator);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL GetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'pipelineCache']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_pipeline_cache(device, pipelineCache, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->GetPipelineCacheData(device, pipelineCache, pDataSize, pData);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL MergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'dstCache', 'pSrcCaches[srcCacheCount]']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_pipeline_cache(device, dstCache, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT, false);
        if (pSrcCaches) {
            for (uint32_t idx0=0; idx0<srcCacheCount; ++idx0) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                skipCall |= validate_pipeline_cache(device, pSrcCaches[idx0], VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT, false);
            }
        }
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->MergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216
VKAPI_ATTR VkResult VKAPI_CALL CreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
{
    return explicit_CreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

// CODEGEN : file ../vk-layer-generate.py line #1216
VKAPI_ATTR VkResult VKAPI_CALL CreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
{
    return explicit_CreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


// CODEGEN : file ../vk-layer-generate.py line #1251


VKAPI_ATTR void VKAPI_CALL DestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'pipeline']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_pipeline(device, pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, false);
    }
    if (skipCall)
        return;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        destroy_pipeline(device, pipeline);
    }
    get_dispatch_table(object_tracker_device_table_map, device)->DestroyPipeline(device, pipeline, pAllocator);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL CreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'pCreateInfo']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
        if (pCreateInfo) {
            if (pCreateInfo->pSetLayouts) {
                for (uint32_t idx0=0; idx0<pCreateInfo->setLayoutCount; ++idx0) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                    skipCall |= validate_descriptor_set_layout(device, pCreateInfo->pSetLayouts[idx0], VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, false);
                }
            }
        }
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->CreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        if (result == VK_SUCCESS) {
            create_pipeline_layout(device, *pPipelineLayout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT);
        }
    }
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


// CODEGEN : file ../vk-layer-generate.py line #1251


VKAPI_ATTR void VKAPI_CALL DestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'pipelineLayout']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_pipeline_layout(device, pipelineLayout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, false);
    }
    if (skipCall)
        return;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        destroy_pipeline_layout(device, pipelineLayout);
    }
    get_dispatch_table(object_tracker_device_table_map, device)->DestroyPipelineLayout(device, pipelineLayout, pAllocator);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL CreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->CreateSampler(device, pCreateInfo, pAllocator, pSampler);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        if (result == VK_SUCCESS) {
            create_sampler(device, *pSampler, VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT);
        }
    }
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


// CODEGEN : file ../vk-layer-generate.py line #1251


VKAPI_ATTR void VKAPI_CALL DestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'sampler']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_sampler(device, sampler, VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, false);
    }
    if (skipCall)
        return;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        destroy_sampler(device, sampler);
    }
    get_dispatch_table(object_tracker_device_table_map, device)->DestroySampler(device, sampler, pAllocator);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'pCreateInfo']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
        if (pCreateInfo) {
            if (pCreateInfo->pBindings) {
// CODEGEN : file ../vk-layer-generate.py line #1055
                for (uint32_t idx0=0; idx0<pCreateInfo->bindingCount; ++idx0) {
                    if (pCreateInfo->pBindings[idx0].pImmutableSamplers) {
                        for (uint32_t idx1=0; idx1<pCreateInfo->pBindings[idx0].descriptorCount; ++idx1) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                            skipCall |= validate_sampler(device, pCreateInfo->pBindings[idx0].pImmutableSamplers[idx1], VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, false);
                        }
                    }
                }
            }
        }
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->CreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        if (result == VK_SUCCESS) {
            create_descriptor_set_layout(device, *pSetLayout, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT);
        }
    }
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


// CODEGEN : file ../vk-layer-generate.py line #1251


VKAPI_ATTR void VKAPI_CALL DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['descriptorSetLayout', 'device']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_descriptor_set_layout(device, descriptorSetLayout, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
    }
    if (skipCall)
        return;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        destroy_descriptor_set_layout(device, descriptorSetLayout);
    }
    get_dispatch_table(object_tracker_device_table_map, device)->DestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->CreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        if (result == VK_SUCCESS) {
            create_descriptor_pool(device, *pDescriptorPool, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT);
        }
    }
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216
VKAPI_ATTR void VKAPI_CALL DestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator)
{
    return explicit_DestroyDescriptorPool(device, descriptorPool, pAllocator);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL ResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['descriptorPool', 'device']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_descriptor_pool(device, descriptorPool, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->ResetDescriptorPool(device, descriptorPool, flags);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216
VKAPI_ATTR VkResult VKAPI_CALL AllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets)
{
    return explicit_AllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
}

// CODEGEN : file ../vk-layer-generate.py line #1216
VKAPI_ATTR VkResult VKAPI_CALL FreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets)
{
    return explicit_FreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL UpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'pDescriptorCopies[descriptorCopyCount]', 'pDescriptorWrites[descriptorWriteCount]']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
        if (pDescriptorCopies) {
// CODEGEN : file ../vk-layer-generate.py line #1055
            for (uint32_t idx0=0; idx0<descriptorCopyCount; ++idx0) {
                if (pDescriptorCopies[idx0].dstSet) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                    skipCall |= validate_descriptor_set(device, pDescriptorCopies[idx0].dstSet, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, false);
                }
                if (pDescriptorCopies[idx0].srcSet) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                    skipCall |= validate_descriptor_set(device, pDescriptorCopies[idx0].srcSet, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, false);
                }
            }
        }
        if (pDescriptorWrites) {
// CODEGEN : file ../vk-layer-generate.py line #1055
            for (uint32_t idx1=0; idx1<descriptorWriteCount; ++idx1) {
                if (pDescriptorWrites[idx1].dstSet) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                    skipCall |= validate_descriptor_set(device, pDescriptorWrites[idx1].dstSet, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, false);
                }
                if ((pDescriptorWrites[idx1].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)         ||
                    (pDescriptorWrites[idx1].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)         ||
                    (pDescriptorWrites[idx1].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) ||
                    (pDescriptorWrites[idx1].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)   ) {
// CODEGEN : file ../vk-layer-generate.py line #1055
                    for (uint32_t idx2=0; idx2<pDescriptorWrites[idx1].descriptorCount; ++idx2) {
                        if (pDescriptorWrites[idx1].pBufferInfo[idx2].buffer) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                            skipCall |= validate_buffer(device, pDescriptorWrites[idx1].pBufferInfo[idx2].buffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, false);
                        }
                    }
                }
                if ((pDescriptorWrites[idx1].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER)                ||
                    (pDescriptorWrites[idx1].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) ||
                    (pDescriptorWrites[idx1].descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)       ||
                    (pDescriptorWrites[idx1].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)          ||
                    (pDescriptorWrites[idx1].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)            ) {
// CODEGEN : file ../vk-layer-generate.py line #1055
                    for (uint32_t idx3=0; idx3<pDescriptorWrites[idx1].descriptorCount; ++idx3) {
                        if (pDescriptorWrites[idx1].pImageInfo[idx3].imageView) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                            skipCall |= validate_image_view(device, pDescriptorWrites[idx1].pImageInfo[idx3].imageView, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, false);
                        }
                        if (pDescriptorWrites[idx1].pImageInfo[idx3].sampler) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                            skipCall |= validate_sampler(device, pDescriptorWrites[idx1].pImageInfo[idx3].sampler, VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, false);
                        }
                    }
                }
                if ((pDescriptorWrites[idx1].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) ||
                    (pDescriptorWrites[idx1].descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER)   ) {
                    for (uint32_t idx4=0; idx4<pDescriptorWrites[idx1].descriptorCount; ++idx4) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                        skipCall |= validate_buffer_view(device, pDescriptorWrites[idx1].pTexelBufferView[idx4], VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT, true);
                    }
                }
            }
        }
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, device)->UpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL CreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'pCreateInfo']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
        if (pCreateInfo) {
            if (pCreateInfo->pAttachments) {
                for (uint32_t idx0=0; idx0<pCreateInfo->attachmentCount; ++idx0) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                    skipCall |= validate_image_view(device, pCreateInfo->pAttachments[idx0], VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, false);
                }
            }
            if (pCreateInfo->renderPass) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                skipCall |= validate_render_pass(device, pCreateInfo->renderPass, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, false);
            }
        }
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->CreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        if (result == VK_SUCCESS) {
            create_framebuffer(device, *pFramebuffer, VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT);
        }
    }
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


// CODEGEN : file ../vk-layer-generate.py line #1251


VKAPI_ATTR void VKAPI_CALL DestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'framebuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_framebuffer(device, framebuffer, VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, false);
    }
    if (skipCall)
        return;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        destroy_framebuffer(device, framebuffer);
    }
    get_dispatch_table(object_tracker_device_table_map, device)->DestroyFramebuffer(device, framebuffer, pAllocator);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL CreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->CreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        if (result == VK_SUCCESS) {
            create_render_pass(device, *pRenderPass, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT);
        }
    }
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


// CODEGEN : file ../vk-layer-generate.py line #1251


VKAPI_ATTR void VKAPI_CALL DestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'renderPass']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_render_pass(device, renderPass, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, false);
    }
    if (skipCall)
        return;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        destroy_render_pass(device, renderPass);
    }
    get_dispatch_table(object_tracker_device_table_map, device)->DestroyRenderPass(device, renderPass, pAllocator);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL GetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'renderPass']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_render_pass(device, renderPass, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, device)->GetRenderAreaGranularity(device, renderPass, pGranularity);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL CreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->CreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        if (result == VK_SUCCESS) {
            create_command_pool(device, *pCommandPool, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT);
        }
    }
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216
VKAPI_ATTR void VKAPI_CALL DestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator)
{
    return explicit_DestroyCommandPool(device, commandPool, pAllocator);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL ResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandPool', 'device']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_pool(device, commandPool, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->ResetCommandPool(device, commandPool, flags);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216
VKAPI_ATTR VkResult VKAPI_CALL AllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers)
{
    return explicit_AllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
}

// CODEGEN : file ../vk-layer-generate.py line #1216
VKAPI_ATTR void VKAPI_CALL FreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
{
    return explicit_FreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL BeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'pBeginInfo']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
        if (pBeginInfo) {
            OBJTRACK_NODE* pNode = VkCommandBufferMap[(uint64_t)commandBuffer];
            if ((pBeginInfo->pInheritanceInfo) && (pNode->status & OBJSTATUS_COMMAND_BUFFER_SECONDARY)) {
// CODEGEN : file ../vk-layer-generate.py line #1099
                skipCall |= validate_framebuffer(commandBuffer, pBeginInfo->pInheritanceInfo->framebuffer, VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, true);
// CODEGEN : file ../vk-layer-generate.py line #1099
                skipCall |= validate_render_pass(commandBuffer, pBeginInfo->pInheritanceInfo->renderPass, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, true);
            }
        }
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, commandBuffer)->BeginCommandBuffer(commandBuffer, pBeginInfo);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL EndCommandBuffer(VkCommandBuffer commandBuffer)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, commandBuffer)->EndCommandBuffer(commandBuffer);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL ResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, commandBuffer)->ResetCommandBuffer(commandBuffer, flags);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'pipeline']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_pipeline(commandBuffer, pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdSetLineWidth(commandBuffer, lineWidth);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4])
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdSetBlendConstants(commandBuffer, blendConstants);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdSetStencilReference(commandBuffer, faceMask, reference);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'layout', 'pDescriptorSets[descriptorSetCount]']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_pipeline_layout(commandBuffer, layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, false);
        if (pDescriptorSets) {
            for (uint32_t idx0=0; idx0<descriptorSetCount; ++idx0) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                skipCall |= validate_descriptor_set(commandBuffer, pDescriptorSets[idx0], VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, false);
            }
        }
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['buffer', 'commandBuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_buffer(commandBuffer, buffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'pBuffers[bindingCount]']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
        if (pBuffers) {
            for (uint32_t idx0=0; idx0<bindingCount; ++idx0) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                skipCall |= validate_buffer(commandBuffer, pBuffers[idx0], VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, false);
            }
        }
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['buffer', 'commandBuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_buffer(commandBuffer, buffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['buffer', 'commandBuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_buffer(commandBuffer, buffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdDispatch(commandBuffer, x, y, z);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['buffer', 'commandBuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_buffer(commandBuffer, buffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdDispatchIndirect(commandBuffer, buffer, offset);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'dstBuffer', 'srcBuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_buffer(commandBuffer, dstBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_buffer(commandBuffer, srcBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'dstImage', 'srcImage']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_image(commandBuffer, dstImage, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_image(commandBuffer, srcImage, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'dstImage', 'srcImage']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_image(commandBuffer, dstImage, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_image(commandBuffer, srcImage, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'dstImage', 'srcBuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_image(commandBuffer, dstImage, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_buffer(commandBuffer, srcBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'dstBuffer', 'srcImage']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_buffer(commandBuffer, dstBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_image(commandBuffer, srcImage, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const uint32_t* pData)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'dstBuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_buffer(commandBuffer, dstBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'dstBuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_buffer(commandBuffer, dstBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'image']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_image(commandBuffer, image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'image']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_image(commandBuffer, image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'dstImage', 'srcImage']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_image(commandBuffer, dstImage, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_image(commandBuffer, srcImage, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'event']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_event(commandBuffer, event, VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdSetEvent(commandBuffer, event, stageMask);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'event']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_event(commandBuffer, event, VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdResetEvent(commandBuffer, event, stageMask);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'pBufferMemoryBarriers[bufferMemoryBarrierCount]', 'pEvents[eventCount]', 'pImageMemoryBarriers[imageMemoryBarrierCount]']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
        if (pBufferMemoryBarriers) {
// CODEGEN : file ../vk-layer-generate.py line #1055
            for (uint32_t idx0=0; idx0<bufferMemoryBarrierCount; ++idx0) {
                if (pBufferMemoryBarriers[idx0].buffer) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                    skipCall |= validate_buffer(commandBuffer, pBufferMemoryBarriers[idx0].buffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, false);
                }
            }
        }
        if (pEvents) {
            for (uint32_t idx1=0; idx1<eventCount; ++idx1) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                skipCall |= validate_event(commandBuffer, pEvents[idx1], VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT, false);
            }
        }
        if (pImageMemoryBarriers) {
// CODEGEN : file ../vk-layer-generate.py line #1055
            for (uint32_t idx2=0; idx2<imageMemoryBarrierCount; ++idx2) {
                if (pImageMemoryBarriers[idx2].image) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                    skipCall |= validate_image(commandBuffer, pImageMemoryBarriers[idx2].image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, false);
                }
            }
        }
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'pBufferMemoryBarriers[bufferMemoryBarrierCount]', 'pImageMemoryBarriers[imageMemoryBarrierCount]']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
        if (pBufferMemoryBarriers) {
// CODEGEN : file ../vk-layer-generate.py line #1055
            for (uint32_t idx0=0; idx0<bufferMemoryBarrierCount; ++idx0) {
                if (pBufferMemoryBarriers[idx0].buffer) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                    skipCall |= validate_buffer(commandBuffer, pBufferMemoryBarriers[idx0].buffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, false);
                }
            }
        }
        if (pImageMemoryBarriers) {
// CODEGEN : file ../vk-layer-generate.py line #1055
            for (uint32_t idx1=0; idx1<imageMemoryBarrierCount; ++idx1) {
                if (pImageMemoryBarriers[idx1].image) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                    skipCall |= validate_image(commandBuffer, pImageMemoryBarriers[idx1].image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, false);
                }
            }
        }
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'queryPool']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_query_pool(commandBuffer, queryPool, VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdBeginQuery(commandBuffer, queryPool, query, flags);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'queryPool']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_query_pool(commandBuffer, queryPool, VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdEndQuery(commandBuffer, queryPool, query);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'queryPool']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_query_pool(commandBuffer, queryPool, VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'queryPool']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_query_pool(commandBuffer, queryPool, VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'dstBuffer', 'queryPool']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_buffer(commandBuffer, dstBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_query_pool(commandBuffer, queryPool, VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'layout']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_pipeline_layout(commandBuffer, layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'pRenderPassBegin']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
        if (pRenderPassBegin) {
// CODEGEN : file ../vk-layer-generate.py line #1099
            skipCall |= validate_framebuffer(commandBuffer, pRenderPassBegin->framebuffer, VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
            skipCall |= validate_render_pass(commandBuffer, pRenderPassBegin->renderPass, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, false);
        }
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdNextSubpass(commandBuffer, contents);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdEndRenderPass(VkCommandBuffer commandBuffer)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdEndRenderPass(commandBuffer);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR void VKAPI_CALL CmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['commandBuffer', 'pCommandBuffers[commandBufferCount]']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_command_buffer(commandBuffer, commandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
        if (pCommandBuffers) {
            for (uint32_t idx0=0; idx0<commandBufferCount; ++idx0) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                skipCall |= validate_command_buffer(commandBuffer, pCommandBuffers[idx0], VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, false);
            }
        }
    }
    if (skipCall)
        return;
    get_dispatch_table(object_tracker_device_table_map, commandBuffer)->CmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


// CODEGEN : file ../vk-layer-generate.py line #1251


VKAPI_ATTR void VKAPI_CALL DestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['instance', 'surface']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_instance(instance, instance, VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_surface_khr(instance, surface, VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT, false);
    }
    if (skipCall)
        return;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        destroy_surface_khr(instance, surface);
    }
    get_dispatch_table(object_tracker_instance_table_map, instance)->DestroySurfaceKHR(instance, surface, pAllocator);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['physicalDevice', 'surface']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_physical_device(physicalDevice, physicalDevice, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_surface_khr(physicalDevice, surface, VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_instance_table_map, physicalDevice)->GetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['physicalDevice', 'surface']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_physical_device(physicalDevice, physicalDevice, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_surface_khr(physicalDevice, surface, VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_instance_table_map, physicalDevice)->GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['physicalDevice', 'surface']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_physical_device(physicalDevice, physicalDevice, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_surface_khr(physicalDevice, surface, VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_instance_table_map, physicalDevice)->GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['physicalDevice', 'surface']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_physical_device(physicalDevice, physicalDevice, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_surface_khr(physicalDevice, surface, VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_instance_table_map, physicalDevice)->GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL CreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'pCreateInfo']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
        if (pCreateInfo) {
// CODEGEN : file ../vk-layer-generate.py line #1099
            skipCall |= validate_swapchain_khr(device, pCreateInfo->oldSwapchain, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT, true);
// CODEGEN : file ../vk-layer-generate.py line #1099
            skipCall |= validate_surface_khr(device, pCreateInfo->surface, VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT, false);
        }
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->CreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        if (result == VK_SUCCESS) {
            create_swapchain_khr(device, *pSwapchain, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT);
        }
    }
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216
VKAPI_ATTR void VKAPI_CALL DestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator)
{
    return explicit_DestroySwapchainKHR(device, swapchain, pAllocator);
}

// CODEGEN : file ../vk-layer-generate.py line #1216
VKAPI_ATTR VkResult VKAPI_CALL GetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages)
{
    return explicit_GetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL AcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['device', 'fence', 'semaphore', 'swapchain']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_device(device, device, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, false);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_fence(device, fence, VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, true);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_semaphore(device, semaphore, VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, true);
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_swapchain_khr(device, swapchain, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, device)->AcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


VKAPI_ATTR VkResult VKAPI_CALL QueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['pPresentInfo', 'queue']
        if (pPresentInfo) {
            if (pPresentInfo->pSwapchains) {
                for (uint32_t idx0=0; idx0<pPresentInfo->swapchainCount; ++idx0) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                    skipCall |= validate_swapchain_khr(queue, pPresentInfo->pSwapchains[idx0], VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT, false);
                }
            }
            if (pPresentInfo->pWaitSemaphores) {
                for (uint32_t idx1=0; idx1<pPresentInfo->waitSemaphoreCount; ++idx1) {
// CODEGEN : file ../vk-layer-generate.py line #1091
                    skipCall |= validate_semaphore(queue, pPresentInfo->pWaitSemaphores[idx1], VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, false);
                }
            }
        }
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_queue(queue, queue, VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_device_table_map, queue)->QueuePresentKHR(queue, pPresentInfo);
    return result;
}

// CODEGEN : file ../vk-layer-generate.py line #1216


#ifdef VK_USE_PLATFORM_ANDROID_KHR

VKAPI_ATTR VkResult VKAPI_CALL CreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    bool skipCall = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
// objects to validate: ['instance']
// CODEGEN : file ../vk-layer-generate.py line #1099
        skipCall |= validate_instance(instance, instance, VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT, false);
    }
    if (skipCall)
        return VK_ERROR_VALIDATION_FAILED_EXT;
    VkResult result = get_dispatch_table(object_tracker_instance_table_map, instance)->CreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    {
        std::lock_guard<std::mutex> lock(global_lock);
        if (result == VK_SUCCESS) {
            create_surface_khr(instance, *pSurface, VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT);
        }
    }
    return result;
}

#endif  // VK_USE_PLATFORM_ANDROID_KHR

// CODEGEN : file ../vk-layer-generate.py line #491
static inline PFN_vkVoidFunction intercept_core_device_command(const char *name)
{
    if (!name || name[0] != 'v' || name[1] != 'k')
        return NULL;

    name += 2;
    if (!strcmp(name, "GetDeviceProcAddr"))
        return (PFN_vkVoidFunction) GetDeviceProcAddr;
    if (!strcmp(name, "DestroyDevice"))
        return (PFN_vkVoidFunction) DestroyDevice;
    if (!strcmp(name, "GetDeviceQueue"))
        return (PFN_vkVoidFunction) GetDeviceQueue;
    if (!strcmp(name, "QueueSubmit"))
        return (PFN_vkVoidFunction) QueueSubmit;
    if (!strcmp(name, "QueueWaitIdle"))
        return (PFN_vkVoidFunction) QueueWaitIdle;
    if (!strcmp(name, "DeviceWaitIdle"))
        return (PFN_vkVoidFunction) DeviceWaitIdle;
    if (!strcmp(name, "AllocateMemory"))
        return (PFN_vkVoidFunction) AllocateMemory;
    if (!strcmp(name, "FreeMemory"))
        return (PFN_vkVoidFunction) FreeMemory;
    if (!strcmp(name, "MapMemory"))
        return (PFN_vkVoidFunction) MapMemory;
    if (!strcmp(name, "UnmapMemory"))
        return (PFN_vkVoidFunction) UnmapMemory;
    if (!strcmp(name, "FlushMappedMemoryRanges"))
        return (PFN_vkVoidFunction) FlushMappedMemoryRanges;
    if (!strcmp(name, "InvalidateMappedMemoryRanges"))
        return (PFN_vkVoidFunction) InvalidateMappedMemoryRanges;
    if (!strcmp(name, "GetDeviceMemoryCommitment"))
        return (PFN_vkVoidFunction) GetDeviceMemoryCommitment;
    if (!strcmp(name, "BindBufferMemory"))
        return (PFN_vkVoidFunction) BindBufferMemory;
    if (!strcmp(name, "BindImageMemory"))
        return (PFN_vkVoidFunction) BindImageMemory;
    if (!strcmp(name, "GetBufferMemoryRequirements"))
        return (PFN_vkVoidFunction) GetBufferMemoryRequirements;
    if (!strcmp(name, "GetImageMemoryRequirements"))
        return (PFN_vkVoidFunction) GetImageMemoryRequirements;
    if (!strcmp(name, "GetImageSparseMemoryRequirements"))
        return (PFN_vkVoidFunction) GetImageSparseMemoryRequirements;
    if (!strcmp(name, "QueueBindSparse"))
        return (PFN_vkVoidFunction) QueueBindSparse;
    if (!strcmp(name, "CreateFence"))
        return (PFN_vkVoidFunction) CreateFence;
    if (!strcmp(name, "DestroyFence"))
        return (PFN_vkVoidFunction) DestroyFence;
    if (!strcmp(name, "ResetFences"))
        return (PFN_vkVoidFunction) ResetFences;
    if (!strcmp(name, "GetFenceStatus"))
        return (PFN_vkVoidFunction) GetFenceStatus;
    if (!strcmp(name, "WaitForFences"))
        return (PFN_vkVoidFunction) WaitForFences;
    if (!strcmp(name, "CreateSemaphore"))
        return (PFN_vkVoidFunction) CreateSemaphore;
    if (!strcmp(name, "DestroySemaphore"))
        return (PFN_vkVoidFunction) DestroySemaphore;
    if (!strcmp(name, "CreateEvent"))
        return (PFN_vkVoidFunction) CreateEvent;
    if (!strcmp(name, "DestroyEvent"))
        return (PFN_vkVoidFunction) DestroyEvent;
    if (!strcmp(name, "GetEventStatus"))
        return (PFN_vkVoidFunction) GetEventStatus;
    if (!strcmp(name, "SetEvent"))
        return (PFN_vkVoidFunction) SetEvent;
    if (!strcmp(name, "ResetEvent"))
        return (PFN_vkVoidFunction) ResetEvent;
    if (!strcmp(name, "CreateQueryPool"))
        return (PFN_vkVoidFunction) CreateQueryPool;
    if (!strcmp(name, "DestroyQueryPool"))
        return (PFN_vkVoidFunction) DestroyQueryPool;
    if (!strcmp(name, "GetQueryPoolResults"))
        return (PFN_vkVoidFunction) GetQueryPoolResults;
    if (!strcmp(name, "CreateBuffer"))
        return (PFN_vkVoidFunction) CreateBuffer;
    if (!strcmp(name, "DestroyBuffer"))
        return (PFN_vkVoidFunction) DestroyBuffer;
    if (!strcmp(name, "CreateBufferView"))
        return (PFN_vkVoidFunction) CreateBufferView;
    if (!strcmp(name, "DestroyBufferView"))
        return (PFN_vkVoidFunction) DestroyBufferView;
    if (!strcmp(name, "CreateImage"))
        return (PFN_vkVoidFunction) CreateImage;
    if (!strcmp(name, "DestroyImage"))
        return (PFN_vkVoidFunction) DestroyImage;
    if (!strcmp(name, "GetImageSubresourceLayout"))
        return (PFN_vkVoidFunction) GetImageSubresourceLayout;
    if (!strcmp(name, "CreateImageView"))
        return (PFN_vkVoidFunction) CreateImageView;
    if (!strcmp(name, "DestroyImageView"))
        return (PFN_vkVoidFunction) DestroyImageView;
    if (!strcmp(name, "CreateShaderModule"))
        return (PFN_vkVoidFunction) CreateShaderModule;
    if (!strcmp(name, "DestroyShaderModule"))
        return (PFN_vkVoidFunction) DestroyShaderModule;
    if (!strcmp(name, "CreatePipelineCache"))
        return (PFN_vkVoidFunction) CreatePipelineCache;
    if (!strcmp(name, "DestroyPipelineCache"))
        return (PFN_vkVoidFunction) DestroyPipelineCache;
    if (!strcmp(name, "GetPipelineCacheData"))
        return (PFN_vkVoidFunction) GetPipelineCacheData;
    if (!strcmp(name, "MergePipelineCaches"))
        return (PFN_vkVoidFunction) MergePipelineCaches;
    if (!strcmp(name, "CreateGraphicsPipelines"))
        return (PFN_vkVoidFunction) CreateGraphicsPipelines;
    if (!strcmp(name, "CreateComputePipelines"))
        return (PFN_vkVoidFunction) CreateComputePipelines;
    if (!strcmp(name, "DestroyPipeline"))
        return (PFN_vkVoidFunction) DestroyPipeline;
    if (!strcmp(name, "CreatePipelineLayout"))
        return (PFN_vkVoidFunction) CreatePipelineLayout;
    if (!strcmp(name, "DestroyPipelineLayout"))
        return (PFN_vkVoidFunction) DestroyPipelineLayout;
    if (!strcmp(name, "CreateSampler"))
        return (PFN_vkVoidFunction) CreateSampler;
    if (!strcmp(name, "DestroySampler"))
        return (PFN_vkVoidFunction) DestroySampler;
    if (!strcmp(name, "CreateDescriptorSetLayout"))
        return (PFN_vkVoidFunction) CreateDescriptorSetLayout;
    if (!strcmp(name, "DestroyDescriptorSetLayout"))
        return (PFN_vkVoidFunction) DestroyDescriptorSetLayout;
    if (!strcmp(name, "CreateDescriptorPool"))
        return (PFN_vkVoidFunction) CreateDescriptorPool;
    if (!strcmp(name, "DestroyDescriptorPool"))
        return (PFN_vkVoidFunction) DestroyDescriptorPool;
    if (!strcmp(name, "ResetDescriptorPool"))
        return (PFN_vkVoidFunction) ResetDescriptorPool;
    if (!strcmp(name, "AllocateDescriptorSets"))
        return (PFN_vkVoidFunction) AllocateDescriptorSets;
    if (!strcmp(name, "FreeDescriptorSets"))
        return (PFN_vkVoidFunction) FreeDescriptorSets;
    if (!strcmp(name, "UpdateDescriptorSets"))
        return (PFN_vkVoidFunction) UpdateDescriptorSets;
    if (!strcmp(name, "CreateFramebuffer"))
        return (PFN_vkVoidFunction) CreateFramebuffer;
    if (!strcmp(name, "DestroyFramebuffer"))
        return (PFN_vkVoidFunction) DestroyFramebuffer;
    if (!strcmp(name, "CreateRenderPass"))
        return (PFN_vkVoidFunction) CreateRenderPass;
    if (!strcmp(name, "DestroyRenderPass"))
        return (PFN_vkVoidFunction) DestroyRenderPass;
    if (!strcmp(name, "GetRenderAreaGranularity"))
        return (PFN_vkVoidFunction) GetRenderAreaGranularity;
    if (!strcmp(name, "CreateCommandPool"))
        return (PFN_vkVoidFunction) CreateCommandPool;
    if (!strcmp(name, "DestroyCommandPool"))
        return (PFN_vkVoidFunction) DestroyCommandPool;
    if (!strcmp(name, "ResetCommandPool"))
        return (PFN_vkVoidFunction) ResetCommandPool;
    if (!strcmp(name, "AllocateCommandBuffers"))
        return (PFN_vkVoidFunction) AllocateCommandBuffers;
    if (!strcmp(name, "FreeCommandBuffers"))
        return (PFN_vkVoidFunction) FreeCommandBuffers;
    if (!strcmp(name, "BeginCommandBuffer"))
        return (PFN_vkVoidFunction) BeginCommandBuffer;
    if (!strcmp(name, "EndCommandBuffer"))
        return (PFN_vkVoidFunction) EndCommandBuffer;
    if (!strcmp(name, "ResetCommandBuffer"))
        return (PFN_vkVoidFunction) ResetCommandBuffer;
    if (!strcmp(name, "CmdBindPipeline"))
        return (PFN_vkVoidFunction) CmdBindPipeline;
    if (!strcmp(name, "CmdSetViewport"))
        return (PFN_vkVoidFunction) CmdSetViewport;
    if (!strcmp(name, "CmdSetScissor"))
        return (PFN_vkVoidFunction) CmdSetScissor;
    if (!strcmp(name, "CmdSetLineWidth"))
        return (PFN_vkVoidFunction) CmdSetLineWidth;
    if (!strcmp(name, "CmdSetDepthBias"))
        return (PFN_vkVoidFunction) CmdSetDepthBias;
    if (!strcmp(name, "CmdSetBlendConstants"))
        return (PFN_vkVoidFunction) CmdSetBlendConstants;
    if (!strcmp(name, "CmdSetDepthBounds"))
        return (PFN_vkVoidFunction) CmdSetDepthBounds;
    if (!strcmp(name, "CmdSetStencilCompareMask"))
        return (PFN_vkVoidFunction) CmdSetStencilCompareMask;
    if (!strcmp(name, "CmdSetStencilWriteMask"))
        return (PFN_vkVoidFunction) CmdSetStencilWriteMask;
    if (!strcmp(name, "CmdSetStencilReference"))
        return (PFN_vkVoidFunction) CmdSetStencilReference;
    if (!strcmp(name, "CmdBindDescriptorSets"))
        return (PFN_vkVoidFunction) CmdBindDescriptorSets;
    if (!strcmp(name, "CmdBindIndexBuffer"))
        return (PFN_vkVoidFunction) CmdBindIndexBuffer;
    if (!strcmp(name, "CmdBindVertexBuffers"))
        return (PFN_vkVoidFunction) CmdBindVertexBuffers;
    if (!strcmp(name, "CmdDraw"))
        return (PFN_vkVoidFunction) CmdDraw;
    if (!strcmp(name, "CmdDrawIndexed"))
        return (PFN_vkVoidFunction) CmdDrawIndexed;
    if (!strcmp(name, "CmdDrawIndirect"))
        return (PFN_vkVoidFunction) CmdDrawIndirect;
    if (!strcmp(name, "CmdDrawIndexedIndirect"))
        return (PFN_vkVoidFunction) CmdDrawIndexedIndirect;
    if (!strcmp(name, "CmdDispatch"))
        return (PFN_vkVoidFunction) CmdDispatch;
    if (!strcmp(name, "CmdDispatchIndirect"))
        return (PFN_vkVoidFunction) CmdDispatchIndirect;
    if (!strcmp(name, "CmdCopyBuffer"))
        return (PFN_vkVoidFunction) CmdCopyBuffer;
    if (!strcmp(name, "CmdCopyImage"))
        return (PFN_vkVoidFunction) CmdCopyImage;
    if (!strcmp(name, "CmdBlitImage"))
        return (PFN_vkVoidFunction) CmdBlitImage;
    if (!strcmp(name, "CmdCopyBufferToImage"))
        return (PFN_vkVoidFunction) CmdCopyBufferToImage;
    if (!strcmp(name, "CmdCopyImageToBuffer"))
        return (PFN_vkVoidFunction) CmdCopyImageToBuffer;
    if (!strcmp(name, "CmdUpdateBuffer"))
        return (PFN_vkVoidFunction) CmdUpdateBuffer;
    if (!strcmp(name, "CmdFillBuffer"))
        return (PFN_vkVoidFunction) CmdFillBuffer;
    if (!strcmp(name, "CmdClearColorImage"))
        return (PFN_vkVoidFunction) CmdClearColorImage;
    if (!strcmp(name, "CmdClearDepthStencilImage"))
        return (PFN_vkVoidFunction) CmdClearDepthStencilImage;
    if (!strcmp(name, "CmdClearAttachments"))
        return (PFN_vkVoidFunction) CmdClearAttachments;
    if (!strcmp(name, "CmdResolveImage"))
        return (PFN_vkVoidFunction) CmdResolveImage;
    if (!strcmp(name, "CmdSetEvent"))
        return (PFN_vkVoidFunction) CmdSetEvent;
    if (!strcmp(name, "CmdResetEvent"))
        return (PFN_vkVoidFunction) CmdResetEvent;
    if (!strcmp(name, "CmdWaitEvents"))
        return (PFN_vkVoidFunction) CmdWaitEvents;
    if (!strcmp(name, "CmdPipelineBarrier"))
        return (PFN_vkVoidFunction) CmdPipelineBarrier;
    if (!strcmp(name, "CmdBeginQuery"))
        return (PFN_vkVoidFunction) CmdBeginQuery;
    if (!strcmp(name, "CmdEndQuery"))
        return (PFN_vkVoidFunction) CmdEndQuery;
    if (!strcmp(name, "CmdResetQueryPool"))
        return (PFN_vkVoidFunction) CmdResetQueryPool;
    if (!strcmp(name, "CmdWriteTimestamp"))
        return (PFN_vkVoidFunction) CmdWriteTimestamp;
    if (!strcmp(name, "CmdCopyQueryPoolResults"))
        return (PFN_vkVoidFunction) CmdCopyQueryPoolResults;
    if (!strcmp(name, "CmdPushConstants"))
        return (PFN_vkVoidFunction) CmdPushConstants;
    if (!strcmp(name, "CmdBeginRenderPass"))
        return (PFN_vkVoidFunction) CmdBeginRenderPass;
    if (!strcmp(name, "CmdNextSubpass"))
        return (PFN_vkVoidFunction) CmdNextSubpass;
    if (!strcmp(name, "CmdEndRenderPass"))
        return (PFN_vkVoidFunction) CmdEndRenderPass;
    if (!strcmp(name, "CmdExecuteCommands"))
        return (PFN_vkVoidFunction) CmdExecuteCommands;

    return NULL;
}
static inline PFN_vkVoidFunction intercept_core_instance_command(const char *name)
{
    if (!name || name[0] != 'v' || name[1] != 'k')
        return NULL;

    // we should never be queried for these commands
    assert(strcmp(name, "vkEnumerateInstanceLayerProperties") &&
           strcmp(name, "vkEnumerateInstanceExtensionProperties") &&
           strcmp(name, "vkEnumerateDeviceLayerProperties"));

    name += 2;
    if (!strcmp(name, "CreateInstance"))
        return (PFN_vkVoidFunction) CreateInstance;
    if (!strcmp(name, "DestroyInstance"))
        return (PFN_vkVoidFunction) DestroyInstance;
    if (!strcmp(name, "EnumeratePhysicalDevices"))
        return (PFN_vkVoidFunction) EnumeratePhysicalDevices;
    if (!strcmp(name, "GetPhysicalDeviceFeatures"))
        return (PFN_vkVoidFunction) GetPhysicalDeviceFeatures;
    if (!strcmp(name, "GetPhysicalDeviceFormatProperties"))
        return (PFN_vkVoidFunction) GetPhysicalDeviceFormatProperties;
    if (!strcmp(name, "GetPhysicalDeviceImageFormatProperties"))
        return (PFN_vkVoidFunction) GetPhysicalDeviceImageFormatProperties;
    if (!strcmp(name, "GetPhysicalDeviceProperties"))
        return (PFN_vkVoidFunction) GetPhysicalDeviceProperties;
    if (!strcmp(name, "GetPhysicalDeviceQueueFamilyProperties"))
        return (PFN_vkVoidFunction) GetPhysicalDeviceQueueFamilyProperties;
    if (!strcmp(name, "GetPhysicalDeviceMemoryProperties"))
        return (PFN_vkVoidFunction) GetPhysicalDeviceMemoryProperties;
    if (!strcmp(name, "GetInstanceProcAddr"))
        return (PFN_vkVoidFunction) GetInstanceProcAddr;
    if (!strcmp(name, "CreateDevice"))
        return (PFN_vkVoidFunction) CreateDevice;
    if (!strcmp(name, "GetPhysicalDeviceSparseImageFormatProperties"))
        return (PFN_vkVoidFunction) GetPhysicalDeviceSparseImageFormatProperties;

    return NULL;
}

// CODEGEN : file ../vk-layer-generate.py line #522
// CODEGEN : file ../vk-layer-generate.py line #291
VKAPI_ATTR VkResult VKAPI_CALL CreateDebugReportCallbackEXT(
        VkInstance                                   instance,
        const VkDebugReportCallbackCreateInfoEXT*    pCreateInfo,
        const VkAllocationCallbacks*                 pAllocator,
        VkDebugReportCallbackEXT*                    pCallback)
{
    VkLayerInstanceDispatchTable *pInstanceTable = get_dispatch_table(object_tracker_instance_table_map, instance);
    VkResult result = pInstanceTable->CreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    if (VK_SUCCESS == result) {
        layer_data *my_data = get_my_data_ptr(get_dispatch_key(instance), layer_data_map);
        result = layer_create_msg_callback(my_data->report_data,
                                           pCreateInfo,
                                           pAllocator,
                                           pCallback);
    }
    return result;
}
// CODEGEN : file ../vk-layer-generate.py line #322
VKAPI_ATTR void VKAPI_CALL DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT msgCallback, const VkAllocationCallbacks *pAllocator)
{
    VkLayerInstanceDispatchTable *pInstanceTable = get_dispatch_table(object_tracker_instance_table_map, instance);
    pInstanceTable->DestroyDebugReportCallbackEXT(instance, msgCallback, pAllocator);
    layer_data *my_data = get_my_data_ptr(get_dispatch_key(instance), layer_data_map);
    layer_destroy_msg_callback(my_data->report_data, msgCallback, pAllocator);
}
// CODEGEN : file ../vk-layer-generate.py line #338
VKAPI_ATTR void VKAPI_CALL DebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT    flags, VkDebugReportObjectTypeEXT objType, uint64_t object, size_t location, int32_t msgCode, const char *pLayerPrefix, const char *pMsg)
{
    VkLayerInstanceDispatchTable *pInstanceTable = get_dispatch_table(object_tracker_instance_table_map, instance);
    pInstanceTable->DebugReportMessageEXT(instance, flags, objType, object, location, msgCode, pLayerPrefix, pMsg);
}

// CODEGEN : file ../vk-layer-generate.py line #535
static inline PFN_vkVoidFunction intercept_wsi_enabled_command(const char *name, VkDevice dev)
{
    if (dev) {
        layer_data *my_data = get_my_data_ptr(get_dispatch_key(dev), layer_data_map);
        if (!my_data->wsi_enabled)
            return nullptr;
    }

    if (!strcmp("vkCreateSwapchainKHR", name))
        return reinterpret_cast<PFN_vkVoidFunction>(CreateSwapchainKHR);
    if (!strcmp("vkDestroySwapchainKHR", name))
        return reinterpret_cast<PFN_vkVoidFunction>(DestroySwapchainKHR);
    if (!strcmp("vkGetSwapchainImagesKHR", name))
        return reinterpret_cast<PFN_vkVoidFunction>(GetSwapchainImagesKHR);
    if (!strcmp("vkAcquireNextImageKHR", name))
        return reinterpret_cast<PFN_vkVoidFunction>(AcquireNextImageKHR);
    if (!strcmp("vkQueuePresentKHR", name))
        return reinterpret_cast<PFN_vkVoidFunction>(QueuePresentKHR);

    return nullptr;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetDeviceProcAddr(VkDevice device, const char* funcName)
{
    PFN_vkVoidFunction addr;
    addr = intercept_core_device_command(funcName);
    if (addr)
        return addr;
    assert(device);

    addr = intercept_wsi_enabled_command(funcName, device);
    if (addr)
        return addr;

    if (get_dispatch_table(object_tracker_device_table_map, device)->GetDeviceProcAddr == NULL)
        return NULL;
    return get_dispatch_table(object_tracker_device_table_map, device)->GetDeviceProcAddr(device, funcName);
}

// CODEGEN : file ../vk-layer-generate.py line #567
static inline PFN_vkVoidFunction intercept_msg_callback_get_proc_addr_command(const char *name, VkInstance instance)
{
    layer_data *my_data = get_my_data_ptr(get_dispatch_key(instance), layer_data_map);
    return debug_report_get_instance_proc_addr(my_data->report_data, name);
}

// CODEGEN : file ../vk-layer-generate.py line #567
static inline PFN_vkVoidFunction intercept_wsi_enabled_command(const char *name, VkInstance instance)
{
    VkLayerInstanceDispatchTable* pTable = get_dispatch_table(object_tracker_instance_table_map, instance);
    if (instanceExtMap.size() == 0 || !instanceExtMap[pTable].wsi_enabled)
        return nullptr;

    if (!strcmp("vkDestroySurfaceKHR", name))
        return reinterpret_cast<PFN_vkVoidFunction>(DestroySurfaceKHR);
    if (!strcmp("vkGetPhysicalDeviceSurfaceSupportKHR", name))
        return reinterpret_cast<PFN_vkVoidFunction>(GetPhysicalDeviceSurfaceSupportKHR);
    if (!strcmp("vkGetPhysicalDeviceSurfaceCapabilitiesKHR", name))
        return reinterpret_cast<PFN_vkVoidFunction>(GetPhysicalDeviceSurfaceCapabilitiesKHR);
    if (!strcmp("vkGetPhysicalDeviceSurfaceFormatsKHR", name))
        return reinterpret_cast<PFN_vkVoidFunction>(GetPhysicalDeviceSurfaceFormatsKHR);
    if (!strcmp("vkGetPhysicalDeviceSurfacePresentModesKHR", name))
        return reinterpret_cast<PFN_vkVoidFunction>(GetPhysicalDeviceSurfacePresentModesKHR);
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    if (!strcmp("vkCreateAndroidSurfaceKHR", name))
        return reinterpret_cast<PFN_vkVoidFunction>(CreateAndroidSurfaceKHR);
#endif  // VK_USE_PLATFORM_ANDROID_KHR

    return nullptr;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetInstanceProcAddr(VkInstance instance, const char* funcName)
{
    PFN_vkVoidFunction addr;
    addr = intercept_core_instance_command(funcName);
    if (!addr) {
        addr = intercept_core_device_command(funcName);
    }
    if (!addr) {
        addr = intercept_wsi_enabled_command(funcName, VkDevice(VK_NULL_HANDLE));
    }
    if (addr) {
        return addr;
    }
    assert(instance);

    addr = intercept_msg_callback_get_proc_addr_command(funcName, instance);
    if (addr)
        return addr;

    addr = intercept_wsi_enabled_command(funcName, instance);
    if (addr)
        return addr;

    if (get_dispatch_table(object_tracker_instance_table_map, instance)->GetInstanceProcAddr == NULL) {
        return NULL;
    }
    return get_dispatch_table(object_tracker_instance_table_map, instance)->GetInstanceProcAddr(instance, funcName);
}


} // namespace object_tracker

// CODEGEN : file ../vk-layer-generate.py line #352
// vk_layer_logging.h expects these to be defined

VKAPI_ATTR VkResult VKAPI_CALL
vkCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                               const VkAllocationCallbacks *pAllocator, VkDebugReportCallbackEXT *pMsgCallback) {
    return object_tracker::CreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pMsgCallback);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(VkInstance instance,
                                                                           VkDebugReportCallbackEXT msgCallback,
                                                                           const VkAllocationCallbacks *pAllocator) {
    object_tracker::DestroyDebugReportCallbackEXT(instance, msgCallback, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL
vkDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t object,
                        size_t location, int32_t msgCode, const char *pLayerPrefix, const char *pMsg) {
    object_tracker::DebugReportMessageEXT(instance, flags, objType, object, location, msgCode, pLayerPrefix, pMsg);
}

// CODEGEN : file ../vk-layer-generate.py line #377
// loader-layer interface v0

static const VkExtensionProperties instance_extensions[] = {
    {
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
        VK_EXT_DEBUG_REPORT_SPEC_VERSION
    }
};

static const VkLayerProperties globalLayerProps = {
    "VK_LAYER_LUNARG_object_tracker",
    VK_LAYER_API_VERSION, // specVersion
    1, // implementationVersion
    "LunarG Validation Layer"
};

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pCount,  VkExtensionProperties* pProperties)
{
    return util_GetExtensionProperties(1, instance_extensions, pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceLayerProperties(uint32_t *pCount,  VkLayerProperties* pProperties)
{
    return util_GetLayerProperties(1, &globalLayerProps, pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t *pCount, VkLayerProperties* pProperties)
{
    return util_GetLayerProperties(1, &globalLayerProps, pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice dev, const char *funcName)
{
    return object_tracker::GetDeviceProcAddr(dev, funcName);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char *funcName)
{
    if (!strcmp(funcName, "vkEnumerateInstanceLayerProperties"))
        return reinterpret_cast<PFN_vkVoidFunction>(vkEnumerateInstanceLayerProperties);
    if (!strcmp(funcName, "vkEnumerateDeviceLayerProperties"))
        return reinterpret_cast<PFN_vkVoidFunction>(vkEnumerateDeviceLayerProperties);
    if (!strcmp(funcName, "vkEnumerateInstanceExtensionProperties"))
        return reinterpret_cast<PFN_vkVoidFunction>(vkEnumerateInstanceExtensionProperties);
    if (!strcmp(funcName, "vkGetInstanceProcAddr"))
        return reinterpret_cast<PFN_vkVoidFunction>(vkGetInstanceProcAddr);

    return object_tracker::GetInstanceProcAddr(instance, funcName);
}
