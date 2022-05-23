# rlviewer
A fast 3D renderer for the reinforcement learning


rlviewer is a Python module that loads a .OBJ file, renders it using
the OpenGL API, and returns the resulting view as a numpy array. The
code is derived from the at [OpenGL
tutorials](https://github.com/opengl-tutorials/ogl)
([code](http://www.opengl-tutorial.org/)).


## Installation

The module requires the following dependencies (most will already be installed):

```
sudo apt install  cmake make g++ libx11-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxrandr-dev libxext-dev libxcursor-dev libxinerama-dev libxi-dev libglfw3-dev libglfw3-dev libglew2.0 libglew-dev
```

To build the code:

```
python3 setup.py build
```


Install the module:

System-wide:
```
sudo python3 setup.py install
```


Or for the current user:
```
python3 setup.py install --user
```

## Usage

Once the module is imported (`import rlviewer`), there are two
functions available: `rlviewer.load(path)` and
`rlviewer.grab(radius, latitude, longitude)`.

The `load` function imports a .OBJ file. The 3D object should be
centered on the origin. The obj loader is very basic and you may run
into issues. (TODO: fix issues :) and/or document work arounds...)

The `grab` function renders the scene. The camera moves on a sphere
around the object. You must specify the `radius` of the sphere and the
camera's position on the sphere with the `latitude` (vertical angle)
and `longitude` (horizontal angle).

The function returns a numpy array that represents the rendered
scene. It's a RGB images and its size is currently fixed at 1024 x 768
pixels. So you get a `np.array((1024, 768, 3), dtype=np.uint8)` matrix
in return.

In the file `render.py` you find a complete example:

```
import rlviewer
import cv2
import numpy as np
import os
 
output_dir = "out"
if not os.path.exists(output_dir):
    os.makedirs(output_dir)
  
rlviewer.load('215.obj') 

for lat in range(0, 90, 5): 
    for lon in range(0, 360, 3):
        # Take a picture of the object positioned in the origin with
        # the camera moving on a sphere with a radius of 20 and
        # positioned at the given latitude and longitude.
        img = rlviewer.grab(20.0, np.radians(lat), np.radians(lon)) 
        cv2.imwrite(f"{output_dir}/image-{lat:03d}-{lon:03d}.png", img) 
```


# Acknowledgements

Thank you to the OpenGL tutorials! The original OpenGL source is
available under the [WTF
license](https://en.wikipedia.org/wiki/WTFPL).
