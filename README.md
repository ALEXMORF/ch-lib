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

win32_kernel.h
. Windows helper functions. Such as Win32CreateWindow(), Win32ToggleFullscreen() 
  Win32InitializeOpengl(), etc.

gl_imgui.cpp
. InitImgui() initializes IMGUI if Opengl context & functions are availible. 
  IT DOES NOT DO KEYMAPPING, IT ONLY HANDLES THE GRAPHICS PART OF THE INIT CODE.