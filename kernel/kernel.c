/* ============================================================================
 *  EarlnuxOS - Kernel Main
 * kernel/kernel.c
 * ============================================================================ */

#include <kernel/kernel.h>
#include <types.h>
#include <mm/mm.h>
#include <net/net.h>
#include <fs/vfs.h>

/* ============================================================================
 * Multiboot info structure (simplified)
 * ============================================================================ */
typedef struct PACKED multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;     /* Kilobytes below 1MB */
    uint32_t mem_upper;     /* Kilobytes above 1MB */
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    /* GRUB memory map */
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
} multiboot_info_t;

typedef struct PACKED mmap_entry {
    uint32_t size;
    uint64_t base;
    uint64_t length;
    uint32_t type;
} mmap_entry_t;

#define MB_FLAG_MMAP        BIT(6)
#define MULTIBOOT_MAGIC_VAL 0x2BADB002u

/* ============================================================================
 * Global kernel state
 * ============================================================================ */
static multiboot_info_t *mb_info = NULL;
static bool kernel_initialized   = false;

/* ============================================================================
 * Hardware detection helpers
 * ============================================================================ */
static uint32_t detect_memory_mb(void) {
    if (!mb_info) return 16;  /* Default 16MB if no multiboot */
    return (mb_info->mem_lower + mb_info->mem_upper) / 1024;
}

/* ============================================================================
 *  EarlnuxOS Boot Banner
 * ============================================================================ */
static void print_banner(void) {
    console_set_color(VGA_ATTR(COLOR_LIGHT_CYAN, COLOR_BLACK));
    kprintf("\n");
    kprintf("  ██╗   ██╗██╗██████╗ ███████╗ ██████╗ ███████╗\n");
    kprintf("  ██║   ██║██║██╔══██╗██╔════╝██╔═══██╗██╔════╝\n");
    kprintf("  ██║   ██║██║██████╔╝█████╗  ██║   ██║███████╗\n");
    kprintf("  ╚██╗ ██╔╝██║██╔══██╗██╔══╝  ██║   ██║╚════██║\n");
    kprintf("   ╚████╔╝ ██║██████╔╝███████╗╚██████╔╝███████║\n");
    kprintf("    ╚═══╝  ╚═╝╚═════╝ ╚══════╝ ╚═════╝ ╚══════╝\n");
    console_set_color(VGA_DEFAULT_ATTR);
    kprintf("\n");
    console_set_color(VGA_ATTR(COLOR_YELLOW, COLOR_BLACK));
    kprintf("   EarlnuxOS v%s \"%s\"  -  Copyright (c) 2025  EarlnuxOS Project\n",
             EarlnuxOS_VERSION_STR,  EarlnuxOS_CODENAME);
    console_set_color(VGA_DEFAULT_ATTR);
    kprintf("  Built: %s %s | Arch: i686\n", __DATE__, __TIME__);
    kprintf("\n");
}

/* ============================================================================
 * Subsystem initialization with status reporting
 * ============================================================================ */
#define INIT_OK   "[ \x1b[32mOK\x1b[0m ]"
#define INIT_FAIL "[\x1b[31mFAIL\x1b[0m]"
#define INIT_SKIP "[\x1b[33mSKIP\x1b[0m]"

static void init_step(const char *name, void (*fn)(void)) {
    kprintf("  Initializing %-30s  ", name);
    fn();
    console_set_color(VGA_SUCCESS_ATTR);
    kprintf("[  OK  ]\n");
    console_set_color(VGA_DEFAULT_ATTR);
}

/* Wrappers for init_step (matching void(*)(void) signature) */
static void do_console_init(void) { console_init(); }
static void do_pmm_init(void)     { /* pmm_init called below with args */ }
static void do_vmm_init(void)     { vmm_init(); }
static void do_heap_init(void)    {
    heap_init(KERNEL_HEAP_START, KERNEL_HEAP_SIZE);
}
static void do_interrupts_init(void); /* forward decl */
static void do_pic_init(void);
static void do_timer_init(void);
static void do_keyboard_init(void);
static void do_vfs_init(void)     { vfs_init(); }
static void do_net_init(void)     { net_init(); }

/* ============================================================================
 * IDT / Interrupts initialization (forward declared)
 * ============================================================================ */
extern void idt_init(void);
extern void pic_init(void);
extern void pit_init(uint32_t hz);
extern void keyboard_init(void);

static void do_interrupts_init(void) { idt_init(); }
static void do_pic_init(void)        { pic_init(); }
static void do_timer_init(void)      { pit_init(1000); /* 1000 Hz */ }
static void do_keyboard_init(void)   { keyboard_init(); }

/* ============================================================================
 * Physical memory map from multiboot
 * ============================================================================ */
static void parse_memory_map(void) {
    if (!mb_info || !(mb_info->flags & MB_FLAG_MMAP)) {
        KWARN("kernel", "No multiboot memory map — assuming 64MB RAM");
        /* Create a fake single-entry memory map */
        static mem_map_entry_t fake_map = {
            .base   = 0x00100000,
            .length = 63 * 1024 * 1024,
            .type   = MEMTYPE_USABLE,
        };
        pmm_init(&fake_map, 1);
        return;
    }

    /* Convert multiboot mmap to our format */
    static mem_map_entry_t entries[64];
    uint32_t count = 0;
    mmap_entry_t *e = (mmap_entry_t *)mb_info->mmap_addr;
    mmap_entry_t *end = (mmap_entry_t *)(mb_info->mmap_addr + mb_info->mmap_length);

    while (e < end && count < 64) {
        entries[count].base   = (uint32_t)e->base;
        entries[count].length = (uint32_t)e->length;
        entries[count].type   = (mem_type_t)e->type;
        count++;
        e = (mmap_entry_t *)((uint8_t *)e + e->size + 4);
    }

    KINFO("kernel", "Memory map: %u entries", count);
    pmm_init(entries, count);
}

/* ============================================================================
 * Mount initial filesystems
 * ============================================================================ */
static void mount_initial_fs(void) {
    /* Register filesystem types */
    vfs_register_fs(ramfs_get_type());
    vfs_register_fs(procfs_get_type());
    vfs_register_fs(vibefs_get_type());

    /* Mount root as ramfs initially */
    if (vfs_mount(NULL, "/", "ramfs", 0) < 0) {
        panic("Failed to mount root filesystem");
    }

    /* Mount /proc */
    vfs_mount(NULL, "/proc", "procfs", 0);

    /* Create essential directories */
    vfs_mkdir("/dev",  0755);
    vfs_mkdir("/tmp",  0777);
    vfs_mkdir("/home", 0755);
    vfs_mkdir("/bin",  0755);
    vfs_mkdir("/etc",  0755);
    vfs_mkdir("/var",  0755);

    /* Write /etc/hostname */
    int fd = vfs_open("/etc/hostname", O_WRONLY | O_CREAT, 0644);
    if (fd >= 0) {
        vfs_write(fd, " EarlnuxOS-host\n", 12);
        vfs_close(fd);
    }

    /* Write /etc/version */
    fd = vfs_open("/etc/version", O_WRONLY | O_CREAT, 0644);
    if (fd >= 0) {
        char buf[64];
        ksprintf(buf, " EarlnuxOS %s (%s)\n",  EarlnuxOS_VERSION_STR,  EarlnuxOS_CODENAME);
        vfs_write(fd, buf, __builtin_strlen(buf));
        vfs_close(fd);
    }

    KINFO("kernel", "Root filesystem mounted");
}

/* ============================================================================
 * Start network interface
 * ============================================================================ */
static void start_networking(void) {
    netif_t *iface = netif_get_default();
    if (!iface) {
        KWARN("net", "No network interface found — networking disabled");
        return;
    }

    KINFO("net", "Interface: %s  MAC: %02x:%02x:%02x:%02x:%02x:%02x",
          iface->name,
          iface->mac[0], iface->mac[1], iface->mac[2],
          iface->mac[3], iface->mac[4], iface->mac[5]);

    /* Attempt DHCP */
    kprintf("  Network: Starting DHCP... ");
    if (dhcp_discover() == 0) {
        ip4_config_t *cfg = &iface->ip_cfg;
        console_set_color(VGA_SUCCESS_ATTR);
        kprintf("OK  IP: %s\n", ip4_to_str(cfg->addr));
        console_set_color(VGA_DEFAULT_ATTR);
    } else {
        /* Fall back to static IP for development */
        iface->ip_cfg.addr    = IP4(10, 0, 2, 15);
        iface->ip_cfg.netmask = IP4(255, 255, 255, 0);
        iface->ip_cfg.gateway = IP4(10, 0, 2, 2);
        iface->ip_cfg.dns[0]  = IP4(8, 8, 8, 8);
        console_set_color(VGA_WARN_ATTR);
        kprintf("DHCP failed — static 10.0.2.15\n");
        console_set_color(VGA_DEFAULT_ATTR);
    }

    dns_set_server(iface->ip_cfg.dns[0]);
}

/* ============================================================================
 * Built-in Mini Shell
 * ============================================================================ */
#define SHELL_BUF_SIZE 256
#define SHELL_HISTORY  16

static char shell_input[SHELL_BUF_SIZE];
static char shell_history[SHELL_HISTORY][SHELL_BUF_SIZE];
static int  shell_hist_idx = 0;

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
                /* Erase character on screen */
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

/* Tiny string library for shell (kernel has no libc) */
static int kstrcmp(const char *a, const char *b) {
    while (*a && *b && *a == *b) { a++; b++; }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

static int kstrncmp(const char *a, const char *b, size_t n) {
    while (n-- && *a && *b && *a == *b) { a++; b++; }
    return n == (size_t)-1 ? 0 : (int)(unsigned char)*a - (int)(unsigned char)*b;
}

static const char *kstrchr(const char *s, int c) {
    while (*s && *s != (char)c) s++;
    return (*s == (char)c) ? s : NULL;
}

/* Simple command tokenizer */
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
    console_set_color(VGA_ATTR(COLOR_LIGHT_CYAN, COLOR_BLACK));
    kprintf("\n EarlnuxOS Built-in Shell Commands:\n");
    console_set_color(VGA_DEFAULT_ATTR);
    kprintf("  help              Show this help\n");
    kprintf("  echo [text]       Print text to console\n");
    kprintf("  clear             Clear screen\n");
    kprintf("  meminfo           Show memory statistics\n");
    kprintf("  netinfo           Show network statistics\n");
    kprintf("  ifconfig          Show/configure network interface\n");
    kprintf("  ping <ip|host>    Ping an IP address\n");
    kprintf("  ls [path]         List directory\n");
    kprintf("  cat <file>        Print file content\n");
    kprintf("  mkdir <dir>       Create directory\n");
    kprintf("  rm <file>         Remove file\n");
    kprintf("  write <file>      Write stdin to file\n");
    kprintf("  ps                List processes\n");
    kprintf("  uptime            Show system uptime\n");
    kprintf("  uname             System information\n");
    kprintf("  reboot            Reboot system\n");
    kprintf("  halt              Halt system\n");
    kprintf("\n");
}

static void cmd_echo(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (i > 1) console_putchar(' ');
        console_puts(argv[i]);
    }
    console_putchar('\n');
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

    console_set_color(VGA_ATTR(COLOR_LIGHT_CYAN, COLOR_BLACK));
    kprintf("\nMemory Information:\n");
    console_set_color(VGA_DEFAULT_ATTR);
    kprintf("  Physical Memory:\n");
    kprintf("    Total:     %u MB  (%u pages)\n",
            ps.total_phys / (1024 * 1024), ps.total_pages);
    kprintf("    Free:      %u MB  (%u pages)\n",
            ps.free_phys / (1024 * 1024), ps.free_pages);
    kprintf("    Used:      %u MB\n",
            (ps.total_phys - ps.free_phys) / (1024 * 1024));
    kprintf("    Kernel:    %u KB\n", ps.kernel_phys / 1024);
    kprintf("    Reserved:  %u KB\n", ps.reserved_phys / 1024);
    kprintf("  Kernel Heap:\n");
    kprintf("    Total:     %u KB\n", hs.heap_size / 1024);
    kprintf("    Used:      %u KB\n", hs.used_bytes / 1024);
    kprintf("    Free:      %u KB\n", hs.free_bytes / 1024);
    kprintf("    Peak:      %u KB\n", hs.peak_usage / 1024);
    kprintf("    Allocs:    %u  Frees: %u\n", hs.alloc_count, hs.free_count);
    kprintf("\n");
}

static void cmd_netinfo(int argc, char *argv[]) {
    (void)argc; (void)argv;
    net_dump_stats();
    arp_dump_cache();
}

static void cmd_ifconfig(int argc, char *argv[]) {
    (void)argc; (void)argv;
    netif_list();
}

static void cmd_ping(int argc, char *argv[]) {
    if (argc < 2) {
        kprintf("Usage: ping <ip_address or hostname>\n");
        return;
    }

    ip4_addr_t dst = 0;
    /* Try to parse as IP address */
    const char *s = argv[1];
    uint32_t parts[4] = {0,0,0,0};
    int part = 0;
    bool parse_ok = true;
    while (*s && part < 4) {
        if (*s >= '0' && *s <= '9') {
            parts[part] = parts[part] * 10 + (*s - '0');
        } else if (*s == '.') {
            part++;
        } else {
            parse_ok = false;
            break;
        }
        s++;
    }
    if (parse_ok && part == 3) {
        dst = IP4(parts[0], parts[1], parts[2], parts[3]);
    } else {
        /* Try DNS */
        kprintf("Resolving %s...\n", argv[1]);
        if (dns_resolve(argv[1], &dst) < 0) {
            kprintf("ping: cannot resolve '%s'\n", argv[1]);
            return;
        }
        kprintf("PING %s (%s)\n", argv[1], ip4_to_str(dst));
    }

    for (int i = 0; i < 4; i++) {
        kprintf("  Sending ICMP echo to %s (seq %d)... ", ip4_to_str(dst), i);
        if (icmp_ping(dst, 0x5ABE, (uint16_t)i) == 0) {
            console_set_color(VGA_SUCCESS_ATTR);
            kprintf("replied\n");
        } else {
            console_set_color(VGA_WARN_ATTR);
            kprintf("timeout\n");
        }
        console_set_color(VGA_DEFAULT_ATTR);
    }
}

static void cmd_ls(int argc, char *argv[]) {
    const char *path = argc >= 2 ? argv[1] : "/";
    int fd = vfs_open(path, O_RDONLY | O_DIRECTORY, 0);
    if (fd < 0) {
        kprintf("ls: cannot open '%s'\n", path);
        return;
    }
    dirent_t entries[32];
    int n = vfs_readdir(fd, entries, 32);
    vfs_close(fd);

    for (int i = 0; i < n; i++) {
        if (S_ISDIR(entries[i].d_type << 12)) {
            console_set_color(VGA_ATTR(COLOR_LIGHT_BLUE, COLOR_BLACK));
            kprintf("  %-20s  [DIR]\n", entries[i].d_name);
        } else {
            console_set_color(VGA_DEFAULT_ATTR);
            kprintf("  %-20s\n", entries[i].d_name);
        }
        console_set_color(VGA_DEFAULT_ATTR);
    }
}

static void cmd_cat(int argc, char *argv[]) {
    if (argc < 2) { kprintf("Usage: cat <file>\n"); return; }
    int fd = vfs_open(argv[1], O_RDONLY, 0);
    if (fd < 0) { kprintf("cat: '%s': No such file\n", argv[1]); return; }
    char buf[256];
    ssize_t n;
    while ((n = vfs_read(fd, buf, sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
        console_puts(buf);
    }
    vfs_close(fd);
    console_putchar('\n');
}

static void cmd_mkdir(int argc, char *argv[]) {
    if (argc < 2) { kprintf("Usage: mkdir <dir>\n"); return; }
    if (vfs_mkdir(argv[1], 0755) < 0) kprintf("mkdir: failed\n");
    else kprintf("Directory '%s' created\n", argv[1]);
}

static void cmd_rm(int argc, char *argv[]) {
    if (argc < 2) { kprintf("Usage: rm <file>\n"); return; }
    if (vfs_unlink(argv[1]) < 0) kprintf("rm: failed\n");
    else kprintf("Removed '%s'\n", argv[1]);
}

extern uint32_t timer_get_uptime_ms(void);

static void cmd_uptime(int argc, char *argv[]) {
    (void)argc; (void)argv;
    uint32_t ms = timer_get_uptime_ms();
    uint32_t s  = ms / 1000;
    uint32_t m  = s / 60;
    uint32_t h  = m / 60;
    kprintf("Uptime: %u hours, %u min, %u sec\n", h, m % 60, s % 60);
}

static void cmd_uname(int argc, char *argv[]) {
    (void)argc; (void)argv;
    kprintf("%s %s #1 %s %s i686  EarlnuxOS\n",
             EarlnuxOS_NAME,  EarlnuxOS_VERSION_STR, __DATE__, __TIME__);
}

extern void ps_dump(void);
static void cmd_ps(int argc, char *argv[]) {
    (void)argc; (void)argv;
    ps_dump();
}

static void cmd_reboot(int argc, char *argv[]) {
    (void)argc; (void)argv;
    kprintf("Rebooting...\n");
    /* Keyboard controller reset */
    outb(0x64, 0xFE);
    /* Triple fault fallback */
    __asm__ volatile("lidt 0; int3");
    for (;;) ;
}

static void cmd_halt(int argc, char *argv[]) {
    (void)argc; (void)argv;
    kprintf("System halted. Safe to power off.\n");
    cli();
    for (;;) cpu_hlt();
}

/* Command dispatch table */
typedef struct { const char *name; void (*fn)(int, char **); } cmd_t;

static const cmd_t commands[] = {
    {"help",     cmd_help},
    {"echo",     cmd_echo},
    {"clear",    cmd_clear},
    {"meminfo",  cmd_meminfo},
    {"netinfo",  cmd_netinfo},
    {"ifconfig", cmd_ifconfig},
    {"ping",     cmd_ping},
    {"ls",       cmd_ls},
    {"cat",      cmd_cat},
    {"mkdir",    cmd_mkdir},
    {"rm",       cmd_rm},
    {"uptime",   cmd_uptime},
    {"uname",    cmd_uname},
    {"ps",       cmd_ps},
    {"reboot",   cmd_reboot},
    {"halt",     cmd_halt},
    {NULL, NULL}
};

static void shell_run(void) {
    char *argv[16];
    console_set_color(VGA_ATTR(COLOR_LIGHT_GREEN, COLOR_BLACK));
    kprintf("\n EarlnuxOS shell ready. Type 'help' for commands.\n\n");
    console_set_color(VGA_DEFAULT_ATTR);

    while (1) {
        /* Prompt */
        console_set_color(VGA_ATTR(COLOR_LIGHT_GREEN, COLOR_BLACK));
        kprintf("root@ EarlnuxOS");
        console_set_color(VGA_DEFAULT_ATTR);
        kprintf(":");
        console_set_color(VGA_ATTR(COLOR_LIGHT_BLUE, COLOR_BLACK));
        kprintf("~");
        console_set_color(VGA_DEFAULT_ATTR);
        kprintf("# ");

        int len = shell_readline(shell_input, SHELL_BUF_SIZE);
        if (len == 0) continue;

        /* Save to history */
        uint32_t hi = shell_hist_idx++ % SHELL_HISTORY;
        for (int i = 0; i < SHELL_BUF_SIZE; i++)
            shell_history[hi][i] = shell_input[i];

        int argc = tokenize(shell_input, argv, 16);
        if (argc == 0) continue;

        bool found = false;
        for (int i = 0; commands[i].name; i++) {
            if (kstrcmp(commands[i].name, argv[0]) == 0) {
                commands[i].fn(argc, argv);
                found = true;
                break;
            }
        }
        if (!found) {
            kprintf("%s: command not found\n", argv[0]);
        }
    }
}

/* ============================================================================
 * kernel_main - C entry point called from entry.asm
 * ============================================================================ */
void kernel_main(uint32_t mb_magic, multiboot_info_t *info) {
    /* Very first step: set up the VGA console (no deps) */
    console_init();
    console_clear();

    /* Validate multiboot */
    if (mb_magic == MULTIBOOT_MAGIC_VAL) {
        mb_info = info;
    } else {
        mb_info = NULL;
    }

    /* Print banner */
    print_banner();

    console_set_color(VGA_ATTR(COLOR_YELLOW, COLOR_BLACK));
    kprintf("  === Kernel Initialization ===\n\n");
    console_set_color(VGA_DEFAULT_ATTR);

    /* Step 1: Physical Memory Manager */
    kprintf("  Parsing memory map...            ");
    parse_memory_map();
    console_set_color(VGA_SUCCESS_ATTR);
    kprintf("[  OK  ]\n");
    console_set_color(VGA_DEFAULT_ATTR);

    /* Step 2: Virtual Memory / Paging */
    init_step("Virtual memory (paging)",    do_vmm_init);

    /* Step 3: Kernel Heap */
    init_step("Kernel heap (SLAB)",         do_heap_init);

    /* Step 4: Interrupts (IDT + PIC) */
    init_step("Interrupt descriptor table", do_interrupts_init);
    init_step("Programmable interrupt ctrl",do_pic_init);

    /* Step 5: Timer */
    init_step("Programmable interval timer",do_timer_init);

    /* Step 6: Keyboard */
    init_step("PS/2 keyboard driver",       do_keyboard_init);

    /* Step 7: Filesystem */
    init_step("Virtual filesystem (VFS)",   do_vfs_init);
    kprintf("  Mounting initial filesystems...  ");
    mount_initial_fs();
    console_set_color(VGA_SUCCESS_ATTR);
    kprintf("[  OK  ]\n");
    console_set_color(VGA_DEFAULT_ATTR);

    /* Step 8: Network */
    init_step("Network stack (TCP/IP)",     do_net_init);

    /* Step 9: Start NIC and configure */
    start_networking();

    /* Step 10: Enable interrupts */
    sti();

    kprintf("\n");
    pmm_dump_info();

    console_set_color(VGA_ATTR(COLOR_LIGHT_GREEN, COLOR_BLACK));
    kprintf("\n  ***  EarlnuxOS kernel initialized successfully ***\n");
    console_set_color(VGA_DEFAULT_ATTR);

    kernel_initialized = true;

    /* Enter interactive shell */
    shell_run();

    /* Should never reach here */
    panic("kernel_main returned unexpectedly");
}
