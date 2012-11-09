if DEBUG
override CFLAGS=-O0 -g
endif

if GENERICBUILD
if DEBUG
override CFLAGS=-mtune=generic -O0 -g
else
override CFLAGS=-mtune=generic
endif
endif

if GRAPHICS
GRAPHICS_CFLAGS=-DHAVE_SDL -DHAVE_ASSIMP -DHAVE_FTGL
GRAPHICS_SOURCES=schedsysgl.c
else
GRAPHICS_SOURCES=
endif

if TESTS
TESTPROGS=tinyqueue tinylist tinyprioq tinyoq
else
TESTPROGS=
endif

bin_PROGRAMS=SimScheduler $(TESTPROGS)
SimScheduler_SOURCES=main.c processlist.c tinyoq.c tinylist.c process.c schedsys.c $(GRAPHICS_SOURCES)
SimScheduler_LDFLAGS=$(SDL_LIBS) $(ASSIMP_LIBS) $(FTGL_LIBS) $(OPENMP_LIBS)
SimScheduler_CFLAGS=$(GRAPHICS_CFLAGS) $(SDL_CFLAGS) $(ASSIMP_CFLAGS) $(FTGL_CFLAGS)
SimScheduler_MANS=simscheduler.1
tinylist_CFLAGS=-DTEST
tinylist_SOURCES=tinylist.c
tinyoq_CFLAGS=-DTEST
tinyoq_SOURCES=tinyoq.c
tinyprioq_CFLAGS=-DTEST
tinyprioq_SOURCES=tinyprioq.c
tinyqueue_CFLAGS=-DTEST
tinyqueue_SOURCES=tinyqueue.c
EXTRA_DIST=in0.txt in1.txt out0.txt out1.txt
dist_data_DATA=cpu.blend hd.blend queuecontainer.blend queuenode.blend
