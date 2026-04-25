/* PROCFS Stub */
#include <fs/vfs.h>
#include <stddef.h>

static fs_type_t procfs_type = {
    .name = "procfs",
};

fs_type_t *procfs_get_type(void) {
    return &procfs_type;
}

void procfs_register(const char *name, ssize_t (*read_fn)(char *buf, size_t len)) {
    (void)name;
    (void)read_fn;
    // Stub implementation
}