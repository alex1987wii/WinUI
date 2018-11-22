
#include "WinUI.h"

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

#ifdef DEBUG
#include <assert.h>
#define WIN_DEBUG(fmt,args...)      do{TCHAR msg_buf[1024];\
    snprintf(msg_buf,1024,TEXT("%s:%d:")fmt,__func__,__LINE__,##args);\
    MessageBox(NULL,msg_buf,TEXT("WIN_DEBUG"),MB_OK);}while(0)      
#else
#define WIN_DEBUG(fmt,args...)
#endif
HWND hwndMain;
HINSTANCE hInst;
struct _wnd_tree_t *WndRoot = NULL;

struct _wnd_tree_t *AddChildWnd(struct _wnd_tree_t *parent,LPCTSTR lpClassName,\
            LPCTSTR lpWindowName,DWORD dwStyle,int iLayOutMode,\
            int x,int y,int nWidth,int nHeight)
{
    /*initialize WndRoot*/
    if(parent == NULL && WndRoot == NULL){
        WndRoot = (struct _wnd_tree_t*)malloc(sizeof(struct _wnd_tree_t));
        if(WndRoot == NULL){
            ERROR_MESSAGE("WndRoot alloc failed!");
            exit(-1);/*if WndRoot Init failed!, can't continue anymore*/
        }
        memset(WndRoot,0,sizeof(struct _wnd_tree_t));

    }
    if(parent == NULL)
        parent = WndRoot;
    /*malloc and init child window*/
    struct _wnd_tree_t *pChildWnd = (struct _wnd_tree_t *)malloc(sizeof(struct _wnd_tree_t));
    if(pChildWnd == NULL){
        WIN_DEBUG("ChildWnd alloc failed");
        return NULL;/*alloc failed,just return NULL*/
    }
    memset(pChildWnd,0,sizeof(struct _wnd_tree_t));
   
    pChildWnd->lpClassName = lpClassName;
    pChildWnd->lpWindowName = lpWindowName;
    pChildWnd->dwStyle = dwStyle;
    if(iLayOutMode == LAYOUT_AUTO){
#warning "unfinished"
        //add code here to process 
    }
    else if(iLayOutMode == LAYOUT_MANUL){
        pChildWnd->x = x;
        pChildWnd->y = y;
        pChildWnd->nWidth = nWidth;
        pChildWnd->nHeight = nHeight;
    }
    else{
        ERROR_MESSAGE("Unregnize Layout Option!");
        assert(false);
    }
   
    /*malloc and copy child list,release old memory*/
    /*it should be replace by realloc£¬that will be much easier*/
    struct _wnd_tree_t **pChildList= (struct _wnd_tree_t **)malloc((parent->wChildCnt+1) * sizeof(struct _wnd_tree_t*));
    if(pChildList == NULL){
        ERROR_MESSAGE("tmp ChildList alloc failed!");
        free(pChildWnd);
        return NULL;
    }
    memcpy(pChildList,parent->pChildList,parent->wChildCnt*sizeof(long));
    pChildList[pChildCnt] = pChildWnd;
    free(parent->pChildList);
    parent->pChildList = pChildList;
    ++parent->pChildCnt;
    return pChildWnd;
}
