#include "fly/path/path_monitor.hpp"

#include "fly/logger/logger.hpp"
#include "fly/path/path_config.hpp"
#include "fly/task/task_runner.hpp"

#include <system_error>

namespace fly {

//==================================================================================================
PathMonitor::PathMonitor(
    const std::shared_ptr<SequencedTaskRunner> &task_runner,
    const std::shared_ptr<PathConfig> &config) noexcept :
    m_task_runner(task_runner),
    m_config(config)
{
}

//==================================================================================================
PathMonitor::~PathMonitor()
{
    remove_all_paths();
}

//==================================================================================================
bool PathMonitor::start()
{
    return poll_paths_later();
}

//==================================================================================================
bool PathMonitor::add_path(const std::filesystem::path &path, PathEventCallback callback)
{
    std::error_code error;

    if (callback == nullptr)
    {
        LOGW("Ignoring null callback for %s", path);
    }
    else if (!std::filesystem::is_directory(path, error))
    {
        LOGW("Ignoring non-directory %s: %s", path, error);
    }
    else
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        PathInfo *info = get_or_create_path_info(path);

        if (info != nullptr)
        {
            LOGD("Monitoring all files in %s", path);
            info->m_path_handler = callback;

            return true;
        }
    }

    return false;
}

//==================================================================================================
bool PathMonitor::remove_path(const std::filesystem::path &path)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_path_info.find(path);

    if (it == m_path_info.end())
    {
        LOGW("Wasn't monitoring %s", path);
        return false;
    }

    LOGI("Removed monitor for %s", path);
    m_path_info.erase(it);

    return true;
}

//==================================================================================================
void PathMonitor::remove_all_paths()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    LOGI("Removed all monitors");
    m_path_info.clear();
}

//==================================================================================================
bool PathMonitor::add_file(const std::filesystem::path &file, PathEventCallback callback)
{
    std::error_code error;

    if (callback == nullptr)
    {
        LOGW("Ignoring null callback for %s", file);
    }
    else if (std::filesystem::is_directory(file, error))
    {
        LOGW("Ignoring directory %s: %s", file, error);
    }
    else if (!std::filesystem::is_directory(file.parent_path(), error))
    {
        LOGW("Ignoring file under non-directory %s: %s", file, error);
    }
    else
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        PathInfo *info = get_or_create_path_info(file.parent_path());

        if (info != nullptr)
        {
            LOGD("Monitoring file %s", file);
            info->m_file_handlers[file.filename()] = callback;

            return true;
        }
    }

    return false;
}

//==================================================================================================
bool PathMonitor::remove_file(const std::filesystem::path &file)
{
    bool prune_path = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto path_it = m_path_info.find(file.parent_path());

        if (path_it == m_path_info.end())
        {
            LOGW("Wasn't monitoring %s", file);
            return false;
        }

        PathInfo *info = path_it->second.get();
        auto file_it = info->m_file_handlers.find(file.filename());

        if (file_it == info->m_file_handlers.end())
        {
            LOGW("Wasn't monitoring %s", file);
            return false;
        }

        LOGD("Stopped monitoring %s", file);
        info->m_file_handlers.erase(file_it);

        prune_path = info->m_file_handlers.empty() && !(info->m_path_handler);
    }

    return prune_path ? remove_path(file.parent_path()) : true;
}

//==================================================================================================
PathMonitor::PathInfo *PathMonitor::get_or_create_path_info(const std::filesystem::path &path)
{
    PathInfo *info = nullptr;

    auto it = m_path_info.find(path);

    if (it == m_path_info.end())
    {
        std::unique_ptr<PathInfo> created_info = create_path_info(path);

        if (created_info && created_info->is_valid())
        {
            info = created_info.get();
            m_path_info[path] = std::move(created_info);
        }
    }
    else
    {
        info = it->second.get();
    }

    return info;
}

//==================================================================================================
bool PathMonitor::poll_paths_later()
{
    if (!is_valid())
    {
        return false;
    }

    auto task = [](std::shared_ptr<PathMonitor> self) {
        self->poll(self->m_config->poll_interval());
        self->poll_paths_later();
    };

    std::weak_ptr<PathMonitor> weak_self = shared_from_this();
    return m_task_runner->post_task(FROM_HERE, std::move(task), std::move(weak_self));
}

//==================================================================================================
std::ostream &operator<<(std::ostream &stream, PathMonitor::PathEvent event)
{
    switch (event)
    {
        case PathMonitor::PathEvent::None:
            stream << "None";
            break;

        case PathMonitor::PathEvent::Created:
            stream << "Created";
            break;

        case PathMonitor::PathEvent::Deleted:
            stream << "Deleted";
            break;

        case PathMonitor::PathEvent::Changed:
            stream << "Changed";
            break;
    }

    return stream;
}

} // namespace fly
