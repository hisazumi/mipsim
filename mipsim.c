#include <stdio.h>
#include <assert.h>

#define LENGTH(ary) (sizeof(ary)/sizeof(ary[0]))

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
    return 0 <= addr && addr <= LENGTH(data_mem);
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
    ADDI=1, ADD, SLT, BNE, BEQ, LW, SW, SRL, SLL, ANDI
};

#define R(op, rs, rt, rd) ((op << (32-6)) | (rs << (32-6-5)) | (rt << (32-6-5-5)) | (rd << (32-6-5-5-5)))
#define I(op, rs, rt, im) ((op << (32-6)) | (rs << (32-6-5)) | (rt << (32-6-5-5)) | (im & 0xffff))

void nop (int rs, int rt ,int rd, int im) {
}

#define ADD(rd, rs, rt)  R(ADD,  rs, rt, rd)
void add (int rs, int rt ,int rd, int im) {
    reg_set (rd, reg_get (rs) + reg_get (rt));
}

void add_test (void) {
    reg_set (1, 10);
    reg_set (2, 20);
    add (1, 2, 3, 0);
    assert (reg_get (3) == 30);
}

#define ADDI(rt, rs, im) I(ADDI, rs, rt, im)
void addi (int rs, int rt ,int rd, int im) {
    reg_set (rt, reg_get (rs) + im);
}

void addi_test (void) {
    reg_set (1, 10);
    addi (1, 2, 0, 20);
    assert (reg_get (2) == 30);
}

#define BNE(rs, rt, im) I(BNE, rs, rt, im)
void bne (int rs, int rt ,int rd, int im) {
    // PC相対アドレッシングではなくて，直接アドレッシング
    if (reg_get (rs) != reg_get (rt)) {
        pc = im;
    }
}

void bne_test (void) {
    pc = 0;
    // not equal
    reg_set (1, 10);
    reg_set (2, 20);
    bne (1, 2, 0, 100);
    assert (pc == 100);
    // equal
    reg_set (1, 10);
    reg_set (2, 10);
    bne (1, 2, 0, 100);
    assert (pc == 100);
}

#define BEQ(rs, rt, im) I(BEQ, rs, rt, im)
void beq (int rs, int rt ,int rd, int im) {
    // PC相対アドレッシングではなくて，直接アドレッシング
    if (reg_get (rs) == reg_get (rt)) {
        pc = im;
    }
}

void beq_test (void) {
    pc = 0;
    // not equal
    reg_set (1, 10);
    reg_set (2, 20);
    beq (1, 2, 0, 100);
    assert (pc == 0);
    // equal
    reg_set (1, 10);
    reg_set (2, 10);
    beq (1, 2, 0, 100);
    assert (pc == 100);
}

#define SLT(rd, rs, rt) R(SLT, rs, rt, rd)
void slt (int rs, int rt ,int rd, int im) {
    reg_set (rd, reg_get (rs) < reg_get (rt));
}

void slt_test (void) {
    // less than
    reg_set (1, 10);
    reg_set (2, 20);
    slt (1, 2, 3, 0);
    assert (reg_get (3) != 0);
    // not less than
    reg_set (1, 10);
    reg_set (2, 10);
    slt (1, 2, 3, 0);
    assert (reg_get (3) == 0);
}

#define LW(rt, im, rs) I(LW, rs, rt, im)
void lw (int rs, int rt ,int rd, int im) {
    reg_set (rt, mem_get (reg_get (rs) + im));
}

void lw_test (void) {
    mem_set (10, 1234);
    // without offset
    reg_set (1, 10); // address
    lw (1, 2, 0, 0);
    assert (reg_get (2) == 1234);
    // with offset
    reg_set (1, 0); // address
    lw (1, 2, 0, 10);
    assert (reg_get (2) == 1234);
}

#define SW(rt, im, rs) I(SW, rs, rt, im)
void sw (int rs, int rt ,int rd, int im) {
    mem_set (reg_get (rs) + im, reg_get (rt));
}

void sw_test (void) {
    // without offset
    reg_set (1, 10); // address
    reg_set (2, 1234); // data
    sw (1, 2, 0, 0);
    assert (mem_get (10) == 1234);
    // with offset
    sw (1, 2, 0, 1);
    assert (mem_get (11) == 1234);
}


#define SRL(rs, rt, im) I(SRL, rs, rt, im)
void srl (int rs, int rt ,int rd, int im) {
    reg_set (rs, reg_get (rt) >> im);
}

void srl_test (void) {
    reg_set (1, 10);
    srl (2, 1, 0, 1);
    assert (reg_get (2) == 5);
}

#define SLL(rs, rt, im) I(SLL, rs, rt, im)
void sll (int rs, int rt ,int rd, int im) {
    reg_set (rs, reg_get (rt) << im);
}

void sll_test (void) {
    reg_set (1, 10);
    sll (2, 1, 0, 1);
    assert (reg_get (2) == 20);
}

#define ANDI(rs, rt, im) I(ANDI, rs, rt, im)
void andi (int rs, int rt, int rd, int im) {
    reg_set (rs, reg_get (rt) & im);
}

void andi_test (void) {
    reg_set (1, 0xff);
    andi (2, 1, 0, 1);
    assert (reg_get (2) == 0x01);
}

void (*instructions[])(int, int, int, int) = {
    0, addi, add, slt, bne, beq, lw, sw, srl, sll, andi
};

void (*tests[])(void) = {
    add_test, addi_test, bne_test, beq_test, slt_test, lw_test, sw_test, srl_test, sll_test, andi_test
};

/*------------------------------------------------------------*/
/* A sample code
 */

int inst_mem[] = {
    ADDI(1, 0, 10), // constant 10
    ADDI(2, 0, 32), // constant 32
    ADD(3, 0, 0), // i
    ADD(4, 0, 0), // j
    ADD(5, 0, 0), // tmp
    ADD(7, 0, 0), // address A
    ADDI(6, 0, 0), // bits = 0
    ADDI(3, 0, 0), // i = 0
//8 LOOP1
    SLT(10, 3, 1), // i >= 10のときEXITに分岐
    BEQ(10, 0, 25), // goto EXIT
    ADD(11, 3, 3), // data[i]のアドレスを計算
    ADD(11, 11, 11),
    ADD(12, 11, 7),
    LW(5, 0, 12),
    ADDI(4, 0, 0), // j = 0
//15 LOOP2
    SLT(13, 4, 2), //  j >= 32のときNEXTに分岐
    BEQ(13, 0, 23), // goto NEXT
    ANDI(14, 5, 1), // (tmp & 0x1) != 0x0のときCONTに分岐
    BNE(14, 0, 20), // goto CONT
    ADDI(6, 6, 1), // bits++
//20 CONT
    SRL(5, 5, 1), // tmp >>= 1
    ADDI(4, 4, 1), // j++
    BEQ(0, 0, 15), // goto LOOP2
//23 NEXT
    ADDI(3, 3, 1), // i++
    BEQ(0, 0, 8), // goto LOOP1
//25 EXIT
    0
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

void do_test (void) {
    int i;
    for (i=0 ; i<LENGTH(tests) ; i++) {
        tests[i] ();
    }
}

int main() {
    do_test ();

    pc = 0;

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

