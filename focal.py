import math
import numpy as np

# https://en.wikipedia.org/wiki/Focal_length
# https://community.khronos.org/t/how-to-set-focal-length-in-opengl/46176/4

w = 1024
h = 768

# Assuming a standard 36mm film
pixelsize = 36.0 / w

fov_degrees = 45
fov = np.radians(45)

# FOV = 2 arctan x/2f
# => f = x/(2.tan(fov/2))


fx = (w * pixelsize) / (2.0 * math.tan(fov / 2.0))
fy = (h * pixelsize) / (2.0 * math.tan(fov / 2.0))

print(f"fx={fx:.2f}, fy={fy:.2f}, pixel size: {pixelsize} mm/px")


