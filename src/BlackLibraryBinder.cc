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

BlackLibraryBinder::BlackLibraryBinder(const std::string &storage_dir) :
    storage_dir_(storage_dir)
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

bool BlackLibraryBinder::Bind(const std::string &uuid)
{
    std::string target_path = storage_dir_ + '/' + uuid;
    std::cout << "Bind target: " << target_path << std::endl;

    if (!black_library::core::common::Exists(target_path))
    {
        std::cout << "Error: binder target does not exist: " << target_path << std::endl;
        return false;
    }

    auto file_list = black_library::core::common::GetFileList(target_path);

    std::sort(file_list.begin(), file_list.end());

    for (const auto & file : file_list)
    {
        std::cout << file << std::endl;
    }

    return true;
}

} // namespace binder
} // namespace core
} // namespace black_library