/*****************************************************************************
   SimSchedulerGL -- An OpenGL simulation visualizer for the SimScheduler.
   Copyright (c) 2012 Orson Teodoro <oteodoro@uci.edu>
   Copyright (c) 2006-2012, assimp team
   All rights reserved.

   Redistribution and use of this software in source and binary forms, 
   with or without modification, are permitted provided that the 
   following conditions are met:

   * Redistributions of source code must retain the above
     copyright notice, this list of conditions and the
     following disclaimer.

   * Redistributions in binary form must reproduce the above
     copyright notice, this list of conditions and the
     following disclaimer in the documentation and/or other
     materials provided with the distribution.

   * Neither the name of the assimp team, nor the names of its
     contributors may be used to endorse or promote products
     derived from this software without specific prior
     written permission of the assimp team.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   AN EXCEPTION applies to all files in the ./test/models-nonbsd folder.
   These are 3d models for testing purposes, from various free sources
   on the internet. They are - unless otherwise stated - copyright of
   their respective creators, which may impose additional requirements
   on the use of their work. For any of these models, see 
   <model-name>.source.txt for more legal information. Contact us if you
   are a copyright holder and believe that we credited you inproperly or 
   if you don't want your files to appear in the repository.
 *****************************************************************************/

#include <stdio.h>
#include <SDL.h>
#include <SDL_opengl.h>
#ifdef HAVE_FTGL
#include <FTGL/ftgl.h>
#endif
#ifdef __LINUX__
#include <assimp/assimp.h>
#include <assimp/aiPostProcess.h>
#endif
#ifdef _WIN32
#include <assimp.h>
#include <aiPostProcess.h>
#ifdef HAVE_SDL
#pragma comment(lib, "opengl32.lib") 
#pragma comment(lib, "Glu32.lib") 
#pragma comment(lib, "SDL.lib")
#pragma comment(lib, "SDLmain.lib") 
#endif
#ifdef HAVE_FTGL
#pragma comment(lib, "ftgl_D.lib") 
#endif
#ifdef HAVE_ASSIMP
#pragma comment(lib, "assimp.lib") 
#endif
#endif

#include "schedsysgl.h"
#include "schedsys.h"

#include "tinyoq.h"

float resw = 640.0f;
float resh = 480.0f;
int fullscreen = 0;

////////////////////////////////////////////////////////////////////////
// Font Operations
////////////////////////////////////////////////////////////////////////

#ifdef HAVE_FTGL
void
font_new (FTGLfont **font, char *path, int ptsize, float depth)
{
  if (depth>=0.0f)
  {
    *font = ftglCreateExtrudeFont (path);
    ftglSetFontDepth (*font, depth);
  }
  else
  {
    *font = ftglCreatePixmapFont (path);
  }
  ftglSetFontFaceSize (*font, ptsize, 72);
}

void
font_draw2d (FTGLfont *font, char *text, float x, float y, int r, int g, int b, int a)
{
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluOrtho2D (0, resw, 0, resh);

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  glColor4ub (r,g,b,a);
  glRasterPos2i (0+x,resh-ftglGetFontAscender (font)-y);
  ftglRenderFont (font, text, FTGL_RENDER_ALL);
}

void
font_draw3d (FTGLfont *font, char *text, float x, float y, float z, int r, int g, int b, int a)
{
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective( 60.0, resw/resh, 1.0, 1024.0 );

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  glTranslatef (x,y,z);
  glScalef (0.01f,0.01f,0.69f);
  //glRotatef (30.0f,1.0f,0.0f,0.0f);
  //glRotatef (0.0f,0.0f,1.0f,0.0f);

  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);  
  
  glColor4ub (r,g,b,a);
  ftglRenderFont (font, text, FTGL_RENDER_ALL);
}

void
font_free (FTGLfont *font)
{
  ftglDestroyFont (font);
}
#endif

////////////////////////////////////////////////////////////////////////
// 3D Models
////////////////////////////////////////////////////////////////////////

#ifdef HAVE_SDL
void
draw_cpu (GLuint scene_list, float sx, float sy, float sz, float x, float y, float z, float rx, float ry, float rz)
{
  GLfloat lightpos[4] = {0.5, 1.0, 1.0, 0.0};
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective( 60.0, resw/resh, 1.0, 1024.0 );

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  glTranslatef (x,y,z);
  glScalef (sx,sy,sz);
  glRotatef (rx,1.0f*sx/2.0,0.0f,0.0f);
  glRotatef (ry,0.0f,1.0f*sy/2.0,0.0f);
  glRotatef (rz,0.0f,0.0f,1.0f*sz/2.0);

  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);  

  //glEnable (GL_COLOR_MATERIAL);

  glEnable (GL_TEXTURE_2D);
  glEnable (GL_LIGHTING);
  glEnable (GL_LIGHT0);

  glLightfv(GL_LIGHT0, GL_POSITION, lightpos);


  glCallList(scene_list);
}

void
draw_hd (GLuint scene_list, float sx, float sy, float sz, float x, float y, float z, float rx, float ry, float rz)
{
  GLfloat lightpos[] = {0.5, 1.0, 1.0, 0.0};
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective( 60.0, resw/resh, 1.0, 1024.0 );

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  glTranslatef (x,y,z);
  glScalef (sx,sy,sz);
  glRotatef (rx,1.0f*sx/2.0,0.0f,0.0f);
  glRotatef (ry,0.0f,1.0f*sy/2.0,0.0f);
  glRotatef (rz,0.0f,0.0f,1.0f*sz/2.0);

  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);  

  //glEnable (GL_COLOR_MATERIAL);

  glEnable (GL_TEXTURE_2D);
  glEnable (GL_LIGHTING);
  glEnable (GL_LIGHT0);

  glLightfv(GL_LIGHT0, GL_POSITION, lightpos);

  glCallList(scene_list);
}

void
draw_qcont (GLuint scene_list, float sx, float sy, float sz, float x, float y, float z, float rx, float ry, float rz)
{
  GLfloat lightpos[] = {0.5, 1.0, 1.0, 0.0};
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective( 60.0, resw/resh, 1.0, 1024.0 );

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  glTranslatef (x,y,z);
  glScalef (sx,sy,sz);
  glRotatef (rx,1.0f*sx/2.0,0.0f,0.0f);
  glRotatef (ry,0.0f,1.0f*sy/2.0,0.0f);
  glRotatef (rz,0.0f,0.0f,1.0f*sz/2.0);

  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);  

  //glEnable (GL_COLOR_MATERIAL);

  glEnable (GL_TEXTURE_2D);
  glEnable (GL_LIGHTING);
  glEnable (GL_LIGHT0);

  glLightfv(GL_LIGHT0, GL_POSITION, lightpos);

  glCallList(scene_list);
}

void
draw_qnode (GLuint scene_list, float sx, float sy, float sz, float x, float y, float z, float rx, float ry, float rz)
{
  GLfloat lightpos[] = {0.5, 1.0, 1.0, 0.0};
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective( 60.0, resw/resh, 1.0, 1024.0 );

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  glTranslatef (x,y,z);
  glScalef (sx,sy,sz);
  glRotatef (rx,1.0f*sx/2.0,0.0f,0.0f);
  glRotatef (ry,0.0f,1.0f*sy/2.0,0.0f);
  glRotatef (rz,0.0f,0.0f,1.0f*sz/2.0);

  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);  

  //glEnable (GL_COLOR_MATERIAL);

  glEnable (GL_TEXTURE_2D);
  glEnable (GL_LIGHTING);
  glEnable (GL_LIGHT0);

  glLightfv(GL_LIGHT0, GL_POSITION, lightpos);

  glCallList(scene_list);
}

////////////////////////////////////////////////////////////////////////
// Scheduler Drawing Code
////////////////////////////////////////////////////////////////////////

void assimp_color4_to_float4(const struct aiColor4D *c, float f[4])
{
  f[0] = c->r;
  f[1] = c->g;
  f[2] = c->b;
  f[3] = c->a;
}

void assimp_set_float4(float f[4], float a, float b, float c, float d)
{
  f[0] = a;
  f[1] = b;
  f[2] = c;
  f[3] = d;
}

void assimp_apply_material(const struct aiMaterial *mtl)
{
#ifdef HAVE_ASSIMP
  float c[4];

  GLenum fill_mode;
  int ret1, ret2;
  struct aiColor4D diffuse;
  struct aiColor4D specular;
  struct aiColor4D ambient;
  struct aiColor4D emission;
  float shininess, strength;
  int two_sided;
  int wireframe;
  int max;

  assimp_set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
  if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
    assimp_color4_to_float4(&diffuse, c);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c);

  assimp_set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
  if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
    assimp_color4_to_float4(&specular, c);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);

  assimp_set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
  if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
    assimp_color4_to_float4(&ambient, c);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, c);

  assimp_set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
  if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
    assimp_color4_to_float4(&emission, c);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, c);

  max = 1;
  ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
  if(ret1 == AI_SUCCESS) {
    max = 1;
    ret2 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, &max);
    if(ret2 == AI_SUCCESS)
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess * strength);
    else
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
  }
  else {
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
    assimp_set_float4(c, 0.0f, 0.0f, 0.0f, 0.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
  }

  max = 1;
  if(AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_ENABLE_WIREFRAME, &wireframe, &max))
    fill_mode = wireframe ? GL_LINE : GL_FILL;
  else
    fill_mode = GL_FILL;
  glPolygonMode(GL_FRONT_AND_BACK, fill_mode);

  max = 1;
  if((AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_TWOSIDED, &two_sided, &max)) && two_sided)
    glDisable(GL_CULL_FACE);
  else 
    glEnable(GL_CULL_FACE);
#endif
}

void assimp_recursive_render (const struct aiScene *sc, const struct aiNode* nd)
{
#ifdef HAVE_ASSIMP
  int i;
  unsigned int n = 0, t;
  struct aiMatrix4x4 m = nd->mTransformation;

  // update transform
  aiTransposeMatrix4(&m);
  glPushMatrix();
  glMultMatrixf((float*)&m);

  // draw all meshes assigned to this node
  for (; n < nd->mNumMeshes; ++n) {
    const struct aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];

    assimp_apply_material(sc->mMaterials[mesh->mMaterialIndex]);

    if(mesh->mNormals == NULL) {
      glDisable(GL_LIGHTING);
    } else {
      glEnable(GL_LIGHTING);
    }

    for (t = 0; t < mesh->mNumFaces; ++t) {
      const struct aiFace* face = &mesh->mFaces[t];
      GLenum face_mode;

      switch(face->mNumIndices) {
      case 1: face_mode = GL_POINTS; break;
      case 2: face_mode = GL_LINES; break;
      case 3: face_mode = GL_TRIANGLES; break;
      default: face_mode = GL_POLYGON; break;
      }

      glBegin(face_mode);

      for(i = 0; i < face->mNumIndices; i++) {
        int index = face->mIndices[i];
        if(mesh->mColors[0] != NULL)
          glColor4fv((GLfloat*)&mesh->mColors[0][index]);
        if(mesh->mNormals != NULL) 
          glNormal3fv(&mesh->mNormals[index].x);
        glVertex3fv(&mesh->mVertices[index].x);
      }

      glEnd();
    }

  }

  // draw all children
  for (n = 0; n < nd->mNumChildren; ++n) {
    assimp_recursive_render(sc, nd->mChildren[n]);
  }

  glPopMatrix();
#endif
}

////////////////////////////////////////////////////////////////////////
// FCFS Scheduler Drawing Code
////////////////////////////////////////////////////////////////////////

void
gl_fcfs_step_init (PSCHEDSYS ss, void *data)
{
  PFCFS_VIS_DATA vd = data;
  glEnable (GL_TEXTURE_2D);
  glViewport (0, 0, resw, resh);
  
  glClearColor((100.0f / 255.0f), (149.0f / 255.0f), (237.0f / 255.0f), 1.0f); //cornflower blue
  glClear (GL_COLOR_BUFFER_BIT);
  
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective( 60.0, resw/resh, 1.0, 1024.0 );
  
  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();

#ifdef HAVE_ASSIMP
  //load assets
  vd->cpumodel = aiImportFile("cpu.blend", aiProcess_GenNormals | aiProcess_GenSmoothNormals | aiProcessPreset_TargetRealtime_MaxQuality);  
  vd->cpu_scene_list = glGenLists(1);
  glNewList(vd->cpu_scene_list, GL_COMPILE);
  assimp_recursive_render(vd->cpumodel, vd->cpumodel->mRootNode);
  glEndList();

  vd->hdmodel = aiImportFile("hd.blend", aiProcess_GenNormals | aiProcess_GenSmoothNormals | aiProcessPreset_TargetRealtime_MaxQuality);  
  vd->hd_scene_list = glGenLists(1);
  glNewList(vd->hd_scene_list, GL_COMPILE);
  assimp_recursive_render(vd->hdmodel, vd->hdmodel->mRootNode);
  glEndList();

  vd->qcontmodel = aiImportFile("queuecontainer.blend", aiProcess_GenNormals | aiProcess_GenSmoothNormals | aiProcessPreset_TargetRealtime_MaxQuality);  
  vd->qcont_scene_list = glGenLists(1);
  glNewList(vd->qcont_scene_list, GL_COMPILE);
  assimp_recursive_render(vd->qcontmodel, vd->qcontmodel->mRootNode);
  glEndList();

  vd->qnodemodel = aiImportFile("queuenode.blend", aiProcess_GenNormals | aiProcess_GenSmoothNormals | aiProcessPreset_TargetRealtime_MaxQuality);  
  vd->qnode_scene_list = glGenLists(1);
  glNewList(vd->qnode_scene_list, GL_COMPILE);
  assimp_recursive_render(vd->qnodemodel, vd->qnodemodel->mRootNode);
  glEndList();
#endif

#ifdef HAVE_FTGL
#ifdef __LINUX__
  font_new(&vd->font, "/usr/share/fonts/libertine-ttf/LinBiolinum_R.ttf", 35, -1.5);
#endif
#ifdef _WIN32
  font_new(&vd->font, "C:\\Windows\\Fonts\\times.ttf", 35, -1.5);
#endif
#endif
}

void
gl_fcfs_step (PSCHEDSYS ss, void *data)
{
  PFCFS_VIS_DATA vd = data;
  char buffer[80];
  int i;
  int l;
  
  glClearColor((100.0f / 255.0f), (149.0f / 255.0f), (237.0f / 255.0f), 1.0f); //cornflower blue
  glClear (GL_COLOR_BUFFER_BIT);

#ifdef HAVE_FTGL
  //gl_test();
  //drawbox(300.0f,25.0f,1.0f, 400.0f,300.0f,0.0f, 0.0f,0.0f,1.0f);
  font_draw2d(vd->font, "FCFS CPU Scheduling Simulation", 0.0f,0.0f, 255,255,255,255);
  sprintf(buffer, "Tick: %d", ss->timer);
  font_draw2d(vd->font, buffer, 0.0f,530.0f/600.0f*resh, 0,0,255,255);
#endif

  //font_draw3d(font, "Ready Queue", -0.1f,0.0f,0.0f, 0,0,255,255);
  //draw_box2(0.5f,0.1f,1f, 0.0f,0.0f,0.0f, 0.0f,0.0f,0.0f, 0,255,0,255);

  //draw new queue
  draw_qcont(vd->qcont_scene_list, 0.26f,0.5f,0.5f, -4.0f,1.2f,-6.0f, 0.0f,0.0f,0.0f);

  //draw waiting queue
  draw_qcont(vd->qcont_scene_list, 0.26f,0.5f,0.5f, -4.0f,0.0f,-6.0f, 0.0f,0.0f,0.0f);

  //draw ready queue
  draw_qcont(vd->qcont_scene_list, 0.26f,0.5f,0.5f, -4.0f,-1.2f,-6.0f, 0.0f,0.0f,0.0f);
  
  l = toq_length(ss->new);
  for(i=0;i<l;i++)
    {
      //draw new nodes
      draw_qnode(vd->qnode_scene_list, 0.26f,0.5f,0.5f, 1.0f-i*0.26f,1.2f,-6.0f, 0.0f,0.0f,0.0f);
    }

  l = toq_length(ss->waiting);
  for(i=0;i<l;i++)
    {
      //draw waiting nodes
      draw_qnode(vd->qnode_scene_list, 0.26f,0.5f,0.5f, 1.0f-i*0.26f,0.0f,-6.0f, 0.0f,0.0f,0.0f);
    }

  l = toq_length(ss->ready);
  for(i=0;i<l;i++)
    {
      //draw ready nodes
      draw_qnode(vd->qnode_scene_list, 0.26f,0.5f,0.5f, 1.0f-i*0.26f,-1.2f,-6.0f, 0.0f,0.0f,0.0f);
    }

  //draw io
  if (toq_notempty(ss->waiting))
    draw_hd(vd->hd_scene_list, 0.5f,0.5f,1.0f, 2.5f,0.5f,-6.0f, 0.0f,0.0f,0.0f);
  else
    ;//draw_hd(vd->hd_scene_list, 0.5f,0.5f,1.0f, 2.5f,0.5f,-6.0f, 0.0f,0.0f,0.0f);

  //draw cpu
  if (ss->cpu)
    draw_cpu(vd->cpu_scene_list, 0.5f,0.5f,1.0f, 2.5f,-1.6f,-6.0f, 0.0f,0.0f+ss->timer%360,0.0f+45.0f);
  else
    ;//draw_cpu(vd->cpu_scene_list, 0.5f,0.5f,1.0f, 2.5f,-1.6f,-6.0f, 0.0f,0.0f+ss->timer%360,0.0f+45.0f);

  SDL_GL_SwapBuffers ();
}

void
gl_fcfs_done(void *data)
{
  PFCFS_VIS_DATA vd = data;
#ifdef HAVE_ASSIMP
  aiReleaseImport(vd->cpumodel);
  aiReleaseImport(vd->hdmodel);
  aiReleaseImport(vd->qcontmodel);
  aiReleaseImport(vd->qnodemodel);
#endif
#ifdef HAVE_FTGL
  font_free(vd->font);
#endif
}

////////////////////////////////////////////////////////////////////////
// RR1/2 Scheduler Drawing Code
////////////////////////////////////////////////////////////////////////

void
gl_rr_step_init (PSCHEDSYS ss, void *data)
{
  PRR_VIS_DATA vd = data;
  glEnable (GL_TEXTURE_2D);
  glViewport (0, 0, resw, resh);
  
  glClearColor((100.0f / 255.0f), (149.0f / 255.0f), (237.0f / 255.0f), 1.0f); //cornflower blue
  glClear (GL_COLOR_BUFFER_BIT);
  
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective( 60.0, resw/resh, 1.0, 1024.0 );
  
  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();

#ifdef HAVE_ASSIMP
  //load assets
  vd->cpumodel = aiImportFile("cpu.blend", aiProcess_GenNormals | aiProcess_GenSmoothNormals | aiProcessPreset_TargetRealtime_MaxQuality);  
  vd->cpu_scene_list = glGenLists(1);
  glNewList(vd->cpu_scene_list, GL_COMPILE);
  assimp_recursive_render(vd->cpumodel, vd->cpumodel->mRootNode);
  glEndList();

  vd->hdmodel = aiImportFile("hd.blend", aiProcess_GenNormals | aiProcess_GenSmoothNormals | aiProcessPreset_TargetRealtime_MaxQuality);  
  vd->hd_scene_list = glGenLists(1);
  glNewList(vd->hd_scene_list, GL_COMPILE);
  assimp_recursive_render(vd->hdmodel, vd->hdmodel->mRootNode);
  glEndList();

  vd->qcontmodel = aiImportFile("queuecontainer.blend", aiProcess_GenNormals | aiProcess_GenSmoothNormals | aiProcessPreset_TargetRealtime_MaxQuality);  
  vd->qcont_scene_list = glGenLists(1);
  glNewList(vd->qcont_scene_list, GL_COMPILE);
  assimp_recursive_render(vd->qcontmodel, vd->qcontmodel->mRootNode);
  glEndList();

  vd->qnodemodel = aiImportFile("queuenode.blend", aiProcess_GenNormals | aiProcess_GenSmoothNormals | aiProcessPreset_TargetRealtime_MaxQuality);  
  vd->qnode_scene_list = glGenLists(1);
  glNewList(vd->qnode_scene_list, GL_COMPILE);
  assimp_recursive_render(vd->qnodemodel, vd->qnodemodel->mRootNode);
  glEndList();
#endif

#ifdef _GNU_SOURCE
  if (ss->mode == SCHEDSYS_MODE_RR1)
    asprintf(&vd->name, "RR1 CPU Scheduling Simulation");
  else
    asprintf(&vd->name, "RR2 CPU Scheduling Simulation");
#else
  if (ss->mode == SCHEDSYS_MODE_RR1)
    vd->name = strdup("RR1 CPU Scheduling Simulation");
  else
    vd->name = strdup("RR2 CPU Scheduling Simulation");
#endif

#ifdef HAVE_FTGL
#ifdef __LINUX__
  font_new(&vd->font, "/usr/share/fonts/libertine-ttf/LinBiolinum_R.ttf", 35, -1.5);
#endif
#ifdef _WIN32
  font_new(&vd->font, "C:\\Windows\\Fonts\\times.ttf", 35, -1.5);
#endif
#endif
}

void
gl_rr_step (PSCHEDSYS ss, void *data)
{
  PRR_VIS_DATA vd = data;
  char buffer[80];
  int i;
  int l;
    
  glClearColor((100.0f / 255.0f), (149.0f / 255.0f), (237.0f / 255.0f), 1.0f); //cornflower blue
  glClear (GL_COLOR_BUFFER_BIT);

#ifdef HAVE_FTGL
  //gl_test();
  //drawbox(300.0f,25.0f,1.0f, 400.0f,300.0f,0.0f, 0.0f,0.0f,1.0f);
  font_draw2d(vd->font, vd->name, 0.0f,0.0f, 255,255,255,255);
  sprintf(buffer, "Tick: %d", ss->timer);
  font_draw2d(vd->font, buffer, 0.0f,530.0f/600.0f*resh, 0,0,255,255);
#endif

  //font_draw3d(font, "Ready Queue", -0.1f,0.0f,0.0f, 0,0,255,255);
  //draw_box2(0.5f,0.1f,1f, 0.0f,0.0f,0.0f, 0.0f,0.0f,0.0f, 0,255,0,255);

  //draw new queue
  draw_qcont(vd->qcont_scene_list, 0.26f,0.5f,0.5f, -4.0f,1.2f,-6.0f, 0.0f,0.0f,0.0f);

  //draw waiting queue
  draw_qcont(vd->qcont_scene_list, 0.26f,0.5f,0.5f, -4.0f,0.0f,-6.0f, 0.0f,0.0f,0.0f);

  //draw ready queue
  draw_qcont(vd->qcont_scene_list, 0.26f,0.5f,0.5f, -4.0f,-1.2f,-6.0f, 0.0f,0.0f,0.0f);

  l = toq_length(ss->new);
  for(i=0;i<l;i++)
    {
      //draw new nodes
      draw_qnode(vd->qnode_scene_list, 0.26f,0.5f,0.5f, 1.0f-i*0.26f,1.2f,-6.0f, 0.0f,0.0f,0.0f);
    }

  l = toq_length(ss->waiting);
  for(i=0;i<l;i++)
    {
      //draw waiting nodes
      draw_qnode(vd->qnode_scene_list, 0.26f,0.5f,0.5f, 1.0f-i*0.26f,0.0f,-6.0f, 0.0f,0.0f,0.0f);
    }

  l = toq_length(ss->ready);
  for(i=0;i<l;i++)
    {
      //draw ready nodes
      draw_qnode(vd->qnode_scene_list, 0.26f,0.5f,0.5f, 1.0f-i*0.26f,-1.2f,-6.0f, 0.0f,0.0f,0.0f);
    }

  //draw io
  if (toq_notempty(ss->waiting))
    draw_hd(vd->hd_scene_list, 0.5f,0.5f,1.0f, 2.5f,0.5f,-6.0f, 0.0f,0.0f,0.0f);
  else
    ;//draw_hd(vd->hd_scene_list, 0.5f,0.5f,1.0f, 2.5f,0.5f,-6.0f, 0.0f,0.0f,0.0f);

  //draw cpu
  if (ss->cpu)
    draw_cpu(vd->cpu_scene_list, 0.5f,0.5f,1.0f, 2.5f,-1.6f,-6.0f, 0.0f,0.0f+ss->timer%360,0.0f+45.0f);
  else
    ;//draw_cpu(vd->cpu_scene_list, 0.5f,0.5f,1.0f, 2.5f,-1.6f,-6.0f, 0.0f,0.0f+ss->timer%360,0.0f+45.0f);

  SDL_GL_SwapBuffers ();
}

void
gl_rr_done(void *data)
{
  PRR_VIS_DATA vd = data;
#ifdef HAVE_ASSIMP
  aiReleaseImport(vd->cpumodel);
  aiReleaseImport(vd->hdmodel);
  aiReleaseImport(vd->qcontmodel);
  aiReleaseImport(vd->qnodemodel);
#endif
#ifdef HAVE_FTGL
  font_free(vd->font);
#endif
  free (vd->name);
}

////////////////////////////////////////////////////////////////////////
// MLFQ Scheduler Drawing Code
////////////////////////////////////////////////////////////////////////

void
gl_mlfq_step_init (PSCHEDSYS ss, void *data)
{
  PMLFQ_VIS_DATA vd = data;
  glEnable (GL_TEXTURE_2D);
  glViewport (0, 0, resw, resh);
  
  glClearColor((100.0f / 255.0f), (149.0f / 255.0f), (237.0f / 255.0f), 1.0f); //cornflower blue
  glClear (GL_COLOR_BUFFER_BIT);
  
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective( 60.0, resw/resh, 1.0, 1024.0 );
  
  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();

#ifdef HAVE_ASSIMP
  //load assets
  vd->cpumodel = aiImportFile("cpu.blend", aiProcess_GenNormals | aiProcess_GenSmoothNormals | aiProcessPreset_TargetRealtime_MaxQuality);  
  vd->cpu_scene_list = glGenLists(1);
  glNewList(vd->cpu_scene_list, GL_COMPILE);
  assimp_recursive_render(vd->cpumodel, vd->cpumodel->mRootNode);
  glEndList();

  vd->hdmodel = aiImportFile("hd.blend", aiProcess_GenNormals | aiProcess_GenSmoothNormals | aiProcessPreset_TargetRealtime_MaxQuality);  
  vd->hd_scene_list = glGenLists(1);
  glNewList(vd->hd_scene_list, GL_COMPILE);
  assimp_recursive_render(vd->hdmodel, vd->hdmodel->mRootNode);
  glEndList();

  vd->qcontmodel = aiImportFile("queuecontainer.blend", aiProcess_GenNormals | aiProcess_GenSmoothNormals | aiProcessPreset_TargetRealtime_MaxQuality);  
  vd->qcont_scene_list = glGenLists(1);
  glNewList(vd->qcont_scene_list, GL_COMPILE);
  assimp_recursive_render(vd->qcontmodel, vd->qcontmodel->mRootNode);
  glEndList();

  vd->qnodemodel = aiImportFile("queuenode.blend", aiProcess_GenNormals | aiProcess_GenSmoothNormals | aiProcessPreset_TargetRealtime_MaxQuality);  
  vd->qnode_scene_list = glGenLists(1);
  glNewList(vd->qnode_scene_list, GL_COMPILE);
  assimp_recursive_render(vd->qnodemodel, vd->qnodemodel->mRootNode);
  glEndList();
#endif

#ifdef HAVE_FTGL
#ifdef __LINUX__
  font_new(&vd->font, "/usr/share/fonts/libertine-ttf/LinBiolinum_R.ttf", 35, -1.5);
#endif
#ifdef _WIN32
  font_new(&vd->font, "C:\\Windows\\Fonts\\times.ttf", 35, -1.5);
#endif
#endif
}

void
gl_mlfq_step (PSCHEDSYS ss, void *data)
{
  PMLFQ_VIS_DATA vd = data;
  char buffer[80];
  int i;
  int l;
  
  glClearColor((100.0f / 255.0f), (149.0f / 255.0f), (237.0f / 255.0f), 1.0f); //cornflower blue
  glClear (GL_COLOR_BUFFER_BIT);

 #ifdef HAVE_FTGL
  //gl_test();
  //drawbox(300.0f,25.0f,1.0f, 400.0f,300.0f,0.0f, 0.0f,0.0f,1.0f);
  font_draw2d(vd->font, "MLFQ CPU Scheduling Simulation", 0.0f,0.0f, 255,255,255,255);
  sprintf(buffer, "Tick: %d", ss->timer);
  font_draw2d(vd->font, buffer, 0.0f,530.0f/600.0*resh, 0,0,255,255);
#endif

  //font_draw3d(font, "Ready Queue", -0.1f,0.0f,0.0f, 0,0,255,255);
  //draw_box2(0.5f,0.1f,1f, 0.0f,0.0f,0.0f, 0.0f,0.0f,0.0f, 0,255,0,255);

  //draw new queue
  draw_qcont(vd->qcont_scene_list, 0.26f,0.5f,0.5f, -4.0f,1.2f,-6.0f, 0.0f,0.0f,0.0f);

  //draw waiting queue
  draw_qcont(vd->qcont_scene_list, 0.26f,0.5f,0.5f, -4.0f,0.0f,-6.0f, 0.0f,0.0f,0.0f);

  //draw ready queue 3
  draw_qcont(vd->qcont_scene_list, 0.26f,0.3f,0.5f, -4.0f,-0.8f,-6.0f, 0.0f,0.0f,0.0f);

  //draw ready queue 2
  draw_qcont(vd->qcont_scene_list, 0.26f,0.3f,0.5f, -4.0f,-1.3f,-6.0f, 0.0f,0.0f,0.0f);

  //draw ready queue 1
  draw_qcont(vd->qcont_scene_list, 0.26f,0.3f,0.5f, -4.0f,-1.8f,-6.0f, 0.0f,0.0f,0.0f);

  //draw ready queue 0
  draw_qcont(vd->qcont_scene_list, 0.26f,0.3f,0.5f, -4.0f,-2.3f,-6.0f, 0.0f,0.0f,0.0f);

  l = toq_length(ss->new);
  for(i=0;i<l;i++)
    {
      //draw new nodes
      draw_qnode(vd->qnode_scene_list, 0.26f,0.5f,0.5f, 1.0f-i*0.26f,1.2f,-6.0f, 0.0f,0.0f,0.0f);
    }

  l = toq_length(ss->waiting);
  for(i=0;i<l;i++)
    {
      //draw waiting nodes
      draw_qnode(vd->qnode_scene_list, 0.26f,0.5f,0.5f, 1.0f-i*0.26f,0.0f,-6.0f, 0.0f,0.0f,0.0f);
    }

  l = toq_length(ss->mlfqready[3]);
  for(i=0;i<l;i++)
    {
      //draw ready nodes 3
      draw_qnode(vd->qnode_scene_list, 0.26f,0.3f,0.5f, 1.0f-i*0.26f,-0.8f,-6.0f, 0.0f,0.0f,0.0f);
    }

  l = toq_length(ss->mlfqready[2]);
  for(i=0;i<l;i++)
    {
      //draw ready nodes 2
      draw_qnode(vd->qnode_scene_list, 0.26f,0.3f,0.5f, 1.0f-i*0.26f,-1.3f,-6.0f, 0.0f,0.0f,0.0f);
    }

  l = toq_length(ss->mlfqready[1]);
  for(i=0;i<l;i++)
    {
      //draw ready nodes 1
      draw_qnode(vd->qnode_scene_list, 0.26f,0.3f,0.5f, 1.0f-i*0.26f,-1.8f,-6.0f, 0.0f,0.0f,0.0f);
    }

  l = toq_length(ss->mlfqready[0]);
  for(i=0;i<l;i++)
    {
      //draw ready nodes 0
      draw_qnode(vd->qnode_scene_list, 0.26f,0.3f,0.5f, 1.0f-i*0.26f,-2.3f,-6.0f, 0.0f,0.0f,0.0f);
    }

  //draw io
  if (toq_notempty(ss->waiting))
    draw_hd(vd->hd_scene_list, 0.5f,0.5f,1.0f, 2.5f,0.5f,-6.0f, 0.0f,0.0f,0.0f);
  else
    ;//draw_hd(vd->hd_scene_list, 0.5f,0.5f,1.0f, 2.5f,0.5f,-6.0f, 0.0f,0.0f,0.0f);

  //draw cpu
  if (ss->cpu)
    draw_cpu(vd->cpu_scene_list, 0.5f,0.5f,1.0f, 2.5f,-1.6f,-6.0f, 0.0f,0.0f+ss->timer%360,0.0f+45.0f);
  else
    ;//draw_cpu(vd->cpu_scene_list, 0.5f,0.5f,1.0f, 2.5f,-1.6f,-6.0f, 0.0f,0.0f+ss->timer%360,0.0f+45.0f);

  SDL_GL_SwapBuffers ();

  //assimp_do_motion();


}

void
gl_mlfq_done(void *data)
{
  PMLFQ_VIS_DATA vd = data;
#ifdef HAVE_ASSIMP
  aiReleaseImport(vd->cpumodel);
  aiReleaseImport(vd->hdmodel);
  aiReleaseImport(vd->qcontmodel);
  aiReleaseImport(vd->qnodemodel);
#endif
#ifdef HAVE_FTGL
  font_free(vd->font);
#endif
}
#endif