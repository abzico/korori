# korori

SDL2 + OpenGL ES 3.0 - Android + Windows/Linux target lightweight game library

See [TODO](TODO.md) for development roadmap.

# Dependencies

You need to install all of the following on your development machine.

* [SDL2](https://www.libsdl.org/download-2.0.php)
* [SDL2_image](https://www.libsdl.org/projects/SDL_image/)
* [SDL2_mixer](https://www.libsdl.org/projects/SDL_mixer/)
* [freetype](https://www.freetype.org/)
* [cglm](https://github.com/recp/cglm)
* [vector_c](https://github.com/haxpor/vector_c)
* [hashmap_c](https://github.com/haxpor/hashmap_c)
* [texpackr](https://github.com/abzico/texpackr)
* [GLAD](https://github.com/Dav1dde/glad) - for ease of installation, use its web service to generate OpenGL ES 3.0 then put its only generated header directory (both `glad` and `KHR`) into your system (i.e. `/usr/local/include`)

# Features

- Apart from normal image file formats, support loading DDS file which encapsulates compressed texture format of S3TC for PC, or ETC2 (and its variants ETC2_RGB, ETC2_RGBA, and ETC2_RGBA1) for Android)
- Terrain generation and rendering
- Skybox
- Forward rendering with multiple light source
- Per-pixel lighting shader
- Support loading, and rendering .obj model file
- Support text rendering with input .ttf file
- ... (more to be added)

# How to build

Build system is based on autotools.

* `./autogen.sh`
* `./configure`
* `make`
* `make install` - to install krr (as static/shared library, along with header files) to your system
* `make samples` - to build all the sample programs in `samples/` directory
* `make sample-...` - to build specific sample program, but you need to execute `make` first once. The list of available sample program listed next section

## Build for Android

Take benefit from another two of our projects to help ease in cross compiling for Android.
Check the following two projects

1. [androidbuildlib](https://github.com/abzico/androidbuildlib) - help in cross compile an autotools-based library project to Android targeting multiple architectures (abi)
2. [setandroidbuilddev](https://github.com/abzico/setandroidbuildenv) - help to set/unset build related environment variable for cross compiling to Android in case you don't use `androidbuildlib` and want to do some manual task

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
* `dds` - reading dds texture (mipmaps, with alpha, in format DXT5), then render on screen. DDS header info will be shown on console.

# Credits

Testing models

* Stall (.obj) model and texture, Stanfort Dragon (.obj) model which grabbed from Thinmatrix tutorial - the former is modified for its scaling, facing orientation, and exported texture.
* [Horse model](https://www.turbosquid.com/3d-models/free-low--horse-3d-model/810753) (.obj) - we've modified its scaling, facing orientation, and exported texture externally.
* Zombie sprite is copyrights and owned by Secrete Character, from [Zombie Hero : Kiki Strikes Back](http://zombie-hero.com).
