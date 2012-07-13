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
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <arpa/inet.h>
#include <errno.h>
#include "common.h"

#define NO_EXTRA_ARGUMENT -1
#define LISTENING_QUEUE_SIZE 4

static int remote_socks[LISTENING_QUEUE_SIZE];
static struct input_event events[EVENTS_QUEUE_SIZE];
static uint32_t buffer[EVENTS_QUEUE_SIZE * 2];

static void usage(char *progname)
{
    fprintf(stderr, "Usage: %s <port>\n", progname);
    exit(1);
}

static int forward_events(int uinput, struct input_event *ev)
{
    int i;
    ssize_t ret;

    for(i = 0; i < EVENTS_QUEUE_SIZE; i++) {

        D("Forwarding event type = %u, code = %u, value = %d to input subsystem\n", ev[i].type, ev[i].code, ev[i].value);

        gettimeofday(&ev[i].time, NULL);
        ret = write(uinput, ev + i, sizeof(struct input_event));

        if (ret != sizeof(struct input_event)) {
            fprintf(stderr, "Unable to forward event %u to uinput (ret = %lu)\n", i, ret);
        }

        if (is_ev_syn(ev + i))
            return 0;
    }
    return 0;
}

static void connection_closed_by_peer(int sock)
{
    int i;

    for (i = 0; i < LISTENING_QUEUE_SIZE; i++)
        if (remote_socks[i] == sock)
            remote_socks[i] = 0;
    D("Connection closed by peer for sock %d\n", sock);
    close(sock);
}

static int receive_input_events(int sock, struct input_event *ev)
{
    int i = 0, j = 1;
    uint32_t size;
    ssize_t ret;

    bzero(ev, sizeof(struct input_event) * EVENTS_QUEUE_SIZE);
    bzero(buffer, sizeof(uint32_t) * EVENTS_QUEUE_SIZE * 2);

    ret = recv(sock, &size, sizeof(uint32_t), MSG_PEEK);
    if (ret == 0) {
        connection_closed_by_peer(sock);
        return -1;
    }

    size = ntohl(size);

    ret = recv(sock, buffer, sizeof(uint32_t) * (size + 1), MSG_WAITALL);
    if (ret == 0) {
        connection_closed_by_peer(sock);
        return -1;
    }

    for(i = 0; i < EVENTS_QUEUE_SIZE; i++) {
        buffer[j] = ntohl(buffer[j]);
        ev[i].type = (buffer[j] >> 16);
        ev[i].code = buffer[j] & 0xffff;
        ev[i].value = ntohl(buffer[j+1]);

        if (is_ev_syn(ev+i))
            break;
        j += 2;
    }
    return 0;
}

static int input_subsystem_init(void)
{
    int uinput_fd;
    struct uinput_user_dev uidev;
    ssize_t ret;

    bzero(&uidev, sizeof(uidev));
    if (getuid() != 0) {
        fprintf(stderr, "aibd: root privileges are required to access to input subsystem\n");
        exit(1);
    }

    uinput_fd = openx("/dev/uinput", O_WRONLY | O_NONBLOCK);

    ioctlx(uinput_fd, UI_SET_EVBIT, EV_KEY);
    ioctlx(uinput_fd, UI_SET_EVBIT, EV_REL);

    ioctlx(uinput_fd, UI_SET_RELBIT, REL_X);
    ioctlx(uinput_fd, UI_SET_RELBIT, REL_Y);
    ioctlx(uinput_fd, UI_SET_RELBIT, REL_WHEEL);

    ioctlx(uinput_fd, UI_SET_KEYBIT, BTN_LEFT);
    ioctlx(uinput_fd, UI_SET_KEYBIT, BTN_RIGHT);
    ioctlx(uinput_fd, UI_SET_KEYBIT, BTN_MIDDLE);

    memcpy(uidev.name, "aibd-device", UINPUT_MAX_NAME_SIZE);
    uidev.id.bustype = BUS_USB;
    uidev.id.version = 1;

    ret = write(uinput_fd, &uidev, sizeof(uidev));
    if (ret != sizeof(uidev)) {
        fprintf(stderr, "aibd: Unable to initialize virtual input device for /dev/uinput\n");
        exit(1);
    }

    ioctlx(uinput_fd, UI_DEV_CREATE, NO_EXTRA_ARGUMENT);

    return uinput_fd;
}

static void mainloop(int sock, int uinput)
{
    fd_set readfds;
    int ret, newsock, i, maxfd;

    while (1) {
        maxfd = sock;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);

        for (i = 0; i < LISTENING_QUEUE_SIZE; i++) {
            if (remote_socks[i] != 0) {
                FD_SET(remote_socks[i], &readfds);
                if (remote_socks[i] > maxfd)
                    maxfd = remote_socks[i];
	  }
	}

        do {
            ret = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        } while (ret < 0 && errno == EINTR);

        if (ret < 0) {
            perror("aibd");
            exit(1);
	}

        if (FD_ISSET(sock, &readfds)) {
	    D("Incoming connection\n");
            newsock = accept(sock, NULL, NULL);

            if (newsock == -1)
                continue;

            for (i = 0; i < LISTENING_QUEUE_SIZE; i++)
                if (remote_socks[i] == 0)
                    remote_socks[i] = newsock;
        } else {
            for (i = 0; i < LISTENING_QUEUE_SIZE; i++) {
                if (FD_ISSET(remote_socks[i], &readfds)) {
                    ret = receive_input_events(remote_socks[i], events);
                    if (ret == -1)
                        continue;
                    forward_events(uinput, events);
                }
            }
        }
    }
}

static int socket_init(uint16_t port)
{
    struct sockaddr_in addr;
    int sock_fd;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd == -1) {
        perror("aibd");
        exit(1);
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("aibd");
        exit(1);
    }

    if (listen(sock_fd, LISTENING_QUEUE_SIZE) == -1) {
        perror("aibd");
        exit(1);
    }
    return sock_fd;
}

int main (int argc, char *argv[])
{
    char *endptr = NULL;
    uint16_t port;
    int sock, uinput;
 
    if (argc < 2)
        usage(argv[0]);

    port = strtoul(argv[1], &endptr, 10);

    if (*endptr != '\0')
        usage(argv[0]);

    uinput = input_subsystem_init();
    sock = socket_init(port);
    mainloop(sock, uinput);

    return 0;
}
