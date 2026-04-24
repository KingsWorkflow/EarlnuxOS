/* PROCFS Stub */
#include <kernel/kernel.h>
#include <fs/vfs.h>
fs_type_t *procfs_get_type(void) { return NULL; }
void procfs_register(const char *name, ssize_t (*read_fn)(char *buf, size_t len)) { }