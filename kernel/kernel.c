/* ============================================================================
 *  EarlnuxOS - Kernel Main
 * kernel/kernel.c
 * ============================================================================ */

#include <kernel/kernel.h>
#include <kernel/console.h>
#include <types.h>
#include <mm/mm.h>
#include <net/net.h>
#include <fs/vfs.h>

#define MB_FLAG_MMAP        BIT(6)
#define MULTIBOOT_MAGIC_VAL 0x2BADB002u

/* ============================================================================
 * Global kernel state
 * ============================================================================ */
static multiboot_info_t *mb_info = NULL;
static bool kernel_initialized   = false;

/* ============================================================================
 *  EarlnuxOS Boot Banner
 * ============================================================================ */
static void print_banner(void) {
    console_set_color(VGA_ATTR(COLOR_LIGHT_CYAN, COLOR_BLACK));
    kprintf("\n");
    kprintf("███████╗ █████╗ ██████╗ ██╗     ███╗   ██╗██╗   ██╗██╗  ██╗ ██████╗ ███████╗\n");
    kprintf("██╔════╝██╔══██╗██╔══██╗██║     ████╗  ██║██║   ██║╚██╗██╔╝██╔═══██╗██╔════╝\n");
    kprintf("█████╗  ███████║██████╔╝██║     ██╔██╗ ██║██║   ██║ ╚███╔╝ ██║   ██║███████╗\n");
    kprintf("██╔══╝  ██╔══██║██╔══██╗██║     ██║╚██╗██║██║   ██║ ██╔██╗ ██║   ██║╚════██║\n");
    kprintf("███████╗██║  ██║██║  ██║███████╗██║ ╚████║╚██████╔╝██╔╝ ██╗╚██████╔╝███████║\n");
    kprintf("╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═══╝ ╚═════╝ ╚═╝  ╚═╝ ╚═════╝ ╚══════╝\n");
    console_set_color(VGA_DEFAULT_ATTR);
    kprintf("\n");
    console_set_color(VGA_ATTR(COLOR_YELLOW, COLOR_BLACK));
    kprintf("   EarlnuxOS v%s \"%s\"  -  Copyright (c) 2026  EarlnuxOS Project\n",
             EarlnuxOS_VERSION_STR,  EarlnuxOS_CODENAME);
    console_set_color(VGA_DEFAULT_ATTR);
    kprintf("  Built: %s %s | Arch: i686\n", __DATE__, __TIME__);
    kprintf("\n");
}

/* ============================================================================
 * Subsystem initialization with status reporting
 * ============================================================================ */
static void init_step(const char *name, void (*fn)(void)) {
    kprintf("  Initializing %-30s  ", name);
    fn();
    console_set_color(VGA_SUCCESS_ATTR);
    kprintf("[  OK  ]\n");
    console_set_color(VGA_DEFAULT_ATTR);
}

/* Wrappers for init_step */
static void do_vmm_init(void)     { vmm_init(); }
static void do_heap_init(void)    { /* Heap initialized in mm_init */ }
static void do_vfs_init(void)     { vfs_init(); }
static void do_net_init(void)     { net_init(); }

extern void gdt_init(void);
extern void idt_init(void);
extern void pic_init(void);
extern void pit_init(uint32_t hz);
extern void keyboard_init(void);

static void do_interrupts_init(void) { 
    gdt_init(); 
    idt_init(); 
}
static void do_pic_init(void)        { pic_init(); }
static void do_timer_init(void)      { pit_init(1000); }
static void do_keyboard_init(void)   { keyboard_init(); }

/* ============================================================================
 * Physical memory map from multiboot
 * ============================================================================ */
static void parse_memory_map(void) {
    if (!mb_info || !(mb_info->flags & MB_FLAG_MMAP)) {
        KWARN("kernel", "No multiboot memory map — assuming default RAM");
        static mem_map_entry_t fake_map = {0x00100000, 63 * 1024 * 1024, MEMTYPE_USABLE};
        pmm_init(&fake_map, 1);
        return;
    }

    static mem_map_entry_t entries[64];
    uint32_t count = 0;
    mmap_entry_t *e = (mmap_entry_t *)mb_info->mmap_addr;
    mmap_entry_t *end = (mmap_entry_t *)(mb_info->mmap_addr + mb_info->mmap_length);

    while (e < end && count < 64) {
        entries[count].base   = ((uint64_t)e->addr_high << 32) | e->addr_low;
        entries[count].length = ((uint64_t)e->len_high << 32) | e->len_low;
        entries[count].type   = (mem_type_t)e->type;
        count++;
        e = (mmap_entry_t *)((uint8_t *)e + e->size + 4);
    }
    pmm_init(entries, count);
}

/* ============================================================================
 * Mount initial filesystems
 * ============================================================================ */
static void mount_initial_fs(void) {
    vfs_register_fs(ramfs_get_type());
    vfs_register_fs(procfs_get_type());
    vfs_register_fs(vibefs_get_type());

    if (vfs_mount(NULL, "/", "ramfs", 0) < 0) {
        panic("Failed to mount root filesystem");
    }

    vfs_mkdir("/dev",  0755);
    vfs_mkdir("/tmp",  0777);
    vfs_mkdir("/home", 0755);
    vfs_mkdir("/bin",  0755);
    vfs_mkdir("/etc",  0755);

    int fd = vfs_open("/etc/hostname", O_WRONLY | O_CREAT, 0644);
    if (fd >= 0) {
        vfs_write(fd, "EarlnuxOS-host\n", 15);
        vfs_close(fd);
    }
}

static void start_networking(void) {
    netif_t *iface = netif_get_default();
    if (!iface) return;

    if (dhcp_discover() != 0) {
        iface->ip_cfg.addr    = IP4(10, 0, 2, 15);
        iface->ip_cfg.netmask = IP4(255, 255, 255, 0);
        iface->ip_cfg.gateway = IP4(10, 0, 2, 2);
        iface->ip_cfg.dns[0]  = IP4(8, 8, 8, 8);
    }
    dns_set_server(iface->ip_cfg.dns[0]);
}

/* ============================================================================
 * Built-in Mini Shell
 * ============================================================================ */
#define SHELL_BUF_SIZE 256
static char shell_input[SHELL_BUF_SIZE];

extern char keyboard_getchar(void);

static int shell_readline(char *buf, size_t max) {
    size_t pos = 0;
    while (pos < max - 1) {
        char c = keyboard_getchar();
        if (c == '\n' || c == '\r') {
            buf[pos] = '\0';
            console_putchar('\n');
            return (int)pos;
        } else if (c == '\b' || c == 0x7F) {
            if (pos > 0) {
                pos--;
                console_putchar('\b');
                console_putchar(' ');
                console_putchar('\b');
            }
        } else if (c >= 0x20) {
            buf[pos++] = c;
            console_putchar(c);
        }
    }
    buf[pos] = '\0';
    return (int)pos;
}

static int kstrcmp(const char *a, const char *b) {
    while (*a && *b && *a == *b) { a++; b++; }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

static int tokenize(char *str, char *argv[], int max_args) {
    int argc = 0;
    char *p = str;
    while (*p && argc < max_args) {
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') break;
        argv[argc++] = p;
        while (*p && *p != ' ' && *p != '\t') p++;
        if (*p) *p++ = '\0';
    }
    return argc;
}

/* ============================================================================
 * Shell Commands
 * ============================================================================ */
static void cmd_help(int argc, char *argv[]) {
    (void)argc; (void)argv;
    kprintf("\nCommands: help, echo, clear, meminfo, netinfo, ls, cat, mkdir, rm, uptime, uname, reboot, halt\n");
}

static void cmd_echo(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        kprintf("%s ", argv[i]);
    }
    kprintf("\n");
}

static void cmd_clear(int argc, char *argv[]) {
    (void)argc; (void)argv;
    console_clear();
}

static void cmd_meminfo(int argc, char *argv[]) {
    (void)argc; (void)argv;
    pmm_stats_t ps;
    heap_stats_t hs;
    pmm_get_stats(&ps);
    heap_get_stats(&hs);
    kprintf("\nMemory: %uMB Total, %uMB Free\nHeap: %uKB Used / %uKB Total\n",
            ps.total_phys / (1024*1024), ps.free_phys / (1024*1024),
            hs.used_bytes / 1024, hs.heap_size / 1024);
}

static void cmd_netinfo(int argc, char *argv[]) {
    (void)argc; (void)argv;
    net_dump_stats();
}

static void cmd_ls(int argc, char *argv[]) {
    const char *path = argc >= 2 ? argv[1] : "/";
    int fd = vfs_open(path, O_RDONLY | O_DIRECTORY, 0);
    if (fd < 0) { kprintf("ls: fail\n"); return; }
    dirent_t entries[32];
    int n = vfs_readdir(fd, entries, 32);
    vfs_close(fd);
    for (int i = 0; i < n; i++) kprintf("  %s\n", entries[i].d_name);
}

static void cmd_cat(int argc, char *argv[]) {
    if (argc < 2) return;
    int fd = vfs_open(argv[1], O_RDONLY, 0);
    if (fd < 0) return;
    char buf[128];
    ssize_t n;
    while ((n = vfs_read(fd, buf, sizeof(buf)-1)) > 0) {
        buf[n] = '\0';
        kprintf("%s", buf);
    }
    vfs_close(fd);
    kprintf("\n");
}

static void cmd_mkdir(int argc, char *argv[]) {
    if (argc < 2) return;
    vfs_mkdir(argv[1], 0755);
}

static void cmd_rm(int argc, char *argv[]) {
    if (argc < 2) return;
    vfs_unlink(argv[1]);
}

extern uint32_t timer_get_uptime_ms(void);
static void cmd_uptime(int argc, char *argv[]) {
    (void)argc; (void)argv;
    kprintf("Uptime: %u ms\n", timer_get_uptime_ms());
}

static void cmd_uname(int argc, char *argv[]) {
    (void)argc; (void)argv;
    kprintf("%s %s i686\n", EarlnuxOS_NAME, EarlnuxOS_VERSION_STR);
}

static void cmd_reboot(int argc, char *argv[]) {
    (void)argc; (void)argv;
    outb(0x64, 0xFE);
}

static void cmd_halt(int argc, char *argv[]) {
    (void)argc; (void)argv;
    kprintf("Halted.\n");
    cli();
    for(;;) cpu_hlt();
}

typedef struct { const char *name; void (*fn)(int, char **); } cmd_t;
static const cmd_t commands[] = {
    {"help", cmd_help}, {"echo", cmd_echo}, {"clear", cmd_clear},
    {"meminfo", cmd_meminfo}, {"netinfo", cmd_netinfo}, {"ls", cmd_ls},
    {"cat", cmd_cat}, {"mkdir", cmd_mkdir}, {"rm", cmd_rm},
    {"uptime", cmd_uptime}, {"uname", cmd_uname}, {"reboot", cmd_reboot},
    {"halt", cmd_halt}, {NULL, NULL}
};

static void shell_run(void) {
    char *argv[16];
    while (1) {
        console_set_color(VGA_ATTR(COLOR_LIGHT_GREEN, COLOR_BLACK));
        kprintf("root@EarlnuxOS");
        console_set_color(VGA_DEFAULT_ATTR);
        kprintf(":~# ");
        int len = shell_readline(shell_input, SHELL_BUF_SIZE);
        if (len == 0) continue;
        int argc = tokenize(shell_input, argv, 16);
        for (int i = 0; commands[i].name; i++) {
            if (kstrcmp(commands[i].name, argv[0]) == 0) {
                commands[i].fn(argc, argv);
                goto next;
            }
        }
        kprintf("%s: command not found\n", argv[0]);
        next:;
    }
}

/* ============================================================================
 * kernel_main - C entry point
 * ============================================================================ */
void kernel_main(uint32_t mb_magic, multiboot_info_t *info) {
    console_init();
    console_clear();

    if (mb_magic == MULTIBOOT_MAGIC_VAL) mb_info = info;

    kprintf("  === Kernel Initialization ===\n\n");
    parse_memory_map();
    init_step("Virtual memory", do_vmm_init);
    init_step("Kernel heap", do_heap_init);
    init_step("Interrupts", do_interrupts_init);
    init_step("PIC Controller", do_pic_init);
    init_step("System Timer", do_timer_init);
    init_step("Keyboard Driver", do_keyboard_init);
    init_step("Virtual Filesystem", do_vfs_init);
    
    mount_initial_fs();
    init_step("Networking", do_net_init);
    start_networking();

    __asm__ volatile("sti");

    console_clear();
    print_banner();
    kernel_initialized = true;
    shell_run();
}
