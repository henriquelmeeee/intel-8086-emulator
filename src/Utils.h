#pragma once

#ifndef UTILS_H
#define UTILS_H

//#define CONNECT_BY_GDB
#define STEP_BY_STEP

#define VIDEO_MEMORY_BASE 0xB8000
#define VIDEO_WIDTH 80
#define VIDEO_HEIGHT 25

extern unsigned long cursor_location;

#define RETURN return {}

/* GDB-related */
  #define PORT 1234
  
#endif
