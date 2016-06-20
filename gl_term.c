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

#define DEBUG
#include <vterm.h>

#include "gl_term.h"

#ifdef __MACH__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

// shamelessly ripped from OGLConsole
#include "console_font.c"
#define CHAR_PIXEL_W 6
#define CHAR_PIXEL_H 13
#define CHAR_WIDTH 0.0234375 /* ogl tex coords */
#define CHAR_HEIGHT 0.203125 /* ogl tex coords */

#define TERM_SIZE 512
GLTerminal* init_gl_term() {
     int rc=0;
     GLTerminal* term  = (GLTerminal*)malloc(sizeof(GLTerminal));
     term->fd_master = posix_openpt(O_RDWR);
     if(term->fd_master < 0) {
        fprintf(stderr,"Error %d in posix_openpt()\n",errno);
        free(term);
        return NULL;
     }
     
     rc = grantpt(term->fd_master);
     if(rc != 0) {
        fprintf(stderr,"Error %d on grantpt()\n",errno);
        free(term);
        return NULL;
     }
     
     rc = unlockpt(term->fd_master);
     if(rc != 0) {
        fprintf(stderr,"Error %d on unlockpt()\n",errno);
        free(term);
        return NULL;
     }
     
     term->fd_slave = open(ptsname(term->fd_master), O_RDWR);

     term->vt = vterm_new(25,80);
     vterm_set_utf8(term->vt, true);
     term->vts = vterm_obtain_screen(term->vt);

     vterm_screen_reset(term->vts,1);
     int i=0;
     for(i=0; i<80*25; i++) vterm_input_write(term->vt," ",1);

     term->render_target_fb=0;

     // setup font
     glGenTextures(1,&(term->font_texture));
     glBindTexture(GL_TEXTURE_2D, term->font_texture);

     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

     glTexImage2D(GL_TEXTURE_2D,0, GL_RGB, Console_FontData.width,
                                           Console_FontData.height,
                                           0, GL_RGB, GL_UNSIGNED_BYTE, Console_FontData.pixel_data);

     // configure render target
     glGenFramebuffers(1,&(term->render_target_fb));
     glBindFramebuffer(GL_FRAMEBUFFER,term->render_target_fb);

     GLuint tex_id;
     glGenTextures(1,&tex_id);
     term->render_target = tex_id;
     glBindTexture(GL_TEXTURE_2D,tex_id);

     glTexImage2D(GL_TEXTURE_2D, 0,3, TERM_SIZE, TERM_SIZE, 0,GL_RGB, GL_UNSIGNED_BYTE,NULL);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, term->render_target,0);
     GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
     glDrawBuffers(1,DrawBuffers);
     glBindFramebuffer(GL_FRAMEBUFFER,0);


     return term;
}

void update_gl_term(GLTerminal* term) {
     int rc;
     char input[150];
     rc = read(term->fd_master,input,sizeof(input)-1);
     if(rc>0) {
         vterm_input_write(term->vt, input,rc);
         // send contents of input to the terminal here
     } else {
//         fprintf(stderr,"GLTerminal closed\n");
//         return;
     }
//     if(term->pending_input_size > 0) {
//        write(term->fd_master,term->pending_input,term->pending_input_size);
//     }
     VTermScreenCell cell;
     VTermRect rect = {0,0,0,0};
     rect.start_row=0;
     rect.start_col=0;
     rect.end_row=25;
     rect.end_col=80;
     memset((void*)term->contents,0,sizeof(char)*80*25);
     vterm_screen_get_text(term->vts, &(term->contents),80*25,rect);
     term->contents[25][80]=0;
     printf("TERMINAL START====\n");
     printf("%s\n",(char*)term->contents);
     printf("TERMINAL END====\n");
}

void render_gl_term(GLTerminal* term) {
//     glBindFramebuffer(GL_FRAMEBUFFER, term->render_target_fb);
     glPushAttrib(GL_ALL_ATTRIB_BITS);
     glDisable(GL_TEXTURE_2D);

     glClearColor(0.0f,0.0f,0.0f,0.0f);
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

     glMatrixMode(GL_PROJECTION);
     glPushMatrix();
     glLoadIdentity();
     glOrtho(0, TERM_SIZE, 0, TERM_SIZE, -1,1);

     glMatrixMode(GL_MODELVIEW);
     glPushMatrix();
     glLoadIdentity();
     
     glDisable(GL_DEPTH_TEST);
     glDisable(GL_TEXTURE_2D);
     glEnable(GL_BLEND);
     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
     glColor4d(.1,0,0,0.5);
     glBegin(GL_QUADS);
       glVertex3d(0,0,0);
       glVertex3d(TERM_SIZE,0,0);
       glVertex3d(TERM_SIZE,TERM_SIZE,0);
       glVertex3d(0,TERM_SIZE,0);
     glEnd();

     glBlendFunc(GL_ONE, GL_ONE);

     glEnable(GL_TEXTURE_2D);
     glBindTexture(GL_TEXTURE_2D, term->font_texture);
     glColor3d(1,1,1); // text colour
     int row=0;
     int col=0;
     int c;
     double cx, cy, cX, cY;
     double x, y, z, w, h;
     glBegin(GL_QUADS);
     w = TERM_SIZE/80;
     h = TERM_SIZE/25;
     for(row=0; row<25; row++) {
         for(col=0; col<80; col++) {
             c  = term->contents[row][col]-' ';
             x  = col * w;
             y  = row * h;
             z  = 0.0f;
             cx = (c % 42) * CHAR_WIDTH;
             cy = 1.0 - (c / 42) * CHAR_HEIGHT;
             cX = cx + CHAR_WIDTH;
             cY = 1.0 - (c / 42 + 1) * CHAR_HEIGHT;
             glTexCoord2d(cx, cy); glVertex3d(x,   y,   z);
             glTexCoord2d(cX, cy); glVertex3d(x+w, y,   z);
             glTexCoord2d(cX, cY); glVertex3d(x+w, y+h, z);
             glTexCoord2d(cx, cY); glVertex3d(x,   y+h, z);

         }
     }
     glEnd();
     glPopMatrix();
     glMatrixMode(GL_PROJECTION);
     glPopMatrix();
     glPopAttrib();
//     glBindFramebuffer(GL_FRAMEBUFFER,0);
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

         rc                = tcgetattr(term->fd_slave, &slave_orig_term_settings);
         new_term_settings = slave_orig_term_settings;
         tcsetattr(term->fd_slave, TCSANOW, &new_term_settings);
         close(0);
         close(1);
         close(2);
         dup(term->fd_slave);
         dup(term->fd_slave);
         dup(term->fd_slave);
         close(term->fd_slave);
         setsid();
         ioctl(0, TIOCSCTTY, 1);
         char *argv[] = {"-l","-c",strdup(cmd),NULL};
         char *envp[] = {"TERM=vt100",NULL};
         execve("/bin/sh",argv,envp);
      }
}

void close_gl_term(GLTerminal* term) {
     close(term->fd_master);
     free(term);
}
