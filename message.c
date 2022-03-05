#include <errno.h>
#include <error.h>
#include <libusb.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "message.h"

struct message *decode_message(uint8_t *raw, size_t len)
{
	if (len < 3)
		error(1, EIO, "not long enough");

	size_t data_len = raw[1] + (raw[2] << 8);
	if (len < data_len + 3)
		error(1, EIO, "not long enough");

	struct message *msg = malloc(sizeof(struct message));
	if (msg == NULL)
		error(1, errno, "could not allocate struct message");

	switch (raw[0]) {
	case 0x53:
		msg->type = MSG_UNKNOWN;
		break;
	case 0x43:
		msg->type = MSG_DEBUG;
		break;
	default:
		msg->type = MSG_UNKNOWN;
	}

	msg->cmd = raw[3];

	msg->len = data_len - 2;
	if (msg->len) {
		msg->data = malloc(msg->len);
		if (msg->data == NULL)
			error(1, errno, "could not allocate message data buffer");

		memcpy(msg->data, raw + 4, msg->len);
	} else {
		msg->data = NULL;
	}

	uint8_t checksum = 0;
	for (size_t i = 0; i < data_len + 2; ++i)
		checksum += raw[i];

	if (checksum != raw[data_len + 2])
		msg->type = MSG_INVALID;

	return msg;
}

uint8_t *encode_message(struct message *msg, size_t *len)
{
	size_t szbuf = msg->len + 5;
	uint8_t *buf = malloc(szbuf);
	if (buf == NULL)
		error(1, errno, "whoopsie");

	buf[0] = msg->type == MSG_NORMAL ? 0x53 : 0x43;
	buf[1] = (msg->len + 2) & 0xff;
	buf[2] = ((msg->len + 2) & 0xff00) >> 8;
	buf[3] = msg->cmd;

	if (msg->len)
		memcpy(buf + 4, msg->data, msg->len);

	uint8_t checksum = 0;
	for (size_t i = 0; i < szbuf - 1; ++i)
		checksum += buf[i];

	buf[szbuf - 1] = checksum;

	*len = szbuf;
	return buf;
}

void send_message(libusb_device_handle *handle, struct message *msg) {
	size_t msg_len = 0;
	uint8_t *msg_buf = encode_message(msg, &msg_len);

	int transferred = 0;
	int err = libusb_bulk_transfer(handle, 0x02, msg_buf, msg_len,
		&transferred, 0);
	free(msg_buf);
	if (err < 0)
		error(1, EIO, "error: %s\n", libusb_error_name(err));

	if (transferred < msg_len)
		error(1, EIO, "received less than `msg_len' bytes");
}

struct message *recv_message(libusb_device_handle *handle) {
	uint8_t *msg_buf = malloc(256 * 256);

	int msg_len = 0;
	int err = libusb_bulk_transfer(handle, 0x81, msg_buf, 256 * 256,
		&msg_len, 100);
	if (err < 0) {
		free(msg_buf);
		error(1, EIO, "error: %s\n", libusb_error_name(err));
	}

	struct message *msg = decode_message(msg_buf, msg_len);
	free(msg_buf);
	return msg;
}

void free_message(struct message *msg)
{
	if(msg->data)
		free(msg->data);
	free(msg);
}