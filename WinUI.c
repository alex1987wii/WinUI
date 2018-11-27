
#include "WinUI.h"
#define DEV_TOOLS_WIDTH         700         /* Unication dev tools main window width */
#define DEV_TOOLS_HEIGHT        550         /* Unication dev tools main window height */
#include <stdio.h>

#define WAKE_THREAD_UP() do { SetEvent(g_event); } while(0)
/* for DataTransferThread */
HWND g_hwnd = NULL;
UINT g_message = 0;
WPARAM g_wParam = 0;
LPARAM g_lParam = 0;
message_handler_t g_handler = NULL;

HWND hwndMain;
HINSTANCE hInst;
static HANDLE g_event = NULL;    // event
static HANDLE hMutex;  
struct _wnd_tree_t *pWndRoot = NULL;/*pWndRoot start with Desktop window, the main window is also it's child,only the main window child-chain will create before message loop*/
struct _wnd_tree_t *pWndMain = NULL;
BOOL InitApplication(void);

/******************interface for WinTree*************************/

static inline BOOL is_message_code_exsit(struct _wnd_tree_t *window,message_code_t message_code)
{
	/*forbid to pass NULL for window*/ 
	assert(window);
	WORD i;
	for(i = 0; i < window->wMessageNodeCnt; ++i)
	{
		/*if message_handler is NULL,we also return FALSE,let it override*/
		if(message_code == window->pMessageNodeList[i].message_code && window->pMessageNodeList[i].message_handler != NULL)
			return TRUE;
	}
	return FALSE;
}
struct _wnd_tree_t *LinkWndTree(struct _wnd_tree_t *parent,struct _wnd_tree_t *child)
{
	if(child == NULL)
		return NULL;
#if (!defined(NDEBUG) || defined(DYNAMIC_CHECK))
	if(child->parent){
		WIN_DEBUG("ChildWnd:0x%p can't be mutiple parent's child!",child);
		return NULL;
	}
#endif
	if(parent)
	{
		struct _wnd_tree_t **ptmp = (struct _wnd_tree_t **)realloc(parent->pChildList,(parent->wChildCnt+1)*sizeof(struct _wnd_tree_t *));
		if(ptmp == NULL)
		{
			WIN_DEBUG("realloc for ptmp failed!");
			return NULL;
		}
		
		parent->pChildList = ptmp;
		parent->pChildList[parent->wChildCnt++] = child;
	}
	child->parent = parent;
	return child;
}
struct _wnd_tree_t *CopyWndTree(struct _wnd_tree_t *root)
{
	if(root == NULL)
		return NULL;	
	WORD i;
	/*malloc for copy*/
	struct _wnd_tree_t *copy = (struct _wnd_tree_t *)malloc(sizeof(struct _wnd_tree_t));
	if(copy == NULL){
		WIN_DEBUG("malloc for copy failed!");
		return NULL;
	}
	memcpy(copy,root,sizeof(struct _wnd_tree_t));
	if(copy->wMessageNodeCnt){
		/*malloc for pMessageNodeList*/
		copy->pMessageNodeList =(struct _message_node_t*)malloc(sizeof(struct _message_node_t)*copy->wMessageNodeCnt);
		if(copy->pMessageNodeList == NULL)
		{
			WIN_DEBUG("malloc for pMessageNodeList failed!");
			free(copy);
			return NULL;
		}
		memcpy(copy->pMessageNodeList,root->pMessageNodeList,sizeof(struct _message_node_t)*copy->wMessageNodeCnt);
	}
	if(copy->wChildCnt){
		/*malloc for pChildList*/
		copy->pChildList =(struct _wnd_tree_t**)malloc(sizeof(struct _wnd_tree_t *)*copy->wChildCnt);
		if(copy->pChildList == NULL)
		{
			WIN_DEBUG("malloc for pChildList failed!");
			if(copy->wMessageNodeCnt)
				free(copy->pMessageNodeList);
			free(copy);
			return NULL;
		}
		for(i = 0 ; i < copy->wChildCnt; ++i)
		{
			copy->pChildList[i] = CopyWndTree(root->pChildList[i]);
			if(copy->pChildList[i] == NULL){				
				/*DestroyWndTree will use wChildCnt to release resource,must correct it first*/
				copy->wChildCnt = i;
				DestroyWndTree(copy);
				return NULL;
			}
			/*need reset child's parent*/
			copy->pChildList[i]->parent = copy;
		}
	}
	return copy;	
}
struct _wnd_tree_t *AddWndTree(struct _wnd_tree_t *parent,struct _wnd_tree_t *child)
{
	if(child == NULL)
		return NULL;
#if (!defined(NDEBUG) || defined(DYNAMIC_CHECK))
	if(child->parent){
		WIN_DEBUG("child:0x%p can't have mutiple parent!",child);
		return NULL;
	}
#endif
	struct _wnd_tree_t *ptmp = CopyWndTree(child);
	if(ptmp == NULL){
		WIN_DEBUG("CopyWndTree Failed!");
		return NULL;
	}
	if(LinkWndTree(parent,ptmp) == NULL){
		WIN_DEBUG("LinkWndTree Failed!");
		DestroyWndTree(ptmp);
		return NULL;
	}
	return ptmp;	
}

void DestroyWndTree(struct _wnd_tree_t *root)
{
	if(root == NULL)
		return ;
	WORD i;	
	for(i = 0; i < root->wChildCnt; ++i)
	{
		DestroyWndTree(root->pChildList[i]);
	}	
	free(root->pChildList);
	free(root->pMessageNodeList);
	free(root);
} 

BOOL AddMessageHandler(struct _wnd_tree_t *window,message_code_t message_code,message_handler_t message_handler)
{
	/*check if this window already have the handler for message_code*/
	/*this can be disabled when release and you ensure not invoke AddMessageHandler after application init complete*/
#if (!defined(NDEBUG) || defined(DYNAMIC_CHECK))
	if(window == NULL){
		WIN_DEBUG("window can't be NULL!",message_code);
		return FALSE;
	}
	if(is_message_code_exsit(window,message_code)){
		WIN_DEBUG("message code:%u already exsit!",message_code);
		return FALSE;
	}
#endif
	/*realloc message_node pointer arrary*/
    struct _message_node_t *ptmp = (struct _message_node_t *)realloc(window->pMessageNodeList,(window->wMessageNodeCnt + 1)*sizeof(struct _message_node_t));
    if(ptmp == NULL){
        WIN_DEBUG("pMessageNodeList realloc failed!");
        return FALSE;
    }
    window->pMessageNodeList = ptmp;
    window->pMessageNodeList[window->wMessageNodeCnt].message_code = message_code;
	window->pMessageNodeList[window->wMessageNodeCnt].message_handler = message_handler;
	++window->wMessageNodeCnt;
    return TRUE;
}


static BOOL CreateWndTree(struct _wnd_tree_t *root,HWND parent)
{
    //assert(root && !strcmp(root->lpClassName,APP_TITLE));
    if(root == NULL)
      return TRUE;	
    root->hwnd = CreateWindowEx(root->dwExStyle,root->lpClassName,root->lpWindowName,root->dwStyle,
			root->x,root->y,root->nWidth,root->nHeight,parent,NULL,hInst,NULL);
    WORD i;
#ifndef NDEBUG
    if(root->lpClassName == NULL || root->lpWindowName == NULL)
    {
	    WIN_DEBUG("lpClassName and lpWindowName Can't be NULL!");
	    exit(-1);
    }
#endif

#if (!defined(NDEBUG) || defined(DYNAMIC_CHECK))
    /*this just for WndTree check*/
    if(strcmp(root->lpClassName,APP_TITLE)){
        for(i = 0; i < root->wChildCnt; ++i)
        {
            if(strcmp(root->pChildList[i]->lpClassName,APP_TITLE)){
                WIN_DEBUG("parent-child window must have one \""APP_TITLE"\" window!Check your WndTree!");
                return FALSE;
            }
        }
    }
#endif
	if(root->hwnd == NULL){
		WIN_DEBUG("HWND:0x%x CreateWindowEx Failed!");
		return FALSE;
	}
    for(i = 0; i < root->wChildCnt; ++i){
        if(FALSE == CreateWndTree(root->pChildList[i],root->hwnd))
          return FALSE;
    }
    return TRUE;
}


static struct _wnd_tree_t *GetWnd(struct _wnd_tree_t *window,HWND hwnd)
{
	/*we had make some check before invoke*/
	if(window->hwnd == hwnd)
		return window;
	struct _wnd_tree_t *child;
	WORD i;
	for(i = 0; i < window->wChildCnt; ++i)
	{		
		child = GetWnd(window->pChildList[i],hwnd);
		if(child)
			return child;
	}
	return NULL;
}

LRESULT CALLBACK MainWndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	message_handler_t handler;
	struct _wnd_tree_t *window;	
	window = GetWnd(pWndRoot->pChildList[0],hwnd);/*assume the first child of pWndRoot is the Main Window*/

	/*When WM_CREATE(some init message) comming,window will be NULL*/
	if(window)
	{
		WORD i;
		for(i = 0; i < window->wMessageNodeCnt; ++i)
		{
			if(window->pMessageNodeList[i].message_code == message)
			{
				handler = window->pMessageNodeList[i].message_handler;
				if(handler)
					return (handler)(hwnd,message,wParam,lParam);
				else/*message_handler is NULL for placehold*/
					return 0;				
			}
		}
	}
	return DefWindowProc(hwnd,message,wParam,lParam);
}

/******************interface for DevTools*************************/
LRESULT CALLBACK OnCommand(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{	
	HWND child = (HWND)lParam;
	if(child){
		WORD i,j;
		struct _wnd_tree_t *pWnd = GetWnd(pWndMain,hwnd);
		assert(pWnd);
		for(i = 0; i < pWnd->wChildCnt;++i)
		{
			if(child == pWnd->pChildList[i]->hwnd)
			{
				pWnd = pWnd->pChildList[i];
				message_code_t msg_code = LOWORD(wParam);
				for(j = 0; j < pWnd->wMessageNodeCnt; ++j)
				{
					if(msg_code == pWnd->pMessageNodeList[j].message_code)
					{
						message_handler_t handler = pWnd->pMessageNodeList[j].message_handler;
						if(handler)
						{
							g_hwnd = hwnd;
							g_message = message;
							g_wParam = wParam;
							g_lParam = lParam;
							g_handler = handler;
							WAKE_THREAD_UP();						
							
						}
					}
					break;
				}
				break;
			}
		}
	}
	return 0;
}
LRESULT CALLBACK OnNotify(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{	
	return 0;
}
LRESULT CALLBACK OnDestory(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	
	WIN_DEBUG("hwnd = %u\tmessage = %u\t wParam = 0x%8x\t lParam = 0x%16x",
	hwnd,message,wParam,lParam);
	PostQuitMessage(0);
	return 0;
}
BOOL InitpWndRoot(void)
{	
	struct _wnd_tree_t *p_ret = NULL;
	DECLARE_WND_TREE(root);
	p_ret = AddWndTree(NULL,&root);
	if(p_ret == NULL)
		goto error_pWndRoot;
	pWndRoot = p_ret;
	DECLARE_WND_TREE(mainWnd);
	mainWnd.dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPSIBLINGS;
	mainWnd.x = (GetSystemMetrics(SM_CXSCREEN)-DEV_TOOLS_WIDTH)/2;
	mainWnd.y = (GetSystemMetrics(SM_CYSCREEN)-DEV_TOOLS_HEIGHT)/2;
	mainWnd.lpClassName = APP_TITLE;
	mainWnd.lpWindowName = APP_TITLE;
	mainWnd.nHeight = DEV_TOOLS_HEIGHT;
	mainWnd.nWidth = DEV_TOOLS_WIDTH;
	
	p_ret = AddWndTree(pWndRoot,&mainWnd);
	if(p_ret == NULL)
		goto error_pWndMain;
	pWndMain = p_ret;
	/*add some default message handler,user alse can add message handler for pWndMain in InitApplication*/
	if(AddMessageHandler(pWndMain,WM_COMMAND,OnCommand) == FALSE)
		goto error_pWndMain;
	if(AddMessageHandler(pWndMain,WM_NOTIFY,OnNotify) == FALSE)
		goto error_pWndMain;
	if(AddMessageHandler(pWndMain,WM_DESTROY,OnDestory) == FALSE)
		goto error_pWndMain;
	return TRUE;

error_pWndMain:
	DestroyWndTree(pWndRoot);
error_pWndRoot:
	return FALSE;
}
static BOOL InitMainWindow(void)
{
	WNDCLASSEX     wndclass;
    
    /* only one instance can run on the system */
    hMutex = CreateMutex(NULL, FALSE, APP_TITLE);
    if(hMutex == NULL)
    {
        WIN_DEBUG("Create mutex error!\n");
        return FALSE;
    }
        
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        CloseHandle(hMutex);
        WIN_DEBUG(APP_TITLE" is running\n");
        return FALSE;
    }
    
    /* Register the Unication UniDevTools main window class */
    	
	memset(&wndclass, 0, sizeof(WNDCLASSEX));
    wndclass.cbSize        = sizeof(WNDCLASSEX);
    wndclass.style         = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc   = MainWndProc;
    wndclass.hInstance     = hInst;
    wndclass.hIcon         = LoadIcon(NULL,IDI_APPLICATION);
    wndclass.hCursor       = LoadCursor(NULL, IDC_HAND);
    wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
    wndclass.lpszMenuName  = NULL;
    wndclass.lpszClassName = APP_TITLE;
    
    if (!RegisterClassEx(&wndclass))
    {        
        return FALSE;
    }
	if(CreateWndTree(pWndMain,NULL) == FALSE)
	{
		CloseHandle(hMutex);
		WIN_DEBUG("CreateWndTree Failed!");
		return FALSE;
	}
	hwndMain = pWndMain->hwnd;
	return TRUE;
}
#warning "test section"
BOOL InitApplication(void)
{
	DECLARE_WND_TREE(button);
	button.lpClassName = "button";
	button.lpWindowName = "click";
	button.dwStyle = BS_PUSHBUTTON | WS_CHILD| WS_VISIBLE;
	button.x = 0;
	button.y = 0;
	button.nWidth = 60;
	button.nHeight = 40;
	struct _wnd_tree_t *pButton = AddWndTree(pWndMain,&button);
	if(pButton == NULL)
		return FALSE;
	button.x = 80;
	button.y = 0;
	button.lpWindowName = "click2";
	pButton = AddWndTree(pWndMain,&button);
	if(pButton == NULL)
		return FALSE;
	AddMessageHandler(pButton,BN_CLICKED,OnNotify);
	return TRUE;
}
static DWORD WINAPI DataTransferThread(LPVOID lpParam)
{
    int retval = -1;

    while (1)
    {
        WaitForSingleObject(g_event, INFINITE);
        ResetEvent(g_event);
		EnableWindow(hwndMain,FALSE);		
		g_handler(g_hwnd,g_message,g_wParam,g_lParam);
		EnableWindow(hwndMain,TRUE);		
    }

    return 0;
}
static BOOL StartThread(void) 
{    
    if (CreateThread(NULL, 0, DataTransferThread, NULL, 0, NULL) == NULL) 
    {
        WIN_DEBUG("create DataTransferThread error\n");        
        return FALSE;
    }
    return TRUE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	
    MSG          msg;
    BOOL         bRet;
	hInst = hInstance;
	/* Init pWndRoot*/
	if(InitpWndRoot() == FALSE)
	{
		MessageBox(NULL, TEXT ("InitpWndRoot error"), APP_TITLE, MB_ICONERROR);
		return 0;
	}	
    
    g_event = CreateEvent(NULL, TRUE, FALSE, NULL); // ManualReset

    if (StartThread() == FALSE)
    {
        MessageBox(NULL, TEXT ("Create thread error"), APP_TITLE, MB_ICONERROR);
        DestroyWndTree(pWndRoot);
        return 0;
    }	
    /* Application init */
    if (InitApplication() == FALSE)
    {
        MessageBox(NULL, TEXT (APP_TITLE" is running"), APP_TITLE, MB_ICONERROR);
        DestroyWndTree(pWndRoot);
        return 0;
    }

	/*Check pWndRoot*/
	#warning "need check pWndRoot here"
#ifndef NDEBUG
	if(pWndRoot == NULL || pWndRoot->pChildList == NULL || pWndRoot->pChildList[0] == NULL || pWndMain == NULL){
		WIN_DEBUG("pWndRoot and Main Window is not add in!");
		exit(-1);
	}
#endif  	
	
    /* Unication Dev Tools main window init */
    if (InitMainWindow() == FALSE)
    {
        MessageBox(NULL, TEXT ("Can not create main window"), APP_TITLE, MB_ICONERROR);
        CloseHandle(hMutex);        
		DestroyWndTree(pWndRoot);
        return 0;
    }
	
    //reset_private_flags();

    /* Show the window and send a WM_PAINT message to the window procedure */
    ShowWindow(hwndMain, iCmdShow);
    UpdateWindow(hwndMain);
	

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
	
	CloseHandle(g_event);

    /* Receive the WM_QUIT message, release mutex and return the exit code to the system */
    CloseHandle(hMutex);   

	DestroyWndTree(pWndRoot);
	return msg.wParam;
}

