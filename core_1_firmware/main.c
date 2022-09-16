// This file is Copyright (c) 2020 Florent Kermarrec <florent@enjoy-digital.fr>
// Modified by Joseph Faye
// Modified by Lucas Esteves <lucas.esteves-rocha@insa-rennes.fr>
// License: BSD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aes.h"
#include "amp_comms.h"
#include "amp_utils.h"
#include "img.h"

#include <irq.h>
#include <libbase/uart.h>
#include <libbase/console.h>
#include <generated/csr.h>

amp_comms_tx_t _tx __attribute__ ((section ("shared_ram_second")));
amp_comms_rx_t _rx __attribute__ ((section ("shared_ram_first")));
private_aes_data_t private_aes __attribute__ ((section ("priv_sp_last")));

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
    amp_aes_init(&private_aes);
    amp_comms_init(&_tx, &_rx);

    /* Initing nonce */
    amp_aes_update_nonce(&private_aes);

    prompt();

    const int MEASURE_STEPS = 50;
    uint32_t t_aes_begin, t_aes_end, counter;
    double lat_aes_ms;
    float time_spent_ms;
    amp_cmds_t cmd_rx;
    int img_size;
    uint8_t sel_op, class_predicted;

    counter = 0;

    while(1) {
        console_service();

         /* Checking comunicatoin data */
        cmd_rx = amp_comms_has_unread(&_rx);

        if(cmd_rx != AMP_NULL)
        {
            if(counter == 0)
            {
                printf("\n");
            }

            switch (cmd_rx)
            {
            case AMP_SEND_PREDICTION:
                amp_comms_receive(&_rx, &class_predicted, sizeof(class_predicted));
                sel_op = 1;
                break;
            case AMP_GET_IMG:
                amp_comms_receive(&_rx, (uint8_t *)&img_size, sizeof(img_size));
                sel_op = 2;
                break;
            default:
                 /* Blocking program, command not implemented */
                while(1);
                sel_op = 0;
                break;
            }

            if(sel_op == 1)
            {

                printf("Class received: %d - Measuring step: %lu/%d\r", class_predicted, counter+1, MEASURE_STEPS);

                t_aes_begin = amp_millis();

                int result = 0;
                
                result = amp_aes_update_nonce(&private_aes);
                result = amp_aes_encrypts(&class_predicted, &private_aes);
                
                t_aes_end = amp_millis();
                time_spent_ms = (t_aes_begin - t_aes_end)/(CONFIG_CLOCK_FREQUENCY/1000.0);
                lat_aes_ms += time_spent_ms;

                counter++;

                if (result != 0)
                {
                    printf("\e[91;1m\nError in the encryption. Err= %d\e[0m\n", result);
                }

            }
            else if(sel_op == 2)
            {
                // TODO: Make protections for img_size, so img_size isn't bigger than img.h/img_len
                amp_comms_send(&_tx, AMP_SEND_IMG, &img[0], img_size);
            }
        }  

        if(counter == MEASURE_STEPS)
        {
            counter = 0;
            time_spent_ms = lat_aes_ms/MEASURE_STEPS;
            int f_left = (int)time_spent_ms;
            int f_right = ((float)(time_spent_ms - f_left)*1000.0);
            printf("\nAES Latency for predicted class: %d is %d.%d ms\n", class_predicted, f_left, f_right);
            lat_aes_ms = 0;
            prompt();
        }
    }

    return 0;
}