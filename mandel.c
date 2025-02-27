#include "bitmap.h"

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

// struct for thread data/parameters
typedef struct
{
	struct bitmap *bm;
	double xmin, xmax, ymin, ymax;
	int max_iterations, start_row, end_row;
} ThreadParameters;

int iteration_to_color(int i, int max);
int iterations_at_point(double x, double y, int max);
void compute_image(struct bitmap *bm, double xmin, double xmax, double ymin, double ymax, int max, int num_threads);

/*
 * This function runs in a thread to help speed up the work.
 * Many threads run at the same time, each working on a different part of the image.
 *
 * Parameters:
 *   - arg: Info about which part of the image this thread should handle.
 *
 * Returns:
 *   - NULL (because threads must return something, but we don’t use it).
 */
void *divide_threads(void *arg)
{
	ThreadParameters *data = (ThreadParameters *)arg;
	int width = bitmap_width(data->bm);

	for (int j = data->start_row; j < data->end_row; j++)
	{
		for (int i = 0; i < width; i++)
		{
			// Determine the point in x,y space for that pixel
			double x = data->xmin + i * (data->xmax - data->xmin) / width;
			double y = data->ymin + j * (data->ymax - data->ymin) / bitmap_height(data->bm);

			// Compute the iterations at that point
			int iters = iterations_at_point(x, y, data->max_iterations);

			// Set the pixel in the bitmap
			bitmap_set(data->bm, i, j, iters);
		}
	}

	return NULL;
}

void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>     The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>   X coordinate of image center point. (default=0)\n");
	printf("-y <coord>   Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>   Scale of the image in Mandlebrot coordinates. (default=4)\n");
	printf("-W <pixels>  Width of the image in pixels. (default=500)\n");
	printf("-H <pixels>  Height of the image in pixels. (default=500)\n");
	printf("-o <file>    Set output file. (default=mandel.bmp)\n");
	printf("-n <threads> The number of threads\n");
	printf("-h           Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}

int main(int argc, char *argv[])
{
	char c;

	// These are the default configuration values used
	// if no command line arguments are given.

	const char *outfile = "mandel.bmp";
	double xcenter = 0;
	double ycenter = 0;
	double scale = 4;
	int image_width = 500;
	int image_height = 500;
	int max = 1000;
	int num_threads = 1; // Default to single thread

	// For each command line argument given,
	// override the appropriate configuration value.

	while ((c = getopt(argc, argv, "x:y:s:W:H:m:o:h:n:")) != -1)
	{
		switch (c)
		{
		case 'x':
			xcenter = atof(optarg);
			break;
		case 'y':
			ycenter = atof(optarg);
			break;
		case 's':
			scale = atof(optarg);
			break;
		case 'W':
			image_width = atoi(optarg);
			break;
		case 'H':
			image_height = atoi(optarg);
			break;
		case 'm':
			max = atoi(optarg);
			break;
		case 'o':
			outfile = optarg;
			break;
		case 'n':
			num_threads = atoi(optarg); // Added option for specifying how many threads to use.
			break;
		case 'h':
			show_help();
			exit(1);
			break;
		}
	}

	// Display the configuration of the image.
	printf("mandel: x=%lf y=%lf scale=%lf max=%d threads=%d outfile=%s\n", xcenter, ycenter, scale, max, num_threads, outfile);

	// Create a bitmap of the appropriate size.
	struct bitmap *bm = bitmap_create(image_width, image_height);

	// Fill it with a dark blue, for debugging
	bitmap_reset(bm, MAKE_RGBA(0, 0, 255, 0));

	// Compute the Mandelbrot image
	compute_image(bm, xcenter - scale, xcenter + scale, ycenter - scale, ycenter + scale, max, num_threads);

	// Save the image in the stated file.
	if (!bitmap_save(bm, outfile))
	{
		fprintf(stderr, "mandel: couldn't write to %s: %s\n", outfile, strerror(errno));
		return 1;
	}

	return 0;
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/

void compute_image(struct bitmap *bm, double xmin, double xmax, double ymin, double ymax, int max, int num_threads)
{
	int height = bitmap_height(bm);
	pthread_t threads[num_threads];
	ThreadParameters thread_data[num_threads];

	// Divide the work among the threads
	int rows_per_thread = height / num_threads;
	for (int i = 0; i < num_threads; i++)
	{
		thread_data[i].bm = bm;
		thread_data[i].xmin = xmin;
		thread_data[i].xmax = xmax;
		thread_data[i].ymin = ymin;
		thread_data[i].ymax = ymax;
		thread_data[i].max_iterations = max;
		thread_data[i].start_row = i * rows_per_thread;
		thread_data[i].end_row = (i == num_threads - 1) ? height : (i + 1) * rows_per_thread;

		// Create the thread
		pthread_create(&threads[i], NULL, divide_threads, &thread_data[i]);
	}

	// Wait for all threads
	for (int i = 0; i < num_threads; i++)
	{
		pthread_join(threads[i], NULL);
	}
}

/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point(double x, double y, int max)
{
	double x0 = x;
	double y0 = y;

	int iter = 0;

	while ((x * x + y * y <= 4) && iter < max)
	{

		double xt = x * x - y * y + x0;
		double yt = 2 * x * y + y0;

		x = xt;
		y = yt;

		iter++;
	}

	return iteration_to_color(iter, max);
}

/*
Convert a iteration number to an RGBA color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/

int iteration_to_color(int i, int max)
{
	int gray = 255 * i / max;
	return MAKE_RGBA(gray, gray, gray, 0);
}
