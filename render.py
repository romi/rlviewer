import rlviewer
import cv2
import numpy as np
import os
import json
import argparse



if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Process some integers.')
    parser.add_argument('file', help='path to the .obj file')

    args = parser.parse_args()

    output_dir = "out"
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    rlviewer.load(args.file) 
    rlviewer.set_light(0, 120, 0, 0, 5000) 
    rlviewer.set_light(1, -120, 0, 0, 5000) 
    rlviewer.set_light(2, 0, 0, 120, 5000) 
    rlviewer.set_light(3, 0, 0, -120, 5000) 

    for lat in range(0, 90, 5): 
        for lon in range(0, 360, 3):
            # Take a picture of the object positioned in the origin with
            # the camera moving on a sphere with a radius of 20 and
            # positioned at the given latitude and longitude.
            m = rlviewer.get_matrix(20.0, np.radians(lat), np.radians(lon))
            with open(f"{output_dir}/image-{lat:03d}-{lon:03d}.json", 'w') as f:
                json.dump(m.tolist(), f, indent=4)
            img = rlviewer.grab(20.0, np.radians(lat), np.radians(lon)) 
            cv2.imwrite(f"{output_dir}/image-{lat:03d}-{lon:03d}.png", img) 
