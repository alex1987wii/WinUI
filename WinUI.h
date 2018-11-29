#ifndef _WIN_UI_H_
#define _WIN_UI_H_

#include <windows.h>
#include <commctrl.h>
/*DEBUG MACRO*/
/*
*	NDEBUG:this macro disable debug code
*	DYNAMIC_CHECK:this macro enable dynamic check
*/
#warning "maybe I should add a mutex for WndRoot"
#warning "I should make dynamic add&del WndTree test when I complete the frame"
#warning "I have not support Unicode yet,I should do that work after all"



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
	struct _wnd_tree_t *parent;/*this member can speed GetParentWnd up,also can prevent invoke AddWndTree mutiplely by same child*/
	DWORD dwExStyle;	
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
#define INIT_WND_TREE(name,m_hwnd,m_parent,m_dwExStyle,m_lpClassName,m_lpWindowName,\
m_dwStyle,m_x,m_y,m_nWidth,m_nHeight,m_wChildCnt,m_wMessageNodeCnt,m_pChildList,m_pMessageNodeList)	\
struct _wnd_tree_t name = {\
.hwnd = m_hwnd,\
.dwExStyle = m_dwExStyle,\
.parent = m_parent,\
.lpClassName = m_lpClassName,\
.lpWindowName = m_lpWindowName,\
.dwStyle = m_dwStyle,\
.x = m_x,\
.y = m_y,\
.nWidth = m_nWidth,\
.nHeight = m_nHeight,\
.wChildCnt = m_wChildCnt,\
.wMessageNodeCnt = m_wMessageNodeCnt,\
.pChildList = m_pChildList,\
.pMessageNodeList = m_pMessageNodeList,\
}

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


/*******************      global varibles         ********************/
extern struct _wnd_tree_t *pWndRoot;
extern struct _wnd_tree_t *pWndMain;
extern HWND hwndMain;
extern HINSTANCE hInst;
BOOL InitApplication(void);
extern TCHAR szAppName[];

/*******************      interface         ********************/
#define DECLARE_HANDLER(name) LRESULT CALLBACK name(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)

DECLARE_HANDLER(OnSelChange);
DECLARE_HANDLER(OnNotify);
DECLARE_HANDLER(OnDestory);
DECLARE_HANDLER(OnCommand);


struct _wnd_tree_t *CopyWndTree(struct _wnd_tree_t *root);
struct _wnd_tree_t *LinkWndTree(struct _wnd_tree_t *parent,struct _wnd_tree_t *child);
struct _wnd_tree_t *AddWndTree(struct _wnd_tree_t *parent,struct _wnd_tree_t *child);
void DestroyWndTree(struct _wnd_tree_t *root);

BOOL AddMessageHandler(struct _wnd_tree_t *window,message_code_t message_code,message_handler_t message_handler);
#endif
