# korori

SDL2 + Opengl 3.3 target based PC game library.

# Dependencies

You need to install all of the following on your development machine.

* [SDL2](https://www.libsdl.org/download-2.0.php)
* [SDL2_image](https://www.libsdl.org/projects/SDL_image/)
* [SDL2_mixer](https://www.libsdl.org/projects/SDL_mixer/)
* [GLEW](http://glew.sourceforge.net/)
* [freetype](https://www.freetype.org/)
* [cglm](https://github.com/recp/cglm)
* [vector_c](https://github.com/haxpor/vector_c)
* [hashmap_c](https://github.com/haxpor/hashmap_c)
* [texpackr](https://github.com/abzico/texpackr)

# How to build

Build system is based on autotools.

* `./autogen.sh`
* `./configure`
* `make`
* `make install` - to install krr (as static/shared library, along with header files) to your system
* `make samples` - to build all the sample programs in `samples/` directory
* `make sample-...` - to build specific sample program, but you need to execute `make` first once. The list of available sample program listed next section

# Samples

The list of available sample to build in `samples/`.

* `doublemulticolorshader` - custom double color shader
* `rotatingcube`
* `rotatingplane`
* `readobjfile_manual` - read .obj file while we build VBO as part of VAO to draw loaded .obj file manually wrapping around `SIMPLEMODEL`
* `readobjfile` - more automatic after loading .obj file to render on screen
* `advanced_model` - load .obj model then render with some other features on top
* `headless` - headless program without GUI window, it will load .obj model then shows it while rotating the model, then after enough frames has been rendered, it will take snapshot of screen then finally write as output .png file `headless-snapshot.tga`
* `terrain`

# Credits

Testing models

* Stall (.obj) model and texture, Stanfort Dragon (.obj) model which grabbed from Thinmatrix tutorial - the former is modified for its scaling, facing orientation, and exported texture.
* [Horse model](https://www.turbosquid.com/3d-models/free-low--horse-3d-model/810753) (.obj) - we've modified its scaling, facing orientation, and exported texture externally.
