import math
import numpy as np

# https://en.wikipedia.org/wiki/Focal_length

w = 1024
h = 768

pixelsize = 36.0 / 1024

fov_degrees = 45
fov = np.radians(45)

# FOV = 2 arctan x/2f
# => f = x/(2.tan(fov/2))


fx = (w * pixelsize) / (2.0 * math.tan(fov / 2.0))
fy = (h * pixelsize) / (2.0 * math.tan(fov / 2.0))

print(f"fx={fx:.2f}, fy={fy:.2f}, pixel size: {pixelsize} mm/px")


