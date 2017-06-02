#include <stdio.h>

/*------------------------------------------------------------*/
/* MIPS like Processor Structure
 */

static int time = 0;
static int pc = 0;

/* register */
static int regs[32] = {0};

int reg_get (int n) {
    if (n == 0) {
        return 0;
    } else if (1 <= n && n <= 31) {
        return regs[n];
    } else {
        printf ("invalid register read: %d\n", n);
        return 0;
    }
}

void reg_set (int n, int value) {
    if (n == 0) {
    } else if (1 <= n && n <= 31) {
        regs[n] = value;
    } else {
        printf ("invalid register write: %d\n", n);
    }
}

/* memory */
static int data_mem[100] = {0};
int is_mem_addr_valid (int addr) {
    return 0 <= addr && addr <= sizeof(data_mem)/sizeof(data_mem[0]);
}

int mem_get (int addr) {
    if (is_mem_addr_valid (addr)) {
        return data_mem[addr];
    }else{
        printf ("invalid memory access %d\n", addr);
        return 0;
    }
}

void mem_set (int addr, int value) {
    if (is_mem_addr_valid (addr)) {
        data_mem[addr] = value;
    }else{
        printf ("invalid memory access %d\n", addr);
    }
}

/*------------------------------------------------------------*/
/* MIPS like Instruction Set
 */

// Macros for decode
#define OP(inst) ((inst >> (32-6)) & 0x3f)
#define RS(inst) ((inst >> (32-6-5)) & 0x1f)
#define RT(inst) ((inst >> (32-6-5-5)) & 0x1f)
#define RD(inst) ((inst >> (32-6-5-5-5)) & 0x1f)
#define IM(inst) (inst & 0x0000ffff)

enum {
    ADDI=1, ADD, SLT, BNE, BEQ, LW, SW, SRL, SLL
};

#define R(op, rs, rt, rd) ((op << (32-6)) | (rs << (32-6-5)) | (rt << (32-6-5-5)) | (rd << (32-6-5-5-5)))
#define I(op, rs, rt, im) ((op << (32-6)) | (rs << (32-6-5)) | (rt << (32-6-5-5)) | (im & 0xffff))

#define DEFINST(name) void name (int rs, int rt ,int rd, int im)

DEFINST(nop) {
}

#define ADD(rd, rs, rt)  R(ADD,  rs, rt, rd)
DEFINST(add) {
    reg_set (rd, reg_get (rs) + reg_get (rt));
}

#define ADDI(rt, rs, im) I(ADDI, rs, rt, im)
DEFINST(addi) {
    reg_set (rt, reg_get (rs) + im);
}

#define BNE(rs, rt, im) I(BNE, rs, rt, im)
DEFINST(bne) {
    // PC相対アドレッシングではなくて，直接アドレッシング
    if (reg_get (rs) != reg_get (rt)) {
        pc = im;
    }
}

#define BEQ(rs, rt, im) I(BEQ, rs, rt, im)
DEFINST (beq) {
    // PC相対アドレッシングではなくて，直接アドレッシング
    if (reg_get (rs) == reg_get (rt)) {
        pc = im;
    }
}

#define SLT(rd, rs, rt) R(SLT, rs, rt, rd)
DEFINST (slt) {
    reg_set (rd, reg_get (rs) + reg_get (rt));
}

#define LW(rt, im, rs) I(LW, rs, rt, im)
DEFINST (lw) {
    reg_set (rt, mem_get (rs + im));
}

#define SW(rt, im, rs) I(SW, rs, rt, im)
DEFINST (sw) {
    mem_set (rs + im, reg_get (rt));
}

#define SRL(rs, rt, im) I(SRL, rs, rt, im)
DEFINST (srl) {
    reg_set (rs, reg_get (rt) >> im);
}

#define SLL(rs, rt, im) I(SLL, rs, rt, im)
DEFINST (sll) {
    reg_set (rs, reg_get (rt) << im);
}

void (*instructions[])(int, int, int, int) = {
    0, addi, add, slt, bne, beq, lw, sw, srl, sll
};

/*------------------------------------------------------------*/
/* A sample code
 */

int inst_mem[] = {
        ADDI(0, 0, 0), //0: 0
        ADDI(1, 0, 1), //1: counter
        ADDI(2, 0, 0), //2; sum
        ADDI(3, 0, 1), //3: 1
        ADDI(4, 0, 10001), //4: 10001
        ADD(2, 2, 1), //9: r2 += r1
        ADD(1, 1, 3), //10: r1 += r3
        BNE(1, 4, 5), //11: if (counter != r3) => jmp 9
        0 //8: terminator
};

void dump (void) {
    printf ("==========\n");
    printf ("pc: %d time: %d\n", pc, time);
    printf ("reg: ");
    for (int i = 0; i < 10; i++) {
        printf ("%x ", regs[i]);
    }
    printf ("\n");
}

#define LENGTH(ary) (sizeof(ary)/sizeof(ary[0]))

int fetch (int addr) {
    if (0 <= addr && addr < LENGTH(inst_mem)){
        return inst_mem[pc];
    } else {
        printf ("invalid memory access %d\n", addr);
        return 0;
    }
}

void execute (int op, int rs, int rt, int rd, int im) {
    pc++;
    if (0 <= op && op <= LENGTH(instructions)) {
        instructions [op](rs, rt, rd, im);
    } else {
        printf ("invalid op: %x\n", op);
    }    
}

int main() {
    while (1) {
        int inst;

        inst = fetch (pc);

        if (inst == 0) {
            return 0;
        }

        // decode
        int op = OP(inst);
        int rs = RS(inst);
        int rt = RT(inst);
        int rd = RD(inst);
        int im = IM(inst);

        // execute
        execute (op, rs, rt, rd, im);
        time++;

        // print status
        dump ();
    }

    return 0;
}

