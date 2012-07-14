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

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

int socketx(int domain, int type, int protocol)
{
    int fd;

    D("socketx++\n");
    fd = socket(domain, type, protocol);

    if (fd == -1) {
        D("errno %d\n", errno);
        perror(APPNAME);
	exit(1);
    }
    D("socketx--\n");
    return fd;
}

int openx(const char *pathname, int flags)
{
    int fd;

    D("openx++\n");
    fd = open(pathname, flags);

    if (fd == -1) {
        D("errno %d\n", errno);
        perror(APPNAME);
        exit(1);
    }
    D("openx--\n");
    return fd;
}

void *mallocx(size_t size)
{
    void *buffer;

    D("mallox++\n");

    buffer = malloc(size);
    if (buffer == NULL) {
        D("errno %d\n", errno);
        perror(APPNAME);
        exit(1);
    }
    D("mallocx--\n");
    return buffer;
}
