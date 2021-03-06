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
#include <util.h>

#define DEBUG
#include <vterm.h>
#include "oglconsole.h"

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
     vterm_set_utf8(term->vt, false);
     term->vts = vterm_obtain_screen(term->vt);

     vterm_screen_reset(term->vts,1);
     int i=0;
     for(i=0; i<80*25; i++) vterm_input_write(term->vt," ",1);

     term->render_target_fb=0;

     // setup font
     glGenTextures(1,&(term->font_texture));
     glBindTexture(GL_TEXTURE_2D, term->font_texture);

     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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


/*********
  TODO - Test some different strings to figure out EOL character
         Split terminal into rows
 *********/
void update_gl_term(GLTerminal* term) {
     int rc;
     char input[150];
     memset((void*)input,0,150);
     rc = read(term->fd_master,input,sizeof(input)-1);
     if(rc>0) {
         vterm_input_write(term->vt, input,rc);
         // send contents of input to the terminal here
     } else {
//         fprintf(stderr,"GLTerminal closed\n");
//         return;
     }
     if(term->pending_input_size > 0) {
        write(term->fd_master,term->pending_input,term->pending_input_size);
        term->pending_input_size = 0;
     }
     VTermScreenCell cell;
     VTermRect rect = {0,0,0,0};
     rect.start_row=0;
     rect.start_col=0;
     rect.end_row=25;
     rect.end_col=80;
     memset((void*)term->contents,' ',sizeof(char)*80*25);
     vterm_screen_get_text(term->vts, &(term->contents),sizeof(char)*80*25,rect);
     int row=0;
     int col=0;
     VTermPos pos;
     for(row=0; row<25; row++) {
         for(col=0; col<80; col++) {
             pos.col = col;
             pos.row = row;
             vterm_screen_get_cell(term->vts,pos,&cell);
             term->contents[25-1-row][col] = cell.chars[0];
         }
     }
}


void render_gl_term(GLTerminal* term) {
     glMatrixMode(GL_TEXTURE);
     glLoadIdentity();
     glBindFramebuffer(GL_FRAMEBUFFER, term->render_target_fb);
     glClearColor(0.0f,0.0f,0.0f,0.0f);
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     int i=0;
     glColor3d(1,1,1);
     glDisable(GL_TEXTURE_2D);
     glMatrixMode(GL_PROJECTION);
     glPushMatrix();
     glLoadIdentity();
//     glOrtho(0, TERM_SIZE*3, 0, TERM_SIZE, -1,1);
     glOrtho(0,TERM_SIZE*3,0,TERM_SIZE*1.5,-1,1);
     glMatrixMode(GL_MODELVIEW);
     glPushMatrix();
     glLoadIdentity();
     glEnable(GL_TEXTURE_2D);
     int row=0;
     int col=0;
     glBindTexture(GL_TEXTURE_2D,term->font_texture);
     glBegin(GL_QUADS);
     char cur_char[2];
     cur_char[1]=0;
     VTermPos cur_pos;
     VTermScreenCell cell;
     VTermPos cursor_pos;
     vterm_state_get_cursorpos( vterm_obtain_state(term->vt),&cursor_pos);
     cursor_pos.row = cursor_pos.row+(25 - CHAR_PIXEL_H);    

     glEnable(GL_BLEND);
     glBlendFunc(GL_ONE, GL_ONE);
     for(col=0; col<80; col++) {
         for(row=0; row<25; row++) {
             cur_pos.row = row+(25-CHAR_PIXEL_H);
             cur_pos.col = col;
             cur_char[0] = term->contents[row][col];
             vterm_screen_get_cell(term->vts,cur_pos,&cell);
             glColor3d(cell.bg.red,cell.bg.green,cell.bg.blue);
             OGLCONSOLE_DrawString(" ",col*(TERM_SIZE/80),row*(TERM_SIZE/25),CHAR_PIXEL_W,CHAR_PIXEL_H,0);
             glEnable(GL_TEXTURE_2D);
             glColor3d(cell.fg.red,cell.fg.green,cell.fg.blue);
             OGLCONSOLE_DrawString(cur_char,col*(TERM_SIZE/80),row*(TERM_SIZE/25),CHAR_PIXEL_W,CHAR_PIXEL_H,0);
         }
     }
     OGLCONSOLE_DrawString("_",cursor_pos.col*(TERM_SIZE/80),cursor_pos.row*(TERM_SIZE/25),CHAR_PIXEL_W,CHAR_PIXEL_H,0);
     glEnd();
     glPopMatrix();
     glMatrixMode(GL_PROJECTION);
     glPopMatrix();
     
     glBindFramebuffer(GL_FRAMEBUFFER,0);
}

FILE* gl_term_run(GLTerminal* term, char* cmd) {
      int rc;
      if(fork()) {
         close(term->fd_slave);
         fcntl(term->fd_master, F_SETFL, FNDELAY);
         return fdopen(term->fd_master,"r+");
      } else {
         close(term->fd_master);
         struct termios slave_orig_term_settings;
         struct termios new_term_settings;

         rc                = tcgetattr(term->fd_slave, &slave_orig_term_settings);
         new_term_settings = slave_orig_term_settings;
         cfmakeraw(&new_term_settings);
         new_term_settings.c_iflag = ICRNL|IXON;
         new_term_settings.c_oflag = OPOST|ONLCR;
         new_term_settings.c_cflag = CS8|CREAD|OFILL;
         new_term_settings.c_lflag = ISIG|ICANON|IEXTEN|ECHO|ECHOE|ECHOK;
  new_term_settings.c_cc[VINTR]    = 0x1f & 'C';
  new_term_settings.c_cc[VQUIT]    = 0x1f & '\\';
  new_term_settings.c_cc[VERASE]   = 0x7f;
  new_term_settings.c_cc[VKILL]    = 0x1f & 'U';
  new_term_settings.c_cc[VEOF]     = 0x1f & 'D';
  new_term_settings.c_cc[VEOL]     = '\r';
  new_term_settings.c_cc[VEOL2]    = '\n';
  new_term_settings.c_cc[VSTART]   = 0x1f & 'Q';
  new_term_settings.c_cc[VSTOP]    = 0x1f & 'S';
  new_term_settings.c_cc[VSUSP]    = 0x1f & 'Z';
  new_term_settings.c_cc[VREPRINT] = 0x1f & 'R';
  new_term_settings.c_cc[VWERASE]  = 0x1f & 'W';
  new_term_settings.c_cc[VLNEXT]   = 0x1f & 'V';
  new_term_settings.c_cc[VMIN]     = 1;
  new_term_settings.c_cc[VTIME]    = 0;
         cfsetspeed(&new_term_settings, 38400);

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
         char *envp[] = {"TERM=xterm-256color",NULL};
         execve("/bin/sh",argv,envp);
      }
}

void send_term_keypress(GLTerminal* term, char key) {
     if(term->pending_input_size >= 150) { // stupid hack i know
        term->pending_input_size = 0;
     }
     term->pending_input[term->pending_input_size]=key;
     term->pending_input_size++;
}

void close_gl_term(GLTerminal* term) {
     close(term->fd_master);
     free(term);
}
