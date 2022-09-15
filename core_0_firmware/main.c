// This file is Copyright (c) 2020 Florent Kermarrec <florent@enjoy-digital.fr>
// Modified by Joseph Faye
// Modified by Lucas Esteves <lucas.esteves-rocha@insa-rennes.fr>
// License: BSD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "amp_comms.h"
#include "amp_utils.h"
#include "svm_model.h"
#include "img.h"

#include <irq.h>
#include <libbase/uart.h>
#include <libbase/console.h>
#include <generated/csr.h>

amp_comms_tx_t _tx __attribute__ ((section ("shared_ram_first")));
amp_comms_rx_t _rx __attribute__ ((section ("shared_ram_second")));
private_aes_data_t private_aes __attribute__ ((section ("priv_sp_last")));

/*-----------------------------------------------------------------------*/
/* Uart                                                                  */
/*-----------------------------------------------------------------------*/

float *f_img = (float *)&img;

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
    printf("\e[92;1mCore0-console\e[0m> ");
}

/*-----------------------------------------------------------------------*/
/* Help                                                                  */
/*-----------------------------------------------------------------------*/

static void help(void)
{
    puts("\nSVM app built "__DATE__" "__TIME__"\n");
    puts("Available commands:");
    puts("help               - Show this command");
    puts("reboot             - Reboot CPU");
    puts("enc                - Synchronized encryption");
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
#ifdef CSR_LEDS_BASE
        else if(strcmp(token, "led") == 0)
		led_cmd();
#endif
    else if(strcmp(token, "enc") == 0){

        const int MEASURE_STEPS = 50;
        double throughput_ms = 0;
        double lat_svm_ms = 0;
        uint32_t time_begin, time_end;
        uint32_t t_svm_begin, t_svm_end;
        float time_spent_ms;
        uint8_t class;
        amp_cmds_t cmd_rx;

        for (int i = 0; i < MEASURE_STEPS; i++)
        {
            printf("Measuring step: %d/%d\r",i+1, MEASURE_STEPS);
            
            time_begin = amp_millis();          
            
            t_svm_begin = time_begin;

            class = (uint8_t) predict(f_img);


            t_svm_end = amp_millis(); 

            /* Checking comunicatoin data */
            cmd_rx = amp_comms_has_unread(&_rx);

            if(cmd_rx != AMP_NULL)
            {
                switch (cmd_rx)
                {
                case AMP_SEND_AES_PRIV_DATA:
                    amp_comms_receive(&_rx, (uint8_t * ) &private_aes, sizeof(private_aes));
                    break;
                
                default:
                    /* Clearing received commands, not implemented */
                    amp_comms_receive(&_rx, NULL, 0);
                    break;
                }
            }  

            int result = amp_comms_send(&_tx, AMP_SEND_PREDICTION, &class, sizeof(class));

            time_end = amp_millis();

            time_spent_ms = (t_svm_begin - t_svm_end)/(CONFIG_CLOCK_FREQUENCY/1000.0);
            lat_svm_ms += time_spent_ms;

            time_spent_ms = (time_begin - time_end)/(CONFIG_CLOCK_FREQUENCY/1000.0);
            throughput_ms += time_spent_ms;
        }

         printf("\n");


        /* Allowing printf to display float will increase code size, so the parts of the float number are being extracted belw */
        time_spent_ms = lat_svm_ms/MEASURE_STEPS;
        int f_left = (int)time_spent_ms;
        int f_right = ((float)(time_spent_ms - f_left)*1000.0);
        printf("SVM Latency for predicted class: %d is %d.%d ms\n", class, f_left, f_right);

        time_spent_ms = throughput_ms/MEASURE_STEPS;
        f_left = (int)time_spent_ms;
        f_right = ((float)(time_spent_ms - f_left)*1000.0);
        printf("Throughput for predicted class: %d is %d.%d ms\n", class, f_left, f_right);
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
    amp_comms_init(&_tx, &_rx);
    amp_millis_init();

    help();
    prompt();

    while(1) {
        console_service();
    }

    return 0;
}