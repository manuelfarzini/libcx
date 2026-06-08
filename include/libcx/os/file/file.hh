/** @file cx/os/file/file.hh **/

#ifndef CX_OS_FILE_FILE_HH
#define CX_OS_FILE_FILE_HH

#include "libcx/conf/macro.hh"
#include "libcx/uti/utilities.hh"
#include "libcx/mem/allocator.hh"

namespace cx {
inline namespace os {
inline namespace file {

////////////////////////////////////////////
// Definitions

using FileMode = u32;
onedef cons FileMode FileMode_Read      = BIT<0>;
onedef cons FileMode FileMode_Write     = BIT<1>;
onedef cons FileMode FileMode_Append    = BIT<2>;
onedef cons FileMode FileMode_ReadWrite = BIT<3>;
onedef cons FileMode FileMode_Mask      = FileMode_Read | FileMode_Write | FileMode_Append | FileMode_ReadWrite;

struct EnumName {
    using Under = T;
    onedef glob cons Under EnumName_First = enum_beg;
    onedef glob cons Under EnumName_Second = enum_beg + 1;
    onedef glob cons Under EnumName_Third = enum_beg + 2;
    onedef glob cons Under EnumName_Fourth = enum_beg + 3;
    onedef glob cons Under EnumName_Fifth = enum_beg + 4;
    onedef glob cons Under EnumName_Sixth = enum_beg + 5;
    onedef glob cons Under EnumName_Seventh = enum_beg + 6;
    Under val;
    EnumName() = delete;
    cons EnumName(Under val) : val(val) {}
    cons operator Under() const { return val; }
    cons bool operator==(EnumName rhs) const { return val == rhs.val; }
    cons bool operator<=>(EnumName rhs) const { return val <=> rhs.val; }

    cons string to_string() const 
    {
        switch (val) {
            case EnumName_Invalid: return "Invalid";
            case EnumName_Filename: return "Filename";
            case EnumName_Exists: return "Exists";
            case EnumName_DontExists: return "DontExists";
            case EnumName_Permission: return "Permission";
            case EnumName_TruncFail: return "TruncFail";
            default: return "Invalid";
        }
    }
};

enum FileWhence {
    FileWhence_Begin,
    FileWhence_Current,
    FileWhence_End,
};

enum FileError {
    FileError_None,
    FileError_Invalid,
    FileError_Filename,
    FileError_Exists,
    FileError_DontExists,
    FileError_Permission,
    FileError_TruncFail,
};

enum FileStandardType {
    FileStandard_Input,
    FileStandard_Output,
    FileStandard_Error,

    FileStandard_Count,
};

union FileDescriptor {
    mutaptr p;
    iptr i;
    uptr u;
};

struct FileOperations;

#define CX_FILE_OPEN(fun_name) \
  func fun_name(FileDescriptor* fd, FileMode mode, cstring name) noexce -> FileError
#define CX_FILE_READ_AT(fun_name) \
  func fun_name(FileDescriptor fd, mutaptr buf, isize size, i64 off, isize* read) noexce -> i32
#define CX_FILE_WRITE_AT(fun_name) \
  func fun_name(FileDescriptor fd, readptr buf, isize size, i64 off, isize* written) noexce -> i32
#define CX_FILE_SEEK(fun_name) \
  func fun_name(FileDescriptor fd, i64 cur_off, FileWhence whence, i64* new_off) noexce -> i32
#define CX_FILE_CLOSE(fun_name) \
  func fun_name(FileDescriptor fd) noexce -> void

typedef CX_FILE_READ_AT(FileReadProc);
typedef CX_FILE_WRITE_AT(FileWriteProc);
typedef CX_FILE_SEEK(FileSeekProc);
typedef CX_FILE_CLOSE(FileCloseProc);

// TODO(manu): If File must also cover stream-like outputs, split sequential
// read/write from read_at/write_at, so sockets and pipes do not depend on seek.
struct FileOperations {
    FileReadProc*     read_at;
    FileWriteProc*    write_at;
    FileSeekProc*     seek;
    FileCloseProc*    close;
};

using FileTime = u64;

struct File {
    FileOperations ops;
    FileDescriptor fd;
    char const* name;
    FileTime mtime;
};

// Platform specific definitions.

#if CX_SYSTEM_WIN

priv fn _win_file_alloc_wide(cstring text) noexce -> wchar_t*
{
    if (text == null) {
        return null;
    }

    isize len = strlen(text); // TODO: custom strlen!!!
    if (len <= 0) {
        return null;
    }

    i32 wlen = MultiByteToWideChar(
        CP_UTF8, MB_ERR_INVALID_CHARS, text, int(len), null, 0
    );
    if (wlen == 0) {
        return null;
    }

    auto [ptr, err] = aligned_alloc(
        heap_allocator(), (wlen + 1) * size_of(wchar_t), align_of(wchar_t)
    );
    if (err) {
        return null;
    }

    wchar_t* wtext = cast(wchar_t*, ptr);

    i32 res = MultiByteToWideChar(
        CP_UTF8, MB_ERR_INVALID_CHARS, text, int(len), wtext, wlen
    );
    if (res == 0) {
        aligned_free(heap_allocator(), wtext);
        return null;
    }

    wtext[wlen] = 0;
    return wtext;
}

priv onedef CX_FILE_SEEK(_win_file_seek)
{
    LARGE_INTEGER li_off;
    li_off.QuadPart = cur_off;

    LARGE_INTEGER li_new;
    li_new.QuadPart = 0;

    if (!SetFilePointerEx(fd.p, li_off, &li_new, whence)) {
        return false;
    }

    if (new_off) {
        *new_off = li_new.QuadPart;
    }

    return true;
}

priv CX_FILE_READ_AT(_win_file_read)
{
    if (buf == null || size < 0) {
        return false;
    }

    DWORD size_ = DWORD(size > I32_MAX ? I32_MAX : size);
    DWORD read_ = 0;

    if (!_win_file_seek(fd, off, FileWhence_Begin, null)) {
        return false;
    }

    if (!ReadFile(fd.p, buf, size_, &read_, null)) {
        return false;
    }

    if (read) {
        *read = read_;
    }

    return true;
}

priv CX_FILE_WRITE_AT(_win_file_write)
{
    if (buf == null || size < 0) {
        return false;
    }

    DWORD size_ = DWORD(size > I32_MAX ? I32_MAX : size);
    DWORD written_ = 0;

    if (!_win_file_seek(fd, off, FileWhence_Begin, null)) {
        return false;
    }

    if (!WriteFile(fd.p, buf, size_, &written_, null)) {
        return false;
    }

    if (written) {
        *written = written_;
    }

    return true;
}

priv CX_FILE_CLOSE(_win_file_close)
{
    CloseHandle(fd.p);
}

priv noinln CX_FILE_OPEN(_win_file_open)
{
    if (fd == null || name == null) {
        return FileError_Invalid;
    }

    DWORD desired_access;
    DWORD creation_disposition;

    switch (mode & FileMode_Mask) {
    case FileMode_Read:
        desired_access = GENERIC_READ;
        creation_disposition = OPEN_EXISTING;
        break;
    case FileMode_Write:
        desired_access = GENERIC_WRITE;
        creation_disposition = CREATE_ALWAYS;
        break;
    case FileMode_Append:
        desired_access = GENERIC_WRITE;
        creation_disposition = OPEN_ALWAYS;
        break;
    case FileMode_Read | FileMode_ReadWrite:
        desired_access = GENERIC_READ | GENERIC_WRITE;
        creation_disposition = OPEN_EXISTING;
        break;
    case FileMode_Write | FileMode_ReadWrite:
        desired_access = GENERIC_READ | GENERIC_WRITE;
        creation_disposition = CREATE_ALWAYS;
        break;
    case FileMode_Append | FileMode_ReadWrite:
        desired_access = GENERIC_READ | GENERIC_WRITE;
        creation_disposition = OPEN_ALWAYS;
        break;
    default:
        // cx_panic("invalid file mode"); // TODO:
        return FileError_Invalid;
    }

    wchar_t* wname = _win_file_alloc_wide(name);
    if (wname == null) {
        return FileError_Filename;
    }

    fd->p = CreateFileW(
        wname,
        desired_access,
        FILE_SHARE_READ | FILE_SHARE_DELETE,
        null,
        creation_disposition,
        FILE_ATTRIBUTE_NORMAL,
        null
    );

    aligned_free(heap_allocator(), wname);

    if (fd->p == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();

        switch (err) {
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            return FileError_DontExists;
        case ERROR_FILE_EXISTS:
        case ERROR_ALREADY_EXISTS:
            return FileError_Exists;
        case ERROR_ACCESS_DENIED:
            return FileError_Permission;
        default:
            return FileError_Invalid;
        }
    }

    if (mode & FileMode_Append) {
        LARGE_INTEGER off;
        off.QuadPart = 0;

        if (!SetFilePointerEx(fd->p, off, null, FILE_END)) {
            CloseHandle(fd->p);
            fd->p = INVALID_HANDLE_VALUE;
            return FileError_Invalid;
        }
    }

    return FileError_None;
}

priv cons fn default_ops() noexce -> FileOperations
{
    return FileOperations{
        .read_at  = _win_file_read,
        .write_at = _win_file_write,
        .seek     = _win_file_seek,
        .close    = _win_file_close,
    };
}

priv fn file_standard_default(FileStandardType type) noexce -> File
{
    FileDescriptor fd{};

    switch (type) {
    case FileStandard_Input:
        fd.p = GetStdHandle(STD_INPUT_HANDLE);
        break;
    case FileStandard_Output:
        fd.p = GetStdHandle(STD_OUTPUT_HANDLE);
        break;
    case FileStandard_Error:
        fd.p = GetStdHandle(STD_ERROR_HANDLE);
        break;
    default:
        fd.p = INVALID_HANDLE_VALUE;
        break;
    }

    return File{
        .ops   = default_ops(),
        .fd    = fd,
        .name  = null,
        .mtime = {},
    };
}

priv File _file_standard[FileStandard_Count] = {
    file_standard_default(FileStandard_Input),
    file_standard_default(FileStandard_Output),
    file_standard_default(FileStandard_Error),
};

priv fn file_set_standard_to_default() noexce -> void
{
    _file_standard[FileStandard_Input] = file_standard_default(FileStandard_Input);
    _file_standard[FileStandard_Output] = file_standard_default(FileStandard_Output);
    _file_standard[FileStandard_Error] = file_standard_default(FileStandard_Error);
}

inln fn file_last_write_time(cstring fpath) noexce -> FileTime
{
    if (fpath == null) {
        return 0;
    }

    wchar_t* wpath = _win_file_alloc_wide(fpath);
    if (wpath == null) {
        return 0;
    }

    WIN32_FILE_ATTRIBUTE_DATA data{};
    b32 ok = GetFileAttributesExW(wpath, GetFileExInfoStandard, &data);

    aligned_free(heap_allocator(), wpath);

    if (!ok) {
        return 0;
    }

    ULARGE_INTEGER li;
    li.LowPart = data.ftLastWriteTime.dwLowDateTime;
    li.HighPart = data.ftLastWriteTime.dwHighDateTime;

    return li.QuadPart;
}

#else  // POSIX

priv onedef CX_FILE_SEEK(_posix_file_seek)
{
#if CX_SYSTEM_OSX
    i64 res = lseek(fd.i, cur_off, whence);
#else  // LINUX
    i64 res = lseek64(fd.i, cur_off, whence);
#endif

    if (res < 0) {
        return false;
    }
    if (new_off) {
        *new_off = res;
    }
    return true;
}

priv CX_FILE_READ_AT(_posix_file_read)
{
    isize res = pread(fd.i, buf, size, off);
    if (res < 0) {
        return false;
    }
    if (read) {
        *read = res;
    }
    return true;
}

priv CX_FILE_WRITE_AT(_posix_file_write)
{
    isize res;
    i64 cur_off = 0;

    _posix_file_seek(fd, 0, FileWhence_Current, &cur_off);

    if (cur_off == off) {
        res = write(int(fd.i), buf, size);
    } else {
        res = pwrite(fd.i, buf, size, off);
    }

    if (res < 0) {
        return false;
    }
    if (written) {
        *written = res;
    }
    return true;
}

priv CX_FILE_CLOSE(_posix_file_close)
{
    close(fd.i);
}

priv noinln CX_FILE_OPEN(_posix_file_open)
{
    if (fd == null || name == null) {
        return FileError_Invalid;
    }

    i32 os_mode;
    switch (mode & FileMode_Mask) {
    case FileMode_Read:
        os_mode = O_RDONLY;
        break;
    case FileMode_Write:
        os_mode = O_WRONLY | O_CREAT | O_TRUNC;
        break;
    case FileMode_Append:
        os_mode = O_WRONLY | O_CREAT | O_APPEND;
        break;
    case FileMode_Read | FileMode_ReadWrite:
        os_mode = O_RDWR;
        break;
    case FileMode_Write | FileMode_ReadWrite:
        os_mode = O_RDWR | O_CREAT | O_TRUNC;
        break;
    case FileMode_Append | FileMode_ReadWrite:
        os_mode = O_RDWR | O_APPEND | O_CREAT;
        break;
    default:
        // cx_panic("invalid file mode"); // TODO:
        return FileError_Invalid;
    }

    fd->i = open(name, os_mode, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (fd->i < 0) {
        return FileError_Invalid;
    }

    return FileError_None;
}

priv cons fn default_ops() noexce -> FileOperations
{
    return FileOperations{
        .read_at  = _posix_file_read,
        .write_at = _posix_file_write,
        .seek     = _posix_file_seek,
        .close    = _posix_file_close,
    };
}

priv fn file_standard_default(FileStandardType type) noexce -> File
{
    FileDescriptor fd{};

    switch (type) {
    case FileStandard_Input:
        fd.i = 0;
        break;
    case FileStandard_Output:
        fd.i = 1;
        break;
    case FileStandard_Error:
        fd.i = 2;
        break;
    default:
        fd.i = -1;
        break;
    }

    return File{
        .ops   = default_ops(),
        .fd    = fd,
        .name  = null,
        .mtime = {},
    };
}

priv File _file_standard[FileStandard_Count] = {
    file_standard_default(FileStandard_Input),
    file_standard_default(FileStandard_Output),
    file_standard_default(FileStandard_Error),
};

priv fn file_set_standard_to_default() noexce -> void
{
    _file_standard[FileStandard_Input] = file_standard_default(FileStandard_Input);
    _file_standard[FileStandard_Output] = file_standard_default(FileStandard_Output);
    _file_standard[FileStandard_Error] = file_standard_default(FileStandard_Error);
}

inln fn file_last_write_time(cstring fpath) noexce -> FileTime
{
    time_t res = 0;
    struct stat file_stat;

    if (fpath != null && stat(fpath, &file_stat) == 0) {
        res = file_stat.st_mtime;
    }

    return res;
}

#endif

inln priv fn file_standard() noexce -> File*
{
    return _file_standard;
}

inln fn file_standard(FileStandardType type) noexce -> File*
{
    return &_file_standard[type];
}

fn file_new(File* file, FileDescriptor fd, FileOperations ops, cstring name) noexce -> FileError
{
    if (file == null || name == null) {
        return FileError_Invalid;
    }

    isize len = strlen(name); // TODO: custom strlen!!!

    auto [ptr, err] = aligned_alloc(heap_allocator(), (len + 1) * size_of(char));
    if (err) {
        return FileError_Invalid;
    }

    mem_move(ptr, name, len);
    cast(char*, ptr)[len] = '\0';

    file->ops = ops;
    file->fd = fd;
    file->name = cstring(ptr);
    file->mtime = file_last_write_time(name);

    return FileError_None;
}

fn file_open_mode(File* file, FileMode mode, cstring name) noexce -> FileError
{
    if (file == null || name == null) {
        return FileError_Invalid;
    }

    FileError err = FileError_None;

#if CX_SYSTEM_WIN
    err = _win_file_open(&file->fd, mode, name);
#else
    err = _posix_file_open(&file->fd, mode, name);
#endif

    if (err != FileError_None) {
        return err;
    }

    err = file_new(file, file->fd, default_ops(), name);
    if (err != FileError_None) {
        file->ops.close(file->fd);
        return err;
    }

    return FileError_None;
}

fn cx_file_close(File* file) noexce -> FileError
{
    if (file == null) {
        return FileError_Invalid;
    }

    if (file->name != null) like {
        aligned_free(heap_allocator(), cast(char*, file->name));
        file->name = null;
    }

#if CX_SYSTEM_WIN
    if (file->fd.p == INVALID_HANDLE_VALUE) {
        return FileError_Invalid;
    }
#else
    if (file->fd.i < 0) {
        return FileError_Invalid;
    }
#endif

    file->ops.close(file->fd);

#if CX_SYSTEM_WIN
    file->fd.p = INVALID_HANDLE_VALUE;
#else
    file->fd.i = -1;
#endif

    return FileError_None;
}

inln fn file_read_at_check(
    File* file, mutaptr buf, isize size, i64 off, isize* read
) noexce -> b32 {
    if (file == null) {
        return false;
    }
    return file->ops.read_at(file->fd, buf, size, off, read);
}

inln fn file_write_at_check(
    File* file, readptr buf, isize size, i64 off, isize* written
) noexce -> b32 {
    if (file == null) {
        return false;
    }
    return file->ops.write_at(file->fd, buf, size, off, written);
}

inln fn file_read_at(File* file, mutaptr buf, isize size, i64 off) noexce -> b32
{
    return file_read_at_check(file, buf, size, off, null);
}

inln fn file_write_at(File* file, readptr buf, isize size, i64 off) noexce -> b32
{
    return file_write_at_check(file, buf, size, off, null);
}

inln fn file_seek(File* file, i64 off) noexce -> i64
{
    if (file == null) {
        return -1;
    }

    i64 new_off = 0;
    if (!file->ops.seek(file->fd, off, FileWhence_Begin, &new_off)) {
        return -1;
    }

    return new_off;
}

inln fn file_seek_end(File* file) noexce -> i64
{
    if (file == null) {
        return -1;
    }

    i64 new_off = 0;
    if (!file->ops.seek(file->fd, 0, FileWhence_End, &new_off)) {
        return -1;
    }

    return new_off;
}

inln fn file_seek_skip(File* file, i64 bytes) noexce -> i64
{
    if (file == null) {
        return -1;
    }

    i64 new_off = 0;
    if (!file->ops.seek(file->fd, bytes, FileWhence_Current, &new_off)) {
        return -1;
    }

    return new_off;
}

inln fn file_tell(File* file) noexce -> i64
{
    if (file == null) {
        return -1;
    }

    i64 new_off = 0;
    if (!file->ops.seek(file->fd, 0, FileWhence_Current, &new_off)) {
        return -1;
    }

    return new_off;
}

inln fn file_read(File* file, mutaptr buf, isize size) noexce -> b32
{
    i64 off = file_tell(file);
    if (off < 0) {
        return false;
    }

    return file_read_at(file, buf, size, off);
}

inln fn file_write(File* file, readptr buf, isize size) noexce -> b32
{
    i64 off = file_tell(file);
    if (off < 0) {
        return false;
    }

    return file_write_at(file, buf, size, off);
}

inln fn file_create(File* file, cstring name) noexce -> FileError
{
    return file_open_mode(file, FileMode_Write | FileMode_ReadWrite, name);
}

inln fn file_open(File* file, cstring name) noexce -> FileError
{
    return file_open_mode(file, FileMode_Read, name);
}

inln fn file_name(File* file) noexce -> cstring
{
    if (file == null) {
        return "";
    }

    return file->name ? file->name : "";
}

inln fn file_has_changed(File* file) noexce -> b32
{
    if (file == null || file->name == null) {
        return false;
    }

    FileTime last_write = file_last_write_time(file->name);
    if (file->mtime != last_write) {
        file->mtime = last_write;
        return true;
    }

    return false;
}

}       // namespace file
}       // namespace os
}       // namespace cx

#endif  // CX_OS_FILE_FILE_HH
