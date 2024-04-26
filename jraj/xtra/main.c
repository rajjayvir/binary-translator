#include <stdio.h>
#include <stdlib.h>
#include "xis.h"

char *reg_map64[] = {
        "%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%rdi", "%r8 ", "%r9 ",
        "%r10", "%r11", "%r12", "%r13", "%r14", "%r15", "%rbp", "%rsp", "%rip"
};

int debug = 0;
int label = 0;

void translateOpcode(unsigned char opcode, unsigned char operand1, unsigned char operand2) {
    switch (opcode) {
        case I_RET:
            printf("ret\n");
            break;
        case I_CLD:
            debug = 0;
            break;
        case I_STD:
            debug = 1;
            break;
        case I_NEG:
            printf("neg %s\n", reg_map64[operand1]);
            break;
        case I_NOT:
            printf("not %s\n", reg_map64[operand1]);
            break;
        case I_INC:
            printf("inc %s\n", reg_map64[operand1]);
            break;
        case I_DEC:
            printf("dec %s\n", reg_map64[operand1]);
            break;
        case I_PUSH:
            printf("push %s\n", reg_map64[operand1]);
            break;
        case I_POP:
            printf("pop %s\n", reg_map64[operand1]);
            break;
        case I_OUT:
            printf("out %s\n", reg_map64[operand1]);
            break;
        case I_BR:
            printf("br .L%04X\n", operand1);
            label += (signed char) operand1 - 2;
            break;
        case I_JR:
            printf("jr $%u\n", operand1);
            break;
        case I_ADD:
            printf("add %s, %s\n", reg_map64[operand1], reg_map64[operand2]);
            break;
        case I_SUB:
            printf("sub %s, %s\n", reg_map64[operand1], reg_map64[operand2]);
            break;
        case I_MUL:
            printf("imul %s, %s\n", reg_map64[operand1], reg_map64[operand2]);
            break;
        case I_AND:
            printf("and %s, %s\n", reg_map64[operand1], reg_map64[operand2]);
            break;
        case I_OR:
            printf("or %s, %s\n", reg_map64[operand1], reg_map64[operand2]);
            break;
        case I_XOR:
            printf("xor %s, %s\n", reg_map64[operand1], reg_map64[operand2]);
            break;
        case I_TEST:
            printf("test %s, %s\n", reg_map64[operand1], reg_map64[operand2]);
            printf("setne %%r15b\n"); // if result is not zero
            break;
        case I_CMP: // cmp instruction
            printf("cmp %s, %s\n", reg_map64[operand1], reg_map64[operand2]);
            printf("setg %%r15b\n"); // if operand1 > operand2
            break;
        case I_EQU: // equ instruction
            printf("cmp %s, %s\n", reg_map64[operand1], reg_map64[operand2]);
            printf("setz %%r15b\n"); // if operand1 == operand2
            break;
        case I_MOV:
            printf("mov %s, %s\n", reg_map64[operand1], reg_map64[operand2]);
            break;
        case I_LOAD:
            printf("load %s, %s\n", reg_map64[operand1], reg_map64[operand2]);
            break;
        case I_STOR:
            printf("stor %s, %s\n", reg_map64[operand1], reg_map64[operand2]);
            break;
        case I_LOADB:
            printf("loadb %s, %s\n", reg_map64[operand1], reg_map64[operand2]);
            break;
        case I_STORB:
            printf("storb %s, %s\n", reg_map64[operand1], reg_map64[operand2]);
            break;
        default:
            printf("Unknown opcode: 0x%02X\n", opcode);
            break;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "rb");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    printf(".globl test\n");
    printf("test:\n");
    printf("push %%rbp\n");
    printf("mov %%rsp, %%rbp\n");

    unsigned char instruction[2];

    while (fread(&instruction, sizeof(unsigned char), 2, file) == 2) {
        printf(".L%04X:\n", label);

        if (debug) {
            printf("call debug\n");
        }
        if (instruction[0] == 0x00 && instruction[1] == 0x00) {
            break;
        }

        unsigned char opcode = instruction[0];
        unsigned char rS = instruction[1];
        unsigned char rD = instruction[1];

        unsigned char twoSignificantBitFromOpcode = opcode >> 6;
        unsigned char thirdSignificantBitFromOpcode = opcode >> 5;

        switch (twoSignificantBitFromOpcode) {
            // 0-operand instruction encoding
            case 0x00:
                translateOpcode(opcode, rS, rD);
                label += 2;
                break;
                // 1 operand (opcode = 01-0 or 01-1)
            case 0x01:
                switch (thirdSignificantBitFromOpcode) {
                    // 01 - 0
                    case 0x02:
                        rS = rS >> 4;
                        rD = rD & 0x0F;
                        translateOpcode(opcode, rS, rD);
                        break;
                        //01-1
                    case 0x03:
                        translateOpcode(opcode, rS, rD);
                        break;
                }
                label += 4;

                break;
                // 2 - operand ( = 10)
            case 0x02:
                rS = rS >> 4;
                rD = rD & 0x0F;
                translateOpcode(opcode, rS, rD);
                label += 4;
                break;
                // extended instruction encoding. ( = 11-0 or 11-1)
            case 0x03:
                fread(&instruction, sizeof(unsigned char), 2, file);
                unsigned short combinedWord = (instruction[0] << 8) | instruction[1];
                label += 4;
                switch (thirdSignificantBitFromOpcode) {
                    //11-0
                    case 0x06:
                        switch (opcode) {
                            case I_JMP:
                                printf("jmp .L%04X\n", combinedWord);
                                break;
                            case I_CALL:
                                printf("call .L%04X\n", combinedWord);
                                break;
                        }
                        break;
                        //11-1
                    case 0x07:
                        rS = rS >> 4;
                        debug = 1;
                        printf("mov $%hu, %s\n", combinedWord, reg_map64[rS]);
                        break;
                }
                break;
        }
    }

    printf("pop %%rbp\n");
    printf("ret\n");

    fclose(file);
    return 0;
}
