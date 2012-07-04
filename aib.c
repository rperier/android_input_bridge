/*
 * Copyright (C) 2012  Romain Perier <romain.perier@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include "common.h"

static void usage(char *progname)
{
    fprintf(stderr, "Usage: %s <port> <inputdevice>\n", progname);
    exit(1);
}

static int input_device_init(const char *device)
{
    int indev_fd;
    char *devname = NULL;

    indev_fd = openx(device, O_RDONLY);
    devname = malloc(64 * sizeof(char));

    if (devname == NULL)
        return indev_fd;
    ioctlx(indev_fd, EVIOCGNAME(64), devname);

    printf("Capturing events for device %s\n", devname);
    free(devname);

    return indev_fd;
}

static int _connect(uint16_t port)
{
    struct sockaddr_in aidd_addr;
    int sock_fd;

    sock_fd = socketx(AF_INET, SOCK_STREAM, 0);
    aidd_addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &(aidd_addr.sin_addr));
    aidd_addr.sin_port = htons(port);

    if (connect(sock_fd, (struct sockaddr *)&aidd_addr, sizeof(struct sockaddr_in)) == -1) {
        perror("aib");
        exit(1);
    }
    return sock_fd;
}

static void send_event(int sock, struct input_event *ev)
{
    ev->type = htons(ev->type);
    ev->code = htons(ev->code);
    ev->value = htonl(ev->value);

    send(sock, &ev->type, sizeof(uint16_t), 0);
    send(sock, &ev->code, sizeof(uint16_t), 0);
    send(sock, &ev->value, sizeof(int32_t), 0);
}

static void wait_evdev_input(int aibd_sock, int inputdev)
{
    struct input_event ev;

    while (read(inputdev, &ev, sizeof(ev))) {
        D("[%lu.%04lu] type %u, code %u, value %d\n", ev.time.tv_sec, ev.time.tv_usec, ev.type, ev.code, ev.value);

        if (ev.type == EV_MSC)
            continue;
	send_event(aibd_sock, &ev);
    }
}

int main (int argc, char *argv[])
{
    uint16_t port;
    char *endptr = NULL;
    int aidd_sock, inputdev;

    if (argc < 3)
        usage(argv[0]);

    port = strtoul(argv[1], &endptr, 10);

    if (*endptr != '\0')
        usage(argv[0]);

    inputdev = input_device_init(argv[2]);
    aidd_sock = _connect(port);
    wait_evdev_input(aidd_sock, inputdev);
  
    return 0;
}
