/* ============================================================================
 *  EarlnuxOS - Simple Math Evaluator for Shell
 * ============================================================================ */

#include <types.h>

static int is_digit(char c) { return c >= '0' && c <= '9'; }
static int is_op(char c) { return c == '+' || c == '-' || c == '*' || c == '/'; }

int shell_eval_math(const char *str, int *result) {
    /* Basic evaluator for "a op b" format */
    int a = 0, b = 0;
    char op = 0;
    const char *p = str;

    /* Skip leading whitespace */
    while (*p == ' ') p++;

    /* Read first number */
    if (!is_digit(*p)) return -1;
    while (is_digit(*p)) {
        a = a * 10 + (*p - '0');
        p++;
    }

    /* Skip whitespace */
    while (*p == ' ') p++;

    /* Read operator */
    if (!is_op(*p)) return -1;
    op = *p++;

    /* Skip whitespace */
    while (*p == ' ') p++;

    /* Read second number */
    if (!is_digit(*p)) return -1;
    while (is_digit(*p)) {
        b = b * 10 + (*p - '0');
        p++;
    }

    /* Final whitespace check */
    while (*p == ' ') p++;
    if (*p != '\0') return -1;

    /* Calculate */
    switch (op) {
        case '+': *result = a + b; break;
        case '-': *result = a - b; break;
        case '*': *result = a * b; break;
        case '/': 
            if (b == 0) return -2; /* Div by zero */
            *result = a / b; break;
        default: return -1;
    }

    return 0;
}
