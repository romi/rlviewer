#ifndef __VIEWER_H
#define __VIEWER_H

#ifdef __cplusplus
extern "C" {
#endif
        
        int viewer_init(void);
        int viewer_load(const char *path);
        int viewer_grab(uint8_t *pixels, float r, float lat, float lan);
        void viewer_set_light(int index, float x, float y, float z, float power);
        int viewer_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif //__VIEWER_H
