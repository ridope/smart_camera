// This file is Copyright (c) 2020 Florent Kermarrec <florent@enjoy-digital.fr>
// Modified by Joseph Faye
// Modified by Lucas Esteves <lucas.esteves-rocha@insa-rennes.fr>
// License: BSD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aes.h"
#include "amp_utils.h"

#include <irq.h>
#include <libbase/uart.h>
#include <libbase/console.h>
#include <generated/csr.h>

shared_data_t ctrl_data __attribute__ ((section ("shared_ram"))) ;
shared_data_t local_ctrl_data;
private_firev_data_t priv_data __attribute__ ((section ("priv_sp"))) ;

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
    printf("\e[92;1mCore1-console\e[0m> ");
}

/*-----------------------------------------------------------------------*/
/* Help                                                                  */
/*-----------------------------------------------------------------------*/

static void help(void)
{
    puts("\nAES app built "__DATE__" "__TIME__"\n");
    puts("Available commands:");
    puts("help               - Show this command");
    puts("reboot             - Reboot CPU");
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

    amp_millis_init();
    amp_aes_init(&priv_data);

    /* Initing nonce */
    amp_aes_update_nonce(&ctrl_data, &priv_data);

    prompt();

    const int MEASURE_STEPS = 50;
    uint32_t t_aes_begin, t_aes_end, counter;
    double lat_aes_ms;
    float time_spent_ms;

    counter = 0;

    while(1) {
        console_service();

        if(ctrl_data.flag == 1){
            
            /* Getting data from shared memory and releasing flag */
            memcpy(&local_ctrl_data, &ctrl_data, sizeof(shared_data_t));
            ctrl_data.flag = 0;

            printf("\n Class received: %d - Measuring step: %lu/%d\r", ctrl_data.predicted_class, counter+1, MEASURE_STEPS);           

            t_aes_begin = amp_millis();

            int result = 0;
            result = amp_aes_update_nonce(&local_ctrl_data, &priv_data);
            result = amp_aes_encrypts(&local_ctrl_data, &priv_data);

            t_aes_end = amp_millis();
            time_spent_ms = (t_aes_begin - t_aes_end)/(CONFIG_CLOCK_FREQUENCY/1000.0);
            lat_aes_ms += time_spent_ms;

            counter++;

            if (result != 0)
            {
                printf("\e[91;1m\nError in the encryption. Err= %d\e[0m\n", result);
            }
        }

        if(counter == MEASURE_STEPS)
        {
            counter = 0;
            time_spent_ms = lat_aes_ms/MEASURE_STEPS;
            int f_left = (int)time_spent_ms;
            int f_right = ((float)(time_spent_ms - f_left)*1000.0);
            printf("\nAES Latency for predicted class: %d is %d.%d ms\n", ctrl_data.predicted_class, f_left, f_right);
            lat_aes_ms = 0;
        }
    }

    return 0;
}