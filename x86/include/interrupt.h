/*
 * Copyright (c) 2018 Jie Zheng
 */
#ifndef _INTERRUPT_H
#define _INTERRUPT_H
#include <lib/include/types.h>

#define EFLAGS_CF 0x0001
#define EFLAGS_ONE 0x0002
#define EFLAGS_PARITY 0x0004
#define EFLAGS_ADJUST 0x0010
#define EFLAGS_ZERO 0x0040
#define EFLAGS_SIGN 0x0080
#define EFLAGS_TRAP 0x0100
#define EFLAGS_INTERRUPT 0x0200
#define EFLAGS_DIRECTION 0x0400
#define EFLAGS_OVERFLOW 0x0800
#define EFLAGS_NESTTASK 0x4000


struct x86_cpustate {
    /*
     * all general purpose registers, the order:
     * http://www.fermimn.gov.it/linux/quarta/x86/pusha.htm
     */
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_temp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    /*
     * segment register excluding cs and ss
     */
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;
    /*
     * the interrupt number which cause context switch to a interrupt gate
     */
    uint32_t vector;
    /*
     * in whatever cases, an errorcode should be put in the stack
     * never the code shoud call int n instructuion where nth exception
     * generates an errorcode, or the program can not find the EIP to return.
     */
    uint32_t errorcode;
    /*
     * when interrupt/exception is happening, the eflags/cs/eip is also pushed
     * into the PL0 stack.
     */
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    /*
     * inter-privilege level context switching will need a stack switched to
     * privilege level 0, and the privilege level 3 stack is stored in the
     * PL 0 stack.
     */
    uint32_t esp;
    uint32_t ss;
}__attribute__((packed));

#define IDT_SIZE 256
#define GATE_SIZE_32 0x0E

struct interrupt_gate_entry {
    uint32_t offset_low:16;
    uint32_t selector:16;
    uint32_t reserved0:8;
    uint32_t gate_size:5;
    uint32_t DPL:2;
    uint32_t present:1;
    uint32_t offset_high:16;
}__attribute__((packed));

struct idt_pointer {
    uint16_t limit;
    uint32_t base;
}__attribute__((packed));

typedef void int_handler(struct x86_cpustate *);

void register_interrupt_handler(int vector_number,
    int_handler * handler,
    char * description);

static inline void
sti(void)
{
    asm volatile("sti;");
}

static inline void
cli(void)
{
    asm volatile("cli;");
}
#define PIC_MASTER_COMMAND_PORT 0x20
#define PIC_MASTER_DATA_PORT 0x21
#define PIC_SLAVE_COMMAND_PORT 0xa0
#define PIC_SLAVE_DATA_PORT 0xa1

void idt_init(void);

void int0(void);
void int1(void);
void int2(void);
void int3(void);
void int4(void);
void int5(void);
void int6(void);
void int7(void);
void int8(void);
void int9(void);
void int10(void);
void int11(void);
void int12(void);
void int13(void);
void int14(void);
void int15(void);
void int16(void);
void int17(void);
void int18(void);
void int19(void);
void int20(void);
void int21(void);
void int22(void);
void int23(void);
void int24(void);
void int25(void);
void int26(void);
void int27(void);
void int28(void);
void int29(void);
void int30(void);
void int31(void);
void int32(void);
void int33(void);
void int34(void);
void int35(void);
void int36(void);
void int37(void);
void int38(void);
void int39(void);
void int40(void);
void int41(void);
void int42(void);
void int43(void);
void int44(void);
void int45(void);
void int46(void);
void int47(void);
void int48(void);
void int49(void);
void int50(void);
void int51(void);
void int52(void);
void int53(void);
void int54(void);
void int55(void);
void int56(void);
void int57(void);
void int58(void);
void int59(void);
void int60(void);
void int61(void);
void int62(void);
void int63(void);
void int64(void);
void int65(void);
void int66(void);
void int67(void);
void int68(void);
void int69(void);
void int70(void);
void int71(void);
void int72(void);
void int73(void);
void int74(void);
void int75(void);
void int76(void);
void int77(void);
void int78(void);
void int79(void);
void int80(void);
void int81(void);
void int82(void);
void int83(void);
void int84(void);
void int85(void);
void int86(void);
void int87(void);
void int88(void);
void int89(void);
void int90(void);
void int91(void);
void int92(void);
void int93(void);
void int94(void);
void int95(void);
void int96(void);
void int97(void);
void int98(void);
void int99(void);
void int100(void);
void int101(void);
void int102(void);
void int103(void);
void int104(void);
void int105(void);
void int106(void);
void int107(void);
void int108(void);
void int109(void);
void int110(void);
void int111(void);
void int112(void);
void int113(void);
void int114(void);
void int115(void);
void int116(void);
void int117(void);
void int118(void);
void int119(void);
void int120(void);
void int121(void);
void int122(void);
void int123(void);
void int124(void);
void int125(void);
void int126(void);
void int127(void);
void int128(void);
void int129(void);
void int130(void);
void int131(void);
void int132(void);
void int133(void);
void int134(void);
void int135(void);
void int136(void);
void int137(void);
void int138(void);
void int139(void);
void int140(void);
void int141(void);
void int142(void);
void int143(void);
void int144(void);
void int145(void);
void int146(void);
void int147(void);
void int148(void);
void int149(void);
void int150(void);
void int151(void);
void int152(void);
void int153(void);
void int154(void);
void int155(void);
void int156(void);
void int157(void);
void int158(void);
void int159(void);
void int160(void);
void int161(void);
void int162(void);
void int163(void);
void int164(void);
void int165(void);
void int166(void);
void int167(void);
void int168(void);
void int169(void);
void int170(void);
void int171(void);
void int172(void);
void int173(void);
void int174(void);
void int175(void);
void int176(void);
void int177(void);
void int178(void);
void int179(void);
void int180(void);
void int181(void);
void int182(void);
void int183(void);
void int184(void);
void int185(void);
void int186(void);
void int187(void);
void int188(void);
void int189(void);
void int190(void);
void int191(void);
void int192(void);
void int193(void);
void int194(void);
void int195(void);
void int196(void);
void int197(void);
void int198(void);
void int199(void);
void int200(void);
void int201(void);
void int202(void);
void int203(void);
void int204(void);
void int205(void);
void int206(void);
void int207(void);
void int208(void);
void int209(void);
void int210(void);
void int211(void);
void int212(void);
void int213(void);
void int214(void);
void int215(void);
void int216(void);
void int217(void);
void int218(void);
void int219(void);
void int220(void);
void int221(void);
void int222(void);
void int223(void);
void int224(void);
void int225(void);
void int226(void);
void int227(void);
void int228(void);
void int229(void);
void int230(void);
void int231(void);
void int232(void);
void int233(void);
void int234(void);
void int235(void);
void int236(void);
void int237(void);
void int238(void);
void int239(void);
void int240(void);
void int241(void);
void int242(void);
void int243(void);
void int244(void);
void int245(void);
void int246(void);
void int247(void);
void int248(void);
void int249(void);
void int250(void);
void int251(void);
void int252(void);
void int253(void);
void int254(void);

#endif
