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

#ifndef __GL_TERM_H_
#define __GL_TERM_H_

#ifdef __MACH__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

typedef struct GLTerminal {
    GLuint render_target;
    GLuint font_texture;
    GLuint render_target_fb;
    int    fd_slave;
    int    fd_master;
    char   pending_input[150];
    int    pending_input_size;
} GLTerminal;

void  init_gl_term(GLTerminal* term);    // Setup the terminal
void  update_gl_term(GLTerminal* term);  // Update the current state of the terminal - call often even when not rendering
void  render_gl_term(GLTerminal* term);  // Render the current state of the terminal to the appropriate texture
FILE* gl_term_run(GLTerminal* term, char* cmd);            // Run a shell command in the terminal (should only be done once)
void  close_gl_term(GLTerminal* term);   // Close the terminal and cleanup

#endif
