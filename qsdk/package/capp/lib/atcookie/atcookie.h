#ifndef __ATCOOKIE_H_396D23194B7C07C5A9B0653AE7AA5104__
#define __ATCOOKIE_H_396D23194B7C07C5A9B0653AE7AA5104__
/******************************************************************************/
#include "utils.h"
/******************************************************************************/
#define AT_CERT_BUS_V1_0        "bus1.0"
#define AT_CERT_LMS_V1_0        "lms1.0"
#define AT_CERT_LSS_V1_0        "lss1.0"
#define AT_PWDFILE_RSYNC_V1_0   "rpf1.0"

#ifndef AT_CERT_BUS
#define AT_CERT_BUS             AT_CERT_BUS_V1_0
#endif

#ifndef AT_CERT_LMS
#define AT_CERT_LMS             AT_CERT_LMS_V1_0
#endif

#ifndef AT_CERT_LSS
#define AT_CERT_LSS             AT_CERT_LSS_V1_0
#endif

#ifndef AT_PWDFILE_RSYNC
#define AT_PWDFILE_RSYNC        AT_PWDFILE_RSYNC_V1_0
#endif

#ifndef AT_CERT
#define AT_CERT                 AT_CERT_LMS
#endif

#ifndef AT_PWD_RSYNC
#define AT_PWD_RSYNC            "ltefiV2rsyncPWD"
#endif

typedef struct {
    char *name;
    char **line;
    int count;
} secookie_t;

#define SECOOKIE_ENTRY(_name, _line) {  \
    .name = _name,                      \
    .line = _line,                      \
    .count= os_count_of(_line),         \
}   /* end */

static inline secookie_t *
at_secookie(char *name)
{
    static char *line_bus_v1_0[] = {
        "-----BEGIN CERTIFICATE-----",
        "MIICSTCCAbKgAwIBAgIEVBJdrDANBgkqhkiG9w0BAQUFADBpMQswCQYDVQQGEwJj",
        "bjELMAkGA1UECBMCYmoxEDAOBgNVBAcTB2hhaWRpYW4xEDAOBgNVBAoTB2F1dGVs",
        "YW4xEDAOBgNVBAsTB2F1dGVsYW4xFzAVBgNVBAMTDjE5Mi4xNjguMjUuMTM5MB4X",
        "DTE0MDkxMjAyNDI1MloXDTI0MDcyMTAyNDI1MlowaTELMAkGA1UEBhMCY24xCzAJ",
        "BgNVBAgTAmJqMRAwDgYDVQQHEwdoYWlkaWFuMRAwDgYDVQQKEwdhdXRlbGFuMRAw",
        "DgYDVQQLEwdhdXRlbGFuMRcwFQYDVQQDEw4xOTIuMTY4LjI1LjEzOTCBnzANBgkq",
        "hkiG9w0BAQEFAAOBjQAwgYkCgYEA8yESW6BZ8ra+l/cRmtCyXsJt0BA4w1iPRHtq",
        "N4pc+P20dlTv+IAMwB6S4oPvInb0+xwEl6SeKi87LDLgAJSBmovhMwE6SO/tdPLf",
        "Sz/LKInZ3vU74NIBorOqesZWnIbK3/189At4kUkuUDNKnsCuy7FeZrA6bVCL0dju",
        "6l8vz5MCAwEAATANBgkqhkiG9w0BAQUFAAOBgQDwAENHVl+Q8lA7L4VGQJ6M+P+L",
        "JZBpQGz9qfauyupmf3GZIwMftINAP9RPwcilxwGHjWdXl92p3gjXwLX5pHV9M7mY",
        "/zhvw3rkf3hP2Esn8dRGFhDexLu1BBZ5gwh5nsHRFTmUX+tBU+Vpj7Ce3wAbxl0T",
        "m3Taucak5rIPYeyx2w==",
        "-----END CERTIFICATE-----",
    };

    static char *line_lms_v1_0[] = {
        "-----BEGIN CERTIFICATE-----",
        "MIICSTCCAbKgAwIBAgIEVBJdrDANBgkqhkiG9w0BAQUFADBpMQswCQYDVQQGEwJj",
        "bjELMAkGA1UECBMCYmoxEDAOBgNVBAcTB2hhaWRpYW4xEDAOBgNVBAoTB2F1dGVs",
        "YW4xEDAOBgNVBAsTB2F1dGVsYW4xFzAVBgNVBAMTDjE5Mi4xNjguMjUuMTM5MB4X",
        "DTE0MDkxMjAyNDI1MloXDTI0MDcyMTAyNDI1MlowaTELMAkGA1UEBhMCY24xCzAJ",
        "BgNVBAgTAmJqMRAwDgYDVQQHEwdoYWlkaWFuMRAwDgYDVQQKEwdhdXRlbGFuMRAw",
        "DgYDVQQLEwdhdXRlbGFuMRcwFQYDVQQDEw4xOTIuMTY4LjI1LjEzOTCBnzANBgkq",
        "hkiG9w0BAQEFAAOBjQAwgYkCgYEA8yESW6BZ8ra+l/cRmtCyXsJt0BA4w1iPRHtq",
        "N4pc+P20dlTv+IAMwB6S4oPvInb0+xwEl6SeKi87LDLgAJSBmovhMwE6SO/tdPLf",
        "Sz/LKInZ3vU74NIBorOqesZWnIbK3/189At4kUkuUDNKnsCuy7FeZrA6bVCL0dju",
        "6l8vz5MCAwEAATANBgkqhkiG9w0BAQUFAAOBgQDwAENHVl+Q8lA7L4VGQJ6M+P+L",
        "JZBpQGz9qfauyupmf3GZIwMftINAP9RPwcilxwGHjWdXl92p3gjXwLX5pHV9M7mY",
        "/zhvw3rkf3hP2Esn8dRGFhDexLu1BBZ5gwh5nsHRFTmUX+tBU+Vpj7Ce3wAbxl0T",
        "m3Taucak5rIPYeyx2w==",
        "-----END CERTIFICATE-----",
    };

    static char *line_lss_v1_0[] = {
        "-----BEGIN CERTIFICATE-----",
        "MIIC/zCCAeegAwIBAgIJALV55uT+mjveMA0GCSqGSIb3DQEBCwUAMBYxFDASBgNV",
        "BAMMC2F1dGVsYW4uY29tMB4XDTE1MDUyMDA1NDEzOVoXDTI1MDUxNzA1NDEzOVow",
        "FjEUMBIGA1UEAwwLYXV0ZWxhbi5jb20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAw",
        "ggEKAoIBAQCkFhM1w22jty5TFPpw/EwrtSC7g/RLTVic87L1YIk4X1O3mlzGiajZ",
        "D8RXtwwQrL2VG0fjxfTB9FZujF3xU/gnJJwF3Vj9phoMoqoxTZqtpK9QYMU7fb6N",
        "C1rBtlsMxSNlxkCuTSYl0TkNg9zXpt+udiKGiw0E/X+kOIAgJERrz3NP7t3zLMuc",
        "tFkjIxNspoWXQlUK2Bljl8tBvk3orfvmj1tK4aNXt1ufmMMbrMeTDc8I9TpKS6rv",
        "Tpx5vDE4U1YTmr8GTNvA/PGmWtsb19VIzaw8T0wXnCYOCeLpjn1mZDr2UFXTGJEE",
        "d8iH+xtczrxZi4uz2dmJ//09Bi6czu6nAgMBAAGjUDBOMB0GA1UdDgQWBBRLE3c+",
        "mz9JyE8O3+oLEmjfqj4r5zAfBgNVHSMEGDAWgBRLE3c+mz9JyE8O3+oLEmjfqj4r",
        "5zAMBgNVHRMEBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQBXS/8S4G5KR1DPrOWY",
        "DgCpr8ID/YkSDTCCvUWEi+1PZCpSMDAjCdT75i0s/SbXLQZKqJZqDWSqprNQuaHu",
        "fLNdT852AKev5DyZIMSTcEn7Yc6EEbNLB3SyrIcXlehB1KSZ7jFEKYgmaqBlF63+",
        "jSgZRFYVHIQ2rGhTHCgbeo8m8NG9gxa5H6Adqw9bTIwzPbEJf6EcfDOSWsZam04d",
        "clWDLbdAhXlN7pkbkm/aya302BUOlApJNzlOJjimc/DUIZ/Yl70V20OdOfuI8i8S",
        "d1bl0sHHekV0R5NLeJsO157fGFaXYihczIaMX6LsigR0ZBcsqy6sMlxnVzaEkfaN",
        "iCza",
        "-----END CERTIFICATE-----",
    };

    static char *line_rpf_v1_0[] = {
        AT_PWD_RSYNC
    };
    
    static secookie_t cert[] = {
        SECOOKIE_ENTRY(AT_CERT_BUS_V1_0, line_bus_v1_0),
        SECOOKIE_ENTRY(AT_CERT_LMS_V1_0, line_lms_v1_0),
        SECOOKIE_ENTRY(AT_CERT_LSS_V1_0, line_lss_v1_0),
        SECOOKIE_ENTRY(AT_PWDFILE_RSYNC_V1_0, line_rpf_v1_0),
    };
    int i;

    if (NULL==name) {
        return NULL;
    }
    
    for (i=0; i<os_count_of(cert); i++) {
        if (0==os_strcmp(name, cert[i].name)) {
            return &cert[i];
        }
    }
    
    return NULL;
}

#define AT_SECOOKIE_PREFIX  "/tmp/.cookie"

static char *
at_secookie_file(char *name)
{
    /*
    * mktemp reutrn it, must static
    */
    static char tmpfile[sizeof(AT_SECOOKIE_PREFIX ".XXXXXXXXXXXXXXXX")];
    int i, err;
    
    secookie_t *cert = at_secookie(name);
    if (NULL==cert) {
        debug_error("no-found cookie %s", name);
        
        return NULL;
    }

    srand(time(NULL));

    do{
        os_saprintf(tmpfile, AT_SECOOKIE_PREFIX ".%x%x", getpid(), rand());
    } while(os_file_exist(tmpfile));

    FILE *f = os_fopen(tmpfile, "w+");
    if (NULL==f) {
        return NULL;
    }
    
    for (i=0; i<cert->count; i++) {
        os_fprintln(f, "%s", cert->line[i]);
    }
    os_fclose(f);

    return tmpfile;
}

/******************************************************************************/
#endif /* __ATCOOKIE_H_396D23194B7C07C5A9B0653AE7AA5104__ */
