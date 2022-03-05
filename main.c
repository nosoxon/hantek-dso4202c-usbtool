#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "message.h"
#include "screenshot.h"

void read_file(libusb_device_handle *handle, char *path)
{
	uint8_t *dbuf = malloc(strlen(path) + 1);
	dbuf[0] = '\0';
	memcpy(dbuf + 1, path, strlen(path));

	struct message msg = {
		.type = MSG_NORMAL,
		.cmd = 0x10,
		.data = dbuf,
		.len = strlen(path) + 1
	};

	send_message(handle, &msg);

	struct message *resp = recv_message(handle);
	while (resp->data[0] != 0x02) {
		for (size_t i = 0; i < resp->len; ++i)
			printf("%02x", resp->data[i]);
		free_message(resp);
		resp = recv_message(handle);
	}

	printf("\n");
	free_message(resp);
}

void beep(libusb_device_handle *handle)
{
	uint8_t dur[] = { 30 };
	struct message msg = {
		.type = MSG_DEBUG,
		.cmd  = 0x44,
		.data = dur,
		.len  = 1
	};

	send_message(handle, &msg);
	free(recv_message(handle));
}

int main(void)
{
	int err = 0;

	err = libusb_init(NULL);
	if (err < 0)
		return err;


	libusb_device **list;
	ssize_t cnt = libusb_get_device_list(NULL, &list);
	if (cnt < 0)
		goto cleanup;

	libusb_device *found = NULL;
	for (ssize_t i = 0; i < cnt; ++i) {
		libusb_device *device = list[i];

		struct libusb_device_descriptor desc;
		err = libusb_get_device_descriptor(device, &desc);
		if (err < 0)
			goto cleanup;


		if (desc.idVendor == 0x049f && desc.idProduct == 0x505a) {
			found = device;
			break;
		}
	}

	if (!found) {
		err = 1;
		goto cleanup;
	}

	libusb_device_handle *handle;
	err = libusb_open(found, &handle);
	if (err < 0)
		goto cleanup;

	err = libusb_reset_device(handle);
	if (err < 0)
		goto cleanup;

	err = libusb_kernel_driver_active(handle, 0);
	if (err == 1) {
		err = libusb_detach_kernel_driver(handle, 0);
		fprintf(stderr, "warning: disabling kernel driver\n");
	}

	if (err < 0)
		goto cleanup;

	err = libusb_claim_interface(handle, 0);
	if (err < 0)
		goto cleanup;

	// beep(handle);
	screenshot(handle, "test.png");

	libusb_release_interface(handle, 0);

	libusb_close(handle);
	libusb_free_device_list(list, 1);

cleanup:
	fprintf(stderr, "error: %s\n", libusb_error_name(err));

	libusb_exit(NULL);
	return err;
}
