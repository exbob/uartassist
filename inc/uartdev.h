/*
Copyright (C) 2025 Lishaocheng <https://shaocheng.li>

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License version 3 as published by the
Free Software Foundation.
*/

#ifndef __UARTDEV_H__
#define __UARTDEV_H__

#include "mydebug.h"
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <termios.h>
#include <unistd.h>

#define UARTDEV_INVALID_FD -1

typedef struct _uartdev_t {
	/* Device descriptor, the return value of open the serial port */
	int fd;
	/* Uart Port: or "/dev/tty.USA19*" on Mac OS X. */
	char *port;
	/* Bauds: 9600, 19200, 57600, 115200, etc */
	int baud;
	/* Data bit : 5, 6, 7, 8 */
	uint8_t data_bit;
	/* Parity: 'N'/'n', 'O'/'o', 'E'/'e' */
	char parity;
	/* Stop bit: 1, 2 */
	uint8_t stop_bit;

} uartdev_t;

/* Converts integer baud to Linux define */
static inline int _get_baud(int baud)
{
	switch (baud) {
	case 1200:
		return B1200;
	case 2400:
		return B2400;
	case 4800:
		return B4800;
	case 9600:
		return B9600;
	case 19200:
		return B19200;
	case 38400:
		return B38400;
	case 57600:
		return B57600;
	case 115200:
		return B115200;
	case 230400:
		return B230400;
	case 460800:
		return B460800;
	case 500000:
		return B500000;
	case 576000:
		return B576000;
	case 921600:
		return B921600;
#ifdef B1000000
	case 1000000:
		return B1000000;
#endif
#ifdef B1152000
	case 1152000:
		return B1152000;
#endif
#ifdef B1500000
	case 1500000:
		return B1500000;
#endif
#ifdef B2000000
	case 2000000:
		return B2000000;
#endif
#ifdef B2500000
	case 2500000:
		return B2500000;
#endif
#ifdef B3000000
	case 3000000:
		return B3000000;
#endif
#ifdef B3500000
	case 3500000:
		return B3500000;
#endif
#ifdef B4000000
	case 4000000:
		return B4000000;
#endif
	default:
		return -1;
	}
}

/* Free UART device memory */
static inline void _uartdev_free(uartdev_t *dev)
{
	if (dev == NULL)
		return;

	if (dev->port) {
		free(dev->port);
	}

	free(dev);
}

/*
Create a new UART device structure , allocate memory and initialize it.
If the creation fails, it will return a null pointer，and set the errno.

arguments : const char *port , UART device file name ,"/dev/ttyS1",
"/dev/ttyUSB0" int baud , 1200 ~ 4000000 char parity , 'N'/'n', 'O'/'o', 'E'/'e'
          int data_bit , data bit , 5, 6, 7, 8
          int stop_bit , stop bit , 1 or 2
*/
static inline uartdev_t *uartdev_new(const char *port, int baud, int data_bit, char parity,
                                     int stop_bit)
{
	pr_debug("%s, %d, %d%c%d\n", port, baud, data_bit, parity, stop_bit);

	uartdev_t *dev;

	/* Check device argument */
	if (port == NULL || *port == 0) {
		errno = EINVAL;
		return NULL;
	}

	/* Check baud argument */
	if (_get_baud(baud) < 0) {
		errno = EINVAL;
		return NULL;
	}

	/* init uartdev_t *dev */
	dev = (uartdev_t *)malloc(sizeof(uartdev_t));
	if (dev == NULL) {
		errno = ENOMEM;
		return NULL;
	}

	/* Device name */
	dev->port = (char *)malloc((strlen(port) + 1) * sizeof(char));
	if (dev->port == NULL) {
		_uartdev_free(dev);
		errno = ENOMEM;
		return NULL;
	}
	strcpy(dev->port, port);

	/* Baud */
	dev->baud = baud;

	/* Data bit */
	switch (data_bit) {
	case 5:
	case 6:
	case 7:
	case 8:
		dev->data_bit = data_bit;
		break;
	default:
		_uartdev_free(dev);
		errno = EINVAL;
		return NULL;
	}

	/* Stop bit */
	switch (stop_bit) {
	case 1:
	case 2:
		dev->stop_bit = stop_bit;
		break;
	default:
		_uartdev_free(dev);
		errno = EINVAL;
		return NULL;
	}

	/* Parity */
	switch (parity) {
	case 'N':
	case 'E':
	case 'O':
	case 'n':
	case 'e':
	case 'o':
		dev->parity = parity;
		break;
	default:
		_uartdev_free(dev);
		errno = EINVAL;
		return NULL;
	}

	/* fd init */
	dev->fd = UARTDEV_INVALID_FD;

	pr_debug("new uartdev_t, %s, %d, %d%c%d\n", dev->port, dev->baud, dev->data_bit,
	         dev->parity, dev->stop_bit);

	return dev;
}

/*
Delete the devicev structure created by uartdev_new(), and free the memory.
The normal return value is 0. Otherwise errno is returned .
*/
static inline int uartdev_del(uartdev_t *dev)
{
	/* Check device argument */
	if (dev == NULL) {
		errno = EINVAL;
		return -EINVAL;
	}

	if (dev->fd >= 0) {
		close(dev->fd);
	}

	_uartdev_free(dev);

	return 0;
}

/*
Open serial port and set the attributes use uartdev_t *dev。
If successful return 0, otherwise errno is returned.
Note: If this function fails, the caller should call uartdev_del() to clean up
resources.
*/
static inline int uartdev_setup(uartdev_t *dev)
{
	struct termios newtio;
	int flags = 0;
	int ret = 0;

	/* Check device argument */
	if (dev == NULL) {
		errno = EINVAL;
		return -EINVAL;
	}

	/* Check if already opened */
	if (dev->fd >= 0) {
		errno = EALREADY;
		return -EALREADY;
	}

	/* Open Serial Device
	    The O_NOCTTY flag tells UNIX that this program doesn't want
	    to be the "controlling terminal" for that port. If you
	    don't specify this then any input (such as keyboard abort
	    signals and so forth) will affect your process
	    Timeouts are ignored in canonical input mode or when the
	    NDELAY option is set on the file via open or fcntl
	*/
	flags = O_RDWR | O_NOCTTY | O_NDELAY | O_EXCL;
	dev->fd = open(dev->port, flags);
	pr_debug("open() return %d\n", dev->fd);
	if (dev->fd < 0) {
		return -errno;
	}

	/* Lock device file */
	if (flock(dev->fd, LOCK_EX | LOCK_NB) < 0) {
		return -errno;
	}

	/* clear struct for new port settings */
	bzero(&newtio, sizeof(newtio));

	/* Get current attribute , and then modify it*/
	/*
	ret = tcgetattr(dev->fd, &newtio);
	if (ret)
	{
	    return -errno;
	}
	*/

	/* Stores baud speed to c_ispeed and c_ospeed of newtio*/
	cfsetspeed(&newtio, _get_baud(dev->baud));

	/*
	CLOCAL       Local line - do not change "owner" of port
	CREAD        Enable receiver
	*/
	newtio.c_cflag |= CLOCAL | CREAD;

	/* CRTSCTS (hardware flow control) ，disable*/
	newtio.c_cflag &= ~CRTSCTS;

	/* Set data bits (5, 6, 7, 8 bits)
	    CSIZE        Bit mask for data bits
	*/
	newtio.c_cflag &= ~CSIZE;

	switch (dev->data_bit) {
	case 5:
		newtio.c_cflag |= CS5;
		break;
	case 6:
		newtio.c_cflag |= CS6;
		break;
	case 7:
		newtio.c_cflag |= CS7;
		break;
	case 8:
		newtio.c_cflag |= CS8;
		break;

	default:
		errno = EINVAL;
		return -EINVAL;
	}

	switch (dev->stop_bit) {
	case 1:
		newtio.c_cflag &= ~CSTOPB;
		break;
	case 2:
		newtio.c_cflag |= CSTOPB;
		break;

	default:
		errno = EINVAL;
		return -EINVAL;
	}

	switch (dev->parity) {
	case 'N':
	case 'n':
		newtio.c_cflag &= ~PARENB;
		newtio.c_cflag &= ~PARODD;
		break;
	case 'E':
	case 'e':
		newtio.c_cflag |= PARENB;
		newtio.c_cflag &= ~PARODD;
		break;
	case 'O':
	case 'o':
		newtio.c_cflag |= PARENB;
		newtio.c_cflag |= PARODD;
		break;

	default:
		errno = EINVAL;
		return -EINVAL;
	}

	/* C_LFLAG      Line options

	    ISIG Enable SIGINTR, SIGSUSP, SIGDSUSP, and SIGQUIT signals
	    ICANON       Enable canonical input (else raw)
	    XCASE        Map uppercase \lowercase (obsolete)
	    ECHO Enable echoing of input characters
	    ECHOE        Echo erase character as BS-SP-BS
	    ECHOK        Echo NL after kill character
	    ECHONL       Echo NL
	    NOFLSH       Disable flushing of input buffers after
	    interrupt or quit characters
	    IEXTEN       Enable extended functions
	    ECHOCTL      Echo control characters as ^char and delete as ~?
	    ECHOPRT      Echo erased character as character erased
	    ECHOKE       BS-SP-BS entire line on line kill
	    FLUSHO       Output being flushed
	    PENDIN       Retype pending input at next read or input char
	    TOSTOP       Send SIGTTOU for background output

	    Canonical input is line-oriented. Input characters are put
	    into a buffer which can be edited interactively by the user
	    until a CR (carriage return) or LF (line feed) character is
	    received.

	    Raw input is unprocessed. Input characters are passed
	    through exactly as they are received, when they are
	    received. Generally you'll deselect the ICANON, ECHO,
	    ECHOE, and ISIG options when using raw input
	*/
	/* Raw input */
	newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	/* C_IFLAG      Input options

	   Constant     Description
	   INPCK        Enable parity check
	   IGNPAR       Ignore parity errors
	   PARMRK       Mark parity errors
	   ISTRIP       Strip parity bits
	   IXON Enable software flow control (outgoing)
	   IXOFF        Enable software flow control (incoming)
	   IXANY        Allow any character to start flow again
	   IGNBRK       Ignore break condition
	   BRKINT       Send a SIGINT when a break condition is detected
	   INLCR        Map NL to CR
	   IGNCR        Ignore CR
	   ICRNL        Map CR to NL
	   IUCLC        Map uppercase to lowercase
	   IMAXBEL      Echo BEL on input line too long
	*/
	if (dev->parity == 'N') {
		/* None */
		newtio.c_iflag &= ~INPCK;
	} else {
		newtio.c_iflag |= INPCK;
	}
	/* Software flow control is disabled */
	newtio.c_iflag &= ~(IXON | IXOFF | IXANY);

	/* Raw output */
	newtio.c_oflag &= ~OPOST;

	newtio.c_cc[VMIN] = 0;
	newtio.c_cc[VTIME] = 0;

	/* now clean the modem line and activate the settings for the port */
	tcflush(dev->fd, TCIOFLUSH);
	ret = tcsetattr(dev->fd, TCSANOW, &newtio);
	if (ret) {
		return -errno;
	}

	/* Clear O_NDELAY flag to allow poll() to work correctly */
	flags = fcntl(dev->fd, F_GETFL, 0);
	if (flags < 0) {
		return -errno;
	}
	flags &= ~O_NDELAY;
	ret = fcntl(dev->fd, F_SETFL, flags);
	if (ret < 0) {
		return -errno;
	}

	return 0;
}

/*
Send data of specified length
*/
static inline int uartdev_send(uartdev_t *dev, const char *buf, int len)
{
	if (dev == NULL || buf == NULL || len < 0 || dev->fd < 0) {
		errno = EINVAL;
		return -1;
	}

	return write(dev->fd, buf, len);
}

/*
Receive data of specified length
*/
static inline int uartdev_recv(uartdev_t *dev, char *buf, int len)
{
	if (dev == NULL || buf == NULL || len < 0 || dev->fd < 0) {
		errno = EINVAL;
		return -1;
	}

	return read(dev->fd, buf, len);
}

/*
Clear the data buffer of serial port, both receiving and sending
*/
static inline int uartdev_flush(uartdev_t *dev)
{
	if (dev == NULL || dev->fd < 0) {
		errno = EINVAL;
		return -1;
	}

	return tcflush(dev->fd, TCIOFLUSH);
}

#endif