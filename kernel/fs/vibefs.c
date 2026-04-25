/* VibeFS Stub */
#include <fs/vfs.h>
#include <stddef.h>

static fs_type_t vibefs_type = {
    .name = "vibefs",
};

fs_type_t *vibefs_get_type(void) {
    return &vibefs_type;
}

int vibefs_mkfs(void *block_dev, size_t size, const char *label) {
    (void)block_dev;
    (void)size;
    (void)label;
    return -1;
}