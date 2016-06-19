//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 2016 by Gareth Nelson (gareth@garethnelson.com)
//
// This file is part of the Lambda engine.
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
//     Test program for the OpenGL terminal
//
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#ifdef __MACH__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <SDL.h>

SDL_Surface *surface;
SDL_Window *gWindow;

#define SCREEN_WIDTH  512
#define SCREEN_HEIGHT 512
#define SCREEN_BPP    24

GLfloat xrot;
GLfloat yrot;
GLfloat zrot;

#include "gl_term.h"

void init_gl() {
     glEnable(GL_TEXTURE_2D);
     glShadeModel(GL_SMOOTH);
     glClearColor(0.0f,0.0f,0.0f,0.0f);
     glClearDepth(1.0f);
     glEnable(GL_DEPTH_TEST);
     glDepthFunc(GL_LEQUAL);
     glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void render_cube(GLTerminal* t) {
     glLoadIdentity();
     glTranslatef(0.0f, 0.0f, -5.0f);

     glRotatef( xrot, 1.0f, 0.0f, 0.0f); /* Rotate On The X Axis */
     glRotatef( yrot, 0.0f, 1.0f, 0.0f); /* Rotate On The Y Axis */
     glRotatef( zrot, 0.0f, 0.0f, 1.0f); /* Rotate On The Z Axis */
     
     glBindTexture(GL_TEXTURE_2D,t->render_target);

     glEnable(GL_TEXTURE_2D);
 glBegin(GL_QUADS);
      /* Front Face */
      /* Bottom Left Of The Texture and Quad */
      glTexCoord2f( 0.0f, 1.0f ); glVertex3f( -1.0f, -1.0f, 1.0f );
      /* Bottom Right Of The Texture and Quad */
      glTexCoord2f( 1.0f, 1.0f ); glVertex3f(  1.0f, -1.0f, 1.0f );
      /* Top Right Of The Texture and Quad */
      glTexCoord2f( 1.0f, 0.0f ); glVertex3f(  1.0f,  1.0f, 1.0f );
      /* Top Left Of The Texture and Quad */
      glTexCoord2f( 0.0f, 0.0f ); glVertex3f( -1.0f,  1.0f, 1.0f );

      /* Back Face */
      /* Bottom Right Of The Texture and Quad */
      glTexCoord2f( 0.0f, 0.0f ); glVertex3f( -1.0f, -1.0f, -1.0f );
      /* Top Right Of The Texture and Quad */
      glTexCoord2f( 0.0f, 1.0f ); glVertex3f( -1.0f,  1.0f, -1.0f );
      /* Top Left Of The Texture and Quad */
      glTexCoord2f( 1.0f, 1.0f ); glVertex3f(  1.0f,  1.0f, -1.0f );
      /* Bottom Left Of The Texture and Quad */
      glTexCoord2f( 1.0f, 0.0f ); glVertex3f(  1.0f, -1.0f, -1.0f );

      /* Top Face */
      /* Top Left Of The Texture and Quad */
      glTexCoord2f( 1.0f, 1.0f ); glVertex3f( -1.0f,  1.0f, -1.0f );
      /* Bottom Left Of The Texture and Quad */
      glTexCoord2f( 1.0f, 0.0f ); glVertex3f( -1.0f,  1.0f,  1.0f );
      /* Bottom Right Of The Texture and Quad */
      glTexCoord2f( 0.0f, 0.0f ); glVertex3f(  1.0f,  1.0f,  1.0f );
      /* Top Right Of The Texture and Quad */
      glTexCoord2f( 0.0f, 1.0f ); glVertex3f(  1.0f,  1.0f, -1.0f );

      /* Bottom Face */
      /* Top Right Of The Texture and Quad */
      glTexCoord2f( 0.0f, 1.0f ); glVertex3f( -1.0f, -1.0f, -1.0f );
      /* Top Left Of The Texture and Quad */
      glTexCoord2f( 1.0f, 1.0f ); glVertex3f(  1.0f, -1.0f, -1.0f );
      /* Bottom Left Of The Texture and Quad */
      glTexCoord2f( 1.0f, 0.0f ); glVertex3f(  1.0f, -1.0f,  1.0f );
      /* Bottom Right Of The Texture and Quad */
      glTexCoord2f( 0.0f, 0.0f ); glVertex3f( -1.0f, -1.0f,  1.0f );

      /* Right face */
      /* Bottom Right Of The Texture and Quad */
      glTexCoord2f( 0.0f, 0.0f ); glVertex3f( 1.0f, -1.0f, -1.0f );
      /* Top Right Of The Texture and Quad */
      glTexCoord2f( 0.0f, 1.0f ); glVertex3f( 1.0f,  1.0f, -1.0f );
      /* Top Left Of The Texture and Quad */
      glTexCoord2f( 1.0f, 1.0f ); glVertex3f( 1.0f,  1.0f,  1.0f );
      /* Bottom Left Of The Texture and Quad */
      glTexCoord2f( 1.0f, 0.0f ); glVertex3f( 1.0f, -1.0f,  1.0f );

      /* Left Face */
      /* Bottom Left Of The Texture and Quad */
      glTexCoord2f( 1.0f, 0.0f ); glVertex3f( -1.0f, -1.0f, -1.0f );
      /* Bottom Right Of The Texture and Quad */
      glTexCoord2f( 0.0f, 0.0f ); glVertex3f( -1.0f, -1.0f,  1.0f );
      /* Top Right Of The Texture and Quad */
      glTexCoord2f( 0.0f, 1.0f ); glVertex3f( -1.0f,  1.0f,  1.0f );
      /* Top Left Of The Texture and Quad */
      glTexCoord2f( 1.0f, 1.0f ); glVertex3f( -1.0f,  1.0f, -1.0f );
    glEnd( );
xrot += 0.3f; /* X Axis Rotation */
    yrot += 0.2f; /* Y Axis Rotation */
    zrot += 0.4f;
}

void resizeWindow( int width, int height )
{
    /* Height / width ration */
    GLfloat ratio;
 
    /* Protect against a divide by zero */
    if ( height == 0 )
	height = 1;

    ratio = ( GLfloat )width / ( GLfloat )height;

    /* Setup our viewport. */
    glViewport( 0, 0, ( GLint )width, ( GLint )height );

    /*
     * change to the projection matrix and set
     * our viewing volume.
     */
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );

    /* Set our perspective */
    gluPerspective( 45.0f, ratio, 0.1f, 100.0f );

    /* Make sure we're chaning the model view and not the projection */
    glMatrixMode( GL_MODELVIEW );

    /* Reset The View */
    glLoadIdentity( );

}


int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_VIDEO);




     gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
      

    SDL_GL_CreateContext(gWindow);

    init_gl();
    resizeWindow(SCREEN_WIDTH, SCREEN_HEIGHT);

    GLTerminal *t=init_gl_term();
    gl_term_run(t, "sh");
    SDL_Event e;
    while(1) {
        while(SDL_PollEvent(&e)) {
           switch(e.type) {
              case SDL_QUIT:
                exit(0);
              break;
           }
        }
       update_gl_term(t);
       glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       render_gl_term(t);
       render_cube(t);
       SDL_GL_SwapWindow(gWindow);
    }
}
