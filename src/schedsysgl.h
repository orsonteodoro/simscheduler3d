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

#ifndef SCHEDSYSGL_H
#define SCHEDSYSGL_H

#ifdef HAVE_FTGL
#include <FTGL/ftgl.h>
#endif

#ifdef __LINUX__
#include <assimp/aiScene.h>
#endif

#ifdef _WIN32
#include <aiScene.h>
#endif

#include "schedsys.h"

extern float resw;
extern float resh;
extern int fullscreen;

typedef struct
{
#ifdef HAVE_FTGL
  FTGLfont *font;
#endif
  const struct aiScene *cpumodel;
  const struct aiScene *hdmodel;
  const struct aiScene *qcontmodel;
  const struct aiScene *qnodemodel;
  GLuint cpu_scene_list;
  GLuint hd_scene_list;
  GLuint qcont_scene_list;
  GLuint qnode_scene_list;
  int j;
} FCFS_VIS_DATA, *PFCFS_VIS_DATA;

typedef struct
{
#ifdef HAVE_FTGL
  FTGLfont *font;
#endif
  const struct aiScene *cpumodel;
  const struct aiScene *hdmodel;
  const struct aiScene *qcontmodel;
  const struct aiScene *qnodemodel;
  GLuint cpu_scene_list;
  GLuint hd_scene_list;
  GLuint qcont_scene_list;
  GLuint qnode_scene_list;
  int j;
  char *name;
} RR_VIS_DATA, *PRR_VIS_DATA;

typedef struct
{
#ifdef HAVE_FTGL
  FTGLfont *font;
#endif
  const struct aiScene *cpumodel;
  const struct aiScene *hdmodel;
  const struct aiScene *qcontmodel;
  const struct aiScene *qnodemodel;
  GLuint cpu_scene_list;
  GLuint hd_scene_list;
  GLuint qcont_scene_list;
  GLuint qnode_scene_list;
  int j;
} MLFQ_VIS_DATA, *PMLFQ_VIS_DATA;

void gl_fcfs_step_init (PSCHEDSYS ss, void *data);
void gl_fcfs_step (PSCHEDSYS ss, void *data);
void gl_fcfs_done();

void gl_rr_step_init (PSCHEDSYS ss, void *data);
void gl_rr_step (PSCHEDSYS ss, void *data);
void gl_rr_done();

void gl_mlfq_step_init (PSCHEDSYS ss, void *data);
void gl_mlfq_step (PSCHEDSYS ss, void *data);
void gl_mlfq_done();

#endif
