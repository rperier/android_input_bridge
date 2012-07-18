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

#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <linux/input.h>

#define EVENTS_QUEUE_SIZE 16

#ifdef DEBUG
#define D(...) \
    fprintf(stderr, __VA_ARGS__);
#else
#define D(...)
#endif

#define ioctlx(d, request, ...)			\
  do {						\
      int ret = ioctl(d, request, __VA_ARGS__);	\
      if (ret == -1) {				\
          perror(APPNAME);			\
          exit(1);				\
      }						\
  } while(0)

#define likely(x)	__builtin_expect((x), 1)
#define unlikely(x)	__builtin_expect((x), 0)

int socketx(int domain, int type, int protocol);
int openx(const char *pathname, int flags);
void *mallocx(size_t size);

static inline uint8_t is_ev_syn(struct input_event *ev)
{
    return ev->type == EV_SYN && ev->code == SYN_REPORT && ev->value == 0;
}

#endif /* COMMON_H */
