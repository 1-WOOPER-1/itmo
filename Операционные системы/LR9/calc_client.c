#include "calc.h"
#include <stdio.h>
#include <rpc/rpc.h>
#include <stdlib.h>

void calc_prog_1(char *host, int a, int b) {
    CLIENT *clnt;
    int *result_1;
    numbers add_1_arg;

    clnt = clnt_create(host, CALC_PROG, CALC_VERS, "udp");
    if (clnt == NULL) {
        clnt_pcreateerror(host);
        exit(1);
    }

    clnt->cl_auth = authunix_create_default();

    add_1_arg.a = a;
    add_1_arg.b = b;

    result_1 = add_1(&add_1_arg, clnt);

    if (result_1 == (int *)NULL) {
        clnt_perror(clnt, "Вызов не удался");
    } else {
        printf("Результат от сервера: %d\n", *result_1);
    }

    auth_destroy(clnt->cl_auth);
    clnt_destroy(clnt);
}

int main(int argc, char *argv[]) {
    char *host;
    if (argc < 4) {
        printf("Использование: %s <host> <num1> <num2>\n", argv[0]);
        exit(1);
    }
    host = argv[1];
    calc_prog_1(host, atoi(argv[2]), atoi(argv[3]));
    exit(0);
}
