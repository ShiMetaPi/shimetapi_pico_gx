/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __CAMERA_H__
#define __CAMERA_H__

typedef struct camera {
    int (*init)(void);
    int (*deinit)(void);
    int (*open)(void);
    int (*close)(void);
    int (*run)(void);
    int (*stop)(void);
} camera;

camera *get_camera(void);

#endif //__CAMERA_H__

