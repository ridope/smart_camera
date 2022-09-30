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
#include "comm_ridope.h"
#include "ridope_sp.h"
#include "imgs.h"

#include <irq.h>
#include <libbase/uart.h>
#include <libbase/console.h>
#include <generated/csr.h>

// Uncomment the next line to active the gaussian filter
//#define GAUSSIAN_FILTER

source_data_t source_data  __attribute__ ((section ("img_float")));
amp_comms_tx_t _tx __attribute__ ((section ("shared_ram_second")));
amp_comms_rx_t _rx __attribute__ ((section ("shared_ram_first")));

uint8_t img[IMG_HEIGTH*IMG_WIDTH]  __attribute__ ((section ("priv_sp")));
private_aes_data_t  priv_data __attribute__ ((section ("shared_ram")));

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

static void get_img(uint8_t *img){

	COMM_RIDOPE_MSG_t msg;
	msg.msg_data.cmd = CAMERA_EXPO;
	msg.msg_data.data = 11264; 

	comm_ridope_send_cmd(&msg);

	msg.msg_data.cmd = CAMERA_AVG;
	msg.msg_data.data = 50; 
	comm_ridope_send_cmd(&msg);
	
	printf("Sending img!\n");

	comm_ridope_send_img(&img[0],TRANS_PHOTO, IMG_WIDTH, IMG_HEIGTH);
	printf("Done sending!\n");	
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
    printf("Image ptr: %p\n", img);
    printf("Float image ptr: %p\n", &source_data.f_img[0]);
    printf("TX ptr: %p\n", &_tx);
    printf("RX ptr: %p\n", &_rx);
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
    amp_comms_init(&_tx, &_rx);

    /* Initing nonce */
    amp_aes_update_nonce(&priv_data);

    prompt();

    const int MEASURE_STEPS = 102;
    uint32_t t_aes_begin, t_aes_end, counter, t_send_begin,  t_send_end, t_receive_begin, t_receive_end;
    uint32_t t_otsu_begin, t_otsu_end, t_gaussian_begin, t_gaussian_end;
    double lat_aes_ms = 0;
    double lat_otsu_ms = 0;
    double lat_gaussian_ms = 0;
    double t_send_ms = 0;
    double t_receive_ms = 0;
    float time_spent_ms;
    amp_cmds_t cmd_rx;
    int img_size;
    uint8_t sel_op, class_predicted;
    uint8_t threshold = 0;
    uint32_t counter_otsu = 0;

    float f_img[IMG_HEIGTH*IMG_WIDTH];
    uint8_t img_temp_1[IMG_HEIGTH*IMG_WIDTH];
    uint8_t img_temp_2[IMG_HEIGTH*IMG_WIDTH];

    counter = 0;

    COMM_RIDOPE_MSG_t rx_msg;

    while(1) {
        /* Checking comunication data */
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
                    t_receive_begin = amp_millis();
                    amp_comms_receive(&_rx, &class_predicted, sizeof(class_predicted));
                    t_receive_end = amp_millis();

                    time_spent_ms = (t_receive_begin - t_receive_end)/(CONFIG_CLOCK_FREQUENCY/1000.0);
                    t_receive_ms += time_spent_ms;

                    if(source_data.flag_received)
                    {
                        memcpy(&source_data.f_img[0], &f_img[0], IMG_WIDTH*IMG_HEIGTH*4);
                        source_data.flag_received = 0;
                    }

                    sel_op = 1;
                    break;

                default:
                    /* Blocking program, command not implemented */
                    while(1);
                    sel_op = 0;
                    break;
            }

            if(sel_op  == 1){
                
                printf("Class received: %d - Measuring step: %lu/%d\n", class_predicted, counter+1, MEASURE_STEPS);          

                t_aes_begin = amp_millis();

                int result = 0;
                result = amp_aes_update_nonce(&priv_data);
                result = amp_aes_encrypts(&class_predicted, &priv_data);

                t_aes_end = amp_millis();
                time_spent_ms = (t_aes_begin - t_aes_end)/(CONFIG_CLOCK_FREQUENCY/1000.0);
                lat_aes_ms += time_spent_ms;
               
                // Copies captured image to local variable
                memcpy(&img_temp_1[0], &img[0], IMG_HEIGTH*IMG_WIDTH);

                #ifdef GAUSSIAN_FILTER
                t_gaussian_begin = amp_millis();
                ridope_gaussian_filter(&img_temp_1[0], &img_temp_2[0], IMG_HEIGTH, IMG_WIDTH, GAUSS_KER_SIZE, SIGMA);
                t_gaussian_end = amp_millis();

                time_spent_ms = (t_gaussian_begin - t_gaussian_end)/(CONFIG_CLOCK_FREQUENCY/1000.0);
                lat_gaussian_ms += time_spent_ms;

                t_otsu_begin = amp_millis();
                ridope_otsu(&img_temp_2[0], &f_img[0], &threshold, IMG_HEIGTH, IMG_WIDTH);
                threshold = 0;
                t_otsu_end = amp_millis();
                #else
                t_otsu_begin = amp_millis();
                ridope_otsu(&img_temp_1[0], &f_img[0], &threshold, IMG_HEIGTH, IMG_WIDTH);
                threshold = 0;
                t_otsu_end = amp_millis();
                #endif

                time_spent_ms = (t_otsu_begin - t_otsu_end)/(CONFIG_CLOCK_FREQUENCY/1000.0);
                lat_otsu_ms += time_spent_ms;

                counter++;

                if (result != 0)
                {
                    printf("\e[91;1m\nError in the encryption. Err= %d\e[0m\n", result);
                }
            }
        }

        if(counter == MEASURE_STEPS)
        {
            counter = 0;
            

            time_spent_ms = lat_aes_ms/MEASURE_STEPS;
            int f_left = (int)time_spent_ms;
            int f_right = ((float)(time_spent_ms - f_left)*1000.0);
            printf("\nAES Latency for predicted class: %d is %d.%d ms\n", class_predicted, f_left, f_right);

            time_spent_ms = lat_gaussian_ms/MEASURE_STEPS;
            f_left = (int)time_spent_ms;
            f_right = ((float)(time_spent_ms - f_left)*1000.0);
            printf("Gaussian Latency for predicted class: %d is %d.%d ms with %d steps\n", class_predicted, f_left, f_right);

            time_spent_ms = lat_otsu_ms/MEASURE_STEPS;
            f_left = (int)time_spent_ms;
            f_right = ((float)(time_spent_ms - f_left)*1000.0);
            printf("Otsu Latency for predicted class: %d is %d.%d ms with %d steps\n", class_predicted, f_left, f_right);

            time_spent_ms = t_send_ms/MEASURE_STEPS;
            f_left = (int)time_spent_ms;
            f_right = ((float)(time_spent_ms - f_left)*1000.0);
            printf("Total Communication send is %d.%d ms\n", f_left, f_right);

            time_spent_ms = t_receive_ms/MEASURE_STEPS;
            f_left = (int)time_spent_ms;
            f_right = ((float)(time_spent_ms - f_left)*1000.0);
            printf("Total Communication receive is %d.%d ms\n", f_left, f_right);

            t_receive_ms = 0;
            t_send_ms = 0;
            lat_aes_ms = 0;
            lat_otsu_ms = 0;

            prompt();
        }
    }

    return 0;
}