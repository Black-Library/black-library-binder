/**
 * BlackLibraryBinder.cc
 */

#include <algorithm>
#include <iostream>

#include <FileOperations.h>

#include <BlackLibraryBinder.h>

namespace black_library {

namespace core {

namespace binder {

#define BIND_FILENAME_BUFFER_SIZE 128

BlackLibraryBinder::BlackLibraryBinder(const std::string &storage_dir) :
    storage_dir_(storage_dir),
    mutex_()
{
    if (storage_dir_.empty())
    {
        storage_dir_ = "/mnt/store";
        std::cout << "Empty storage dir given, using default: " << storage_dir_ << std::endl;
    }

    // okay to pop_back(), string isn't empty
    if (storage_dir_.back() == '/')
        storage_dir_.pop_back();

    if (!black_library::core::common::CheckFilePermission(storage_dir_))
    {
        std::cout << "Error: binder could not access storage directory" << std::endl;
        return;
    }

    std::cout << "Using storage dir: " << storage_dir_ << std::endl;
}

bool BlackLibraryBinder::Bind(const std::string &uuid, const std::string &name)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    std::string target_path = storage_dir_ + '/' + uuid;
    std::cout << "Bind target: " << target_path << std::endl;

    if (!black_library::core::common::Exists(target_path))
    {
        std::cout << "Error: binder target does not exist: " << target_path << std::endl;
        return false;
    }

    auto file_list = black_library::core::common::GetFileList(target_path, "^CH?\\d*");

    std::sort(file_list.begin(), file_list.end());

    for (const auto & file : file_list)
    {
        std::cout << "Chapter file: " + file + " - Number: " << black_library::core::common::GetChapterIndex(file) << std::endl;
    }

    size_t last_chapter_num = black_library::core::common::GetChapterIndex(file_list.back());

    if (last_chapter_num != file_list.size())
    {
        std::cout << "Error: chapter number " << last_chapter_num << " and list size: " << file_list.size() << " do not match" << std::endl;
        return false;
    }

    auto bind_list = black_library::core::common::GetFileList(target_path, "_VER[0-9]{4}.\\html$");

    std::sort(bind_list.begin(), bind_list.end());

    for (const auto & file : bind_list)
    {
        std::cout << "Bind file: " + file + " - " << black_library::core::common::GetChapterIndex(file) << std::endl;
    }

    size_t bind_index = 0;

    if (!bind_list.empty())
        bind_index = black_library::core::common::GetBindIndex(bind_list.back()) + 1;

    char name_buffer [BIND_FILENAME_BUFFER_SIZE];

    int res = snprintf(name_buffer, BIND_FILENAME_BUFFER_SIZE, "%s_VER%04lu.html", name.c_str(), bind_index);
    if (res < 0)
        return false;

    const std::string file_name = std::string(name_buffer);

    std::cout << "Adding: " << file_name << std::endl;

    return true;
}

} // namespace binder
} // namespace core
} // namespace black_library