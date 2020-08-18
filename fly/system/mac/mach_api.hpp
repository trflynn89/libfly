#pragma once

#include <mach/mach.h>

namespace fly::detail {

/**
 * Fetch basic information about this macOS host. Uses the following structures and methods:
 *
 *     host_basic_info_data_t:
 *     https://opensource.apple.com/source/xnu/xnu-6153.81.5/osfmk/mach/host_info.h.auto.
 *
 *     HOST_BASIC_INFO:
 *     https://opensource.apple.com/source/xnu/xnu-6153.81.5/osfmk/mach/host_info.h.auto.html
 *
 *     ::host_info():
 *     https://opensource.apple.com/source/xnu/xnu-6153.81.5/osfmk/mach/mach_host.defs.auto.html
 *
 * @param host_info Reference to the structure to hold the fetched data.
 *
 * @return True if the data could be fetched.
 */
bool host_basic_info(host_basic_info_data_t &host_info);

/**
 * Fetch CPU load statistics about this macOS host. Uses the following structures and methods:
 *
 *     host_cpu_load_info_data_t:
 *     https://opensource.apple.com/source/xnu/xnu-6153.81.5/osfmk/mach/host_info.h.auto.html
 *
 *     HOST_CPU_LOAD_INFO:
 *     https://opensource.apple.com/source/xnu/xnu-6153.81.5/osfmk/mach/host_info.h.auto.html
 *
 *     ::host_statistics():
 *     https://opensource.apple.com/source/xnu/xnu-6153.81.5/osfmk/mach/mach_host.defs.auto.html
 *
 * @param cpu_load Reference to the structure to hold the fetched data.
 *
 * @return True if the data could be fetched.
 */
bool host_cpu_load(host_cpu_load_info_data_t &cpu_load);

/**
 * Fetch the page size for this macOS host. Uses the following structures and methods:
 *
 *     vm_size_t:
 *     https://opensource.apple.com/source/xnu/xnu-6153.81.5/osfmk/mach/vm_types.h.auto.html
 *
 *     ::host_page_size():
 *     https://opensource.apple.com/source/xnu/xnu-6153.81.5/libsyscall/mach/mach/mach_init.h.auto.html
 *
 * @param page_size Reference to the structure to hold the fetched data.
 *
 * @return True if the data could be fetched.
 */
bool host_page_size(vm_size_t &page_size);

/**
 * Fetch virtual memory statistics about this macOS host. Uses the following structures and methods:
 *
 *     vm_statistics64_data_t:
 *     https://opensource.apple.com/source/xnu/xnu-6153.81.5/osfmk/mach/vm_statistics.h.auto.html
 *
 *     HOST_VM_INFO64:
 *     https://opensource.apple.com/source/xnu/xnu-6153.81.5/osfmk/mach/host_info.h.auto.html
 *
 *     ::host_statistics64():
 *     https://opensource.apple.com/source/xnu/xnu-6153.81.5/osfmk/mach/mach_host.defs.auto.html
 *
 * @param vm_info Reference to the structure to hold the fetched data.
 *
 * @return True if the data could be fetched.
 */
bool host_vm_info(vm_statistics64_data_t &vm_info);

/**
 * Fetch basic information about this macOS process. Uses the following structures and methods:
 *
 *     task_basic_info_64_data_t:
 *     https://opensource.apple.com/source/xnu/xnu-6153.81.5/osfmk/mach/task_info.h.auto.html
 *
 *     TASK_BASIC_INFO_64:
 *     https://opensource.apple.com/source/xnu/xnu-6153.81.5/osfmk/mach/task_info.h.auto.html
 *
 *     ::task_info():
 *     https://opensource.apple.com/source/xnu/xnu-6153.81.5/osfmk/mach/task.defs.auto.html
 *
 * @param task_info Reference to the structure to hold the fetched data.
 *
 * @return True if the data could be fetched.
 */
bool task_basic_info(task_basic_info_64_data_t &task_info);

/**
 * Fetch live thread time information about this macOS process. Uses the following structures and
 * methods:
 *
 *     task_thread_times_info_data_t:
 *     https://opensource.apple.com/source/xnu/xnu-6153.81.5/osfmk/mach/task_info.h.auto.html
 *
 *     TASK_THREAD_TIMES_INFO:
 *     https://opensource.apple.com/source/xnu/xnu-6153.81.5/osfmk/mach/task_info.h.auto.html
 *
 *     ::task_info():
 *     https://opensource.apple.com/source/xnu/xnu-6153.81.5/osfmk/mach/task.defs.auto.html
 *
 * @param thread_times Reference to the structure to hold the fetched data.
 *
 * @return True if the data could be fetched.
 */
bool task_thread_times(task_thread_times_info_data_t &thread_times);

} // namespace fly::detail
