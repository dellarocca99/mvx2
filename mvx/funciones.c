#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funciones.h"

int devuelveValor(int Top, int op, int RAM[], int reg[]){
    int segMemoria,valor, regInd;
    if(Top == 0x1)
        op = reg[op];
    else{
        if(Top == 0x2){
            segMemoria = op >> 28;
            valor = op & 0xfffffff;
            op = RAM[reg[segMemoria]+valor];
        }
        else if (Top == 0x3){
            segMemoria= op >>28;
            regInd= op & 0xf;
            valor= (op >> 4) & 0xffffff;
            if((valor>>23)==0)
                op = RAM[reg[segMemoria]+reg[regInd]+valor];
            else{
                valor= -1*(op >> 4) & 0xffffff;
                op = RAM[reg[segMemoria]+reg[regInd]-valor];
            }
        }
    }
    return op;
}

void push(int codop, int op1, int op2, int RAM[], int reg[])
{
    if (reg[6] != 0){ //puedo poner en la pila
        reg[6]--;
        RAM[reg[5]+reg[6]]=devuelveValor((codop>>8)&0xf,op1,RAM,reg);
    }
    else{
        printf("Error: pila llena\n");
        RAM[1]=RAM[0];
        stop(0,0,0,RAM,reg);
    }
}

void pop(int codop, int op1, int op2, int RAM[], int reg[])
{
    int segMemoria,Top1;
    if (reg[5]+reg[6]<reg[1]+reg[0]){ // puedo sacar de la pila
        segMemoria= op1>>28;
        Top1=(codop>>8)&0xf;
        if(Top1 == 1)
            reg[op1] = RAM[reg[5]+reg[6]];
        else if (Top1 == 2)
            RAM[reg[segMemoria]+(op1 & 0xfffffff)] = RAM[reg[5]+reg[6]];
        else{
            if(((op1>>27)&0x1) == 0)
                RAM[reg[segMemoria] + ((op1>>4) & 0xffffff)+ reg[op1&0xf]] = RAM[reg[5]+reg[6]];
            else
                RAM[reg[segMemoria] - (-1*(op1>>4) & 0xffffff)+ reg[op1&0xf]] = RAM[reg[5]+reg[6]];
        }
        reg[6]++;
    }
    else{
        printf("Error: pila vacia\n");
        RAM[1]=RAM[0];
        stop(0,0,0,RAM,reg);
    }
}

void call(int codop, int op1, int op2, int RAM[], int reg[])
{
    if (reg[6] != 0){ //puedo poner en la pila
        reg[6]--;
        RAM[reg[5]+reg[6]]=reg[4];
        reg[4]=reg[1] + devuelveValor((codop>>8)&0xf,op1,RAM,reg); //LO METI ACA PORQUE SI HAY UN ERROR DE QUE LA PILA ESTE LLENA NO
                                                            //SE TENDRIA POR QUE CAMBIAR EL IP
    }
    else{
        printf("Error: pila llena\n");
        RAM[1]=RAM[0];
        stop(0,0,0,RAM,reg);
    }

}

void ret(int codop, int op1, int op2, int RAM[], int reg[])
{
    if (reg[5]+reg[6]<reg[1]+reg[0]){ // puedo sacar de la pila
        reg[4]=RAM[reg[5]+reg[6]];
        reg[6]++;
    }
    else{
        printf("Error: pila vacia\n");
        RAM[1]=RAM[0];
        stop(0,0,0,RAM,reg);
    }
}

void slen(int codop, int op1, int op2, int RAM[], int reg[])
{
    int codRegBase2,pos,cont=0;
    codRegBase2=op2>>28;
    if ((codop&0xf)==2) //si el operando 2 es directo
        pos=reg[codRegBase2]+(op2 & 0xfffffff);
    else{
            if(((op1>>27)&0x1) == 0)
                pos=reg[codRegBase2] + ((op2>>4) & 0xffffff)+ reg[op2&0xf];
            else
                pos=reg[codRegBase2] - (-1*(op2>>4) & 0xffffff)+ reg[op2&0xf];
        }
    while (RAM[pos]){
            cont++;
            pos++;
        }
    int Top1=(codop>>8)&0xf;
    if (Top1==1) // si el operando 1 es registro
        reg[op1]=cont;
    else if (Top1==2) // si el operando 1 es directo
        RAM[reg[op1>>28]+(op1 & 0xfffffff)]=cont;
    else{
            if(((op1>>27)&0x1) == 0)
                RAM[reg[op1>>28] + ((op1>>4) & 0xffffff)+ reg[op1&0xf]] = cont;
            else
                RAM[reg[op1>>28] - (-1*(op1>>4) & 0xffffff)+ reg[op1&0xf]] = cont;
        }
}

void smov(int codop, int op1, int op2, int RAM[], int reg[])
{
    int codRegBase1,codRegBase2,Top1,Top2,pos1,pos2;
    codRegBase1=op1>>28;
    codRegBase2=op2>>28;
    Top1=(codop>>8)&0xf;
    Top2=codop&0xf;
    if (Top2 == 0)
        pos2=op2;
    else if (Top2 == 2)
        pos2=reg[codRegBase2]+(op2&0xfffffff);
    else{
            if(((op1>>27)&0x1) == 0)
                pos2=reg[codRegBase2] + ((op2>>4) & 0xffffff)+ reg[op2&0xf];
            else
                pos2=reg[codRegBase2] - (-1*(op2>>4) & 0xffffff)+ reg[op2&0xf];
        }
    if (Top1 == 0)
        pos1=op1;
    else if (Top1 == 2)
        pos1=reg[codRegBase1]+(op1&0xfffffff);
    else
        pos1=reg[codRegBase1]+((op1>>4)&0xffffff)+reg[op1&0xf];
    while (RAM[pos2]){
        RAM[pos1]=RAM[pos2];
        pos1++;
        pos2++;
    }
}

void scmp(int codop, int op1, int op2, int RAM[], int reg[])
{
    int codRegBase1,codRegBase2,Top1,Top2,pos1,pos2,resta;
    codRegBase1=op1>>28;
    codRegBase2=op2>>28;
    Top1=(codop>>8)&0xf;
    Top2=codop&0xf;
    if (Top2 == 2)
        pos2=reg[codRegBase2]+(op2&0xfffffff);
    else{
            if(((op2>>27)&0x1) == 0)
                pos2=reg[codRegBase2] + ((op2>>4) & 0xffffff)+ reg[op2&0xf];
            else
                pos2=reg[codRegBase2] - (-1*(op2>>4) & 0xffffff)+ reg[op2&0xf];
        }
    if (Top1 == 2)
        pos1=reg[codRegBase1]+(op1&0xfffffff);
    else{
            if(((op1>>27)&0x1) == 0)
                pos1=reg[codRegBase1] + ((op1>>4) & 0xffffff)+ reg[op1&0xf];
            else
                pos1=reg[codRegBase1] - (-1*(op1>>4) & 0xffffff)+ reg[op1&0xf];
        }
    do {
        resta=RAM[pos1]-RAM[pos2];
        if(resta == 0) // modifico el registro CC
            reg[9]= 1;
        else{
            if(resta < 0){
                reg[9]= 1;
                reg[9]= reg[9] << 31;
            }
            else
                reg[9]= 0;
        }
        pos1++;
        pos2++;
    } while ((RAM[pos2] != 0 || RAM[pos1] != 0) && (reg[9]&0x1)==1);
}

void mov(int codop, int op1, int op2, int RAM[], int reg[])
{
    int segMemoria, Top1;

    segMemoria = op1 >> 28;
    Top1=(codop>>8)&0xf;
    if (Top1 == 1)
        reg[op1]= devuelveValor(codop&0xff, op2, RAM, reg);
    else if (Top1 == 2)
        RAM[reg[segMemoria] + (op1 & 0xfffffff)] = devuelveValor(codop&0xff, op2, RAM, reg);
    else{
        if(((op1>>27)&0x1) == 0)
            RAM[reg[segMemoria] + ((op1>>4) & 0xffffff)+ reg[op1&0xf]] = devuelveValor(codop&0xff, op2, RAM, reg);
        else
            RAM[reg[segMemoria] - (-1*(op1>>4) & 0xffffff)+ reg[op1&0xf]] = devuelveValor(codop&0xff, op2, RAM, reg);
    }
}

void add(int codop, int op1, int op2, int RAM[], int reg[])
{
    int valor1, valor2, segMemoria, Top1;

    valor1=devuelveValor((codop>>8)&0xff, op1, RAM, reg);
    valor2=devuelveValor(codop&0xff, op2, RAM, reg);
    segMemoria = op1 >> 28;
    Top1=(codop>>8)&0xf;
    if(Top1 == 1)
        reg[op1] = valor1 + valor2;
    else if (Top1 == 2)
        RAM[reg[segMemoria]+(op1 & 0xfffffff)] = valor1 + valor2;
    else{
        if(((op1>>27)&0x1) == 0)
            RAM[reg[segMemoria] + ((op1>>4) & 0xffffff)+ reg[op1&0xf]] = valor1+valor2;
        else
            RAM[reg[segMemoria] - (-1*(op1>>4) & 0xffffff)+ reg[op1&0xf]] = valor1+valor2;
    }
    if((valor1 + valor2) == 0)
        reg[9]= 1;
    else{
        if((valor1 + valor2) < 0){
            reg[9]= 1;
            reg[9]= reg[9] << 31;
        }
        else
            reg[9]= 0;
    }
}

void sub(int codop, int op1, int op2, int RAM[], int reg[])
{
    int valor1, valor2, segMemoria, Top1;

    valor1=devuelveValor((codop>>8)&0xff, op1, RAM, reg);
    valor2=devuelveValor(codop&0xff, op2, RAM, reg);
    segMemoria = op1 >> 28;
    Top1=(codop>>8)&0xf;
    if(Top1 == 1)
        reg[op1] = valor1 - valor2;
    else if (Top1 == 2)
        RAM[reg[segMemoria]+(op1 & 0xfffffff)] = valor1 - valor2;
    else{
        if(((op1>>27)&0x1) == 0)
            RAM[reg[segMemoria] + ((op1>>4) & 0xffffff)+ reg[op1&0xf]] = valor1 - valor2;
        else
            RAM[reg[segMemoria] - (-1*(op1>>4) & 0xffffff)+ reg[op1&0xf]] = valor1 - valor2;
    }
    if((valor1 - valor2) == 0)
        reg[9]= 1;
    else{
        if((valor1 - valor2) < 0){
            reg[9]= 1;
            reg[9]= reg[9] << 31;
        }
        else
            reg[9]= 0;
    }
}

void mul(int codop, int op1, int op2, int RAM[], int reg[])
{
    int valor1, valor2, segMemoria, Top1;

    valor1=devuelveValor((codop>>8)&0xff, op1, RAM, reg);
    valor2=devuelveValor(codop&0xff, op2, RAM, reg);
    segMemoria = op1 >> 28;
    Top1=(codop>>8)&0xf;
    if(Top1 == 1)
        reg[op1] = valor1 * valor2;
    else if (Top1 == 2)
        RAM[reg[segMemoria]+(op1 & 0xfffffff)] = valor1 * valor2;
    else{
        if(((op1>>27)&0x1) == 0)
            RAM[reg[segMemoria] + ((op1>>4) & 0xffffff)+ reg[op1&0xf]] = valor1 * valor2;
        else
            RAM[reg[segMemoria] - (-1*(op1>>4) & 0xffffff)+ reg[op1&0xf]] = valor1 * valor2;
    }
    if((valor1 * valor2) == 0)
        reg[9]= 1;
    else{
        if((valor1 * valor2) < 0){
            reg[9]= 1;
            reg[9]= reg[9] << 31;
        }
        else
            reg[9]= 0;
    }
}

void DIV(int codop, int op1, int op2, int RAM[], int reg[])
{
    int valor1, valor2, segMemoria, Top1;

    valor1=devuelveValor((codop>>8)&0xff, op1, RAM, reg);
    valor2=devuelveValor(codop&0xff, op2, RAM, reg);
    segMemoria = op1 >> 28;
    if(valor2 != 0){
        Top1=(codop>>8)&0xf;
        if(Top1 == 1)
            reg[op1] = valor1 / valor2;
        else if (Top1 == 2)
            RAM[reg[segMemoria]+(op1 & 0xfffffff)] = valor1 / valor2;
        else{
        if(((op1>>27)&0x1) == 0)
            RAM[reg[segMemoria] + ((op1>>4) & 0xffffff)+ reg[op1&0xf]] = valor1 / valor2;
        else
            RAM[reg[segMemoria] - (-1*(op1>>4) & 0xffffff)+ reg[op1&0xf]] = valor1 / valor2;
    }
        if((valor1 / valor2) == 0)
            reg[9]= 1;
        else{
            if((valor1 / valor2) < 0){
                reg[9]= 1;
                reg[9]= reg[9] << 31;
            }
            else
                reg[9]= 0;
        }
    }
    else{
        RAM[1]=RAM[0];
        stop(0,0,0,RAM,reg);
    }
}

void mod(int codop, int op1, int op2, int RAM[], int reg[])
{
    int valor1, valor2, segMemoria, Top1;

    valor1=devuelveValor((codop>>8)&0xff, op1, RAM, reg);
    valor2=devuelveValor(codop&0xff, op2, RAM, reg);
    segMemoria = op1 >> 28;
    if(valor2 != 0){
        Top1=(codop>>8)&0xf;
        if(Top1 == 1)
            reg[op1] = valor1 % valor2;
        else if (Top1 == 2)
            RAM[reg[segMemoria]+(op1 & 0xfffffff)] = valor1 % valor2;
        else{
            if(((op1>>27)&0x1) == 0)
                RAM[reg[segMemoria] + ((op1>>4) & 0xffffff)+ reg[op1&0xf]] = valor1 % valor2;
            else
                RAM[reg[segMemoria] - (-1*(op1>>4) & 0xffffff)+ reg[op1&0xf]] = valor1 % valor2;
        }
        if((valor1 / valor2) == 0)
            reg[9]= 1;
        else{
            if((valor1 / valor2) < 0){
                reg[9]= 1;
                reg[9]= reg[9] << 31;
            }
            else
                reg[9]= 0;
        }
    }
    else{
        RAM[1]=RAM[0];
        stop(0,0,0,RAM,reg);
    }
}

void cmp(int codop, int op1, int op2, int RAM[], int reg[])
{
    int valor1, valor2;

    valor1= devuelveValor((codop>>8)&0xff, op1, RAM, reg);
    //printf("%d\n", valor1);
    valor2= devuelveValor(codop & 0xff, op2, RAM, reg);
    //printf("%d\n", valor2);
    if((valor1 - valor2) == 0)
        reg[9]= 1;
    else{
        if((valor1-valor2) < 0){
            reg[9]= 1;
            reg[9]= reg[9] << 31;
        }
        else
            reg[9]= 0;
    }
}

void SWAP(int codop, int op1, int op2, int RAM[], int reg[])
{
    int valor1, valor2, segMemoria1, segMemoria2, Top1, Top2;

    if((((codop>>8)&0xff) != 0)&&((codop&0xff) != 0)){          /*VERIFICO QUE EL OP1 Y OP2 SEAN REGISTROS O MEMORIA*/
        valor1=devuelveValor((codop>>8)&0xff,op1, RAM, reg);    /*Y NO UN VALOR INMEDIATO */
        valor2=devuelveValor(codop&0xff, op2, RAM, reg);
        segMemoria1=op1>>28;
        segMemoria2=op2>>28;
        Top1=(codop>>8)&0xf;
        if(Top1 == 1)
            reg[op1] = valor2;
        else if (Top1 == 2)
            RAM[reg[segMemoria1]+(op1 & 0xfffffff)] = valor2;
        else{
        if(((op1>>27)&0x1) == 0)
            RAM[reg[segMemoria1] + ((op1>>4) & 0xffffff)+ reg[op1&0xf]] = valor2;
        else
            RAM[reg[segMemoria1] - (-1*(op1>>4) & 0xffffff)+ reg[op1&0xf]] = valor2;
    }
        Top2=codop&0xf;
        if(Top2 == 1)
            reg[op2] = valor1;
        else if (Top2 == 2)
            RAM[reg[segMemoria2]+(op2 & 0xfffffff)] = valor1;
        else{
            if(((op2>>27)&0x1) == 0)
                RAM[reg[segMemoria2] + ((op2>>4) & 0xffffff)+ reg[op2&0xf]] = valor1;
            else
                RAM[reg[segMemoria2] - (-1*(op2>>4) & 0xffffff)+ reg[op2&0xf]] = valor1;
        }
    }
}

void rnd(int codop, int op1, int op2, int RAM[], int reg[])
{
    int valor2, segMemoria1, Top1;

    valor2=devuelveValor(codop&0xff, op2, RAM, reg);
    segMemoria1= op1 >> 28;
    Top1=(codop>>8)&0xf;
    if(Top1 == 1) /*BUSCAMOS UN NUMEOR RANDOM ENTRE 0 Y EL VALOR DEL OP2 */
        reg[op1] = rand() % (valor2+1);
    else if (Top1 == 2)
        RAM[reg[segMemoria1]+(op1 & 0xfffffff)] = rand() % (valor2+1);
    else{
        if(((op1>>27)&0x1) == 0)
            RAM[reg[segMemoria1] + ((op1>>4) & 0xffffff)+ reg[op1&0xf]] = rand() %(valor2+1);
        else
            RAM[reg[segMemoria1] - (-1*(op1>>4) & 0xffffff)+ reg[op1&0xf]] = rand() % (valor2 + 1);
    }
}

void and(int codop, int op1, int op2, int RAM[], int reg[])
{
    int valor1, valor2, segMemoria1, Top1;

    valor1=devuelveValor((codop>>8)&0xff, op1, RAM, reg);
    valor2=devuelveValor(codop&0xff, op2, RAM, reg);
    segMemoria1= op1 >> 28;
    Top1=(codop>>8)&0xf;
    if(Top1 == 1)
        reg[op1] = valor1 & valor2;
    else if (Top1 == 2)
        RAM[reg[segMemoria1]+(op1 & 0xfffffff)] = valor1 & valor2;
    else{
        if(((op1>>27)&0x1) == 0)
            RAM[reg[segMemoria1] + ((op1>>4) & 0xffffff)+ reg[op1&0xf]] = valor1 & valor2;
        else
            RAM[reg[segMemoria1] - (-1*(op1>>4) & 0xffffff)+ reg[op1&0xf]] = valor1 & valor2;
    }
    if((valor1&valor2) == 0) /* MODIFICO EL REGISTRO CC */
        reg[9]= 1;
    else{
        if((valor1&valor2) < 0){
            reg[9]=1;
            reg[9]= reg[9] << 31;
        }
        else
            reg[9]=0;
    }
}

void or(int codop, int op1, int op2, int RAM[], int reg[])
{
    int valor1, valor2, segMemoria1, Top1;

    valor1=devuelveValor((codop>>8)&0xff, op1, RAM, reg);
    valor2=devuelveValor(codop&0xff, op2, RAM, reg);
    segMemoria1= op1 >> 28;
    Top1=(codop>>8)&0xf;
    if(Top1 == 1)
        reg[op1] = valor1 | valor2;
    else if (Top1 == 2)
        RAM[reg[segMemoria1]+(op1 & 0xfffffff)] = valor1 | valor2;
    else{
        if(((op1>>27)&0x1) == 0)
            RAM[reg[segMemoria1] + ((op1>>4) & 0xffffff)+ reg[op1&0xf]] = valor1 | valor2;
        else
            RAM[reg[segMemoria1] - (-1*(op1>>4) & 0xffffff)+ reg[op1&0xf]] = valor1 | valor2;
    }
    if((valor1|valor2) == 0) /* MODIFICO EL REGISTRO CC */
        reg[9]= 1;
    else{
        if((valor1|valor2) < 0){
            reg[9]=1;
            reg[9]= reg[9] << 31;
        }
        else
            reg[9]=0;
    }
}

void not(int codop, int op1, int op2, int RAM[], int reg[])
{
    int valor1, segMemoria1, Top1;

    valor1=devuelveValor((codop>>8)&0xff, op1, RAM, reg);
    segMemoria1= op1 >> 28;
    Top1=(codop>>8)&0xf;
    if(Top1 == 1)
        reg[op1] = ~valor1;
    else if (Top1 == 2)
        RAM[reg[segMemoria1]+(op1 & 0xfffffff)] = ~valor1;
    else{
        if(((op1>>27)&0x1) == 0)
            RAM[reg[segMemoria1] + ((op1>>4) & 0xffffff)+ reg[op1&0xf]] = ~valor1;
        else
            RAM[reg[segMemoria1] - (-1*(op1>>4) & 0xffffff)+ reg[op1&0xf]] = ~valor1;
    }
    if((~valor1) == 0)  /* MODIFICO EL REGISTRO CC */
        reg[9]= 1;
    else{
        if((~valor1) < 0){
            reg[9]= 1;
            reg[9]= reg[9] << 31;
        }
        else
            reg[9]= 0;
    }
}

void xor(int codop, int op1, int op2, int RAM[], int reg[])
{
    int valor1, valor2, segMemoria, Top1;

    valor1=devuelveValor((codop>>8)&0xff, op1, RAM, reg);
    valor2=devuelveValor(codop&0xff, op2, RAM, reg);
    segMemoria=op1 >> 28;
    Top1=(codop>>8)&0xf;
    if(Top1 == 1)
        reg[op1] = valor1 ^ valor2;
    else if (Top1 == 2)
        RAM[reg[segMemoria]+(op1 & 0xfffffff)] = valor1 ^ valor2;
    else{
        if(((op1>>27)&0x1) == 0)
            RAM[reg[segMemoria] + ((op1>>4) & 0xffffff)+ reg[op1&0xf]] = valor1 ^ valor2;
        else
            RAM[reg[segMemoria] - (-1*(op1>>4) & 0xffffff)+ reg[op1&0xf]] = valor1 ^ valor2;
    }
    if((valor1 ^ valor2) == 0)
        reg[9]=1;
    else{
        if((valor1 ^ valor2) < 0){
            reg[9]=1;
            reg[9]= reg[9] << 31;
        }
        else
            reg[9]=0;
    }
}

void shl(int codop, int op1, int op2, int RAM[], int reg[])
{
    int valor1, segMemoria1, valor2, Top1;

    valor1= devuelveValor((codop>>8)&0xff, op1, RAM, reg);
    segMemoria1= op1>>28;
    valor2= devuelveValor(codop&0xff, op2, RAM, reg);
    Top1=(codop>>8)&0xf;
    if(Top1 == 1)
        reg[op1] = valor1 << valor2;
    else if (Top1 == 2)
        RAM[reg[segMemoria1]+(op1 & 0xfffffff)] = valor1 << valor2;
    else{
        if(((op1>>27)&0x1) == 0)
            RAM[reg[segMemoria1] + ((op1>>4) & 0xffffff)+ reg[op1&0xf]] = valor1 << valor2;
        else
            RAM[reg[segMemoria1] - (-1*(op1>>4) & 0xffffff)+ reg[op1&0xf]] = valor1 << valor2;
    }
    if((valor1 << valor2)== 0)
        reg[9]=1;
    else{
        if((valor1<<valor2)<0){
            reg[9]=1;
            reg[9]= reg[9] << 31;
        }
        else
            reg[9]=0;
        }
}

void shr(int codop, int op1, int op2, int RAM[], int reg[])
{
    int valor1, valor2, segMemoria, i, auxi=0, pot=1, Top1;

    valor1=devuelveValor((codop>>8)&0xff, op1, RAM, reg);
    valor2=devuelveValor(codop&0xff, op2, RAM, reg);
    for(i=0; i<(32-valor2); i++){
        auxi+=  pot;
        pot*=2;
    }
    segMemoria=op1>>28;
    Top1=(codop>>8)&0xf;
    if(Top1 == 1)
        reg[op1] = (valor1 >> valor2)&auxi;
    else if (Top1 == 2)
        RAM[reg[segMemoria]+(op1 & 0xfffffff)] = (valor1 >> valor2)&auxi;
    else{
        if(((op1>>27)&0x1) == 0)
            RAM[reg[segMemoria] + ((op1>>4) & 0xffffff)+ reg[op1&0xf]] = (valor1 >> valor2)&auxi;
        else
            RAM[reg[segMemoria] - (-1*(op1>>4) & 0xffffff)+ reg[op1&0xf]] = (valor1 >> valor2)&auxi;
    }
    if(((valor1>>valor2)&auxi)==0)
        reg[9]=1;
    else{
        if(((valor1>>valor2)&auxi)<0){
            reg[9]=1;
            reg[9]= reg[9]<<31;
        }
        else
            reg[9]=0;
    }
}

void jmp(int codop, int op1, int op2, int RAM[], int reg[])
{
    int valor1;

    valor1=devuelveValor((codop>>8)&0xff, op1, RAM, reg);
    reg[4]=reg[1] + valor1;
}

void je(int codop, int op1, int op2, int RAM[], int reg[])
{
    int ax, valor1, valor2;

    ax=reg[10];
    valor1=devuelveValor((codop>>8)&0xff, op1, RAM, reg);
    valor2=devuelveValor(codop>>8, op2, RAM, reg);
    if(ax == valor1)
        reg[4]=reg[1] + valor2;
}

void jg(int codop, int op1, int op2, int RAM[], int reg[])
{
    int ax, valor1, valor2;

    ax=reg[10];
    valor1=devuelveValor((codop>>8)&0xff, op1, RAM, reg);
    valor2=devuelveValor(codop>>8, op2, RAM, reg);
    if(ax < valor1)
        reg[4]=reg[1] + valor2;
}

void jl(int codop, int op1, int op2, int RAM[], int reg[])
{
    int ax, valor1, valor2;

    ax=reg[10];
    valor1=devuelveValor((codop>>8)&0xff, op1, RAM, reg);
    valor2=devuelveValor(codop>>8, op2, RAM, reg);
    if(ax > valor1)
        reg[4]=reg[1] + valor2;
}

void jz(int codop, int op1, int op2, int RAM[], int reg[])
{
    int valor1;

    valor1=devuelveValor((codop>>8)&0xff, op1, RAM, reg);
    if(abs(reg[9]&0x1)==1)
        reg[4]=reg[1] + valor1;
}

void jp(int codop, int op1, int op2, int RAM[], int reg[])
{
    int valor1;

    valor1=devuelveValor((codop>>8)&0xff, op1, RAM, reg);
    if(abs(reg[9]>>31)==0 && (reg[9] & 0x1) == 0)
        reg[4]=reg[1] + valor1;
}

void JN(int codop, int op1, int op2, int RAM[], int reg[])
{
    int valor1;

    valor1=devuelveValor((codop>>8)&0xff, op1, RAM, reg);
    if(abs(reg[9]>>31) == 1)
        reg[4]= reg[1] + valor1;
}

void jnz(int codop, int op1, int op2, int RAM[], int reg[])
{
    int valor1;

    valor1=devuelveValor((codop>>8)&0xff, op1, RAM, reg);
    if(abs(reg[9]&0x1) == 0)
        reg[4]= reg[1] + valor1;
}

void jnp(int codop, int op1, int op2, int RAM[], int reg[])
{
    int valor1;

    valor1=devuelveValor((codop>>8)&0xff, op1, RAM, reg);
    if((abs(reg[9]>>31) != 0) || ((reg[9]&0x0000001)==1))
        reg[4]= reg[1] + valor1;
}

void jnn(int codop, int op1, int op2, int RAM[], int reg[])
{
    int valor1;

    valor1=devuelveValor((codop>>8)&0xff, op1, RAM, reg);
    if(abs(reg[9]>>31) != 1)
        reg[4]= reg[1] + valor1;
}

void sys(int codop, int op1, int op2, int RAM[], int reg[])
{
    int valor1, i,pos, aux=reg[10];
    int ax[16];
    char c[reg[12]+1], por='%',s[50];

    valor1=devuelveValor((codop>>8)&0xff, op1, RAM, reg);
    for(i=0; i<16; i++){
        ax[i]= aux & 0x1;
        aux= aux >> 1;
    }
    if(valor1 == 1){
        i=0;
        if(ax[8]==0){ /* verificamos el bit 8, si lee dsp del enter o lee caracter a caracter */
            while(i<reg[12]){ /* leemos una cantidad de veces igual al registro CX */
                if(ax[12]==0)
                    printf("[%04d]: ", reg[2]+reg[13]+i);
                if(ax[0]==1)
                    scanf(" %d", &(RAM[reg[2]+reg[13]+i]));
                else{
                    if(ax[2] == 1)
                        scanf(" %o", &(RAM[reg[2]+reg[13]+i]));
                    else
                        if(ax[3] == 1)
                        scanf(" %x", &(RAM[reg[2]+reg[13]+i]));
                }
                i++;
            }
        }
        else{
            i=0;
            if(ax[12]==0)
                printf("[%04d]: ", (reg[2]+reg[13]));
            fflush(stdin);
            fgets(c, reg[12]+1, stdin);
            while((i<reg[12]) && (c[i]!=10)){
                RAM[reg[2]+reg[13]+i] = c[i];
                i++;
            }
            RAM[reg[2]+reg[13]+i]=0;
           // if((i<reg[12]))
           //     RAM[reg[2]+reg[13]+i] = 0;
        }
    }
    else{
        if((valor1==2)||(valor1==3)){
            if(ax[12]==0){ /* AGREGA PROMT DE MEMORIA */
                if(ax[8]==0){ /* AGREGA ENDLINE POR CADA CELDA DE MEMORIA */
                    for(i=0;i<reg[12];i++){
                        printf("[%04d]:", reg[2]+reg[13]+i);
                        if(ax[0]==1)
                            printf(" %d", RAM[reg[2]+reg[13]+i]);
                        if(ax[2]==1)
                            printf(" @%o", RAM[reg[2]+reg[13]+i]);
                        if(ax[3]==1)
                            printf(" %c%x", por, RAM[reg[2]+reg[13]+i]);
                        if(ax[4]==1){
                            if(((RAM[reg[2]+reg[13]+i]&0xff)>31) && ((RAM[reg[2]+reg[13]+i]&0xff)<127))
                                printf(" %c", (RAM[reg[2]+reg[13]+i]&0xff));
                            else
                                printf(" .");
                        }
                        printf("\n");
                    }
                }
                else{ /* NO AGREGA ENDLINE */
                    printf("[%04d]: ", reg[2]+reg[13]);
                    for(i=0;i<reg[12];i++){
                        if(ax[0]==1)
                            printf(" %d", RAM[reg[2]+reg[13]+i]);
                        if(ax[2]==1)
                            printf(" @%o", RAM[reg[2]+reg[13]+i]);
                        if(ax[3]==1)
                            printf(" %c%x", por, RAM[reg[2]+reg[13]+i]);
                        if(ax[4]==1){
                            if(((RAM[reg[2]+reg[13]+i]&0xff)>31) && ((RAM[reg[2]+reg[13]+i]&0xff)<127))
                                printf(" %c", (RAM[reg[2]+reg[13]+i]&0xff));
                            else
                                printf(" .");
                        }
                        printf(" ");
                    }
                }
            }
            else{ /* NO AGREGA PROMT DE MEMORIA */
                if(ax[8]==0){ /* AGREGA ENDLINE */
                    for(i=0;i<reg[12];i++){
                        if(ax[0]==1)
                            printf(" %d", RAM[reg[2]+reg[13]+i]);
                        if(ax[2]==1)
                            printf(" @%o", RAM[reg[2]+reg[13]+i]);
                        if(ax[3]==1)
                            printf(" %c%x", por, RAM[reg[2]+reg[13]+i]);
                        if(ax[4]==1){
                            if(((RAM[reg[2]+reg[13]+i]&0xff)>31) && ((RAM[reg[2]+reg[13]+i]&0xff)<127))
                                printf(" %c", (RAM[reg[2]+reg[13]+i]&0xff));
                            else
                                printf(" .");
                        }
                        printf("\n");
                    }
                }
            else{ /* NO AGREGA ENDLINE */
                    for(i=0;i<reg[12];i++){
                        if(ax[0]==1)
                            printf(" %d", RAM[reg[2]+reg[13]+i]);
                        if(ax[2]==1)
                            printf(" @%o", RAM[reg[2]+reg[13]+i]);
                        if(ax[3]==1)
                            printf(" %c%x", por, RAM[reg[2]+reg[13]+i]);
                        if(ax[4]==1){
                            if(((RAM[reg[2]+reg[13]+i]&0xff)>31) && ((RAM[reg[2]+reg[13]+i]&0xff)<127))
                                printf(" %c", (RAM[reg[2]+reg[13]+i]&0xff));
                            else
                                printf(" .");
                        }
                        printf(" ");
                    }
                }
            }
        }
        else if (valor1 == 10){ // string read
            pos=reg[reg[11]]+reg[13];
            if (((reg[10]>>12)&0x1) == 0)
                printf("[%d]: ", pos);
            fflush(stdin);
            fgets(s, 100, stdin);
            i=0;
            while (s[i] != 10 && s[i] != 0 && s[i] != 13)
                i++;
            s[i]=0;
            for (i=0;i<=strlen(s);i++){
                RAM[pos]=s[i];
                pos++;
            }
        }
        else if (valor1 == 20) { // string write
            pos=reg[reg[11]]+reg[13];
            if (((reg[10]>>12)&0x1) == 0)
                printf("[%d]: ", pos);
            while (RAM[pos]){
                printf("%c", RAM[pos]);
                pos++;
            }
            if (((reg[10]>>8)&0x1) == 0)
                printf("\n");
        }
    }
    printf("\n");
    if(valor1 == 3){
        if(ax[0]==1){
            for(i=0;i<16;i++)
                printf("[%s]: %04d\n", registros[i], reg[i]);
        }
        if(ax[2]==1){
            for(i=0;i<16;i++)
                printf("[%s]: @%011o\n", registros[i], reg[i]);
        }
        if(ax[3]==1){
            for(i=0;i<16;i++)
                printf("[%s]: %c%08x\n", registros[i], por,reg[i]);
        }
    }
}

void stop(int codop, int op1, int op2, int RAM[], int reg[])
{
    reg[4]=reg[2];
}

void diccionario()
{
    strcpy(mnemonicos[0x01], "MOV");
    strcpy(mnemonicos[0x02], "ADD");
    strcpy(mnemonicos[0x03], "SUB");
    strcpy(mnemonicos[0x04], "MUL");
    strcpy(mnemonicos[0x05], "DIV");
    strcpy(mnemonicos[0x06], "MOD");
    strcpy(mnemonicos[0x13], "CMP");
    strcpy(mnemonicos[0x17], "SWAP");
    strcpy(mnemonicos[0x19], "RND");
    strcpy(mnemonicos[0x31], "AND");
    strcpy(mnemonicos[0x32], "OT");
    strcpy(mnemonicos[0x33], "NOT");
    strcpy(mnemonicos[0x34], "XOR");
    strcpy(mnemonicos[0x37], "SHL");
    strcpy(mnemonicos[0x38], "SHR");
    strcpy(mnemonicos[0x20], "JMP");
    strcpy(mnemonicos[0x21], "JE");
    strcpy(mnemonicos[0x22], "JG");
    strcpy(mnemonicos[0x23], "JL");
    strcpy(mnemonicos[0x24], "JZ");
    strcpy(mnemonicos[0x25], "JP");
    strcpy(mnemonicos[0x26], "JN");
    strcpy(mnemonicos[0x27], "JNZ");
    strcpy(mnemonicos[0x28], "JNP");
    strcpy(mnemonicos[0x29], "JNN");
    strcpy(mnemonicos[0x81], "SYS");
    strcpy(mnemonicos[0x8f], "STOP");
    strcpy(mnemonicos[0x50], "SLEN");
    strcpy(mnemonicos[0x51], "SMOV");
    strcpy(mnemonicos[0x53], "SCMP");
    strcpy(mnemonicos[0x40], "CALL");
    strcpy(mnemonicos[0x44], "PUSH");
    strcpy(mnemonicos[0x45], "POP");
    strcpy(mnemonicos[0x48], "RET");

    strcpy(registros[0],"PS");
    strcpy(registros[1],"CS");
    strcpy(registros[2],"DS");
    strcpy(registros[3],"ES");
    strcpy(registros[4],"IP");
    strcpy(registros[5],"SS");
    strcpy(registros[6],"SP");
    strcpy(registros[7],"BP");
    strcpy(registros[8],"AC");
    strcpy(registros[9],"CC");
    strcpy(registros[10],"AX");
    strcpy(registros[11],"BX");
    strcpy(registros[12],"CX");
    strcpy(registros[13],"DX");
    strcpy(registros[14],"EX");
    strcpy(registros[15],"FX");
}
