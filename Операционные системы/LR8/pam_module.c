#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <security/pam_modules.h>
#include <security/pam_ext.h>
#include <unistd.h>
#include <pwd.h>
#include <syslog.h>


PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    const char *user;
    int retval;

    retval = pam_get_user(pamh, &user, "Username: ");
    if (retval != PAM_SUCCESS) return retval;

    struct passwd *pw = getpwnam(user);
    if (!pw) return PAM_USER_UNKNOWN;

    char path[512];
    snprintf(path, sizeof(path), "%s/.secret_token", pw->pw_dir);

    FILE *f = fopen(path, "r");
    if (f == NULL) {
        pam_syslog(pamh, LOG_ERR, "Secret file not found for user %s at %s", user, path);
        return PAM_AUTH_ERR;
    }

    char buffer[32];
    if (fgets(buffer, sizeof(buffer), f) != NULL) {
        if (strncmp(buffer, "OPEN_SESAME", 11) == 0) {
            fclose(f);
            return PAM_SUCCESS;
        }
    }

    fclose(f);
    return PAM_AUTH_ERR;
}

PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}