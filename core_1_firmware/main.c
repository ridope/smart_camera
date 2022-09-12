// This file is Copyright (c) 2020 Florent Kermarrec <florent@enjoy-digital.fr>
// Modified by Joseph Faye
// Modified by Lucas Esteves <lucas.esteves-rocha@insa-rennes.fr>
// License: BSD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aes.h"

#include <irq.h>
#include <libbase/uart.h>
#include <libbase/console.h>
#include <generated/csr.h>

#include <mbedtls/gcm.h>
#include <mbedtls/cipher.h>

DATA data __attribute__ ((section ("joseph"))) ;

/*-----------------------------------------------------------------------*/
/* Uart                                                                  */
/*-----------------------------------------------------------------------*/

static char *readstr(void)
{
    char c[2];
    static char s[64];
    static int ptr = 0;

    if(readchar_nonblock()) {
        c[0] = getchar();
        c[1] = 0;
        switch(c[0]) {
            case 0x7f:
            case 0x08:
                if(ptr > 0) {
                    ptr--;
                    fputs("\x08 \x08", stdout);
                }
                break;
            case 0x07:
                break;
            case '\r':
            case '\n':
                s[ptr] = 0x00;
                fputs("\n", stdout);
                ptr = 0;
                return s;
            default:
                if(ptr >= (sizeof(s) - 1))
                    break;
                fputs(c, stdout);
                s[ptr] = c[0];
                ptr++;
                break;
        }
    }

    return NULL;
}

static char *get_token(char **str)
{
    char *c, *d;

    c = (char *)strchr(*str, ' ');
    if(c == NULL) {
        d = *str;
        *str = *str+strlen(*str);
        return d;
    }
    *c = 0;
    d = *str;
    *str = c+1;
    return d;
}

static void prompt(void)
{
    printf("\e[92;1mfirev-console\e[0m> ");
}

/*-----------------------------------------------------------------------*/
/* Help                                                                  */
/*-----------------------------------------------------------------------*/

static void help(void)
{
    puts("\nLiteX minimal demo app built "__DATE__" "__TIME__"\n");
    puts("Available commands:");
    puts("help               - Show this command");
    puts("reboot             - Reboot CPU");
#ifdef CSR_LEDS_BASE
    puts("led                - Led demo");
#endif
    puts("enc              - Synchronized Encryption");
    puts("dec              - Decryption function test");
}

/*-----------------------------------------------------------------------*/
/* Commands                                                              */
/*-----------------------------------------------------------------------*/

static void reboot_cmd(void)
{
    ctrl_reset_write(1);
}

/*-----------------------------------------------------------------------*/
/* Console service / Main                                                */
/*-----------------------------------------------------------------------*/

static void console_service(void)
{
    char *str;
    char *token;

    str = readstr();
    if(str == NULL) return;
    token = get_token(&str);
    if(strcmp(token, "help") == 0)
        help();
    else if(strcmp(token, "reboot") == 0)
        reboot_cmd();
    else if(strcmp(token, "enc") == 0){
        int result = 0;

        // TODO: Only call this function if an encryption can be performed
        result = amp_aes_update_nonce(&data);

        if (result == -1)
        {
            printf("\e[91;1mError updating the nonce: struct pointer is NULL\e[0m\n");
        }

        printf("Class received: %d\n", data.predicted_class);


        result = amp_aes_encrypts(&data);

        if (result == -1)
        {
            printf("\e[91;1mError in the encryption: struct pointer is NULL\e[0m\n");
        }
        else if (result == -4)
        {
            printf("\e[91;1mError in the encryption: flag is not active\e[0m\n");
        }


    }

    prompt();
}

int main(void)
{
#ifdef CONFIG_CPU_HAS_INTERRUPT
    irq_setmask(0);
	irq_setie(1);
#endif
    uart_init();
    
    help();

    amp_aes_init(&data);

    /* Initing nonce */
    amp_aes_update_nonce(&data);

    prompt();

    while(1) {
        console_service();
    }

    return 0;
}