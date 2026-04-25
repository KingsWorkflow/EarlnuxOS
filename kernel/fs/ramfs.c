/* ============================================================================
 *  EarlnuxOS - RAM Filesystem (ramfs / tmpfs)
 * kernel/fs/ramfs.c
 * ============================================================================ */

#include <fs/vfs.h>
#include <kernel/kernel.h>
#include <types.h>
#include <string.h>
#include <mm/mm.h>

/* Forward declarations */
static inode_ops_t ramfs_inode_ops;
static file_ops_t ramfs_file_ops;

static inode_t *ramfs_get_inode_by_path(inode_t *root, const char *path) {
    if (!path || *path == '\0' || (path[0] == '/' && path[1] == '\0')) 
        return root;
    
    inode_t *curr = root;
    char buf[NAME_MAX + 1];
    const char *p = path;
    
    while (*p) {
        while (*p == '/') p++;
        if (!*p) break;
        
        int i = 0;
        while (*p && *p != '/' && i < NAME_MAX) {
            buf[i++] = *p++;
        }
        buf[i] = '\0';
        
        if (strcmp(buf, ".") == 0) continue;
        if (strcmp(buf, "..") == 0) continue;
        
        if (!S_ISDIR(curr->mode)) return NULL;
        
        if (!curr->ops || !curr->ops->lookup) return NULL;
        inode_t *next = curr->ops->lookup(curr, buf);
        if (!next) return NULL;
        curr = next;
    }
    return curr;
}

typedef struct ramfs_dirent {
    char                 name[NAME_MAX + 1];
    inode_t             *inode;
    struct ramfs_dirent *next;
} ramfs_dirent_t;

typedef struct {
    uint8_t  *data;
    uint64_t  capacity;
    ramfs_dirent_t *entries;
    char *link_target;
} ramfs_inode_data_t;

static uint32_t next_ino = 1;

static inode_t *ramfs_alloc_inode(fs_type_t *fs, uint16_t mode) {
    inode_t *node = kmalloc(sizeof(inode_t));
    if (!node) return NULL;
    memset(node, 0, sizeof(inode_t));

    ramfs_inode_data_t *priv = kmalloc(sizeof(ramfs_inode_data_t));
    if (!priv) { kfree(node); return NULL; }
    memset(priv, 0, sizeof(ramfs_inode_data_t));

    node->ino        = next_ino++;
    node->mode       = mode;
    node->ref_count  = 1;
    node->nlinks     = 1;
    node->block_size = 4096;
    node->fs         = fs;
    node->fs_data    = priv;
    node->ops        = &ramfs_inode_ops;
    return node;
}

static inode_t *ramfs_lookup(inode_t *dir, const char *name) {
    ramfs_inode_data_t *priv = dir->fs_data;
    for (ramfs_dirent_t *de = priv->entries; de; de = de->next) {
        if (strncmp(de->name, name, NAME_MAX) == 0) {
            de->inode->ref_count++;
            return de->inode;
        }
    }
    return NULL;
}

static int ramfs_create(inode_t *dir, const char *name, uint16_t mode, inode_t **out) {
    inode_t *node = ramfs_alloc_inode(dir->fs, mode);
    if (!node) return -1;

    ramfs_dirent_t *de = kmalloc(sizeof(ramfs_dirent_t));
    if (!de) { kfree(node->fs_data); kfree(node); return -1; }

    strncpy(de->name, name, NAME_MAX);
    de->name[NAME_MAX] = '\0';
    de->inode = node;

    ramfs_inode_data_t *dpriv = dir->fs_data;
    de->next       = dpriv->entries;
    dpriv->entries = de;
    dir->nlinks++;

    if (out) *out = node;
    return 0;
}

static int ramfs_mkdir(inode_t *dir, const char *name, uint16_t mode) {
    inode_t *unused = NULL;
    return ramfs_create(dir, name, S_IFDIR | (mode & 0777), &unused);
}

static int ramfs_unlink(inode_t *dir, const char *name) {
    ramfs_inode_data_t *priv = dir->fs_data;
    ramfs_dirent_t **pp = &priv->entries;
    while (*pp) {
        if (strncmp((*pp)->name, name, NAME_MAX) == 0) {
            ramfs_dirent_t *victim = *pp;
            *pp = victim->next;
            victim->inode->nlinks--;
            if (victim->inode->nlinks == 0 && victim->inode->ref_count <= 1) {
                ramfs_inode_data_t *ipriv = victim->inode->fs_data;
                if (ipriv->data)        kfree(ipriv->data);
                if (ipriv->link_target) kfree(ipriv->link_target);
                kfree(ipriv);
                kfree(victim->inode);
            }
            kfree(victim);
            dir->nlinks--;
            return 0;
        }
        pp = &(*pp)->next;
    }
    return -1;
}

static int ramfs_rmdir(inode_t *dir, const char *name) { return ramfs_unlink(dir, name); }

static int ramfs_rename(inode_t *old_dir, const char *old_name, inode_t *new_dir, const char *new_name) {
    (void)new_dir; (void)new_name;
    ramfs_inode_data_t *opriv = old_dir->fs_data;
    for (ramfs_dirent_t *de = opriv->entries; de; de = de->next) {
        if (strncmp(de->name, old_name, NAME_MAX) == 0) {
            strncpy(de->name, new_name, NAME_MAX);
            return 0;
        }
    }
    return -1;
}

static int ramfs_symlink(inode_t *dir, const char *name, const char *target) {
    inode_t *node = NULL;
    if (ramfs_create(dir, name, S_IFLNK | 0777, &node) != 0) return -1;
    ramfs_inode_data_t *priv = node->fs_data;
    size_t len = strlen(target) + 1;
    priv->link_target = kmalloc(len);
    if (!priv->link_target) return -1;
    memcpy(priv->link_target, target, len);
    node->size = len - 1;
    return 0;
}

static int ramfs_readlink(inode_t *node, char *buf, size_t len) {
    ramfs_inode_data_t *priv = node->fs_data;
    if (!priv->link_target) return -1;
    strncpy(buf, priv->link_target, len);
    buf[len - 1] = '\0';
    return (int)strlen(buf);
}

static int ramfs_chmod(inode_t *node, uint16_t mode) {
    node->mode = (node->mode & S_IFMT) | (mode & 0777);
    return 0;
}

static int ramfs_chown(inode_t *node, uint32_t uid, uint32_t gid) {
    node->uid = uid; node->gid = gid; return 0;
}

static int ramfs_truncate(inode_t *node, uint64_t size) {
    ramfs_inode_data_t *priv = node->fs_data;
    if (size == 0) {
        if (priv->data) { kfree(priv->data); priv->data = NULL; }
        priv->capacity = 0; node->size = 0; return 0;
    }
    uint8_t *nb = kmalloc((size_t)size);
    if (!nb) return -1;
    memset(nb, 0, (size_t)size);
    if (priv->data) {
        size_t copy = (size_t)(node->size < size ? node->size : size);
        memcpy(nb, priv->data, copy);
        kfree(priv->data);
    }
    priv->data = nb; priv->capacity = size; node->size = size;
    return 0;
}

static void ramfs_sync(inode_t *node) { (void)node; }

static void ramfs_destroy(inode_t *node) {
    ramfs_inode_data_t *priv = node->fs_data;
    if (priv) {
        if (priv->data)        kfree(priv->data);
        if (priv->link_target) kfree(priv->link_target);
        ramfs_dirent_t *de = priv->entries;
        while (de) { ramfs_dirent_t *next = de->next; kfree(de); de = next; }
        kfree(priv);
    }
    kfree(node);
}

static int ramfs_file_read(file_t *file, void *buf, size_t count) {
    inode_t *node = file->inode;
    ramfs_inode_data_t *priv = node->fs_data;
    if (!priv->data || (uint64_t)file->pos >= (uint64_t)node->size) return 0;
    size_t avail = (size_t)(node->size - (uint64_t)file->pos);
    size_t n = count < avail ? count : avail;
    memcpy(buf, priv->data + file->pos, n);
    file->pos += (int64_t)n;
    return (int)n;
}

static int ramfs_file_write(file_t *file, const void *buf, size_t count) {
    inode_t *node = file->inode;
    ramfs_inode_data_t *priv = node->fs_data;
    uint64_t new_end = (uint64_t)file->pos + count;
    if (new_end > priv->capacity) {
        uint64_t new_cap = new_end < 4096 ? 4096 : new_end * 2;
        uint8_t *nb = kmalloc((size_t)new_cap);
        if (!nb) return -1;
        memset(nb, 0, (size_t)new_cap);
        if (priv->data) { memcpy(nb, priv->data, (size_t)priv->capacity); kfree(priv->data); }
        priv->data = nb; priv->capacity = new_cap;
    }
    memcpy(priv->data + file->pos, buf, count);
    file->pos += (int64_t)count;
    if ((uint64_t)file->pos > node->size) node->size = (uint64_t)file->pos;
    node->dirty = true;
    return (int)count;
}

static int ramfs_file_readdir(file_t *file, void *buf, size_t buf_size) {
    inode_t *node = file->inode;
    ramfs_inode_data_t *priv = node->fs_data;
    ramfs_dirent_t *de = priv->entries;
    dirent_t *dbuf = (dirent_t *)buf;
    uint32_t count = (uint32_t)(buf_size / sizeof(dirent_t));
    
    /* Skip to file->pos */
    for (uint64_t i = 0; i < file->pos && de; i++) {
        de = de->next;
    }
    
    uint32_t n = 0;
    while (de && n < count) {
        strncpy(dbuf[n].d_name, de->name, NAME_MAX);
        dbuf[n].d_ino = de->inode->ino;
        de = de->next;
        n++;
        file->pos++;
    }
    return (int)n;
}

static int ramfs_file_open(file_t *file, inode_t *inode, int flags) {
    (void)flags; file->inode = inode; file->pos = 0; return 0;
}

static void ramfs_file_close(file_t *file) { (void)file; }

static int ramfs_fs_open(mountpoint_t *mnt, const char *path, int flags, uint16_t mode, file_t *file) {
    inode_t *node = ramfs_get_inode_by_path(mnt->root, path);
    if (!node && (flags & O_CREAT)) {
        char dname[PATH_MAX], bname[PATH_MAX];
        path_dirname(path, dname, PATH_MAX); path_basename(path, bname, PATH_MAX);
        inode_t *dir = ramfs_get_inode_by_path(mnt->root, dname);
        if (!dir || !S_ISDIR(dir->mode)) return -1;
        if (ramfs_create(dir, bname, S_IFREG | (mode & 0777), &node) != 0) return -1;
    }
    if (!node) return -1;
    file->inode = node; file->ops = &ramfs_file_ops; file->pos = 0;
    if (flags & O_TRUNC) ramfs_truncate(node, 0);
    if (flags & O_APPEND) file->pos = node->size;
    return 0;
}

static int ramfs_fs_mkdir(mountpoint_t *mnt, const char *path, uint16_t mode) {
    char dname[PATH_MAX], bname[PATH_MAX];
    path_dirname(path, dname, PATH_MAX);
    path_basename(path, bname, PATH_MAX);
    inode_t *dir = ramfs_get_inode_by_path(mnt->root, dname);
    if (!dir || !S_ISDIR(dir->mode)) return -1;
    return ramfs_mkdir(dir, bname, mode);
}

static int ramfs_fs_unlink(mountpoint_t *mnt, const char *path) {
    char dname[PATH_MAX], bname[PATH_MAX];
    path_dirname(path, dname, PATH_MAX); path_basename(path, bname, PATH_MAX);
    inode_t *dir = ramfs_get_inode_by_path(mnt->root, dname);
    if (!dir || !S_ISDIR(dir->mode)) return -1;
    return ramfs_unlink(dir, bname);
}

static inode_ops_t ramfs_inode_ops = {
    .lookup   = ramfs_lookup, .create   = ramfs_create, .mkdir    = ramfs_mkdir,
    .unlink   = ramfs_unlink, .rmdir    = ramfs_rmdir, .rename   = ramfs_rename,
    .symlink  = ramfs_symlink, .readlink = ramfs_readlink, .chmod    = ramfs_chmod,
    .chown    = ramfs_chown, .truncate = ramfs_truncate, .sync     = ramfs_sync,
    .destroy  = ramfs_destroy,
};

static file_ops_t ramfs_file_ops = {
    .open = ramfs_file_open, .close = ramfs_file_close, .read = ramfs_file_read, 
    .write = ramfs_file_write, .readdir = ramfs_file_readdir,
};

static inode_t *ramfs_mount(fs_type_t *type, const char *source, uint32_t flags) {
    (void)source; (void)flags;
    inode_t *root = ramfs_alloc_inode(type, S_IFDIR | 0755);
    if (!root) return NULL;
    root->ops = &ramfs_inode_ops;
    return root;
}

inode_t *ramfs_create_root(void) {
    static fs_type_t dummy_type = { .name = "ramfs", .mount = ramfs_mount };
    inode_t *root = ramfs_alloc_inode(&dummy_type, S_IFDIR | 0755);
    if (root) root->ops = &ramfs_inode_ops;
    return root;
}

static fs_type_t ramfs_type = {
    .name   = "ramfs", .mount  = ramfs_mount, .open   = ramfs_fs_open,
    .mkdir  = ramfs_fs_mkdir, .unlink = ramfs_fs_unlink,
};

fs_type_t *ramfs_get_type(void) { return &ramfs_type; }