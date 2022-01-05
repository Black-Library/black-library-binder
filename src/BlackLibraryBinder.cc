/**
 * BlackLibraryBinder.cc
 */

#include <algorithm>
#include <iostream>
#include <fstream>

#include <FileOperations.h>
#include <LogOperations.h>

#include <BlackLibraryBinder.h>

namespace black_library {

namespace core {

namespace binder {

#define BIND_FILENAME_BUFFER_SIZE 128

namespace BlackLibraryCommon = black_library::core::common;

BlackLibraryBinder::BlackLibraryBinder(const njson &config) :
    bind_dir_(BlackLibraryCommon::DefaultStoragePath),
    storage_path_(BlackLibraryCommon::DefaultStoragePath),
    mutex_()
{
    njson nconfig = config["config"];

    std::string logger_path = BlackLibraryCommon::DefaultLogPath;
    bool logger_level = BlackLibraryCommon::DefaultLogLevel;

    if (nconfig.contains("logger_path"))
    {
        logger_path = nconfig["logger_path"];
    }

    if (nconfig.contains("binder_debug_log"))
    {
        logger_level = nconfig["binder_debug_log"];
    }

    BlackLibraryCommon::InitRotatingLogger("binder", logger_path, logger_level);

    if (nconfig.contains("storage_path"))
    {
        storage_path_ = nconfig["storage_path"];
    }

    if (storage_path_.empty())
    {
        storage_path_ = BlackLibraryCommon::DefaultStoragePath;
        BlackLibraryCommon::LogDebug("binder", "Empty storage dir given, using default: {}", storage_path_);
    }

    if (!BlackLibraryCommon::CheckFilePermission(storage_path_))
    {
        BlackLibraryCommon::LogError("binder", "Could not access storage directory: {}", storage_path_);
        return;
    }

    BlackLibraryCommon::LogInfo("binder", "Binder using storage dir: {}", storage_path_);
}

bool BlackLibraryBinder::Bind(const std::string &uuid, const std::string &name)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    std::string target_path = storage_path_ + uuid;
    BlackLibraryCommon::LogInfo("binder", "Bind target: {}", target_path);

    if (!BlackLibraryCommon::FileExists(target_path))
    {
        BlackLibraryCommon::LogError("binder", "Binder target does not exist: {}", target_path);
        return false;
    }

    auto doc_list = BlackLibraryCommon::GetFileList(target_path, "^SEC?\\d*");

    std::sort(doc_list.begin(), doc_list.end());

    for (const auto & file : doc_list)
    {
        BlackLibraryCommon::LogTrace("binder", "Section file: {} - Number: {}", file, BlackLibraryCommon::GetSectionIndex(file));
    }

    size_t last_section_num = BlackLibraryCommon::GetSectionIndex(doc_list.back());

    // document files start at 1
    if (last_section_num != doc_list.size())
    {
        BlackLibraryCommon::LogError("binder", "Section number {} - list size: {} do not match", last_section_num, doc_list.size());
        return false;
    }

    auto bind_list = BlackLibraryCommon::GetFileList(target_path, "_VER[0-9]{4}.\\html$");

    std::sort(bind_list.begin(), bind_list.end());

    for (const auto & file : bind_list)
    {
        BlackLibraryCommon::LogDebug("binder", "Bind file: {} - {}", file, BlackLibraryCommon::GetBindIndex(file));
    }

    size_t bind_index = 0;

    if (!bind_list.empty())
        bind_index = BlackLibraryCommon::GetBindIndex(bind_list.back()) + 1;

    char name_buffer [BIND_FILENAME_BUFFER_SIZE];

    int res = snprintf(name_buffer, BIND_FILENAME_BUFFER_SIZE, "%s_VER%04lu.html", name.c_str(), bind_index);
    if (res < 0)
        return false;

    const std::string bind_name = std::string(name_buffer);
    const std::string bind_target = bind_dir_ + '/' + bind_name;

    BlackLibraryCommon::LogInfo("binder", "Binding: {}", bind_name);

    if (BlackLibraryCommon::FileExists(bind_target))
    {
        BlackLibraryCommon::LogError("binder", "File {} already exists", bind_target);
        return false;
    }

    std::fstream output_file;

    output_file.open(bind_target, std::fstream::out | std::fstream::trunc);

    output_file << "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"><style></style></head><body>";

    for (const auto & file : doc_list)
    {
        std::ifstream input_file;

        BlackLibraryCommon::LogTrace("binder", "Bind: {}", file);

        const auto complete_file_path = target_path + '/' + file;

        input_file.open(complete_file_path, std::fstream::in);

        if (!input_file.is_open())
        {
            BlackLibraryCommon::LogError("binder", "Input file: {} failed to open", complete_file_path);
            return false;
        }

        output_file << input_file.rdbuf();

        input_file.close();
    }

    output_file << "</body></html>";

    output_file.close();

    return true;
}

bool BlackLibraryBinder::SetBindDir(const std::string &bind_dir)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    bind_dir_ = BlackLibraryCommon::SanitizeFilePath(bind_dir);

    if (bind_dir_.empty())
    {
        bind_dir_ = "/mnt/black-library/output";
        BlackLibraryCommon::LogDebug("binder", "Empty bind_dir given, using default: {}", bind_dir_);
    }

    // okay to pop_back(), string isn't empty
    if (bind_dir_.back() == '/')
        bind_dir_.pop_back();

    if (!BlackLibraryCommon::CheckFilePermission(bind_dir_))
    {
        BlackLibraryCommon::LogError("binder", "Binder could not accses bind directory: {}", bind_dir_);
        return false;
    }

    BlackLibraryCommon::LogInfo("binder", "Set bind_dir: {}", bind_dir_);

    return true;
}

} // namespace binder
} // namespace core
} // namespace black_library