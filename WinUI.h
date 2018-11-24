#ifndef _WIN_UI_H_
#define _WIN_UI_H_

#include <windows.h>

/*DEBUG MACRO*/
/*
*	NDEBUG:this macro disable debug code
*	DYNAMIC_CHECK:this macro enable dynamic check
*/

#define APP_TITLE       "Unication DevTool"

/*layout mode*/
#define LAYOUT_AUTO     0
#define LAYOUT_MANUAL   1

/*for message handler argument */
typedef union {
	HWND hwnd;/*reference when WM_COMMAND message occur*/
	LPNMHDR pnmh;/*reference when WM_NOTIFY message occur*/	
}message_handler_arg_t;

/*for message handler*/
typedef BOOL (*message_handler_t)(message_handler_arg_t);
typedef UINT message_code_t;
/*for message_node*/
typedef struct _message_node_t{
	message_code_t message_code;
	message_handler_t message_handler;
}message_node_t;

/*for wnd_tree_t: most of the member are argments of CreateWindow*/
typedef struct _wnd_tree_t{
	HWND hwnd;
	int Flags;/*for layout : AUTO or MANUAL*/
	LPCTSTR lpClassName;
	LPCTSTR lpWindowName;
	DWORD dwStyle;
	int x;
	int y;
	int nWidth;
	int nHeight;
	WORD wChildCnt;
	WORD wMessageNodeCnt;
	struct _wnd_tree_t **pChildList;/*child window list,dynamic alloc when it's needed,and free memory by this lib,but need programer to malloc the memory*/
	struct _message_node_t **pMessageNodeList;/*message node list,memory mangament is same as child window list.*/

}wnd_tree_t;

/*             interface            */

/* Function:
 * Description:
 * Parameter:
 * Return Value:
 * 	success: pointer of wnd_tree_t which just added
 * 	fail:NULL
 * */
struct _wnd_tree_t *AddWnd(struct _wnd_tree_t *parent,LPCTSTR lpClassName,\
		LPCTSTR lpWindowName,DWORD dwStyle,int x,int y,\
		int nWidth,int nHeight);
struct _wnd_tree_t *AddWndTree(struct _wnd_tree_t *parent,struct _wnd_tree_t *child);
BOOL AddMessageHandler(struct _wnd_tree_t *window,message_code_t message_code,message_handler_t message_handler);
#endif
