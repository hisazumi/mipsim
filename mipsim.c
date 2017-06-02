#include <stdio.h>

/*------------------------------------------------------------*/
/* MIPS like Instruction Set
 */

enum {
    ADDI=1, ADD, SLT, BNE, BEQ, LW, SW
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
#define BEQ(rs, rt, im) I(BEQ, rs, rt, im)
#define SLT(rd, rs, rt) R(SLT, rs, rt, rd)
#define LW(rt, im, rs) I(LW, rs, rt, im)
#define SW(rt, im, rs) I(SW, rs, rt, im)

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

int fetch (int addr) {
    if (0 <= addr && addr < sizeof(inst_mem)/sizeof(inst_mem[0])){
        return inst_mem[pc];
    }else{
        printf ("invalid memory access %d\n", addr);
        return 0;
    }
}

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

int main() {
    while (1) {
        int inst;

        inst = fetch (pc);

        if (inst == 0) {
            return 0;
        }

        switch (OP(inst)) {
        case ADDI:
            reg_set (RT(inst), reg_get (RS(inst)) + IM(inst));
            pc++;
            break;
        case ADD:
            reg_set (RD(inst), reg_get (RS(inst)) + reg_get (RT(inst)));
            pc++;
            break;
        case SLT:
            reg_set (RD(inst), reg_get (RS(inst)) < reg_get (RT(inst)));
            pc++;
            break;
        case BNE:
            if (reg_get (RS(inst)) != reg_get (RT(inst))) {
                // PC相対アドレッシングはやらない
                pc = IM(inst);
            }else{
                pc++;
            }
            break;
        case BEQ:
            if (reg_get (RS(inst)) == reg_get (RT(inst))) {
                // PC相対アドレッシングはやらない
                pc = IM(inst);
            }else{
                pc++;
            }
            break;
        case LW:
            reg_set (RT(inst), mem_get (RS(inst) + IM(inst)));
            pc++;
            break;
        case SW:
            mem_set (RS(inst) + IM(inst), reg_get (RT(inst)));
            pc++;
            break;
        default:
            printf ("invalid op: %x (in %x)\n", OP(inst), inst);
        }

        dump ();
    }

    return 0;
}

