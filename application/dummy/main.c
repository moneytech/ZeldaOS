/*
 * Copyright (c) 2018 Jie Zheng
 */
#include <stdint.h>
#define asm __asm__
int cute = 1;
int bar[1024*1024*10];
#define COM1_PORT 0x3f8

uint8_t
inb(uint16_t portid)
{
    uint8_t ret;
    asm volatile("inb %1, %0;"
        :"=a"(ret)
        :"Nd"(portid));
    return ret;
}

void
outb(uint16_t portid, uint8_t val)
{
    asm volatile("outb %%al, %%dx;"
        :
        :"a"(val), "Nd"(portid));
}


int
is_transmit_empty(void)
{
    return inb(COM1_PORT + 5) & 0x20;
}

void
write_serial(uint8_t a)
{
    while (is_transmit_empty() == 0);
    outb(COM1_PORT,a);
}


void
foo(void)
{
    __asm__ volatile("movl $0x1, %%eax;"
        "int $0x87;"
        :
        :
        :"%eax");
}
void
print_serial(char * str)
{
     while(*str)
         write_serial(*str++);
}

void
print_hexdecimal(uint32_t val)
{
    int idx = 0;
    uint8_t dict[] = "0123456789abcdef";
    uint8_t * ptr = (uint8_t *)&val;

    for(idx = 0, ptr += 3; idx < 4; idx++, ptr--) {
        write_serial(dict[((*ptr) >> 4) & 0xf]);
        write_serial(dict[*ptr & 0xf]);
    }
}
extern void * _zelda_constructor_init_start;
extern void * _zelda_constructor_init_end;

static void
interrupt_handler(int signal)
{
    char buffer[] = "ZELDA=/root/ZeldaOS/ make _zelda_constructor_init_start";
    int fd = open("/dev//ptm1", O_RDWR);
    print_serial("Application Signal interrupted\n");
    write(fd, buffer, sizeof(buffer));
    close(fd);
    sleep(100);
    print_serial("Application Signal interrupted ends\n");
    kill(-1, SIGKILL);
}
int main(int argc, char *argv[])
{
    int index = 0;
    int32_t ret = 0;
    int32_t pid = getpid();
    print_serial("pid:");
    print_hexdecimal(pid);
    print_serial("\n");
    ret = signal(SIGINT, interrupt_handler);
    while(0) {
        print_serial(argv[0]);
        print_serial("\n");
        sleep(10000);
        //kill(-1, SIGINT);
    }
    //for (index = 0; index < 5; index ++)
    while (1){
        void * boundary = sbrk(4096); 
        print_serial("sbrk:0x");
        print_hexdecimal((uint32_t)boundary);
        print_serial("\n");
        if (boundary == (void *)-1)
            break;
    }
    if (1) {
        int idx = 0;
        char buffer[256];
        int fd = open("/home/foo", O_RDWR|O_CREAT, 0x0);
        //int fd = open("/etc/dummy.cfg", O_RDWR);
        print_serial("opened fd:");
        print_hexdecimal(fd);
        print_serial("\n");
        for (idx = 0; idx < 256; idx++)
            buffer[idx] = 0;
        idx = read(fd, buffer,16);
        print_serial("read file:");
        print_serial(buffer);
        print_serial("\n");
        idx = write(fd, "Hello world,jjjzz",16);
        print_serial("write count:");
        print_hexdecimal(idx);
        print_serial("\n");

        //fd = open("/home/foo", O_RDWR);
        print_hexdecimal(fd);
        print_serial("\n");
        for (idx = 0; idx < 256; idx++)
            buffer[idx] = 0;
        lseek(fd, 1, 0);
        idx = read(fd, buffer,15);
        print_serial("read file:");
        print_serial(buffer);
        print_serial("\n");
        print_serial("read count:");
        print_hexdecimal(idx);
        print_serial("\n");

        {
            int fd0 = open("/usr/bin/dummy", 0);
            struct stat _stat;
            int rc = fstat(fd0, &_stat);
            print_hexdecimal(fd0);
            print_serial("\n");
            print_serial("file path stat:");
            print_hexdecimal(_stat.st_size);
            print_serial("\n");
        }
        {
            int fd1 = open("/dev/ptm1", O_RDWR);
            //int fd1 = open("/dev/serial0", O_RDWR);
            int writerc = write(fd1, "written to serial0\n", 20);
            print_serial("serial write:");
            print_hexdecimal(writerc);
            print_serial("\n");
        }
        close(fd);
    }
    {
        int idx;
        int read_rc;
        char buff[32];
        int fd2 = open("/dev/ptm0", O_RDWR);
        while (1) {
            for (idx = 0; idx < 32; idx++)
                buff[idx] = 0;
            read_rc = read(fd2, buff, 32);
            print_hexdecimal(read_rc);
            print_serial(":");
            print_serial(buff);
            print_serial("\n");
        }
    }
    while(1) {
        sleep(100);
    }
#if 0
    *(uint32_t *)0x42802000 = 0;
    int _start = (int)&_zelda_constructor_init_start;
    int _end = (int)&_zelda_constructor_init_end;
    int addr = 0;
    int idx = 0;
    print_serial("hello Application\n");
    for(addr = _start; addr < _end; addr += 4) {
        ((void (*)(void))*(int*)addr)();
    }
    for(idx = 0; idx < argc; idx++) {
        print_hexdecimal(argv[idx]);
        print_serial("\n");
    }
    print_serial("End of Application\n");
    *(uint32_t *)0x507 = 0;
#endif
    return 0;
}
