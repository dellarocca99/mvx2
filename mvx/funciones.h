typedef char cadena[5];
cadena mnemonicos[144];
cadena registros[16];

void diccionario();
void mov(int codop, int op1, int op2, int RAM[], int reg[]);
void push(int codop, int op1, int op2, int RAM[], int reg[]);
void pop(int codop, int op1, int op2, int RAM[], int reg[]);
void call(int codop, int op1, int op2, int RAM[], int reg[]);
void ret(int codop, int op1, int op2, int RAM[], int reg[]);
void slen(int codop, int op1, int op2, int RAM[], int reg[]);
void smov(int codop, int op1, int op2, int RAM[], int reg[]);
void scmp(int codop, int op1, int op2, int RAM[], int reg[]);
void add(int codop, int op1, int op2, int RAM[], int reg[]);
void sub(int codop, int op1, int op2, int RAM[], int reg[]);
void mul(int codop, int op1, int op2, int RAM[], int reg[]);
void DIV(int codop, int op1, int op2, int RAM[], int reg[]);
void mod(int codop, int op1, int op2, int RAM[], int reg[]);
void cmp(int codop, int op1, int op2, int RAM[], int reg[]);
void SWAP(int codop, int op1, int op2, int RAM[], int reg[]);
void rnd(int codop, int op1, int op2, int RAM[], int reg[]);
void and(int codop, int op1, int op2, int RAM[], int reg[]);
void or(int codop, int op1, int op2, int RAM[], int reg[]);
void not(int codop, int op1, int op2, int RAM[], int reg[]);
void xor(int codop, int op1, int op2, int RAM[], int reg[]);
void shl(int codop, int op1, int op2, int RAM[], int reg[]);
void shr(int codop, int op1, int op2, int RAM[], int reg[]);
void jmp(int codop, int op1, int op2, int RAM[], int reg[]);
void je(int codop, int op1, int op2, int RAM[], int reg[]);
void jg(int codop, int op1, int op2, int RAM[], int reg[]);
void jl(int codop, int op1, int op2, int RAM[], int reg[]);
void jz(int codop, int op1, int op2, int RAM[], int reg[]);
void jp(int codop, int op1, int op2, int RAM[], int reg[]);
void JN(int codop, int op1, int op2, int RAM[], int reg[]);
void jnz(int codop, int op1, int op2, int RAM[], int reg[]);
void jnp(int codop, int op1, int op2, int RAM[], int reg[]);
void jnn(int codop, int op1, int op2, int RAM[], int reg[]);
void sys(int codop, int op1, int op2, int RAM[], int reg[]);
void stop(int codop, int op1, int op2, int RAM[], int reg[]);
int devuelveValor(int Top, int op, int RAM[], int reg[]);
