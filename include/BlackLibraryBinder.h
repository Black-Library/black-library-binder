/**
 * BlackLibraryBinder.h
 */

#ifndef __BLACK_LIBRARY_CORE_BINDER_BLACKLIBRARYBINDER_H__
#define __BLACK_LIBRARY_CORE_BINDER_BLACKLIBRARYBINDER_H__

#include <mutex>
#include <string>

namespace black_library {

namespace core {

namespace binder {

class BlackLibraryBinder {
public:
    explicit BlackLibraryBinder(const std::string &storage_dir);
    BlackLibraryBinder &operator = (BlackLibraryBinder &&) = default;

    bool Bind(const std::string &uuid, const std::string &name);
    bool SetBindDir(const std::string &bind_dir);

private:
    std::string bind_dir_;
    std::string storage_dir_;
    std::mutex mutex_;
};

} // namespace binder
} // namespace core
} // namespace black_library

#endif
