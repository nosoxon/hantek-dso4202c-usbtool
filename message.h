#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <libusb.h>
#include <stdint.h>

#define MSG_NORMAL	0x01
#define MSG_DEBUG	0x02
#define MSG_UNKNOWN	0x04
#define MSG_INVALID	0x80

struct message {
	uint8_t	type;
	uint8_t	cmd;
	uint8_t	*data;
	size_t	len;
};

uint8_t *encode_message(struct message *msg, size_t *len);
struct message *decode_message(uint8_t *raw, size_t len);

void send_message(libusb_device_handle *handle, struct message *msg);
struct message *recv_message(libusb_device_handle *handle);

void free_message(struct message *msg);

#endif /* _MESSAGE_H */