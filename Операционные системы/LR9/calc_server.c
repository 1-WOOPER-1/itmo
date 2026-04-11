#include "calc.h"
#include <rpc/rpc.h>
#include <stdio.h>

int *
add_1_svc(numbers *argp, struct svc_req *rqstp)
{
    static int result;

    printf("Запрос от клиента...\n");

    if (rqstp->rq_cred.oa_flavor != AUTH_SYS && rqstp->rq_cred.oa_flavor != AUTH_UNIX) {
        svcerr_weakauth(rqstp->rq_xprt);
        return NULL;
    }

    struct authunix_parms *aup = (struct authunix_parms *)rqstp->rq_clntcred;
    if (aup != NULL) {
        printf("Аутентификация пройдена. UID: %d, Host: %s\n", aup->aup_uid, aup->aup_machname);
    }

    result = argp->a + argp->b;
    printf("Сложение: %d + %d = %d\n", argp->a, argp->b, result);

    return &result;
}
