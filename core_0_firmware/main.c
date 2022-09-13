// This file is Copyright (c) 2020 Florent Kermarrec <florent@enjoy-digital.fr>
// Modified by Joseph Faye
// License: BSD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "amp_send.h"
#include "amp_utils.h"
#include "svm_model.h"
#include "img.h"

#include <irq.h>
#include <libbase/uart.h>
#include <libbase/console.h>
#include <generated/csr.h>

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
    printf("\e[92;1mfemtrov-console\e[0m> ");
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
    puts("enc                - Synchronized encryption");
#ifdef WITH_CXX
    puts("hellocpp           - Hello C++");
#endif
}

/*-----------------------------------------------------------------------*/
/* Commands                                                              */
/*-----------------------------------------------------------------------*/

static void reboot_cmd(void)
{
    ctrl_reset_write(1);
}

#ifdef CSR_LEDS_BASE
static void led_cmd(void)
{
	int i;
	printf("Led demo...\n");

	printf("Counter mode...\n");
	for(i=0; i<32; i++) {
		leds_out_write(i);
		busy_wait(100);
	}

	printf("Shift mode...\n");
	for(i=0; i<4; i++) {
		leds_out_write(1<<i);
		busy_wait(200);
	}
	for(i=0; i<4; i++) {
		leds_out_write(1<<(3-i));
		busy_wait(200);
	}

	printf("Dance mode...\n");
	for(i=0; i<4; i++) {
		leds_out_write(0x55);
		busy_wait(200);
		leds_out_write(0xaa);
		busy_wait(200);
	}
}
#endif


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

        const int MEASURE_STEPS = 100;
        double pred_time = 0;
        double aes_time = 0;
        uint32_t time_begin, time_end;
        float time_spent_ms;
        int class;

        for (int i = 0; i < MEASURE_STEPS; i++)
        {
            printf("Measuring step: %d/%d\r",i+1, MEASURE_STEPS);
            time_begin = amp_millis();
            class = predict(f_img);
            time_end = amp_millis();

            time_spent_ms = (time_begin - time_end)/(CONFIG_CLOCK_FREQUENCY/1000.0);
            pred_time += time_spent_ms;

            time_begin = amp_millis();
            amp_send_class(class);
            time_end = amp_millis();
            time_spent_ms = (time_begin - time_end)/(CONFIG_CLOCK_FREQUENCY/1000.0);
            aes_time += time_spent_ms;

        }

         printf("\n");

        time_spent_ms = pred_time/MEASURE_STEPS;

        /* Allowing printf to display float will increase code size, so the parts of the float number are being extracted belw */
        int f_left = (int)time_spent_ms;
        int f_right = ((float)(time_spent_ms - f_left)*1000.0);
        printf("Predicted class: %d in %d.%d ms\n", class, f_left, f_right);

        
        time_spent_ms = aes_time/MEASURE_STEPS;
        f_left = (int)time_spent_ms;
        f_right = ((float)(time_spent_ms - f_left)*1000.0);
        printf("Encrypted class: %d in %d.%d ms\n", class, f_left, f_right);
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
    amp_send_init();
    amp_millis_init();

    help();
    prompt();

    while(1) {
        console_service();
    }

    return 0;
}