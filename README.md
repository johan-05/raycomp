# Raycomp

## Running the code
first clone the repo
'git clone https://github.com/johan-05/raycomp.git'
'cd raycomp'

**Linux**
'gcc -o raycomp poly.c glad.c -lglfw -lm'

**Windows**
make sure you have Visual Studio installed so you have access to the cl.exe.
If not you can get it from https://visualstudio.microsoft.com/#vs-section
'cl.exe glad.c poly.c glfw3dll.lib opengl32.lib'

