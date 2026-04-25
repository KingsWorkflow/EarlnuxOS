/* ============================================================================
 *  EarlnuxOS - Virtual Filesystem (VFS) Header
 * include/fs/fs.h
 * ============================================================================ */

#ifndef  EarlnuxOS_FS_H
#define  EarlnuxOS_FS_H

#include <types.h>

/* Forward declarations */
typedef struct mountpoint mountpoint_t;
typedef struct file file_t;

/* ============================================================================
 * File types and flags
 * ============================================================================ */
#define S_IFMT    0xF000  /* File type mask */
#define S_IFREG   0x8000  /* Regular file */
#define S_IFDIR   0x4000  /* Directory */
#define S_IFLNK   0xA000  /* Symbolic link */
#define S_IFCHR   0x2000  /* Character device */
#define S_IFBLK   0x6000  /* Block device */
#define S_IFIFO   0x1000  /* FIFO/pipe */
#define S_IFSOCK  0xC000  /* Socket */

#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)
#define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)  (((m) & S_IFMT) == S_IFBLK)

/* Permission bits */
#define S_IRUSR   0400  /* Owner read */
#define S_IWUSR   0200  /* Owner write */
#define S_IXUSR   0100  /* Owner exec */
#define S_IRGRP   0040
#define S_IWGRP   0020
#define S_IXGRP   0010
#define S_IROTH   0004
#define S_IWOTH   0002
#define S_IXOTH   0001
#define S_IRWXU   (S_IRUSR|S_IWUSR|S_IXUSR)
#define S_IRWXG   (S_IRGRP|S_IWGRP|S_IXGRP)
#define S_IRWXO   (S_IROTH|S_IWOTH|S_IXOTH)

/* Open flags */
#define O_RDONLY  0x0000
#define O_WRONLY  0x0001
#define O_RDWR    0x0002
#define O_CREAT   0x0040
#define O_EXCL    0x0080
#define O_TRUNC   0x0200
#define O_APPEND  0x0400
#define O_NONBLOCK 0x0800
#define O_DIRECTORY 0x10000

/* Seek origins */
#define SEEK_SET  0
#define SEEK_CUR  1
#define SEEK_END  2

/* Max values */
#define PATH_MAX  1024
#define NAME_MAX  255
#define FD_MAX    1024

/* ============================================================================
 * VFS Inode
 * ============================================================================ */
typedef struct inode {
    uint32_t    ino;            /* Inode number */
    uint16_t    mode;           /* File type & permissions */
    uint32_t    uid, gid;
    uint64_t    size;           /* File size in bytes */
    uint32_t    atime, mtime, ctime;
    uint32_t    nlinks;         /* Hard link count */
    uint32_t    nblocks;        /* Number of allocated blocks */
    uint32_t    block_size;
    uint32_t    ref_count;      /* In-memory ref count */
    bool        dirty;
    struct fs_type  *fs;        /* Owning filesystem */
    struct inode_ops *ops;
    void        *fs_data;       /* FS-specific data */
    struct inode *hash_next;    /* Inode hash table linkage */
} inode_t;

/* Inode operations */
typedef struct inode_ops {
    inode_t  *(*lookup)(inode_t *dir, const char *name);
    int       (*create)(inode_t *dir, const char *name, uint16_t mode, inode_t **out);
    int       (*mkdir)(inode_t *dir, const char *name, uint16_t mode);
    int       (*unlink)(inode_t *dir, const char *name);
    int       (*rmdir)(inode_t *dir, const char *name);
    int       (*rename)(inode_t *old_dir, const char *old_name,
                        inode_t *new_dir, const char *new_name);
    int       (*symlink)(inode_t *dir, const char *name, const char *target);
    int       (*readlink)(inode_t *node, char *buf, size_t len);
    int       (*chmod)(inode_t *node, uint16_t mode);
    int       (*chown)(inode_t *node, uint32_t uid, uint32_t gid);
    int       (*truncate)(inode_t *node, uint64_t size);
    void      (*sync)(inode_t *node);
    void      (*destroy)(inode_t *node);
} inode_ops_t;

/* ============================================================================
 * VFS File (open file descriptor)
 * ============================================================================ */
typedef struct file {
    inode_t     *inode;
    uint64_t     pos;       /* Current position */
    int          flags;     /* O_RDONLY / O_WRONLY / etc. */
    uint32_t     ref_count;
    struct file_ops *ops;
    void        *private;   /* Driver/FS private data */
} file_t;

/* File operations */
typedef struct file_ops {
    int     (*open)(file_t *file, inode_t *inode, int flags);
    void    (*close)(file_t *file);
    ssize_t (*read)(file_t *file, void *buf, size_t len);
    ssize_t (*write)(file_t *file, const void *buf, size_t len);
    int64_t (*seek)(file_t *file, int64_t offset, int whence);
    int     (*ioctl)(file_t *file, uint32_t cmd, void *arg);
    int     (*readdir)(file_t *file, void *buf, size_t buf_size);
    int     (*mmap)(file_t *file, uint32_t virt, size_t len, uint32_t flags);
    int     (*fsync)(file_t *file);
    int     (*ftruncate)(file_t *file, uint64_t size);
} file_ops_t;

/* Directory entry (readdir result) */
typedef struct dirent {
    uint32_t d_ino;
    uint32_t d_off;
    uint16_t d_reclen;
    uint8_t  d_type;
    char     d_name[NAME_MAX + 1];
} dirent_t;

/* stat structure */
typedef struct stat {
    uint32_t  st_dev;
    uint32_t  st_ino;
    uint16_t  st_mode;
    uint32_t  st_nlink;
    uint32_t  st_uid;
    uint32_t  st_gid;
    uint64_t  st_size;
    uint32_t  st_blksize;
    uint32_t  st_blocks;
    uint32_t  st_atime;
    uint32_t  st_mtime;
    uint32_t  st_ctime;
} stat_t;

/* ============================================================================
 * VFS Mount / Filesystem type
 * ============================================================================ */
typedef struct fs_type {
    const char *name;
    /* Mount: read superblock from block device or create in-memory */
    inode_t *(*mount)(struct fs_type *type, const char *source, uint32_t flags);
    void     (*umount)(struct fs_type *type);
    int      (*sync)(struct fs_type *type);
    void     (*statfs)(struct fs_type *type, uint64_t *total, uint64_t *free_b);
    /* Path-based operations */
    int      (*open)(mountpoint_t *mnt, const char *path, int flags, uint16_t mode, file_t *file);
    int      (*mkdir)(mountpoint_t *mnt, const char *path, uint16_t mode);
    int      (*unlink)(mountpoint_t *mnt, const char *path);
    struct fs_type *next;
} fs_type_t;

typedef struct mountpoint {
    char        path[PATH_MAX];
    inode_t    *root;
    inode_t    *covered;    /* Inode the mount covers */
    fs_type_t  *fs;
    uint32_t    flags;
    struct mountpoint *next;
} mountpoint_t;

/* ============================================================================
 * VFS API (kernel-facing)
 * ============================================================================ */
void     vfs_init(void);
int      vfs_register_fs(fs_type_t *type);
int      vfs_mount(const char *source, const char *target,
                   const char *fstype, uint32_t flags);
int      vfs_umount(const char *target);

/* inode operations */
inode_t *vfs_path_to_inode(const char *path, bool follow_links);
inode_t *vfs_inode_get(inode_t *inode);
void     vfs_inode_put(inode_t *inode);

/* File descriptor table (per-process) */
#define FD_STDIN    0
#define FD_STDOUT   1
#define FD_STDERR   2

int     vfs_open(const char *path, int flags, uint16_t mode);
int     vfs_close(int fd);
ssize_t vfs_read(int fd, void *buf, size_t len);
ssize_t vfs_write(int fd, const void *buf, size_t len);
int64_t vfs_seek(int fd, int64_t offset, int whence);
int     vfs_stat(const char *path, stat_t *st);
int     vfs_fstat(int fd, stat_t *st);
int     vfs_mkdir(const char *path, uint16_t mode);
int     vfs_unlink(const char *path);
int     vfs_rmdir(const char *path);
int     vfs_rename(const char *old, const char *newpath);
int     vfs_readdir(int fd, dirent_t *buf, uint32_t count);
int     vfs_ioctl(int fd, uint32_t cmd, void *arg);
int     vfs_symlink(const char *target, const char *linkpath);
int     vfs_readlink(const char *path, char *buf, size_t len);

/* ============================================================================
 * VIBE Filesystem (VibeFS) - Simple log-structured FS
 * ============================================================================ */
#define VIBEFS_MAGIC    0x56494245  /* "VIBE" */
#define VIBEFS_VERSION  1
#define VIBEFS_BLOCK_SIZE 4096

typedef struct PACKED vibefs_super {
    uint32_t magic;
    uint32_t version;
    uint32_t block_size;
    uint32_t block_count;
    uint32_t inode_count;
    uint32_t free_blocks;
    uint32_t free_inodes;
    uint32_t root_inode;
    uint32_t inode_table_block;
    uint32_t block_bitmap_block;
    uint32_t inode_bitmap_block;
    uint32_t data_start_block;
    uint8_t  uuid[16];
    char     label[64];
    uint32_t ctime;         /* Creation time */
    uint32_t mtime;         /* Last mount time */
    uint32_t checksum;
} vibefs_super_t;

typedef struct PACKED vibefs_inode {
    uint16_t mode;
    uint32_t uid, gid;
    uint64_t size;
    uint32_t atime, mtime, ctime;
    uint32_t nlinks;
    uint32_t direct[12];    /* Direct block pointers */
    uint32_t indirect;      /* Singly indirect */
    uint32_t dindirect;     /* Doubly indirect */
    uint32_t tindirect;     /* Triply indirect */
} vibefs_inode_t;

fs_type_t *vibefs_get_type(void);
int vibefs_mkfs(void *block_dev, size_t size, const char *label);

/* ============================================================================
 * RAMFS (in-memory tmpfs)
 * ============================================================================ */
fs_type_t *ramfs_get_type(void);
inode_t   *ramfs_create_root(void);

/* ============================================================================
 * Proc filesystem (/proc)
 * ============================================================================ */
fs_type_t *procfs_get_type(void);
void       procfs_register(const char *name,
    ssize_t (*read_fn)(char *buf, size_t len));

/* ============================================================================
 * Path utilities
 * ============================================================================ */
int  path_canonicalize(const char *in, char *out, size_t len);
int  path_dirname(const char *path, char *out, size_t len);
int  path_basename(const char *path, char *out, size_t len);
bool path_is_absolute(const char *path);

#endif /*  EarlnuxOS_FS_H */
