//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 2016 by Gareth Nelson (gareth@garethnelson.com)
//
// The Lambda engine is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// The Lambda engine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
//
// $Log:$
//
// DESCRIPTION:
//     A terminal that renders to an OpenGL texture
//
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "gl_term.h"

#ifdef __MACH__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

void init_gl_term(GLTerminal* term) {
     int rc;
     term            = (GLterminal*)malloc(sizeof(GLTerminal));
     term->fd_master = posix_openpt(O_RDWR);
     if(term->fd_master < 0) {
        fprintf(stderr,"Error %d in posix_openpt()\n",errno);
        free(term);
        return;
     }
     
     rc = grantpt(term->fd_master);
     if(rc != 0) {
        fprintf(stderr,"Error %d on grantpt()\n",errno);
        free(term);
        return;
     }
     
     rc = unlockpt(term->fd_master);
     if(rc != 0) {
        fprintf(stderr,"Error %d on unlockpt()\n",errno);
        free(term);
        return;
     }
     
     term->fd_slave = open(ptsname(term->fd_master), O_RDWR);
}

void update_gl_term(GLTerminal* term) {
     int rc;
     char input[150];
     rc = read(term->fd_master,sizeof(input)-1);
     if(rc>0) {
         // send contents of input to the terminal here
     } else {
         fprintf(stderr,"GLTerminal closed\n");
         return;
     }
     if(term->pending_input_size > 0) {
        write(term->fd_master,term->pending_input,term->pending_input_size);
     }
}

void render_gl_term(GLTerminal* term) {
     
}

FILE* gl_term_run(GLTerminal* term, char* cmd) {
      int rc;
      if(fork()) {
         close(term->fd_slave);
         return fdopen(term->fd_master,"r+");
      } else {
         close(term->fd_master);
         struct termios slave_orig_term_settings;
         struct termios new_term_settings;

         rc                = tcgetaddr(term->fd_slave, &slave_orig_term_settings);
         new_term_settings = slave_orig_term_settings;
         cfmakeraw(&new_term_settings);
         tcsetattr(term->fd_slave, TCSANOW, &new_term_settings);
         close(0);
         close(1);
         close(2);
         dup(fds);
         dup(fds);
         dup(fds);
         close(term->fd_slave);
         setsid();
         ioctl(0, TIOCSCTTY, 1);
         system(cmd);
      }
}

void close_gl_term(GLTerminal* term) {
}
