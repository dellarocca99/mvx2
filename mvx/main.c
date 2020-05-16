#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "funciones.h"
    int reg[16];
    int RAM[8192];
    void (*funciones[144])(int codop, int op1, int op2, int RAM[], int reg[]);
void cargaMemoria(int argc, char *argv[]);
void imprimeReg();
void ejecuta(int a,int b,int c,int d);
void flagD();
void flagA();
void flagB();
void BuscaImprime(int instr, int op1, int op2);
void activaBooleanos(int argc,char *argv[],int cantFlag,int a,int b,int c,int d);
int main(int argc, char *argv[])
{
    int cantFlag;
    int a=0,b=0,c=0,d=0; // variables booleanas respectivas a los flags

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
    funciones[0x40]=&call;
    funciones[0x44]=&push;
    funciones[0x45]=&pop;
    funciones[0x48]=&ret;
    funciones[0x50]=&slen;
    funciones[0x51]=&smov;
    funciones[0x53]=&scmp;
    funciones[0x81]=&sys;
    funciones[0x8f]=&stop;

    cargaMemoria(argc, argv);
    diccionario();
    cantFlag=argc-RAM[0]-1;
    activaBooleanos(argc,argv,cantFlag,a,b,c,d);
    //imprimeReg();
    if (c)
        system("cls");
    ejecuta(a,b,c, d);
    if (a)
        flagA();
    return 0;
}

void cargaMemoria(int argc, char *argv[]){
    int aux, swapped;
    int i,j=1,cantImg=0,PSacum;
    FILE *arch;

    // calculo cantidad de imagenes
    while (argv[1+cantImg][0]!='-')
            cantImg++;
        RAM[0]=cantImg;
        RAM[1]=0;

    // guardo en memoria los registros de todos los .img
    for(j=0;j<cantImg;j++){
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
                RAM[j*16+2+i]=swapped;
            }
            fclose(arch);
            }
    }

    // recalculo los registros CS
    for(j=0;j<cantImg;j++){
            PSacum=0;
            RAM[j*16+2+1]=cantImg*16+2+PSacum; // CS_n=CS_1+...+PS_(n-1)+PS_n
            PSacum+=RAM[j*16+2];
    }

     //recalculo los registros DS,ES,SS
     for(j=0;j<cantImg;j++){
            for(i=2;i<=3;i++)
                if (j>=1 && RAM[j*16+2+i] == -1)
                    RAM[j*16+2+i]=RAM[(j-1)*16+2+i];
                else
                    RAM[j*16+2+i]+=RAM[j*16+2+1];
            if (j>=1 && RAM[j*16+2+5] == -1)
                RAM[j*16+2+5]=RAM[(j-1)*16+2+5];
            else
                RAM[j*16+2+5]+=RAM[j*16+2+1];
    }


    // si CS_n+PS_n>8192, no alcanza la memoria y se lanza el mensaje "memoria insuficiente"
    if (RAM[(cantImg-1)*16+2+1]+RAM[(cantImg-1)*16+2+0] <= 8192){
        // guardo en memoria los code segment de cada uno de los procesos en las respectivas posiciones
        for(j=0;j<cantImg;j++){
            if ((arch=fopen(argv[1], "rb"))==NULL)
                printf("el archivo no se encontro");
            else
                {
                fread(&aux,sizeof(int),16,arch);
                for (i=RAM[j*16+2+1];i<RAM[j*16+2+1];i++){ // desde el valor del registro CS hasta el valor del registro DS guardo las instrucciones
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
    }
    else{
        printf("memoria insuficiente");
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

void ejecuta(int a,int b,int c, int d)
{
    int instruccion, op1, op2,i;

    while (RAM[1] < RAM[0]){
        for(i=0;i<16;i++)
            reg[i]=RAM[i+16*RAM[1]+2];
        reg[4]=reg[1];
        if(d)
            flagD();
        while((reg[4] >= 0) && (reg[4] < reg[2])){
            instruccion = RAM[reg[4]];
            op1=RAM[reg[4]+1];
            op2=RAM[reg[4]+2];
            if((instruccion == 143) || ((((instruccion>>16)==5)||((instruccion>>16)==6)) && (devuelveValor(instruccion&0xff, op2, RAM, reg)==0)))
                stop(0,0,0, RAM, reg);
            else{
                if (instruccion == 0x81 && op1 == 0){ // SYS 0 implica breakpoint
                   reg[4]+=3;
                    if (c)
                        system("cls");
                    if(b){
                        if(d)
                            flagD();
                        flagB();
                    }
                }
                else{
                    reg[4] += 3;
                    funciones[instruccion >> 16](instruccion, op1, op2, RAM, reg);
                }
            }
        }
        for(i=0;i<16;i++)
            RAM[i+16*RAM[1]+2]=reg[i];
        RAM[1]++;
    }
}

void activaBooleanos(int argc,char *argv[],int cantFlag,int a,int b,int c,int d)
{
    int i;
    for(i=argc-cantFlag;i<argc;i++){
        if(argv[i][1] == 'a')
            a=1;
        else if(argv[i][1] == 'b')
            b=1;
        else if(argv[i][1] == 'c')
            c=1;
        else if(argv[i][1] == 'd')
            d=1;
    }
}

void flagA()
{
    int i,j;
    printf("Cantidad total de procesos = %d\n", RAM[0]);
    printf("Cantidad de procesos finalizados correctamente = %d\n", RAM[1]);
    printf("\n");
    for(i=0;i<RAM[0];i++){
        printf("Proceso %d:\n",i+1);
        for(j=0;j<16;j+=4)
            printf("%s = %10d | %s = %10d | %s = %10d | %s = %10d |\n",registros[j],RAM[i*16+2+j],registros[j+1],RAM[i*16+2+j+1],registros[j+2],RAM[i*16+2+j+2],registros[j+3],RAM[i*16+2+j+3]);
        printf("\n");
    }
}

void flagD()
{
    int instruccion, op1, op2, ip=reg[1], i, j;

    printf("\n");
    while(ip < reg[2]){
        instruccion= RAM[ip];
        op1=RAM[ip+1];
        op2=RAM[ip+2];
        if(ip == (reg[4]))
            printf(">[%04x %04x] \t %04x %04x %04x %04x %04x %04x \t",
               ip>>16, ip&0x0000FFFF, instruccion>>16, instruccion&0x0000FFFF, op1>>16, op1&0x0000FFFF, (op2&0xffff0000)>>16, op2&0x0000FFFF);
        else
            printf(" [%04x %04x] \t %04x %04x %04x %04x %04x %04x \t",
               ip>>16, ip&0x0000FFFF, instruccion>>16, instruccion&0x0000FFFF, op1>>16, op1&0x0000FFFF, (op2&0xffff0000)>>16, op2&0x0000FFFF);
        ip+=3;
        BuscaImprime(instruccion, op1, op2);
    }
    printf("Registros:\n");
    for(i=0;i<RAM[0];i++){
        for(j=0;j<16;j+=4)
            printf("%s = %10d | %s = %10d | %s = %10d | %s = %10d |\n",registros[j],RAM[i*16+2+j],registros[j+1],RAM[i*16+2+j+1],registros[j+2],RAM[i*16+2+j+2],registros[j+3],RAM[i*16+2+j+3]);
        printf("\n");
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

void flagB()
{
    int a=0, b=0, i=0,j,iant,aux,cantNum=0;
    char s[10];
    printf("[%03d] cmd: ", ((reg[4]-reg[1])/3)+1);
    scanf("%s", s);
    if (s[0]){
        while (s[i] != 32 && s[i])
            i++;
        for(j=i-1;j>=0;j--){
            aux=(int)pow(10,(i-1-j));
            a+=(s[i-1]-48)*aux;
        }
        cantNum++;
        if (s[i] == 32){
            cantNum++;
            i++;
            iant=i;
            while (s[i])
                i++;
            for(j=i-1;j>=iant;j--){
                aux=(int)pow(10,(i-1-j));
                b+=(s[i-1]-48)*aux;
            }
        }

        if(cantNum == 1){
            printf("[%04d]: %04x %04x ", a, RAM[a]>>16, RAM[a]&0xffff);
            if(RAM[a]>=32 && RAM[a]<=126)
                printf("%c ", RAM[a]);
            else
                printf(". ");
            printf("%10d", RAM[a]);
            printf("\n");
        }
        else{
            for(i=a;i<=b;i++){
                printf("[%04d]: %04x %04x ", i, RAM[i]>>16, RAM[i]&0xffff);
                if(RAM[i]>=32 && RAM[i]<=126)
                    printf("%c ", RAM[i]);
                else
                    printf(". ");
                printf("%10d", RAM[i]);
                printf("\n");
            }
        }
        flagB();
    }
}
