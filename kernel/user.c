/* ============================================================================
 *  EarlnuxOS - User Management & Authentication
 * kernel/user.c
 * ============================================================================ */

#include <kernel/kernel.h>
#include <lib/string.h>
#include <fs/vfs.h>

extern char keyboard_getchar(void);
extern int shell_readline(char *buf, size_t max);

static char current_user[32] = "guest";

void user_init(void) {
    /* Ensure /etc exists */
    vfs_mkdir("/etc", 0755);
}

static void get_password(char *buf, size_t max) {
    size_t pos = 0;
    while (pos < max - 1) {
        char c = keyboard_getchar();
        if (c == '\n') break;
        if (c == '\b' && pos > 0) {
            pos--;
            kprintf("\b \b");
        } else if (c >= 32) {
            buf[pos++] = c;
            kprintf("*"); /* Masked password */
        }
    }
    buf[pos] = '\0';
    kprintf("\n");
}

int user_login_sequence(void) {
    char username[32];
    char password[32];
    
    console_set_color(VGA_ATTR(COLOR_LIGHT_GREEN, COLOR_BLACK));
    kprintf("\n[ SECURITY ] System is Locked. Please Authenticate.\n\n");
    console_set_color(VGA_DEFAULT_ATTR);

    /* Check if /etc/passwd exists */
    int fd = vfs_open("/etc/passwd", O_RDONLY, 0);
    if (fd < 0) {
        /* First run - Create admin */
        kprintf("No user accounts found. Initializing Administrative Setup...\n");
        kprintf("New Username: ");
        shell_readline(username, 32);
        kprintf("New Password: ");
        get_password(password, 32);

        /* Save to /etc/passwd */
        fd = vfs_open("/etc/passwd", O_WRONLY | O_CREAT, 0644);
        if (fd >= 0) {
            vfs_write(fd, username, strlen(username));
            vfs_write(fd, ":", 1);
            vfs_write(fd, password, strlen(password));
            vfs_close(fd);
        }
        kprintf("\nAccount '%s' created successfully.\n", username);
    } else {
        /* Existing user - Login */
        char stored_creds[128];
        vfs_read(fd, stored_creds, sizeof(stored_creds)-1);
        vfs_close(fd);

        while (1) {
            kprintf("Username: ");
            shell_readline(username, 32);
            kprintf("Password: ");
            get_password(password, 32);

            /* Very basic check: username:password */
            char check[128];
            ksnprintf(check, sizeof(check), "%s:%s", username, password);
            if (strstr(stored_creds, check)) {
                break;
            }
            console_set_color(VGA_ATTR(COLOR_RED, COLOR_BLACK));
            kprintf("Login failed. Access Denied.\n");
            console_set_color(VGA_DEFAULT_ATTR);
        }
    }

    strncpy(current_user, username, 32);
    return 0;
}

const char *user_get_current(void) {
    return current_user;
}
