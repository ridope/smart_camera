/*
 * ridope_sp.c
 *
 *  Created on: 22 août 2022
 *      Author: lucas
 */
#include "ridope_sp.h"

/**
 * @brief Generates the normalized histogram of the image
 * 
 * @param img_in 	Pointer to the input image
 * @param img_size  Size of the image
 * @param hist_out  Pointer to the output histrogram
 * @param hist_max 	Max level of the histogram
 * @return uint8_t  Returns 0 if success
 */
uint8_t ridope_histogram(const uint8_t *img_in, size_t img_size, float *hist_out, uint16_t hist_max)
{
	/* Checking input pointers */
	if(img_in == NULL)
	{
		return 1;
	}

	if(hist_out == NULL)
	{
		return 2;
	}

	for (int i = 0; i <= hist_max; i++)
	{
		hist_out[i] = 0;
	}


	for(int i = 0; i < img_size; i++)
	{
		uint8_t value = img_in[i];
		hist_out[value] += 1;
	}

	return 0;
}

/**
 * @brief Otsu's method
 * 
 * @param img_in 	Pointer to the input image
 * @param img_out 	Pointer to the output image
 * @param height 	Height of the input image
 * @param width 	Width of the input image	
 * @return uint8_t  Return 0 if success
 */
uint8_t ridope_otsu(const uint8_t *img_in, float *img_out, uint8_t *threshold, size_t height, size_t width)
{
	/* Checking input pointers */
	if(img_in == NULL)
	{
		return 1;
	}

	if(img_out == NULL && threshold == NULL)
	{
		return 2;
	}

	if(threshold == NULL)
	{
		return 3;
	}

	/* Variable initialization */
	int N = height*width;

	*threshold = 0;
	uint8_t max_intensity = 255;
	double var_class = 0;
	double var_max = 0;
	double sum = 0;
	double sumB = 0;
	double q1 = 0;
	double q2 = 0;
	double u1 = 0;
	double u2 = 0;
	float histogram[max_intensity+1];

	/* Image histogram */
	ridope_histogram(img_in, N, &histogram[0], max_intensity);

	for (int i = 0; i <= max_intensity; i++)
	{
		sum += i*histogram[i];
	}

	for (int i = 0; i <= max_intensity; i++)
	{
		q1 += histogram[i];

		if(q1 == 0) continue;

		q2 = N - q1;

		sumB += i*histogram[i];
		u1 = sumB/q1;
		u2 = (sum - sumB)/q2;

		var_class = q1*q2*((u1 - u2)*(u1 - u2));

		if (var_class > var_max)
		{
			*threshold = i;
			var_max = var_class;
		}

	}

	/* Applying Threshold if output image is not null */
	if(img_out != NULL)
	{
		for(int i = 0; i < N; i++)
			{
				if (img_in[i] > (*threshold))
				{
					img_out[i] = 0;
				}
				else
				{
					img_out[i] = 255;
				}
			}
	}

	return 0;
}

/**
 * @brief Generates a gaussian kernel
 * 
 * @param kernel_out 	Pointer to store the kernel generated
 * @param kernel_size 	Size of the kernel
 * @param sigma 		The standard deviation of the kernel
 * @return uint8_t 		Returns 0 if success
 */
uint8_t ridope_gaussian_kernel(double *kernel_out, size_t kernel_size, float sigma)
{
	if(kernel_out == NULL)
	{
		return 1;
	}
	double sum = 0;

	// Middle of the kernel
	double offset = (kernel_size - 1) / 2.0;

	 for (int i = 0; i < kernel_size; i++)
	 {
		for (int j = 0; j < kernel_size; j++)
		{
			double x = i - offset;
			double y = j - offset;
			kernel_out[kernel_size * i + j] = exp(-(x*x + y*y) / (2 * sigma* sigma ));
			sum += kernel_out[kernel_size * i + j];
		}
	}

	 for (int i = 0; i < kernel_size*kernel_size; i++)
	 {
		 kernel_out[i] /= sum;
	 }

	 return 0;
}

/**
 * @brief Generates a Sobel kernel
 * 
 * @param kernel_out 	Pointer to store the kernel generated
 * @param kernel_size 	Size of the kernel
 * @return uint8_t 		Returns 0 if success
 */
uint8_t ridope_sobel_kernel(double *Gx_out, double *Gy_out , size_t kernel_size)
{
	/* Checking input pointers */
	if(Gx_out == NULL)
	{
		return 1;
	}

	if(Gy_out == NULL)
	{
		return 2;
	}

	// Middle of the kernel
	uint8_t offset = (kernel_size - 1) / 2;

	for (int i = 0; i < kernel_size; i++)
	{
		for (int j = 0; j < kernel_size; j++)
		{
			double x = i - offset;
			double y = j - offset;

			if (x == 0 && y == 0)
			{
				Gy_out[kernel_size * i + j] = 0;
				Gx_out[kernel_size * i + j] = 0;
			}
			else
			{
				Gy_out[kernel_size * i + j] = x / (2*(x*x + y*y));
				Gx_out[kernel_size * i + j] = y / (2*(x*x + y*y));
			}
		}
	}

	return 0;
}


/**
 * @brief Makes a convolution of the filter with an image
 * 
 * @param img_in 		Pointer to the input image
 * @param img_out 		Pointer to the output image
 * @param height 		Height of the input image
 * @param width 		Width of the input image
 * @param kernel_in 	Pointer to the filter
 * @param kernel_size 	Filter size
 * @return uint8_t 		Returns 0 if success
 */
uint8_t ridope_conv(const uint8_t *img_in, uint8_t *img_out, size_t height, size_t width, double *kernel_in, size_t kernel_size)
{
	/* Checking input pointers */
	if(img_in == NULL)
	{
		return 1;
	}

	if(img_out == NULL)
	{
		return 2;
	}

	if(kernel_in == NULL)
	{
		return 3;
	}

	// Middle of the kernel
	uint8_t offset = kernel_size / 2;
	double pixel;
	double pixel_result;
	double pixel_mask;

	for(int x = 0; x < height; x++)
	{
		for(int y = 0; y < width; y++)
		{
			pixel_result = 0;

			for(int a = 0; a < kernel_size; a++)
			{
				for(int b = 0; b < kernel_size; b++)
				{
					int x_n = x + a - offset;
					int y_n = y + b - offset;

					if (x_n < 0 || y_n < 0 || x_n == width || y_n == height)
					{
						int x_near = x_n;
						int y_near = y_n;

						if(y_n < 0)
						{
							y_near = 0;
						}

						if(x_n < 0)
						{
							x_near = 0;
						}

						if(x_n == width)
						{
							x_near = width - 1;
						}

						if(y_n == height)
						{
							y_near = height - 1;
						}
						pixel_mask = kernel_in[kernel_size * a + b];
						pixel = img_in[width * x_near + y_near]*pixel_mask;
					}
					else
					{
						pixel_mask = kernel_in[kernel_size * a + b];
						pixel = img_in[width * x_n + y_n]*pixel_mask;
					}
					pixel_result += pixel;
				}
			}
			if (pixel_result < 0){
				pixel_result = pixel_result * -1;
			}
			img_out[width * x + y] = pixel_result;

		}
	}

	return 0;

}

/**
 * @brief Applies the Gaussian filter in a image
 * 
 * @param img_in 		Pointer to the input image
 * @param img_out 		Pointer to the output image
 * @param height 		Height of the input image
 * @param width 		Width of the input image 
 * @param kernel_size 	Filter size
 * @param sigma 		The standard deviation of the kernel
 * @return uint8_t 		Returns 0 if success
 */
uint8_t ridope_gaussian_filter(const uint8_t *img_in, uint8_t *img_out, size_t height, size_t width, size_t kernel_size, float sigma)
{
	/* Checking input pointers */
	if(img_in == NULL)
	{
		return 1;
	}

	if(img_out == NULL)
	{
		return 2;
	}

	double *kernel = (double *)malloc(kernel_size*kernel_size*sizeof(double));

	/* Checking memory allocation */
	if(kernel == NULL)
	{
		return 3;
	}


	ridope_gaussian_kernel(kernel, kernel_size, sigma);

	ridope_conv(img_in, img_out, height, width, kernel, kernel_size);

	free(kernel);

	return 0;
}

/**
 * @brief Applies the Sobel filter in a image
 * 
 * @param img_in 		Pointer to the input image	
 * @param img_x_out 	Pointer to the X component output image
 * @param img_y_out 	Pointer to the Y component output image
 * @param height 		Height of the input image
 * @param width 		Width of the input image 
 * @param kernel_size 	Filter size
 * @return uint8_t 		Returns 0 if success
 */
uint8_t ridope_sobel_filter(const uint8_t *img_in, uint8_t *img_x_out, uint8_t *img_y_out, size_t height, size_t width, size_t kernel_size)
{
	/* Checking input pointers */
	if(img_in == NULL)
	{
		return 1;
	}

	if(img_x_out == NULL)
	{
		return 2;
	}

	if(img_y_out == NULL)
	{
		return 3;
	}

	double *Gx = (double *)malloc(kernel_size*kernel_size*sizeof(double));
	double *Gy = (double *)malloc(kernel_size*kernel_size*sizeof(double));

	/* Checking memory allocation */
	if(Gx == NULL)
	{
		return 4;
	}

	if(Gy == NULL)
	{
		return 5;
	}

	ridope_sobel_kernel(&Gx[0],  &Gy[0], kernel_size);

	ridope_conv(img_in, img_x_out, height, width, &Gx[0], kernel_size);
	ridope_conv(img_in, img_y_out, height, width, &Gy[0], kernel_size);

	free(Gx);
	free(Gy);
	return 0;
}

/**
 * @brief Retrieves the magnitude and angle from the Sobel filter components 
 * 
 * @param img_x_in 		Pointer to the Sobel X component input image
 * @param img_y_in 		Pointer to the Sobel Y component input image
 * @param mag_out 		Pointer to the magnitude output matrix
 * @param ang_out 		Pointer to the angle output matrix
 * @param height 		Height of the input image
 * @param width 		Width of the input image  
 * @return uint8_t 		Returns 0 if success
 */
uint8_t ridope_get_mag_ang(const uint8_t *img_x_in, const uint8_t *img_y_in, uint8_t *mag_out, uint8_t *ang_out, size_t height, size_t width)
{
	/* Checking input pointers */
	if(img_x_in == NULL)
	{
		return 1;
	}

	if(img_y_in == NULL)
	{
		return 2;
	}

	if(mag_out == NULL && ang_out == NULL)
	{
		return 3;
	}

	for(int x = 0; x < height; x++)
	{
		for(int y = 0; y < width; y++)
		{
			if(mag_out != NULL)
			{
				mag_out[width * x + y] = sqrt( img_x_in[width * x + y]*img_x_in[width * x + y] + img_y_in[width * x + y]*img_y_in[width * x + y]);
			}

			if(ang_out != NULL)
			{
				double ang_d = atan2(img_y_in[width * x + y], img_x_in[width * x + y]) * 180/M_PI;

				if((ang_d >= 0 &&  ang_d <= 22.5) || (ang_d > 157.5 &&  ang_d <= 180))
				{
					ang_out[width * x + y] = 0;
				}
				else if(ang_d > 22.5 &&  ang_d <= 67.5)
				{
					ang_out[width * x + y] = 45;
				}
				else if(ang_d > 67.5 &&  ang_d <= 112.5)
				{
					ang_out[width * x + y] = 90;
				}
				else if(ang_d > 112.5 &&  ang_d <= 157.5)
				{
					ang_out[width * x + y] = 135;
				}

			}
		}
	}

	ridope_scaling(mag_out, height, width);

	return 0;
}

uint8_t ridope_complement(uint8_t *img_in_out, size_t height, size_t width)
{
	/* Checking input pointers */
	if(img_in_out == NULL)
	{
		return 1;
	}

	for(int i = 0; i < height*width; i++)
	{
		img_in_out[i] = 255 - img_in_out[i];
	}

	return 0;
}

/**
 * @brief Searches and returns the max pixel value of the image
 * 
 * @param img_in 		Pointer to the input image	
 * @param height 		Height of the input image
 * @param width 		Width of the input image  
 * @return uint8_t 		The max pixel value of the image input 
 */
uint8_t ridope_get_max(const uint8_t *img_in, size_t height, size_t width)
{
	/* Checking input pointers */
	if(img_in == NULL)
	{
		return 0;
	}

	int max = 0;

	for(int i = 0; i < height*width; i++)
	{
		if(img_in[i] > max) max = img_in[i];
	}

	return max;
}

/**
 * @brief Scales the image between 0 and 255
 * 
 * @param img_in_out 	Pointer to the input/output image	
 * @param height 		Height of the input image
 * @param width 		Width of the input image  
 * @return uint8_t 		Returns 0 if success  
 */
uint8_t ridope_scaling(uint8_t *img_in_out, size_t height, size_t width)
{
	/* Checking input pointers */
	if(img_in_out == NULL)
	{
		return 1;
	}

	int max = ridope_get_max(img_in_out, height, width);

	if (max == 0){
		return 0;
	}

	for(int i = 0; i < height*width; i++)
	{
		img_in_out[i] = img_in_out[i] * 255/ max;
	}
	return 0;
}


/**
 * @brief Checks if a index is out of bounds and corrects it to the limits
 * 
 * @param index 		The index to be checked
 * @param limit_min 	The lower limit
 * @param limit_max 	The upper limit
 * @return uint8_t 		The corrected index
 */
uint8_t  get_updated_index(int index, uint8_t limit_min, uint8_t limit_max)
{

	if(index < limit_min)
	{
		return limit_min;
	}

	if(index > limit_max)
	{
		return limit_max;
	}

	return index;
}

/**
 * @brief Supresses the non-local-maximum pixels in the image
 * 
 * @param mag_in 		Pointer to the Sobel magnitude matrix
 * @param ang_in 		Pointer to the Sobel angle matrix
 * @param img_out 		Pointer to the output image
 * @param height 		Height of the input image
 * @param width 		Width of the input image  
 * @return uint8_t 		Returns 0 if success
 */
uint8_t ridope_non_max_supression(uint8_t *mag_in, uint8_t *ang_in, uint8_t *img_out, size_t height, size_t width)
{
	/* Checking input pointers */
	if(mag_in == NULL)
	{
		return 1;
	}

	if(ang_in == NULL)
	{
		return 2;
	}

	if(img_out == NULL)
	{
		return 3;
	}

	for(int x = 0; x < height; x++)
	{
		for(int y = 0; y < width; y++)
		{
			int neighbor_x_1, neighbor_y_1;
			int neighbor_x_2, neighbor_y_2;

			if(ang_in[width * x + y] == 0)
			{
				neighbor_x_1 = x;
				neighbor_y_1 = get_updated_index(y - 1, 0, width-1);
				neighbor_x_2 = x;
				neighbor_y_2 = get_updated_index(y + 1, 0, width-1);
			}
			else if(ang_in[width * x + y] == 45)
			{
				neighbor_x_1 = get_updated_index(x - 1, 0, height-1);
				neighbor_y_1 = get_updated_index(y + 1, 0, width-1);
				neighbor_x_2 = get_updated_index(x + 1, 0, height-1);
				neighbor_y_2 = get_updated_index(y - 1, 0, width-1);
			}
			else if(ang_in[width * x + y] == 90)
			{
				neighbor_x_1 = get_updated_index(x - 1, 0, height-1);
				neighbor_y_1 = y;
				neighbor_x_2 = get_updated_index(x + 1, 0, height-1);
				neighbor_y_2 = y;
			}
			else if(ang_in[width * x + y] == 135)
			{
				neighbor_x_1 = get_updated_index(x - 1, 0, height-1);
				neighbor_y_1 = get_updated_index(y - 1, 0, width-1);
				neighbor_x_2 = get_updated_index(x + 1, 0, height-1);
				neighbor_y_2 = get_updated_index(y + 1, 0, width-1);
			}

			if((mag_in[width * x + y] >= mag_in[width * neighbor_x_1 + neighbor_y_1]) && (mag_in[width * x + y] >= mag_in[width * neighbor_x_2 + neighbor_y_2]))
			{
				img_out[width * x + y] = mag_in[width * x + y];
			}
			else
			{
				img_out[width * x + y] = 0;
			}
		}
	}

	return 0;
}

/**
 * @brief Performs the Hysteresis Thresholding
 * 
 * @param img_in_out 		Pointer to the input/output image
 * @param high_threshold 	The upper limit threshold
 * @param low_threshold 	The lower limit threshold
 * @param height 			Height of the input image
 * @param width 			Width of the input image  
 * @return uint8_t 			Returns 0 if success
 */
uint8_t ridope_edge_tracking(uint8_t *img_in_out, uint8_t high_threshold, uint8_t low_threshold, size_t height, size_t width)
{
	/* Checking input pointers */
	if(img_in_out == NULL)
	{
		return 1;
	}

	int strong_edges_size = 0;
	int weak_edges_size = 0;

	/* Counting strong and weak edges */
	for(int x = 0; x < height; x++)
	{
		for(int y = 0; y < width; y++)
		{

			if(img_in_out[width * x + y] >= high_threshold)
			{
				strong_edges_size++;
			}
			else if((img_in_out[width * x + y] >= low_threshold) && (img_in_out[width * x + y] < high_threshold))
			{
				weak_edges_size++;
			}

		}
	}

	/* Thresholding edges and saving indexes */
	int * weak_edges_indexes = (int * ) malloc(2*weak_edges_size*sizeof(int));

	int weak_i = 0;

	for(int x = 0; x < height; x++)
	{
		for(int y = 0; y < width; y++)
		{

			if(img_in_out[width * x + y] < low_threshold)
			{
				img_in_out[width * x + y] = 0;
			}
			else if(img_in_out[width * x + y] >= high_threshold)
			{
				img_in_out[width * x + y] = 1;
			}
			else
			{
				weak_edges_indexes[weak_i++] = x;
				weak_edges_indexes[weak_i++] = y;
			}

		}
	}

	/* Finding weak edges connected to strong edges (8-connected neighborhood) */
	for(int i = 0; i < weak_edges_size; i += 2)
	{
		uint8_t is_connected = 0;
		int weak_edge_x = weak_edges_indexes[i];
		int weak_edge_y = weak_edges_indexes[i+1];

		for(int x = -1; x <1; x++)
		{
			for(int y = -1; y < 1; y++)
			{
				int img_x = get_updated_index(weak_edge_x-x, 0, height);
				int img_y = get_updated_index(weak_edge_y-y, 0, width);

				if(img_in_out[width * img_x + img_y] == 1)
				{
					is_connected = 1;
				}
			}
		}

		if(is_connected)
		{
			img_in_out[width * weak_edge_x + weak_edge_y] = 1;
			weak_edges_indexes[i] = -1;
			weak_edges_indexes[i+1] = -1;
		}
	}

	/* Removing weak edges that are not connected */
	for(int i = 0; i < weak_edges_size; i += 2)
	{
		int weak_edge_x = weak_edges_indexes[i];
		int weak_edge_y = weak_edges_indexes[i+1];

		if(weak_edge_x != -1 && weak_edge_y != -1)
		{
			img_in_out[width * weak_edge_x + weak_edge_y] = 0;
		}
	}

	free(weak_edges_indexes);
	return 0;
}

/**
 * @brief Performs the Canny edge detection
 * 
 * @param img_in 			The pointer to the input image	
 * @param img_out 			The pointer to the output image
 * @param high_threshold 	The upper limit threshold
 * @param low_threshold 	The lowzer limit threshold
 * @param height 			Height of the input image
 * @param width 			Width of the input image  
 * @return uint8_t 			Returns 0 if success
 */
uint8_t ridope_canny(uint8_t *img_in, uint8_t *img_out, uint8_t high_threshold, uint8_t low_threshold, size_t height, size_t width)
{
	/* Checking input pointers */
	if(img_in == NULL)
	{
		return 1;
	}

	if(img_out == NULL)
	{
		return 2;
	}

	uint8_t *g_x = (uint8_t *) malloc(height*width*sizeof(uint8_t));
	uint8_t *g_y = (uint8_t *) malloc(height*width*sizeof(uint8_t));
	uint8_t *ang = (uint8_t *) malloc(height*width*sizeof(uint8_t));

	if(g_x == NULL)
	{
		return 3;
	}

	if(g_y == NULL)
	{
		return 4;
	}

	if(ang == NULL)
	{
		return 5;
	}

	uint8_t gaussian_ker_size = 5;
	uint8_t sobel_ker_size = 3;
	float sigma = 1.0;


	ridope_gaussian_filter(img_in, img_out, height, width, gaussian_ker_size, sigma);
	ridope_sobel_filter(img_out, g_x, g_y, height, width, sobel_ker_size);
	ridope_get_mag_ang(g_x, g_y, img_out, ang, height, width);

	free(g_y);

	uint8_t *img_temp = g_x;
	g_x = NULL;

	ridope_non_max_supression(img_out, ang, img_temp, height, width);
	ridope_edge_tracking(img_temp, high_threshold, low_threshold, height, width);

	memcpy(img_out,img_temp,height*width);
	free(img_temp);
	free(ang);
	return 0;
}
