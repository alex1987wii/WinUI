#include "DevTools.h"
struct _wnd_tree_t *InitNandProgramWindow(void);
static BOOL GetFileName(char *fileName, int size)
{
    OPENFILENAME ofn;
    
    /* Initialize OPENFILENAME */
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = fileName;
    
    /* Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
         use the contents of szFile to initialize itself */
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = size;
    ofn.Flags = OFN_PATHMUSTEXIST;
    
    return GetOpenFileName(&ofn);
}
#warning "testing"
static LRESULT CALLBACK ButtonBrowseHandler(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	TCHAR szFile[MAX_PATH];
	 if(GetFileName(szFile, MAX_PATH) == TRUE)
	 {
		WIN_DEBUG("get button_browse click!");
	 }
	 return 0;
}
BOOL InitApplication(void)
{
	/* Initialize tab controls */
	/* INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TAB_CLASSES;
    InitCommonControlsEx(&icex); */
	
	INIT_WND_TREE(tab,NULL,NULL,0,WC_TABCONTROL,TEXT(""),WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,0, 0, 0,0,0,0,NULL,NULL);
	
	
	struct _wnd_tree_t *pTab = AddWndTree(pWndMain,&tab);
	if(pTab == NULL)
		return FALSE;
	if(AddMessageHandler(pTab,TCN_SELCHANGE,OnSelChange) == FALSE)
		return FALSE;
	struct _wnd_tree_t *pNandProgramPage = InitNandProgramWindow();
	if(pNandProgramPage == NULL)
		return FALSE;
	if(AddWndTree(pTab,pNandProgramPage) == NULL)
		return FALSE;	
	return TRUE;
}
struct _wnd_tree_t *InitNandProgramWindow(void)
{
	int relative_x = 0;
    int relative_y = 0;
#ifdef DEVELOPMENT
	INIT_WND_TREE(NandProgramPage ,NULL,NULL,0, szAppName, TEXT("Nand Flash programming"),
                         WS_VISIBLE | WS_CHILD,
                         2*TAB_CLIENT_START_X, TAB_CLIENT_START_Y,
                         TAB_CLIENT_WIDTH, TAB_CLIENT_HEIGHT,0,0,NULL,NULL);
#else
	INIT_WND_TREE(NandProgramPage ,NULL,NULL,0, szAppName, TEXT("Nand Flash programming"),
                         WS_VISIBLE | WS_CHILD,
                         2*TAB_CLIENT_START_X, TAB_CLIENT_START_Y,
                         TAB_CLIENT_WIDTH, TAB_CLIENT_HEIGHT-14*Y_MARGIN,0,0,NULL,NULL);
#endif	
	INIT_WND_TREE(NandFlashProgram,NULL,NULL,0,TEXT("button"), "Nand Flash programming",
                         WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                         30, 0,
                         TAB_NANDFLASH_WIDTH, TAB_NANDFLASH_HEIGHT,
                         0,0, NULL,
                          NULL);

    INIT_WND_TREE(NpSrcFile,NULL,NULL,0,TEXT("static"), TEXT("Nand Img:"),
                         WS_CHILD | WS_VISIBLE  | SS_LEFT,
                         TAB_NANDFLASH_START_X, TAB_CLIENT_START_Y+WIN_STATIC_OFFSET,
                         WIDTH_TEXT, HEIGHT_CONTROL,
                         0,0,
                         NULL,NULL);

    relative_x = TAB_NANDFLASH_START_X;
	relative_y = TAB_CLIENT_START_Y;

    INIT_WND_TREE(NpEditSrcFile,NULL,NULL,WS_EX_CLIENTEDGE, TEXT("edit"), TEXT(""),
                         WS_CHILD | WS_VISIBLE  | ES_LEFT | ES_AUTOHSCROLL | ES_READONLY,
                         relative_x + WIDTH_TEXT + X_MARGIN, relative_y,
                         WIDTH_EDIT_NAND, HEIGHT_CONTROL,
                        0,0,
                         NULL,NULL);
    relative_x += WIDTH_TEXT + X_MARGIN + WIDTH_EDIT_NAND;

    INIT_WND_TREE(NpButtonBrowse,NULL,NULL,0,TEXT("button"), TEXT("Browse..."),
                         WS_CHILD | WS_VISIBLE  | BS_PUSHBUTTON,
                         relative_x + X_MARGIN, relative_y,
                         WIDTH_BUTTON_NANDFLASH_PROGRAM, HEIGHT_CONTROL,
                        0,0,
                         NULL,NULL);
    relative_y += HEIGHT_CONTROL;

#ifdef DEVELOPMENT    
    INIT_WND_TREE (NpBatchProgram,NULL,NULL,0,TEXT("button"), TEXT("SPL only ?"),
                         WS_CHILD /*| WS_VISIBLE */  | BS_AUTOCHECKBOX,
                         TAB_NANDFLASH_START_X, relative_y + Y_MARGIN*2,
                         WIDTH_CHECKBOX, HEIGHT_CONTROL,
                         0,0,
                         NULL,NULL);
#else    
    INIT_WND_TREE(NpBatchProgram,NULL,NULL,0,TEXT("button"), TEXT("Batch programming"),
                         WS_CHILD | WS_VISIBLE  | BS_AUTOCHECKBOX,
                         TAB_NANDFLASH_START_X, relative_y + Y_MARGIN*2,
                         WIDTH_CHECKBOX, HEIGHT_CONTROL,
                         0,0,
                         NULL,NULL);
#endif

    relative_y += Y_MARGIN*2;

    INIT_WND_TREE(NpButtonProgram,NULL,NULL,0,TEXT("button"), TEXT("Programming"),
                         WS_CHILD | WS_VISIBLE  | BS_PUSHBUTTON,
                         TAB_NANDFLASH_START_X + WIDTH_TEXT + WIDTH_EDIT_NAND + X_MARGIN*2, 
                         relative_y,
                         WIDTH_BUTTON_NANDFLASH_PROGRAM, HEIGHT_CONTROL,
                         0,0,
                         NULL,NULL);
    
    INIT_WND_TREE(NandNotify,NULL,NULL,0,TEXT("static"), TEXT(""),
                         WS_CHILD | WS_VISIBLE  /*| SS_CENTER*/,
                         TAB_NANDFLASH_START_X + WIDTH_TEXT + X_MARGIN,
                         TAB_NANDFLASH_HEIGHT - Y_MARGIN*4,
                         WIDTH_EDIT_NAND, HEIGHT_CONTROL,
                         0,0,
                         NULL,NULL);
	
	/*init WndTree*/
	struct _wnd_tree_t *pProgramPage = AddWndTree(NULL,&NandProgramPage);
	if(pProgramPage == NULL)
		return NULL;
	if(AddWndTree(pProgramPage,&NandFlashProgram) == NULL)
		return NULL;
	if(AddWndTree(pProgramPage,&NpSrcFile) == NULL)
		return NULL;
	if(AddWndTree(pProgramPage,&NpEditSrcFile) == NULL)
		return NULL;
	if(AddMessageHandler(AddWndTree(pProgramPage,&NpButtonBrowse),BN_CLICKED,ButtonBrowseHandler) == FALSE)
		return NULL;
	if(AddWndTree(pProgramPage,&NpBatchProgram) == NULL)
		return NULL;
	if(AddWndTree(pProgramPage,&NpButtonProgram) == NULL)
		return NULL;
	if(AddWndTree(pProgramPage,&NandNotify) == NULL)
		return NULL;
	if(AddWndTree(pWndRoot,pProgramPage) == NULL)
		return NULL;
	return pProgramPage;
}


