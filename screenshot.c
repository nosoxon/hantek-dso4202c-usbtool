#include <errno.h>
#include <error.h>
#include <png.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "message.h"

#define HEIGHT	480
#define	WIDTH	800
#define PIXELS	((HEIGHT)*(WIDTH))

void bitmap_to_png(uint8_t *rgb_buf, char *output_path) {
	uint8_t **row_pointers = malloc(HEIGHT * sizeof(uint8_t*));
	if (!row_pointers)
		error(1, errno, "oop");

	for (size_t i = 0; i < HEIGHT; ++i)
		row_pointers[i] = rgb_buf + 3 * WIDTH * i;

	FILE *fp = fopen(output_path, "wb");
	if (!fp)
		error(1, errno, "could not open file for writing");

	png_structp png_ptr = png_create_write_struct(
		PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
		error(1, EIO, "failed to create png_struct");

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
		error(1, EIO, "failed to create png_info");
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		error(1, EIO, "couldn't jmp");
	}

	png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr, WIDTH, HEIGHT, 8,
		PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);

	// png_set_tIME

	png_color_8 sig_bit = { .red = 5, .green = 6, .blue = 5 };
	png_set_sBIT(png_ptr, info_ptr, &sig_bit);
	png_set_rows(png_ptr, info_ptr, row_pointers);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	free(row_pointers);
	fclose(fp);
}

void screenshot(libusb_device_handle *handle, char *output_path)
{
	struct message msg = {
		.type = MSG_NORMAL,
		.cmd  = 0x20,
		.data = NULL,
		.len  = 0
	};

	send_message(handle, &msg);

	uint8_t *imgbuf = malloc(2 * PIXELS);
	size_t imgptr = 0;

	struct message *resp = recv_message(handle);
	while(resp->data[0] != 0x02) {
		memcpy(imgbuf + imgptr, resp->data + 1, resp->len - 1);
		imgptr += resp->len - 1;
		free_message(resp);
		resp = recv_message(handle);
	}

	// check checksum?
	free_message(resp);

	uint8_t *rgbbuf = malloc(3 * PIXELS);
	if (rgbbuf == NULL)
		error(1, errno, "failed to allocate image buffer");

	for (size_t i = 0; i < PIXELS; ++i) {
		uint16_t val = imgbuf[2*i] + (imgbuf[2*i + 1] << 8);
		rgbbuf[3*i+0] = (val & 0xf800) >> 8;
		rgbbuf[3*i+1] = (val & 0x07e0) >> 3;
		rgbbuf[3*i+2] = (val & 0x001f) << 3;
	}

	free(imgbuf);
	bitmap_to_png(rgbbuf, output_path);
	free(rgbbuf);
}