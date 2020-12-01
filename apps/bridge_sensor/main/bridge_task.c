#include "bridge.h"

/*
static const char *TAG = "BRIDGE_TASK";

static size_t prepopulated_num = 0;


static heap_task_totals_t   totals_arr[BRIDGE_MAX_TASK_NUM];
static heap_task_block_t    block_arr[BRIDGE_MAX_BLOCK_NUM];

void bridge_dump_per_task_heap_info(void) {
    ESP_LOGV(TAG, "bridge_dump_per_task_heap_info");


    heap_task_info_params_t heap_info = {0};
    heap_info.caps[0] = MALLOC_CAP_8BIT;        // Gets heap with CAP_8BIT capabilities
    heap_info.mask[0] = MALLOC_CAP_8BIT;
    heap_info.caps[1] = MALLOC_CAP_32BIT;       // Gets heap info with CAP_32BIT capabilities
    heap_info.mask[1] = MALLOC_CAP_32BIT;
    heap_info.tasks = NULL;                     // Passing NULL captures heap info for all tasks
    heap_info.num_tasks = 0;
    heap_info.totals = totals_arr;            // Gets task wise allocation details
    heap_info.num_totals = &prepopulated_num;
    heap_info.max_totals = BRIDGE_MAX_TASK_NUM;        // Maximum length of "s_totals_arr"
    heap_info.blocks = block_arr;             // Gets block wise allocation details. For each block, gets owner task, address and size
    heap_info.max_blocks = BRIDGE_MAX_BLOCK_NUM;       // Maximum length of "s_block_arr"

    heap_caps_get_per_task_info(&heap_info);

    for (int i = 0 ; i < *heap_info.num_totals; i++) {
        ESP_LOGW(TAG, "Task: %s -> CAP_8BIT: %d CAP_32BIT: %d",
                heap_info.totals[i].task ? pcTaskGetTaskName(heap_info.totals[i].task) : "Pre-Scheduler allocs" ,
                heap_info.totals[i].size[0],    // Heap size with CAP_8BIT capabilities
                heap_info.totals[i].size[1]);   // Heap size with CAP32_BIT capabilities
    }
    ESP_LOGW(TAG, "-----------------------------------------------------");
}
*/