# korori

SDL2 + Opengl 3.3 target based PC game library.

# How to build

First pull down all git submodules (included with nested submodules) with `git submodule update --init --recursive`.

Then go to root directory then `make` to build krr into static library `libkrr.a`.

There are test samples in `test` directory. `cd test` then build it with `make` or individual one like `make <name of source file>`.

# Credits

Testing models

* Stall (.obj) model and texture, Stanfort Dragon (.obj) model which grabbed from Thinmatrix tutorial - the former is modified for its scaling, facing orientation, and exported texture.
* [Horse model](https://www.turbosquid.com/3d-models/free-low--horse-3d-model/810753) (.obj) - we've modified its scaling, facing orientation, and exported texture externally.
