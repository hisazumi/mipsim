#include <stdio.h>

/*------------------------------------------------------------*/
/* MIPS like Instruction Set
 */

enum {
    RET = 0, ADDI=1, ADD=2, BNE=3
};

#define R(op, rs, rt, rd) ((op << (32-6)) | (rs << (32-6-5)) | (rt << (32-6-5-5)) | (rd << (32-6-5-5-5)))
#define I(op, rs, rt, im) ((op << (32-6)) | (rs << (32-6-5)) | (rt << (32-6-5-5)) | (im & 0xffff))

#define OP(inst) ((inst >> (32-6)) & 0x3f)
#define RS(inst) ((inst >> (32-6-5)) & 0x1f)
#define RT(inst) ((inst >> (32-6-5-5)) & 0x1f)
#define RD(inst) ((inst >> (32-6-5-5-5)) & 0x1f)
#define IM(inst) (inst & 0x0000ffff)

#define ADD(rd, rs, rt)  R(ADD,  rs, rt, rd)
#define ADDI(rt, rs, im) I(ADDI, rs, rt, im)
#define BNE(rs, rt, im) I(BNE, rs, rt, im)

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
        BNE(1, 4, -2), //11: if (counter != r3) => jmp 9
        0 //8: terminator
};

static int pc = 0;
static int regs[32] = {0};

void dump (void) {
    printf ("==========\n");
    printf ("pc: %d\n", pc);
    printf ("reg: ");
    for (int i = 0; i < 10; i++) {
        printf ("%x ", regs[i]);
    }
    printf ("\n");
}

int reg_get (int n) {
    if (n == 0) {
        return 0;
    } else if (1 <= n && n <= 31) {
        return regs[n];
    } else {
        printf ("invalid register access: %d\n", n);
        return 0;
    }
}

void reg_set (int n, int value) {
    if (n == 0) {
    } else if (1 <= n && n <= 31) {
        regs[n] = value;
    } else {
        printf ("invalid register access: %d\n", n);
    }
}

int fetch (int addr) {
    return inst_mem[pc];
}

int main() {
    while (1) {
        int inst;

        inst = fetch (pc);

        if (inst == 0) {
            return 0;
        }

        switch (OP(inst)) {
        case ADDI:
            reg_set (RS(inst), reg_get (RT(inst)) + IM(inst));
            pc++;
            break;
        case ADD:
            reg_set (RS(inst), reg_get (RT(inst)) + reg_get (RD(inst)));
            pc++;
            break;
        case BNE:
            if (reg_get (RS(inst)) != reg_get (RT(inst))) {
                pc += IM(inst);
            }else{
                pc++;
            }
            break;
        }
    }

    return 0;
}

