/* ============================================================================
 *  EarlnuxOS - Virtual Filesystem (VFS) Core
 * kernel/fs/vfs.c
 * ============================================================================ */

#include <fs/fs.h>
#include <kernel/kernel.h>
#include <lib/string.h>
#include <mm/mm.h>

/* Filesystem type list */
static fs_type_t *fs_list = NULL;

/* Mount points */
static mountpoint_t *mount_list = NULL;

/* File descriptor table */
static file_t *fd_table[FD_MAX] = {0};

/* ==========================================================================
 * Register a filesystem type
 * ========================================================================== */
int vfs_register_fs(fs_type_t *type) {
    if (!type || !type->name) return -1;
    type->next = fs_list;
    fs_list = type;
    KINFO("vfs", "Registered filesystem: %s", type->name);
    return 0;
}

/* ==========================================================================
 * Find the longest matching mount point for a given path
 * Returns mountpoint and sets 'remainder' to path after mount prefix
 * ========================================================================== */
static mountpoint_t *find_mount(const char *path, const char **remainder) {
    mountpoint_t *best = NULL;
    size_t best_len = 0;

    for (mountpoint_t *m = mount_list; m; m = m->next) {
        size_t mlen = strlen(m->path);
        if (strncmp(path, m->path, mlen) == 0) {
            /* If this mount is longer than current best, use it */
            if (mlen > best_len) {
                best = m;
                best_len = mlen;
            }
        }
    }

    if (best) {
        *remainder = path + best_len;
        /* Skip leading '/' */
        while (**remainder == '/') (*remainder)++;
    } else {
        *remainder = path;
    }
    return best;
}

/* ==========================================================================
 * Mount a filesystem
 * ========================================================================== */
int vfs_mount(const char *source, const char *target,
              const char *fstype, uint32_t flags) {
    if (!target || !fstype) return -1;

    /* Find filesystem type */
    fs_type_t *fs = NULL;
    for (fs_type_t *f = fs_list; f; f = f->next) {
        if (strcmp(f->name, fstype) == 0) { fs = f; break; }
    }
    if (!fs) {
        KERROR("vfs", "Unknown filesystem: %s", fstype);
        return -1;
    }

    /* Call filesystem's mount to get root inode */
    inode_t *root = fs->mount(fs, source, flags);
    if (!root) {
        KERROR("vfs", "Filesystem %s mount failed", fstype);
        return -1;
    }

    /* Allocate and link mountpoint */
    mountpoint_t *mnt = (mountpoint_t *)kmalloc(sizeof(mountpoint_t));
    if (!mnt) return -1;
    memset(mnt, 0, sizeof(mountpoint_t));
    strlcpy(mnt->path, target, PATH_MAX);
    mnt->root = root;
    mnt->fs = fs;
    mnt->flags = flags;
    mnt->next = mount_list;
    mount_list = mnt;

    KINFO("vfs", "Mounted %s at %s", fstype, target);
    return 0;
}

/* ==========================================================================
 * Unmount (stub)
 * ========================================================================== */
int vfs_umount(const char *target) {
    /* Not implemented for this simple kernel */
    return -1;
}

/* ==========================================================================
 * Open a file
 * ========================================================================== */
int vfs_open(const char *path, int flags, uint16_t mode) {
    if (!path) return -1;

    const char *remainder;
    mountpoint_t *mnt = find_mount(path, &remainder);
    if (!mnt) return -1;

    if (!mnt->fs || !mnt->fs->open) return -1;

    file_t *file = (file_t *)kmalloc(sizeof(file_t));
    if (!file) return -1;
    memset(file, 0, sizeof(file_t));
    file->flags = flags;
    file->ref_count = 1;

    int fd = -1;
    for (int i = 0; i < FD_MAX; i++) {
        if (!fd_table[i]) { fd = i; fd_table[i] = file; break; }
    }
    if (fd < 0) { kfree(file); return -1; }

    if (mnt->fs->open(mnt, remainder, flags, mode, file) < 0) {
        fd_table[fd] = NULL;
        kfree(file);
        return -1;
    }
    return fd;
}

int vfs_close(int fd) {
    if (fd < 0 || fd >= FD_MAX || !fd_table[fd]) return -1;
    file_t *file = fd_table[fd];
    if (file->ops && file->ops->close) file->ops->close(file);
    kfree(file);
    fd_table[fd] = NULL;
    return 0;
}

ssize_t vfs_read(int fd, void *buf, size_t len) {
    if (fd < 0 || fd >= FD_MAX || !fd_table[fd]) return -1;
    file_t *file = fd_table[fd];
    if (!file->ops || !file->ops->read) return -1;
    return file->ops->read(file, buf, len);
}

ssize_t vfs_write(int fd, const void *buf, size_t len) {
    if (fd < 0 || fd >= FD_MAX || !fd_table[fd]) return -1;
    file_t *file = fd_table[fd];
    if (!file->ops || !file->ops->write) return -1;
    return file->ops->write(file, buf, len);
}

int64_t vfs_seek(int fd, int64_t offset, int whence) {
    if (fd < 0 || fd >= FD_MAX || !fd_table[fd]) return -1;
    file_t *file = fd_table[fd];
    switch (whence) {
        case SEEK_SET: file->pos = offset; break;
        case SEEK_CUR: file->pos += offset; break;
        case SEEK_END: /* would need inode size */ break;
        default: return -1;
    }
    return (int64_t)file->pos;
}

/* ==========================================================================
 * Directory operations
 * ========================================================================== */
int vfs_readdir(int fd, dirent_t *buf, uint32_t count) {
    if (fd < 0 || fd >= FD_MAX || !fd_table[fd]) return -1;
    file_t *file = fd_table[fd];
    if (!(file->flags & O_DIRECTORY)) return -1;
    if (!file->ops || !file->ops->readdir) return -1;
    return file->ops->readdir(file, buf, count);
}

/* ==========================================================================
 * mkdir - create directory
 * ========================================================================== */
int vfs_mkdir(const char *path, uint16_t mode) {
    if (!path) return -1;
    const char *remainder;
    mountpoint_t *mnt = find_mount(path, &remainder);
    if (!mnt) return -1;
    if (!mnt->fs || !mnt->fs->mkdir) return -1;
    return mnt->fs->mkdir(mnt, remainder, mode);
}

/* ==========================================================================
 * unlink - remove a file
 * ========================================================================== */
int vfs_unlink(const char *path) {
    if (!path) return -1;
    const char *remainder;
    mountpoint_t *mnt = find_mount(path, &remainder);
    if (!mnt) return -1;
    if (!mnt->fs || !mnt->fs->unlink) return -1;
    return mnt->fs->unlink(mnt, remainder);
}

/* ==========================================================================
 * VFS init
 * ========================================================================== */
void vfs_init(void) {
    memset(fd_table, 0, sizeof(fd_table));
    /* Standard fds (stdin/out/err) can be opened to console later */
    KINFO("vfs", "VFS initialized");
}

/* Path canonicalize stub */
int path_canonicalize(const char *in, char *out, size_t len) {
    strlcpy(out, in, len);
    return 0;
}
int path_dirname(const char *path, char *out, size_t len) {
    const char *p = strrchr(path, '/');
    if (!p) { out[0] = '/\0'; return 0; }
    size_t n = p - path;
    if (n >= len) n = len - 1;
    memcpy(out, path, n);
    out[n] = '\0';
    return 0;
}
int path_basename(const char *path, char *out, size_t len) {
    const char *p = strrchr(path, '/');
    if (!p) p = path; else p++;
    strlcpy(out, p, len);
    return 0;
}
bool path_is_absolute(const char *path) {
    return path[0] == '/';
}

/* Stubs */
int vfs_rmdir(const char *path) { return -1; }
int vfs_rename(const char *old, const char *newpath) { return -1; }
int vfs_symlink(const char *target, const char *linkpath) { return -1; }
int vfs_readlink(const char *path, char *buf, size_t len) { return -1; }
int vfs_chmod(const char *path, uint16_t mode) { return -1; }
int vfs_chown(const char *path, uint32_t uid, uint32_t gid) { return -1; }
int vfs_stat(const char *path, stat_t *st) { return -1; }
int vfs_fstat(int fd, stat_t *st) { return -1; }
int vfs_ioctl(int fd, uint32_t cmd, void *arg) { return -1; }
int vfs_mmap(int fd, uint32_t virt, size_t len, uint32_t flags) { return -1; }
int vfs_fsync(int fd) { return -1; }
int vfs_ftruncate(int fd, uint64_t size) { return -1; }
