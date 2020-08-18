#include "fly/system/mac/mach_api.hpp"

#include "fly/logger/logger.hpp"

namespace fly::detail {

namespace {

    bool check_kernel_status(const char *caller, const kern_return_t &status)
    {
        if (status != KERN_SUCCESS)
        {
            LOGW("Kernel error gethering %s (%d): %s", caller, status, ::mach_error_string(status));
            return false;
        }

        return true;
    }

} // namespace

//==================================================================================================
bool host_basic_info(host_basic_info_data_t &basic_info)
{
    mach_msg_type_number_t count = HOST_BASIC_INFO_COUNT;

    const kern_return_t status = ::host_info(
        mach_host_self(),
        HOST_BASIC_INFO,
        reinterpret_cast<host_info_t>(&basic_info),
        &count);

    return check_kernel_status(__FUNCTION__, status);
}

//==================================================================================================
bool host_cpu_load(host_cpu_load_info_data_t &cpu_load)
{
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;

    const kern_return_t status = ::host_statistics(
        mach_host_self(),
        HOST_CPU_LOAD_INFO,
        reinterpret_cast<host_info_t>(&cpu_load),
        &count);

    return check_kernel_status(__FUNCTION__, status);
}

//==================================================================================================
bool host_page_size(vm_size_t &page_size)
{
    const kern_return_t status = ::host_page_size(mach_host_self(), &page_size);
    return check_kernel_status(__FUNCTION__, status);
}

//==================================================================================================
bool host_vm_info(vm_statistics64_data_t &vm_info)
{
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;

    const kern_return_t status = ::host_statistics64(
        mach_host_self(),
        HOST_VM_INFO64,
        reinterpret_cast<host_info64_t>(&vm_info),
        &count);

    return check_kernel_status(__FUNCTION__, status);
}

//==================================================================================================
bool task_basic_info(task_basic_info_64_data_t &task_info)
{
    mach_msg_type_number_t count = TASK_BASIC_INFO_64_COUNT;

    const kern_return_t status = ::task_info(
        mach_task_self(),
        TASK_BASIC_INFO_64,
        reinterpret_cast<task_info_t>(&task_info),
        &count);

    return check_kernel_status(__FUNCTION__, status);
}

//==================================================================================================
bool task_thread_times(task_thread_times_info_data_t &thread_times)
{
    mach_msg_type_number_t count = TASK_THREAD_TIMES_INFO_COUNT;

    const kern_return_t status = ::task_info(
        mach_task_self(),
        TASK_THREAD_TIMES_INFO,
        reinterpret_cast<task_info_t>(&thread_times),
        &count);

    return check_kernel_status(__FUNCTION__, status);
}

} // namespace fly::detail
