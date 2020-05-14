#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "funciones.h"
    int reg[16];
    int RAM[2000];
    void (*funciones[144])(int codop, int op1, int op2, int RAM[], int reg[]);
void cargaMemoria(int argc, char *argv[]);
void imprimeReg();
void ejecuta();
void flag();
void BuscaImprime(int instr, int op1, int op2);
int main(int argc, char *argv[])
{

printf("fede gay");
printf("fede re contra gay");
    funciones[1]=&mov;
    funciones[2]=&add;
    funciones[3]=&sub;
    funciones[4]=&mul;
    funciones[5]=&DIV;
    funciones[6]=&mod;
    funciones[0x13]=&cmp;
    funciones[0x17]=&SWAP;
    funciones[0x19]=&rnd;
    funciones[0x31]=&and;
    funciones[0x32]=&or;
    funciones[0x33]=&not;
    funciones[0x34]=&xor;
    funciones[0x37]=&shl;
    funciones[0x38]=&shr;
    funciones[0x20]=&jmp;
    funciones[0x21]=&je;
    funciones[0x22]=&jg;
    funciones[0x23]=&jl;
    funciones[0x24]=&jz;
    funciones[0x25]=&jp;
    funciones[0x26]=&JN;
    funciones[0x27]=&jnz;
    funciones[0x28]=&jnp;
    funciones[0x29]=&jnn;
    funciones[0x81]=&sys;
    funciones[0x8f]=&stop;

    cargaMemoria(argc, argv);
    //imprimeReg();
    ejecuta();
    if((argc == 3) && (strcmp(argv[2], "-d") == 0)){
        diccionario();
        flag();
    }
    return 0;
}

void cargaMemoria(int argc, char *argv[]){
    int aux, swapped;
    int i;
    FILE *arch;

    if ((arch=fopen(argv[1], "rb"))==NULL)
        printf("el archivo no se encontro");
    else
        {
        for (i=0;i<16;i++){
            fread(&aux,sizeof(int),1,arch);
            swapped = ((aux>>24)&0xff) |
                    ((aux<<8)&0xff0000) |
                    ((aux>>8)&0xff00) |
                    ((aux<<24)&0xff000000);
            reg[i]=swapped;
        }
        for (i=0;i<2000;i++){
            fread(&aux,sizeof(int),1,arch);
            swapped = ((aux>>24)&0xff) |
                    ((aux<<8)&0xff0000) |
                    ((aux>>8)&0xff00) |
                    ((aux<<24)&0xff000000);
            RAM[i]=swapped;
        }
        fclose(arch);
        }
}

void imprimeReg(){
    int i;
    for (i=0;i<16;i++)
        printf("%d\n", reg[i]);
    printf("\n");
    for (i=0;i<32;i++)
        printf("%d\n", RAM[i]);
}

void ejecuta()
{
    int instruccion, op1, op2;

    while((reg[4] >= 0) && (reg[4] < reg[2])){
        instruccion = RAM[reg[4]];
        op1=RAM[reg[4]+1];
        op2=RAM[reg[4]+2];
        if((instruccion == 143) || ((((instruccion>>16)==5)||((instruccion>>16)==6)) && (devuelveValor(instruccion&0xff, op2, RAM, reg)==0)))
            stop(0,0,0, RAM, reg);
        else{
            reg[4] += 3;
            funciones[instruccion >> 16](instruccion, op1, op2, RAM, reg);
        }
    }
}

void flag()
{
    int instruccion, op1, op2, ip=0;

    printf("\n");
    while((ip>=0) && (ip < reg[2])){
        instruccion= RAM[ip];
        op1=RAM[ip+1];
        op2=RAM[ip+2];
        printf("[%04x %04x] \t %04x %04x %04x %04x %04x %04x \t",
               ip>>16, ip&0x0000FFFF, instruccion>>16, instruccion&0x0000FFFF, op1>>16, op1&0x0000FFFF, (op2&0xffff0000)>>16, op2&0x0000FFFF);
        ip+=3;
        BuscaImprime(instruccion, op1, op2);
    }
}

void BuscaImprime(int instr, int op1, int op2)
{
    int mnemo, segMemoria;

    mnemo= instr>>16;
    printf("%s \t", mnemonicos[mnemo]);
    if(mnemo != 0x8f){
        if(((instr>>8)&0xff)==1)
            printf("%s", registros[op1]);
        else{
            if(((instr>>8)&0xff)==2){
                segMemoria=op1>>28;
                printf("[%s:%d]", registros[segMemoria], op1&0xfffffff);
            }
            else{
                printf("%d", op1);
            }
        }
        if((mnemo!=0x33)&&(mnemo!=0x20)&&(mnemo!=0x24)&&(mnemo!=0x25)&&(mnemo!=0x26)&&(mnemo!=0x27)
        &&(mnemo!=0x28)&&(mnemo!=0x29)&&(mnemo!=0x81)){
            if((instr&0xff)==0)
                printf(", %d", op2);
            else{
                if((instr&0xff)==1)
                    printf(", %s", registros[op2]);
                else{
                    segMemoria=op2>>28;
                    printf(", [%s:%d]", registros[segMemoria], op2&0xfffffff);
                }
            }
        }
    }
    printf("\n");
}
