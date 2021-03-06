/*
 * Copyright (c) 2018 Jie Zheng
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zelda.h>

int32_t
sleep(int32_t milisecond);

int
main(void)
{
    int index = 0;
    for (index = 0; index < 5; index++) {
        printf("%d: Hello World\n", index);
        sleep(2000);
    }
    return 0;
}
#if 0
static void
interrupt_handler(int signal)
{
    printf("Application Signal interrupted\n");
    sleep(100);
    printf("Application Signal interrupted ends\n");
    kill(-1, SIGKILL);
}
int main(int argc, char *argv[])
{
    char ch;
    int32_t ret = 0;
    char buffer[64];
    char * hint = "[link@hyrule.kingdom zelda] #";
    ret = signal(SIGINT, interrupt_handler);
    printf("pid:%d\n", getpid());
    pseudo_terminal_enable_master();
    pseudo_terminal_foreground(1);
    pseudo_terminal_write_slave(hint, 17);
    {
        uint8_t cwd[256];
        printf("cwd:%s\n", getcwd(cwd, 256));
        int fd = open("/dev/console", O_WRONLY);
        write(fd, "Hello Console\n", 14);
    }
    {
        if (argc > 1 && !strcmp(argv[1], "execve")) {
            uint8_t *argv[] = {
                "/usr/bin/dummy",
                "execve",
                "sub program",
                NULL,
            };
            extern uint8_t ** environ;
            uint8_t ** envp = environ;
            //printf("going to execute an program:%d\n", execve("/usr/bin/dummy", argv, envp));
        }
    }
    while(0) {
        memset(buffer, 0x0, sizeof(buffer));
        ret = read(0, buffer, 64);
        write(1, buffer, ret);
        if (buffer[ret -1] == '\n') {
            clear_screen();
            write(1, hint, strlen(hint));
        }
    }
    while(1)
        sleep(100);
    return 0;
}
#endif
