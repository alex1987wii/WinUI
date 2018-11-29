#ifndef DEV_TOOLS_H
#define DEV_TOOLS_H
#include "WinUI.h"
/* layout */
#define DEV_TOOLS_WIDTH         700         /* Unication dev tools main window width */
#define DEV_TOOLS_HEIGHT        550         /* Unication dev tools main window height */

#define X_MARGIN                10
#define Y_MARGIN                10
#define HEIGHT_CONTROL          30
#define TAB_HEAD_HEIGHT         30

#define TAB_CLIENT_WIDTH        (DEV_TOOLS_WIDTH - X_MARGIN*4)
#define TAB_CLIENT_HEIGHT       (DEV_TOOLS_HEIGHT - Y_MARGIN*4 - TAB_HEAD_HEIGHT - Y_MARGIN)

#define POP_WIN_HEIGHT          110

#define TAB_CLIENT_START_X      X_MARGIN
#define TAB_CLIENT_START_Y      (TAB_HEAD_HEIGHT + Y_MARGIN*2)
#define WIDTH_EDIT              420
#define WIDTH_EDIT_NAND         360
#define WIDTH_EDIT1             135

#define WIDTH_CHECKBOX          120
#define WIDTH_BUTTON            90
#define WIDTH_BUTTON_NANDFLASH_PROGRAM 110
#define WIDTH_TEXT              70
#define WIDTH_COMBO             200
#define TAB_NANDFLASH_START_X   5*X_MARGIN
#define TAB_NANDFLASH_WIDTH     (DEV_TOOLS_WIDTH - X_MARGIN*4) - 60
#define TAB_NANDFLASH_HEIGHT    (DEV_TOOLS_HEIGHT - Y_MARGIN*4 - TAB_HEAD_HEIGHT)/2 - 50

#define FILE_DL_START_X         10
#define FILE_DL_START_Y         0
#define FILE_DL_WIDTH           TAB_CLIENT_WIDTH-X_MARGIN*2
#define FILE_DL_HEIGHT          (TAB_CLIENT_HEIGHT - 2*Y_MARGIN)/2 - 30
#define WIN_STATIC_OFFSET       (3)
#define WIN_COMBO_OFFSET        (10)

#define CBS_PER_LINE            (3)

#endif