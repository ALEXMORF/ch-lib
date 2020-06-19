# CH-lib

Code that I found reusable when making my own projects, here's a rundown.

gl_define.h:
. All the opengl definitions for core profile, up to opengl 4.5
. LoadOpenglFunction() function, that loads all the opengl functions. It uses the 
  function loader parameter fed to it to grab memory address for each function then
  assign them to the functions ptrs defined inside the file.

kernel.h
. utility functions. ASSERT(), ARRAY_COUNT(), defer(), etc.
. 3D math functions. vectors, matrices, quaternions, etc.
. Some Opengl helper functions 

ch_win32.h
. win32 helpers such as window creation, window toggles.

ch_d3d12.h
. d3d12 helpers and gpu context structure for multi-frame in flight rendering

ch_math.h
. math stuff

ch_bmp.h
. a small bmp writer

ch_gl.h
. opengl related functions and loaders extracted from kernel.h

