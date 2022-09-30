/*
 * ridope_sp.h
 *
 *  Created on: 22 août 2022
 *      Author: lucas
 */

#ifndef RIDOPE_SP_H_
#define RIDOPE_SP_H_

#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <string.h>

#define GAUSS_KER_SIZE 5
#define SOBEL_KER_SIZE 3
#define SIGMA 1.0

typedef enum ROTATION_TYPE
{
	CLOCK_WISE =0,
	COUNTERCLOCK_WISE

} rotation_t;

uint8_t ridope_histogram(const uint8_t *img_in, size_t img_size, float *hist_out, uint16_t hist_max);
uint8_t  get_updated_index(int index, uint8_t limit_min, uint8_t limit_max);
uint8_t ridope_get_max(const uint8_t *img_in, size_t height, size_t width);
uint8_t ridope_otsu(const uint8_t *img_in, float *img_out, uint8_t *threshold, size_t height, size_t width);
uint8_t ridope_gaussian_kernel(double *kernel_out, size_t kernel_size, float sigma);
uint8_t ridope_sobel_kernel(double *Gx_out, double *Gy_out , size_t kernel_size);
uint8_t ridope_conv(const uint8_t *img_in, uint8_t *img_out, size_t height, size_t width, double *kernel_in, size_t kernel_size);
uint8_t ridope_gaussian_filter(const uint8_t *img_in, uint8_t *img_out, size_t height, size_t width, size_t kernel_size, float sigma);
uint8_t ridope_sobel_filter(const uint8_t *img_in, uint8_t *img_x_out, uint8_t *img_y_out, size_t height, size_t width, size_t kernel_size);
uint8_t ridope_get_mag_ang(const uint8_t *img_x_in, const uint8_t *img_y_in, uint8_t *mag_out, uint8_t *ang_out, size_t height, size_t width);
uint8_t ridope_complement(uint8_t *img_in_out, size_t height, size_t width);
uint8_t ridope_scaling(uint8_t *img_in_out, size_t height, size_t width);
uint8_t ridope_non_max_supression(uint8_t *mag_in, uint8_t *ang_in, uint8_t *img_out, size_t height, size_t width);
uint8_t ridope_edge_tracking(uint8_t *img_in_out, uint8_t high_threshold, uint8_t low_threshold, size_t height, size_t width);
uint8_t ridope_canny(uint8_t *img_in, uint8_t *img_out, uint8_t high_threshold, uint8_t low_threshold, size_t height, size_t width);
#endif /* RIDOPE_SP_H_ */
