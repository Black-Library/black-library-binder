/**
 * BlackLibraryBinder.cc
 */

#include <algorithm>
#include <iostream>
#include <fstream>

#include <FileOperations.h>

#include <BlackLibraryBinder.h>

namespace black_library {

namespace core {

namespace binder {

#define BIND_FILENAME_BUFFER_SIZE 128

namespace BlackLibraryCommon = black_library::core::common;

BlackLibraryBinder::BlackLibraryBinder(const std::string &storage_dir) :
    bind_dir_(storage_dir),
    storage_dir_(storage_dir),
    mutex_()
{
    if (storage_dir_.empty())
    {
        storage_dir_ = "/mnt/black-library/store";
        std::cout << "Empty storage dir given, using default: " << storage_dir_ << std::endl;
    }

    // okay to pop_back(), string isn't empty
    if (storage_dir_.back() == '/')
        storage_dir_.pop_back();

    if (!BlackLibraryCommon::CheckFilePermission(storage_dir_))
    {
        std::cout << "Error: binder could not access storage directory" << storage_dir_ << std::endl;
        return;
    }

    std::cout << "Using storage dir: " << storage_dir_ << std::endl;
}

bool BlackLibraryBinder::Bind(const std::string &uuid, const std::string &name)
{
    const std::lock_guard<std::mutex> lock(mutex_);

    std::string target_path = storage_dir_ + '/' + uuid;
    std::cout << "Bind target: " << target_path << std::endl;

    if (!BlackLibraryCommon::FileExists(target_path))
    {
        std::cout << "Error: binder target does not exist: " << target_path << std::endl;
        return false;
    }

    auto doc_list = BlackLibraryCommon::GetFileList(target_path, "^SEC?\\d*");

    std::sort(doc_list.begin(), doc_list.end());

    // for (const auto & file : doc_list)
    // {
    //     std::cout << "Section file: " + file + " - Number: " << BlackLibraryCommon::GetSectionIndex(file) << std::endl;
    // }

    size_t last_section_num = BlackLibraryCommon::GetSectionIndex(doc_list.back());

    // document files start at 1
    if (last_section_num != doc_list.size())
    {
        std::cout << "Error: section number " << last_section_num << " and list size: " << doc_list.size() << " do not match" << std::endl;
        return false;
    }

    auto bind_list = BlackLibraryCommon::GetFileList(target_path, "_VER[0-9]{4}.\\html$");

    std::sort(bind_list.begin(), bind_list.end());

    for (const auto & file : bind_list)
    {
        std::cout << "Bind file: " + file + " - " << BlackLibraryCommon::GetBindIndex(file) << std::endl;
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

    std::cout << "Binding: " << bind_name << std::endl;

    if (BlackLibraryCommon::FileExists(bind_target))
    {
        std::cout << "Error: file already exists" << std::endl;
        return false;
    }

    std::fstream output_file;

    output_file.open(bind_target, std::fstream::out | std::fstream::trunc);

    for (const auto & file : doc_list)
    {
        std::ifstream input_file;

        // std::cout << "bind: " << file << std::endl;

        input_file.open(target_path + '/' + file, std::fstream::in);

        if (!input_file.is_open())
        {
            std::cout << "Error: input file: " << target_path + '/' + file << " could not be opened" << std::endl;
            return false;
        }

        output_file << input_file.rdbuf();

        input_file.close();
    }

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
        std::cout << "Empty bind dir given, using default: " << bind_dir_ << std::endl;
    }

    // okay to pop_back(), string isn't empty
    if (bind_dir_.back() == '/')
        bind_dir_.pop_back();

    if (!BlackLibraryCommon::CheckFilePermission(bind_dir_))
    {
        std::cout << "Error: binder could not access bind directory" << bind_dir_ << std::endl;
        return false;
    }

    std::cout << "Set bind dir: " << bind_dir_ << std::endl;

    return true;
}

} // namespace binder
} // namespace core
} // namespace black_library