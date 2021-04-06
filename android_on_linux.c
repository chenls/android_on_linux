#define _GNU_SOURCE

#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define STACK_SIZE (10 * 1024 * 1024)
static char g_container_stack[STACK_SIZE];

static const char *gp_container_args[10] = {"/system/lib64/arm64/houdini64"};

static int container_main(void *arg) {
    (void)arg;

    if (mount("proc", "rootfs/proc", "proc", 0, NULL) != 0) {
        perror("proc");
    }

    if (chdir("./rootfs") != 0 || chroot("./") != 0) {
        perror("chdir/chroot");
    }

    setenv("ANDROID_DATA", "/data", 1);
    setenv("ANDROID_ROOT", "/", 1);

    printf("execv %s\n", gp_container_args[1]);
    execv(gp_container_args[0], (char **)gp_container_args);
    perror("exec");
    printf("Something's wrong!  %s\n", gp_container_args[0]);
    return 1;
}

static void my_umount() {
    umount("rootfs/proc");
}

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        printf("%s need parameters: exe path!\n", argv[0]);
        return -1;
    }
    gp_container_args[1] = argv[1];
    for (int i = 2; i <= argc; i++) {
        gp_container_args[i] = argv[i - 1];
    }

    my_umount();
    // printf("Parent [%5d] - start a container!\n", getpid());
    int container_pid = clone(container_main, g_container_stack + STACK_SIZE, CLONE_NEWPID | SIGCHLD, NULL);
    waitpid(container_pid, NULL, 0);
    // printf("Parent - container stopped!\n");
    my_umount();
    return 0;
}
