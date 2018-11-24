
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
struct _wnd_tree_t *WndRoot = NULL;/*WndRoot start with Desktop window, the main window is also it's child,only the main window child-chain will create before message loop*/
BOOL InitApplication(void);
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
        /*set default value,only that not zero*/
        WndRoot->hwnd = GetDesktopWindow();
        WndRoot->nWidth = GetSystemMetric(SM_CXSCREEN);
        WndRoot->nHeight = GetSystemMetric(SM_CYSCREEN);

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
#warning "bug here"
    if(iLayOutMode == LAYOUT_MANUAL){
        pChildWnd->Flags |= LAYOUT_MANUAL;
    }
    else if(iLayOutMode != LAYOUT_AUTO){
        ERROR_MESSAGE("Unregnize Layout Option!");
        assert(FALSE);
    }
    pChildWnd->x = x;
    pChildWnd->y = y;
    pChildWnd->nWidth = nWidth;
    pChildWnd->nHeight = nHeight;
   
    /*malloc and copy child list,release old memory*/
    /*it should be replace by realloc£¬that will be much easier*/
    struct _wnd_tree_t **pChildList= (struct _wnd_tree_t **)malloc((parent->wChildCnt+1) * sizeof(struct _wnd_tree_t*));
    if(pChildList == NULL){
        ERROR_MESSAGE("tmp ChildList alloc failed!");
        free(pChildWnd);
        return NULL;
    }
    memcpy(pChildList,parent->pChildList,parent->wChildCnt*sizeof(long));
    pChildList[parent->wChildCnt] = pChildWnd;
    free(parent->pChildList);
    parent->pChildList = pChildList;
    ++parent->wChildCnt;
    return pChildWnd;
}

BOOL AddMessageHandler(struct _wnd_tree_t *window,message_code_t message_code,message_handler_t message_handler)
{
    /*window should never be NULL*/
    assert(window);
    struct _message_node_t *message_node = (struct _message_node_t *)malloc(sizeof(struct _message_node_t));
    if(message_node == NULL){
        WIN_DEBUG("message_node alloc failed!");
        return FALSE;
    }
    message_node->message_code = message_code;
    message_node->message_handler = message_handler;
    /*realloc message_node pointer arrary*/
    struct _message_node_t **ptmp = (struct _message_node_t **)realloc(window->pMessageNodeList,(window->wMessageNodeCnt + 1)*sizeof(struct _message_node_t *));
    if(ptmp == NULL){
        WIN_DEBUG("pMessageNodeList realloc failed!");
        return FALSE;
    }
    window->pMessageNodeList = ptmp;
    window->pMessageNodeList[window->wMessageNodeCnt++] = message_node;
    return TRUE;
}
/*generally,you should pass WndRoot to window*/
BOOL WndAutoLayout(struct _wnd_tree_t *window)
{
    /*this function should invoke after WndRoot init*/
	assert(WndRoot);
    if(WndRoot == NULL)
		return FALSE;
    if(window == NULL)
      return TRUE;
    /* layout this window*/
    if(WndRoot != window && (window->Flags & LAYOUT_MANUAL) == 0){
        //do the auto layout work
#warning "should complete in future"
        WIN_DEBUG("not support LAYOUT_AUTO yet!");
        exit(-1);
    }
    /* layout the child window recursively*/
    WORD i = 0;
    for(;i < window->wChildCnt; ++i){
        if(FALSE == WndAutoLayout(window->pChildList[i]))
          return FALSE;
    }
    return TRUE;

}

BOOL CreateWndTree(struct _wnd_tree_t *root,HWND parent)
{
    //assert(root && !strcmp(root->lpClassName,APP_TITLE));
    if(root == NULL)
      return TRUE;
    root->hwnd = CreateWindowEx(0,root->lpClassName,root->lpWindowName,root->dwStyle,\
                root->x,root->y,root->nWidth,root->nHeight,parent,NULL,hInst,NULL);
    WORD i;
#ifndef NDEBUG
    /*this just for WndTree check*/
    if(strcmp(root->lpClassName,APP_TITLE)){
        for(i = 0; i < root->wChildCnt; ++i)
        {
            if(strcmp(root->pChildList[i]->lpClassName,APP_TITLE)){
                WIN_DEBUG("parent-child window must have one \""APP_TITLE"\" window!Check your WinTree!");
                exit(-1);
            }
        }
    }
#endif
    for(i = 0; i < root->wChildCnt; ++i){
        if(FALSE == CreateWndTree(root->pChildList[i],root->hwnd))
          return FALSE;
    }
    return TRUE;
}
#warning "winmain just for test now"
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	
    MSG          msg;
    BOOL         bRet;
    /* Init Application*/
	if (InitApplication() == FALSE)
    {
        MessageBox(NULL, TEXT ("InitApplication failed!"), APP_TITLE, MB_ICONERROR);        
        return 0;
    }
	
	/*auto layout*/
	if (WndAutoLayout(WndRoot) == FALSE)
    {
        MessageBox(NULL, TEXT ("WndAutoLayout failed!"), APP_TITLE, MB_ICONERROR);        
        return 0;
    }
	/*Create WinMain Tree*/
	if(CreateWndTree(WndRoot->pChildList[0],NULL) == FALSE)
	{
        MessageBox(NULL, TEXT ("CreateWndTree failed!"), APP_TITLE, MB_ICONERROR);        
        return 0;
    }
    hInst = hInstance;
	hwndMain = WndRoot->pChildList[0]->hwnd;/*assume the first child of WndRoot is the Main Window*/
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
    { 
        if (bRet == -1)
        {
            /* handle the error and possibly exit */                   
            MessageBox(NULL, TEXT ("GetMessage error"), APP_TITLE, MB_ICONERROR) ;
            return -1;
        }
        else
        {
            TranslateMessage(&msg); 
            DispatchMessage(&msg); 
        }
    }   
    return msg.wParam;
}
