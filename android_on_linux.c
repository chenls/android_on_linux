#define _GNU_SOURCE

#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mount.h>

/* 定义一个给 clone 用的栈，栈大小10M */
#define STACK_SIZE (10 * 1024 * 1024)
static char container_stack[STACK_SIZE];

char *container_args[] = {"/system/lib64/arm64/houdini64", "/system/bin/main_arm64"};

int container_main(void *arg)
{
    printf("Container [%5d] - inside the container!\n", getpid());

    if (mount("proc", "rootfs/proc", "proc", 0, NULL) != 0)
    {
        perror("proc");
    }

    // if (mount("binfmt_misc", "rootfs/proc/sys/fs/binfmt_misc", "binfmt_misc", 0, NULL) != 0)
    // {
    //     perror("binfmt_misc");
    // }

    if (chdir("./rootfs") != 0 || chroot("./") != 0)
    {
        perror("chdir/chroot");
    }

    setenv("ANDROID_DATA", "/data", 1);
    setenv("ANDROID_ROOT", "/", 1);

    printf("execv %s\n", container_args[0]);
    execv(container_args[0], container_args);
    perror("exec");
    printf("Something's wrong!  %s\n", container_args[0]);
    return 1;
}

void my_umount()
{
    umount("rootfs/proc/sys/fs/binfmt_misc");
    usleep(10);
    umount("rootfs/proc");
}

int main(int argc, char const *argv[])
{
    my_umount();
    printf("Parent [%5d] - start a container!\n", getpid());
    int container_pid = clone(container_main, container_stack + STACK_SIZE,
                              CLONE_NEWPID | SIGCHLD, NULL);
    waitpid(container_pid, NULL, 0);
    printf("Parent - container stopped!\n");
    my_umount();
    return 0;
}
