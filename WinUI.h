#ifndef _WIN_UI_H_
#define _WIN_UI_H_

#include <windows.h>

/*DEBUG MACRO*/
/*
*	NDEBUG:this macro disable debug code
*	DYNAMIC_CHECK:this macro enable dynamic check
*/
#warning "maybe I should add a mutex for WndRoot"
#warning "I should make dynamic add&del WndTree test when I complete the frame"
#warning "I have not support Unicode yet,I should do that work after all"

#define APP_TITLE       "Unication DevTool"

/*layout mode*/
#define LAYOUT_AUTO     0
#define LAYOUT_MANUAL   1


/*for message handler*/
typedef WNDPROC message_handler_t;
typedef UINT message_code_t;
/*for message_node*/
typedef struct _message_node_t{
	message_code_t message_code;
	message_handler_t message_handler;
}message_node_t;

/*for wnd_tree_t: most of the member are argments of CreateWindowEx*/
typedef struct _wnd_tree_t{
	HWND hwnd;	
	DWORD dwExStyle;	
	struct _wnd_tree_t *parent;/*this member can speed GetParentWnd up,also can prevent invoke AddWndTree mutiplely by same child*/
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
	struct _message_node_t *pMessageNodeList;/*message node list,memory mangament is same as child window list.*/

}wnd_tree_t;

/*             macro            */

#define WND_TREE_INIT	{\
.hwnd = NULL,\
.dwExStyle = 0,\
.parent = NULL,\
.lpClassName = NULL,\
.lpWindowName = NULL,\
.dwStyle = 0,\
.x = 0,\
.y = 0,\
.nWidth = 0,\
.nHeight = 0,\
.wChildCnt = 0,\
.wMessageNodeCnt = 0,\
.pChildList = NULL,\
.pMessageNodeList = NULL,\
}
#define DECLARE_WND_TREE(name)	struct _wnd_tree_t name = WND_TREE_INIT

#define INFO_MESSAGE(fmt,args...)   do{TCHAR msg_buf[1024];\
    snprintf(msg_buf,1024,fmt,##args);\
    MessageBox(NULL,msg_buf,TEXT("Information"),MB_OK);}while(0)      
#define WARNING_MESSAGE(fmt,args...)    do{TCHAR msg_buf[1024];\
    snprintf(msg_buf,1024,fmt,##args);\
    MessageBox(NULL,msg_buf,TEXT("Warning"),MB_ICONWARNING);}while(0)      
#define ERROR_MESSAGE(fmt,args...)  do{TCHAR msg_buf[1024];\
    snprintf(msg_buf,1024,fmt,##args);\
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,NULL,GetLastError(),0,\
                msg_buf+lstrlen(msg_buf),1024-lstrlen(msg_buf),NULL);\
    MessageBox(NULL,msg_buf,TEXT("Error"),MB_ICONERROR);}while(0)      

#ifndef NDEBUG
#include <assert.h>
#define WIN_DEBUG(fmt,args...)      do{TCHAR msg_buf[1024];\
    snprintf(msg_buf,1024,TEXT("%s:%s:%d:")fmt,__FILE__,__func__,__LINE__,##args);\
    MessageBox(NULL,msg_buf,TEXT("WIN_DEBUG"),MB_OK);}while(0)      
#else
#define WIN_DEBUG(fmt,args...)
#endif
/* Function:
 * Description:
 * Parameter:
 * Return Value:
 * 	success: pointer of wnd_tree_t which just added
 * 	fail:NULL
 * */

 /*             interface                */
struct _wnd_tree_t *CopyWndTree(struct _wnd_tree_t *root);
struct _wnd_tree_t *LinkWndTree(struct _wnd_tree_t *parent,struct _wnd_tree_t *child);
struct _wnd_tree_t *AddWndTree(struct _wnd_tree_t *parent,struct _wnd_tree_t *child);
void DestroyWndTree(struct _wnd_tree_t *root);

BOOL AddMessageHandler(struct _wnd_tree_t *window,message_code_t message_code,message_handler_t message_handler);
#endif
