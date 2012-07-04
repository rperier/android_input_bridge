/*
 * Copyright 2011  Romain Perier <romain.perier@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of 
 * the License, or (at your option) any later version.
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

int socketx(int domain, int type, int protocol);
int openx(const char *pathname, int flags);

#endif /* COMMON_H */
