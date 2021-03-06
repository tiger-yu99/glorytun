#include "common.h"
#include "str.h"

#include <stdio.h>

volatile sig_atomic_t gt_alarm;
volatile sig_atomic_t gt_reload;
volatile sig_atomic_t gt_quit;

static void
gt_quit_handler(int sig)
{
    switch (sig) {
    case SIGALRM:
        gt_alarm = 1;
        return;
    case SIGHUP:
        gt_reload = 1; /* FALLTHRU */
    default:
        gt_quit = 1;
    }
}

static void
gt_set_signal(void)
{
    struct sigaction sa = {
        .sa_flags = 0,
    };

    sigemptyset(&sa.sa_mask);

    sa.sa_handler = gt_quit_handler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);

    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
}

static int
gt_version(int argc, char **argv)
{
    printf(PACKAGE_VERSION "\n");
    return 0;
}

int
main(int argc, char **argv)
{
    gt_set_signal();

    struct {
        char *name;
        char *help;
        int (*call)(int, char **);
    } cmd[] = {
        {"show", "show all running tunnels", gt_show},
        {"bench", "start a crypto bench", gt_bench},
        {"bind", "start a new tunnel", gt_bind},
        {"set", "change tunnel properties", gt_set},
        {"keygen", "generate a new secret key", gt_keygen},
        {"path", "manage paths", gt_path},
        {"version", "show version", gt_version},
        {NULL}};

    if (argc < 2)
        return gt_show(argc, argv);

    for (int k = 0; cmd[k].name; k++) {
        if (!str_cmp(cmd[k].name, argv[1]))
            return cmd[k].call(argc - 1, argv + 1);
    }

    printf("unknown command `%s', available commands:\n\n", argv[1]);

    int len = 0;

    for (int k = 0; cmd[k].name; k++)
        len = MAX(len, (int)str_len(cmd[k].name, 32));

    for (int k = 0; cmd[k].name; k++)
        printf("  %-*s  %s\n", len, cmd[k].name, cmd[k].help);

    printf("\n");

    return 1;
}
