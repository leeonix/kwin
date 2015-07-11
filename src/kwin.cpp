
#include <windows.h>
#include <shellapi.h>
#include <stdlib.h>
#include "resource.h"

///////////////////////////////////////////////////////////////////////

//define tray menu ID
#define IDM_ACTIVE 3000
#define IDM_EXIT 3010
#define WM_TRAY WM_USER+5

/////////////////////////////////////////////////////////////////////

//global var
HINSTANCE hInst;//hInstance

//old wnd broc
WNDPROC oldAimProc;
WNDPROC oldTargetListProc;
WNDPROC oldChildListProc;

//window handle
HWND hMainWnd; //our main wnd
HWND hTargetList;//listbox that shows all top-level windows
HWND hChildList;//listbox that shows all child windows of target
HWND hTarget;  //target wnd

char IniLocation[1024];//ini file name buffer

RECT TargetRect;//target rect
NOTIFYICONDATA trayicon;//tray notify data
HMENU hTrayMenu;//tray menu
BOOL IsWndowTopmost;//tag that if target is topmost
BOOL ShowHiddenOnly;//tag that whether show hidden window only in listbox
POINT MousePos;//mouse position,for capture target from cursur


//global var

//////////////////////////////////////////////////////////////////////

//function declear
//Dialog Proc
LRESULT CALLBACK MainDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK AboutProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK OptionProc(HWND, UINT, WPARAM, LPARAM);

//subclassed window proc
LRESULT CALLBACK AimProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK TargetListProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ChildListProc(HWND, UINT, WPARAM, LPARAM);
//enum windows proc
BOOL CALLBACK EnumChildWindowsProc(HWND, LPARAM);
BOOL CALLBACK EnumWindowsProc(HWND, LPARAM);
BOOL CALLBACK EnumTargetProc(HWND, LPARAM);
//function declear

///////////////////////////////////////////////////////////////////////

//user functions

//--------------------------------------------------------------//

// ini operation  //
//-- create --//
void SaveConfig()
{
    RECT wndrect;
    char buffer[6];

    GetWindowRect(hMainWnd, &wndrect);


    //mainwnd left
    wsprintf(buffer, "%d", wndrect.left);
    WritePrivateProfileString("wndpos", "l", buffer, IniLocation);



    //mainwnd top
    wsprintf(buffer, "%d", wndrect.top);
    WritePrivateProfileString("wndpos", "t", buffer, IniLocation);





    //list box
    GetWindowRect(hTargetList, &wndrect);



    //target list left
    wsprintf(buffer, "%d", wndrect.left);
    WritePrivateProfileString("wndpos", "List1l", buffer, IniLocation);

    //target list top
    wsprintf(buffer, "%d", wndrect.top);
    WritePrivateProfileString("wndpos", "List1t", buffer, IniLocation);

    //target list width
    wsprintf(buffer, "%d", wndrect.right - wndrect.left);
    WritePrivateProfileString("wndpos", "List1w", buffer, IniLocation);

    //target list height
    wsprintf(buffer, "%d", wndrect.bottom - wndrect.top);
    WritePrivateProfileString("wndpos", "List1h", buffer, IniLocation);



    GetWindowRect(hChildList, &wndrect);

    //child list left
    wsprintf(buffer, "%d", wndrect.left);
    WritePrivateProfileString("wndpos", "List2l", buffer, IniLocation);

    //child list top
    wsprintf(buffer, "%d", wndrect.top);
    WritePrivateProfileString("wndpos", "List2t", buffer, IniLocation);

    //child list width
    wsprintf(buffer, "%d", wndrect.right - wndrect.left);
    WritePrivateProfileString("wndpos", "List2w", buffer, IniLocation);

    //child list height
    wsprintf(buffer, "%d", wndrect.bottom - wndrect.top);
    WritePrivateProfileString("wndpos", "List2h", buffer, IniLocation);





    if (GetWindowLong(hMainWnd, GWL_EXSTYLE) & WS_EX_TOPMOST) {
        //on top tag
        WritePrivateProfileString("other", "OnTop", "1", IniLocation);
    } else {
        WritePrivateProfileString("other", "OnTop", "0", IniLocation);
    }

    if (SendMessage(GetDlgItem(hMainWnd, IDC_SHOWHIDDENONLY),
                    BM_GETCHECK,
                    0,
                    0) == BST_CHECKED) {
        //show hidden only list tag
        WritePrivateProfileString("other", "ShowHidden", "1", IniLocation);
    } else {
        WritePrivateProfileString("other", "ShowHidden", "0", IniLocation);
    }
}


//-- read --//
void ReadConfig()
{
    unsigned int ontop = GetPrivateProfileInt("other", "OnTop", 0, IniLocation);
    HWND after = ontop ? HWND_TOPMOST : HWND_TOP;
    
    //restore window position
    SetWindowPos(hMainWnd, after,
                 //mainwnd left
                 GetPrivateProfileInt("wndpos", "l", 100, IniLocation),
                 //mainwnd top
                 GetPrivateProfileInt("wndpos", "t", 100, IniLocation),
                 0, 0, SWP_NOSIZE);

    //restore window position
//    MoveWindow(hMainWnd,
//               //mainwnd left
//               GetPrivateProfileInt("wndpos", "l", 100, IniLocation),
//               //mainwnd top
//               GetPrivateProfileInt("wndpos", "t", 100, IniLocation),
//               344,
//               241,
//               TRUE);

    //restore target list position
    MoveWindow(hTargetList,
               //target list left
               GetPrivateProfileInt("wndpos", "List1l", 457, IniLocation),
               //target list top
               GetPrivateProfileInt("wndpos", "List1t", 100, IniLocation),
               //target list width
               GetPrivateProfileInt("wndpos", "List1w", 340, IniLocation),
               //target list height
               GetPrivateProfileInt("wndpos", "List1h", 250, IniLocation),
               TRUE);

    //restore child list position
    MoveWindow(hChildList,
               //child list left
               GetPrivateProfileInt("wndpos", "List2l", 100, IniLocation),
               //child list top
               GetPrivateProfileInt("wndpos", "List2t", 350, IniLocation),
               //target list width
               GetPrivateProfileInt("wndpos", "List2w", 357, IniLocation),
               //target list height
               GetPrivateProfileInt("wndpos", "List2h", 200, IniLocation),
               TRUE);

    //on top tag
//    if (GetPrivateProfileInt("other", "OnTop", 0, IniLocation)) {
//        //set window position
//        SetWindowPos(hMainWnd,    // handle of window
//                     HWND_TOPMOST,   // placement-order handle
//                     0,  // horizontal position
//                     0,  // vertical position
//                     0,  // width
//                     0,  // height
//                     SWP_NOMOVE |
//                     SWP_NOSIZE |
//                     SWP_SHOWWINDOW    // window-positioning flags
//                    );
//    }


    //show target list tag
    if (GetPrivateProfileInt("other", "ShowList1", 0, IniLocation)) {
        //clear list box
        SendMessage(hTargetList, LB_RESETCONTENT, 0, 0);

        //show all window
        EnumWindows(EnumWindowsProc, 0);
        ShowWindow(hTargetList, SW_SHOW);
    }

    //show child list tag
    if (GetPrivateProfileInt("other", "ShowList2", 0, IniLocation)) {
        ShowWindow(hChildList, SW_SHOW);
    }

    //show hidden only list tag
    if (GetPrivateProfileInt("other", "ShowHidden", 0, IniLocation)) {
        SendMessage(GetDlgItem(hMainWnd, IDC_SHOWHIDDENONLY),
                    BM_SETCHECK,
                    BST_CHECKED,
                    0);
        ShowHiddenOnly = 1;
    } else {
        SendMessage(GetDlgItem(hMainWnd, IDC_SHOWHIDDENONLY),
                    BM_SETCHECK,
                    BST_UNCHECKED,
                    0);
        ShowHiddenOnly = 0;
    }
}


//------------------------------------------------------------//


//------enum windows proc-----//
//---------------//
BOOL CALLBACK EnumTargetProc(HWND Target, LPARAM lParam)
//enum targets'child window (for capture disabled window)
{
    RECT targetRect;

    GetWindowRect(Target, &targetRect);

    if (MousePos.x > targetRect.left &&
        MousePos.x < targetRect.right &&
        MousePos.y > targetRect.top &&
        MousePos.y < targetRect.bottom) {
        hTarget = Target;
    }


    return (BOOL) Target;
}

//---------------//

BOOL CALLBACK EnumWindowsProc(HWND TargetWnd, LPARAM lParam)
//enum all window
{
    char title[256];
    char classname[256];
    char buffer[530];
    BOOL visible;
    visible = !IsWindowVisible(TargetWnd);

    switch (visible) {
    //if window is visible
    case 0:
        //if "ShowHiddenOnly" is checker,do not display
        if (ShowHiddenOnly) {
            break;
        }

    default:
        //print hwnd
        wsprintf(buffer, "%010i-(", TargetWnd);

        //get window text
        SendMessage(TargetWnd, WM_GETTEXT, (WPARAM) 255, (LPARAM) title);
        //if target's title is empty,replace with"(无标题)"
        if (!strlen(title)) {
            wsprintf(title, "%s", "(无标题)");
        }

        //get window class
        GetClassName(TargetWnd,   // handle of window
                     classname,//address of buffer for class name
                     255     // size of buffer, in characters
                    );

        //join handle & classname
        lstrcat(buffer, classname);
        lstrcat(buffer, ")-> ");

        //join title
        lstrcat(buffer, title);

        //add string to listbox
        SendMessage(hTargetList, LB_ADDSTRING, 0, (LPARAM) buffer);
    }


    return (BOOL) TargetWnd;
}
//--------------------//
BOOL CALLBACK EnumChildWindowsProc(HWND TargetWnd, LPARAM lParam)
//enum child window
{
    char title[256];
    char classname[256];
    char buffer[530];

    BOOL visible;
    visible = !IsWindowVisible(TargetWnd);

    switch (visible) {
    //if window is visible
    case 0:
        //if "ShowHiddenOnly" is checker,do not display
        if (ShowHiddenOnly) {
            break;
        }

    default:
        //print hwnd
        wsprintf(buffer, "%010i-(", TargetWnd);

        //get window text
        SendMessage(TargetWnd, WM_GETTEXT, (WPARAM) 255, (LPARAM) title);
        //if target's title is empty,replace with"(无标题)"
        if (!strlen(title)) {
            wsprintf(title, "%s", "(无标题)");
        }
        //get window class
        GetClassName(TargetWnd,   // handle of window
                     classname,//address of buffer for class name
                     255     // size of buffer, in characters
                    );

        //join handle & classname
        lstrcat(buffer, classname);
        lstrcat(buffer, ")-> ");

        //join title
        lstrcat(buffer, title);

        //add string to list box
        SendMessage(hChildList, LB_ADDSTRING, 0, (LPARAM) buffer);
    }

    return (BOOL) TargetWnd;
}

//----------------------------------------------------------//

void CaptureTarget()
//find target from cursor
{
    //get cursor position
    GetCursorPos(&MousePos);

    //capture target
    hTarget = WindowFromPoint(MousePos);

    EnumChildWindows(hTarget, EnumTargetProc, 0);
}
//----------------------------------------------------------------//
void ShowTargetProperty()
//Show Target's Property
{
    //local varible
    char buffer[256];//put target's title

    //show mouse pos
    wsprintf(buffer, "X:%i,Y:%i", MousePos.x, MousePos.y);
    SetDlgItemText(hMainWnd, IDE_MOUSEPOS, buffer);

    //show target hwnd
    wsprintf(buffer, "%08XH", hTarget);
    SetDlgItemText(hMainWnd, IDE_HWND, buffer);

    //show target ID
    SetDlgItemInt(hMainWnd,
                  IDE_ID,
                  (LONG) GetWindowLong(hTarget,       // handle of window
                                       GWL_ID  // offset of value to retrieve
                                      ),
                  FALSE);

    //get target  title
    SendMessage(hTarget, WM_GETTEXT, (WPARAM) 255, (LPARAM) buffer);
    SetDlgItemText(hMainWnd, IDE_TITLE, buffer);

    //get target classname
    GetClassName(hTarget,     // handle of window
                 buffer,//address of buffer for class name
                 255     // size of buffer, in characters
                );
    SetDlgItemText(hMainWnd, IDE_CLASS, buffer);

    //is target topmost ?
    if (GetWindowLong(hTarget, GWL_EXSTYLE) & WS_EX_TOPMOST) {
        SendMessage(GetDlgItem(hMainWnd, IDR_TOPMOST),
                    BM_SETCHECK,
                    BST_CHECKED,
                    0);

        SendMessage(GetDlgItem(hMainWnd, IDR_NOTOPMOST),
                    BM_SETCHECK,
                    BST_UNCHECKED,
                    0);
    } else {
        SendMessage(GetDlgItem(hMainWnd, IDR_NOTOPMOST),
                    BM_SETCHECK,
                    BST_CHECKED,
                    0);

        SendMessage(GetDlgItem(hMainWnd, IDR_TOPMOST),
                    BM_SETCHECK,
                    BST_UNCHECKED,
                    0);
    }

    //is target enabled?
    if (IsWindowEnabled(hTarget)) {
        SendMessage(GetDlgItem(hMainWnd, IDR_ENABLED),
                    BM_SETCHECK,
                    BST_CHECKED,
                    0);

        SendMessage(GetDlgItem(hMainWnd, IDR_DISABLED),
                    BM_SETCHECK,
                    BST_UNCHECKED,
                    0);
    } else {
        SendMessage(GetDlgItem(hMainWnd, IDR_DISABLED),
                    BM_SETCHECK,
                    BST_CHECKED,
                    0);

        SendMessage(GetDlgItem(hMainWnd, IDR_ENABLED),
                    BM_SETCHECK,
                    BST_UNCHECKED,
                    0);
    }

    //is target visible?
    if (IsWindowVisible(hTarget)) {
        SendMessage(GetDlgItem(hMainWnd, IDR_VISIBLE),
                    BM_SETCHECK,
                    BST_CHECKED,
                    0);

        SendMessage(GetDlgItem(hMainWnd, IDR_INVISIBLE),
                    BM_SETCHECK,
                    BST_UNCHECKED,
                    0);
    } else {
        SendMessage(GetDlgItem(hMainWnd, IDR_INVISIBLE),
                    BM_SETCHECK,
                    BST_CHECKED,
                    0);

        SendMessage(GetDlgItem(hMainWnd, IDR_VISIBLE),
                    BM_SETCHECK,
                    BST_UNCHECKED,
                    0);
    }

    //show target's RECT (target is not child window)
    if (GetParent(hTarget) == NULL) {     //if target is not a child window
        //show target rect
        //left
        SetDlgItemInt(hMainWnd, IDE_LEFT, TargetRect.left, TRUE);
        //top
        SetDlgItemInt(hMainWnd, IDE_TOP, TargetRect.top, TRUE);
    } else { //if target is child window,use its parent window to measure
        POINT point;//temporary var
        HWND hParent;//temporary var,target's parent

        //get target's parent
        hParent = GetParent(hTarget);

        //show target rect (target is child window)
        point.x = TargetRect.left;
        point.y = TargetRect.top;

        //convert screen point to parent window point
        ScreenToClient(hParent, &point);

        //left
        SetDlgItemInt(hMainWnd, IDE_LEFT, (UINT) point.x, TRUE);
        //top
        SetDlgItemInt(hMainWnd, IDE_TOP, (UINT) point.y, TRUE);
    }


    //no matter child window or not,width & height is same

    //width=right-left
    SetDlgItemInt(hMainWnd,   //handle
                  IDE_WIDTH,  //control id
                  TargetRect.right - TargetRect.left,  //int value
                  TRUE       //signed
                 );

    //height=bottom-top
    SetDlgItemInt(hMainWnd,   //hanle
                  IDE_HEIGHT,  //control id
                  TargetRect.bottom - TargetRect.top,   //int value
                  TRUE               //signed
                 );
}

//----------------------------------------------------------------//
//create listbox for show all windows
void CreateListBox()
{
    //font
    HFONT hFont = CreateFont(14, 0, 0, 0, FW_MEDIUM, 0, 0, 0, GB2312_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, "宋体");
    //--create Target List
    hTargetList = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_CLIENTEDGE,
                                 TEXT("LISTBOX"), "所有窗口",
                                 WS_OVERLAPPEDWINDOW | WS_VSCROLL | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY,
                                 0, 0, 70, 100,
                                 hMainWnd, NULL, hInst, NULL);
    SendMessage(hTargetList, WM_SETFONT, (WPARAM) hFont, 0);

    //subclass it
    oldTargetListProc = (WNDPROC) SetWindowLong(hTargetList, GWL_WNDPROC, (LONG) TargetListProc);

    //---create Child List
//    hChildList = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_CLIENTEDGE | WS_CHILD,
    hChildList = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_CLIENTEDGE,
                                TEXT("LISTBOX"), "子窗口",
                                WS_OVERLAPPEDWINDOW | WS_VSCROLL | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY,
                                0, 0, 70, 100,
                                hMainWnd, NULL, hInst, NULL);
    //set font
    SendMessage(hChildList, WM_SETFONT, (WPARAM) hFont, 0);

    //subclass it
    oldChildListProc = (WNDPROC) SetWindowLong(hChildList, GWL_WNDPROC, (LONG) ChildListProc);
}

//------------------------------------------------------------//
void InitMainDialog()
//init our main dialog
{
    int n;//ini file name length

    //ini file name
    n = GetModuleFileName(NULL, IniLocation, sizeof(IniLocation));

    lstrcpy(&(IniLocation[n - 3]), "ini");


    //create list box
    CreateListBox();

    //read config fron INI file
    ReadConfig();


    //create menu
    hTrayMenu = CreatePopupMenu();
    AppendMenu(hTrayMenu, MF_STRING, IDM_ACTIVE, (LPCTSTR) "激活");
    AppendMenu(hTrayMenu, MF_SEPARATOR, 3020, NULL);
    AppendMenu(hTrayMenu, MF_STRING, IDM_EXIT, (LPCTSTR) "退出");


    //change icon
    SendMessage(hMainWnd,    // handle of destination window
                WM_SETICON, // message to send
                (WPARAM)
                ICON_BIG,   // first message parameter
                (LPARAM)
                LoadIcon(hInst, (LPCSTR) IDI_MAIN)        // second message parameter
               );




    //sub class our contole

    //static ID_AIM
    oldAimProc = (WNDPROC) SetWindowLong(GetDlgItem(hMainWnd, ID_AIM),
                                         GWL_WNDPROC,
                                         (LONG) AimProc);
}
//----------------------------------------------------------------//
void ProcessCommandMessage(WPARAM wParam)
//process WM_COMMAND message in main window
{
    switch (LOWORD(wParam)) {
    //refresh all values
    case ID_REFRESH:
        if (IsWindow(hTarget)) {
            //get target rect
            GetWindowRect(hTarget, &TargetRect);
            //ShowTargetProperty
            ShowTargetProperty();

            //show all target's child windows
            SendMessage(hChildList, LB_RESETCONTENT, 0, 0);
            EnumChildWindows(hTarget, EnumChildWindowsProc, 0);
        } else {
            MessageBox(hMainWnd, "目标窗口已不存在", "出错", MB_OK);
        }
        break;

    //show target
    case IDR_VISIBLE:
        ShowWindow(hTarget, SW_SHOW);
        break;

    //hide target
    case IDR_INVISIBLE:
        ShowWindow(hTarget, SW_HIDE);
        break;

    //kill target
    case ID_KILL:
        SetFocus(hTarget);
        //ShowWindow(hMainWnd,SW_HIDE);
        DestroyWindow(hTarget);
        SendMessage(hTarget, WM_SYSCOMMAND, SC_CLOSE, 0);
        //ShowWindow(hMainWnd,SW_SHOW);

        break;

    //set topmost
    case IDR_TOPMOST:
        GetWindowRect(hTarget, &TargetRect);
        SetWindowPos(hTarget,     // handle of window
                     HWND_TOPMOST,   // placement-order handle
                     0,  // horizontal position
                     0,  // vertical position
                     0,  // width
                     0,  // height
                     SWP_NOMOVE |
                     SWP_NOSIZE |
                     SWP_SHOWWINDOW    // window-positioning flags
                    ) ;
        break;

    //cancel topmost
    case IDR_NOTOPMOST:
        GetWindowRect(hTarget, &TargetRect);
        SetWindowPos(hTarget,     // handle of window
                     HWND_NOTOPMOST, // placement-order handle
                     0,  // horizontal position
                     0,  // vertical position
                     0,  // width
                     0,  // height
                     SWP_NOMOVE |
                     SWP_NOSIZE |
                     SWP_SHOWWINDOW    // window-positioning flags
                    ) ;
        break;

    //enable target
    case IDR_ENABLED:
        EnableWindow(hTarget, TRUE);
        break;

    //disable target
    case IDR_DISABLED:
        EnableWindow(hTarget, FALSE);
        break;

    // minium to tray
    case ID_TOTRAY:
        ShowWindow(hMainWnd, SW_MINIMIZE);
        ShowWindow(hMainWnd, SW_HIDE);
        trayicon.cbSize = sizeof(NOTIFYICONDATA);
        trayicon.hIcon = LoadIcon(hInst, (LPCSTR) IDI_TRAY);
        trayicon.hWnd = hMainWnd;
        lstrcpy(trayicon.szTip,  // address of buffer
                "k_window"  // address of string to copy
               );
        trayicon.uID = 0;
        trayicon.uFlags = NIF_ICON + NIF_MESSAGE + NIF_TIP ;
        trayicon.uCallbackMessage = WM_USER + 5;

        Shell_NotifyIcon(NIM_ADD,     // message identifier
                         & trayicon   // pointer to structure
                        );
        break;

    //option
    case ID_OPTION:
        DialogBoxParam(hInst,
                       MAKEINTRESOURCE(IDD_OPTION),
                       hMainWnd,
                       (DLGPROC) OptionProc,
                       0);
        break;


    // list all window
    case ID_LISTTOP:
        //clear list box
        SendMessage(hTargetList, LB_RESETCONTENT, 0, 0);

        //show all window
        EnumWindows(EnumWindowsProc, 0);

        //show listBox
        ShowWindow(hTargetList, SW_SHOW);
        break;

    //list child window
    case ID_LISTCHILD:
        //clear list box
        SendMessage(hChildList, LB_RESETCONTENT, 0, 0);

        //show all window
        EnumChildWindows(hTarget, EnumChildWindowsProc, 0);

        //show listBox
        ShowWindow(hChildList, SW_SHOW);
        break;

    //show hidden window only?
    case IDC_SHOWHIDDENONLY:
        ShowHiddenOnly = !ShowHiddenOnly;

        //show all window
        SendMessage(hTargetList, LB_RESETCONTENT, 0, 0);
        EnumWindows(EnumWindowsProc, 0);

        //show all target's child windows
        SendMessage(hChildList, LB_RESETCONTENT, 0, 0);
        EnumChildWindows(hTarget, EnumChildWindowsProc, 0);
        break;

    // about
    case ID_ABOUT:
        DialogBox(hInst, (LPCSTR) IDD_ABOUT, hMainWnd, (DLGPROC) AboutProc);

        break;

    //激活(Tray Menu)
    case IDM_ACTIVE:
        ShowWindow(hMainWnd, SW_SHOW);
        ShowWindow(hMainWnd, SW_RESTORE);
        Shell_NotifyIcon(NIM_DELETE,  // message identifier
                         & trayicon   // pointer to structure
                        );
        break;

    //退出(Tray menu)
    case IDM_EXIT:
        Shell_NotifyIcon(NIM_DELETE,  // message identifier
                         & trayicon   // pointer to structure
                        );

        SendMessage(hMainWnd, WM_SYSCOMMAND, SC_CLOSE, 0);

        break;


    //change target's title
    case ID_CHANGETITLE:
        char TargetText[256];
        GetDlgItemText(hMainWnd, IDE_TITLE, (LPSTR) TargetText, 256);

        SetWindowText(hTarget, (LPCSTR) TargetText);

        //repaint target window
        InvalidateRect(hTarget, NULL, TRUE);
        break;

    //change target rect
    case ID_CHANGERECT:
        if (IsWindow(hTarget)) {     //if target is valid
            //tag that if the inputed value is valid
            BOOL vLeft;
            BOOL vTop;
            BOOL vWidth;
            BOOL vHeight;

            //new value for the target
            int left;
            int top;
            int width;
            int height;

            //get value that we input
            left = GetDlgItemInt(hMainWnd, IDE_LEFT, &vLeft, TRUE);
            top = GetDlgItemInt(hMainWnd, IDE_TOP, &vTop, TRUE);
            width = GetDlgItemInt(hMainWnd, IDE_WIDTH, &vWidth, TRUE);
            height = GetDlgItemInt(hMainWnd, IDE_HEIGHT, &vHeight, TRUE);
            if (vLeft && vTop && vWidth && vHeight) {   //if the value is valid
                //change target's rect
                MoveWindow(hTarget, left, top, width, height, TRUE);
//                MoveWindow(hTarget, 0, 0, width, height, TRUE);
                PostMessage(hTarget, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(560, 188));
                PostMessage(hTarget, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(560, 188));
                Sleep(100);
                PostMessage(hTarget, WM_LBUTTONUP, 0, MAKELPARAM(560, 188));
            }
        }
        break;

    //recover target's RECT
    case ID_RECOVERRECT:
        if (GetParent(hTarget)) {     //if target is a child window
            POINT point;//temp var

            point.x = TargetRect.left;
            point.y = TargetRect.top;

            ScreenToClient(GetParent(hTarget), &point);

            MoveWindow(hTarget,
                       point.x,
                       point.y,
                       TargetRect.right - TargetRect.left,
                       TargetRect.bottom - TargetRect.top,
                       TRUE);
        } else { //if target is not child window
            MoveWindow(hTarget,
                       TargetRect.left,
                       TargetRect.top,
                       TargetRect.right - TargetRect.left,
                       TargetRect.bottom - TargetRect.top,
                       TRUE);
        }

        break;
    }
}



//----------------------------------------------------------------//
//user functions
///////////////////////////////////////////////////////////////////////

//Entry Point
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HWND hPrevWnd;

    //if previous window exits
    if (hPrevWnd = FindWindow("#32770", "K_Window")) {
        SendMessage(hPrevWnd, WM_USER + 5, 0, WM_LBUTTONDBLCLK);
    } else { //if previous window dose not exit
        //get application instance handle
        hInst = hInstance;
        //create main dialog
        DialogBoxParam(hInst,
                       (LPCSTR) IDD_MAIN,
                       NULL,
                       (DLGPROC) MainDlgProc,
                       NULL);
    }

    ExitProcess(0);   //quit
}

///////////////////////////////////////////////////////////////////////

////         main wnd proc

LRESULT CALLBACK MainDlgProc(HWND hDlg,
                             UINT message,
                             WPARAM wParam,
                             LPARAM lParam)
{
    //varible
    //code start
    switch (message) {
    //init dialog
    case WM_INITDIALOG:
        hMainWnd = hDlg;//save main window handle into a global varible
        InitMainDialog();

        return TRUE;
        break;

    //left button down,make all window can be dragged
    case WM_LBUTTONDOWN:
        //you can drag the window by drag the client area
        SendMessage(hDlg, WM_SYSCOMMAND, 0xF012, 0);
        break;

    //wm_command message
    case WM_COMMAND:
        ProcessCommandMessage(wParam);

        return TRUE;
        break;

    //tray notify message
    case WM_USER + 5:
        if (wParam == 0) {
            if (lParam == WM_LBUTTONDBLCLK) {
                ShowWindow(hDlg, SW_RESTORE);
                Shell_NotifyIcon(NIM_DELETE,  // message identifier
                                 & trayicon   // pointer to structure
                                );
            }

            if (lParam == WM_RBUTTONDOWN) {
                POINT point;//Temp var


                //create menu
                GetCursorPos(&point);
                SetForegroundWindow(hDlg);
                TrackPopupMenu(hTrayMenu,
                               TPM_RIGHTALIGN,
                               point.x,
                               point.y,
                               0,
                               hDlg,
                               NULL);
            }
        }
        break;

    //close
    case WM_CLOSE:
        Shell_NotifyIcon(NIM_DELETE,  // message identifier
                         & trayicon   // pointer to structure
                        );
        SaveConfig();
        EndDialog(hDlg, 0);
        return TRUE;
        break;
    }
    return FALSE;
}

///////////////////////////////////////////////////////////////////////

//////////////////    option dialog proc
LRESULT CALLBACK OptionProc(HWND hOption, UINT msg, WPARAM wParam, LPARAM lParam)
{
    char tmpbuffer[2];





    //process message
    switch (msg) {
    //init
    case WM_INITDIALOG:
        //show target list
        if (GetPrivateProfileInt("other", "ShowList1", 0, IniLocation)) {
            SendMessage(GetDlgItem(hOption, IDC_SHOWTARGETLIST),
                        BM_SETCHECK,
                        BST_CHECKED,
                        0);
        } else {
            SendMessage(GetDlgItem(hOption, IDC_SHOWTARGETLIST),
                        BM_SETCHECK,
                        BST_UNCHECKED,
                        0);
        }

        //show child list
        if (GetPrivateProfileInt("other", "ShowList2", 0, IniLocation)) {
            SendMessage(GetDlgItem(hOption, IDC_SHOWCHILDLIST),
                        BM_SETCHECK,
                        BST_CHECKED,
                        0);
        } else {
            SendMessage(GetDlgItem(hOption, IDC_SHOWCHILDLIST),
                        BM_SETCHECK,
                        BST_UNCHECKED,
                        0);
        }

        //on top
        if (GetWindowLong(hMainWnd, GWL_EXSTYLE) & WS_EX_TOPMOST) {
            SendMessage(GetDlgItem(hOption, IDC_OPTIONONTOP),
                        BM_SETCHECK,
                        BST_CHECKED,
                        0);
        } else {
            SendMessage(GetDlgItem(hOption, IDC_OPTIONONTOP),
                        BM_SETCHECK,
                        BST_UNCHECKED,
                        0);
        }

        break;

    //close
    case WM_CLOSE:
        EndDialog(hOption, 0);
        return 0;
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        //ok
        case ID_OPTIONOK:
            //ontop tag
            BOOL ontop;


            ontop = SendMessage(GetDlgItem(hOption, IDC_OPTIONONTOP),
                                BM_GETCHECK,
                                0,
                                0);

            //save ontop tag
            wsprintf(tmpbuffer, "%d", ontop);
            WritePrivateProfileString("other", "OnTop", tmpbuffer, IniLocation);

            //set whether ontop
            if (ontop) {
                SetWindowPos(hMainWnd,      // handle of window
                             HWND_TOPMOST,  // placement-order handle
                             0,             // horizontal position
                             0,             // vertical position
                             0,             // width
                             0,             // height
                             SWP_NOMOVE |
                             SWP_NOSIZE |
                             SWP_SHOWWINDOW // window-positioning flags
                            ) ;
            } else {
                SetWindowPos(hMainWnd,       // handle of window
                             HWND_NOTOPMOST, // placement-order handle
                             0,              // horizontal position
                             0,              // vertical position
                             0,              // width
                             0,              // height
                             SWP_NOMOVE |
                             SWP_NOSIZE |
                             SWP_SHOWWINDOW  // window-positioning flags
                            ) ;
            }


            //save ShowTargetlist tag
            wsprintf(tmpbuffer,
                     "%d",
                     SendMessage(GetDlgItem(hOption, IDC_SHOWTARGETLIST),
                                 BM_GETCHECK,
                                 0,
                                 0));
            WritePrivateProfileString("other",
                                      "ShowList1",
                                      tmpbuffer,
                                      IniLocation);

            //save ShowChildlist tag
            wsprintf(tmpbuffer,
                     "%d",
                     SendMessage(GetDlgItem(hOption, IDC_SHOWCHILDLIST),
                                 BM_GETCHECK,
                                 0,
                                 0));
            WritePrivateProfileString("other",
                                      "ShowList2",
                                      tmpbuffer,
                                      IniLocation);


            EndDialog(hOption, 0);
            break;


        //cancel
        case ID_OPTIONCANCEL:
            EndDialog(hOption, 0);
            break;
        }
        return 0;
        break;
    }

    return 0;
}

/////////////////////      About Dlg Proc

LRESULT CALLBACK AboutProc(HWND hAbout,
                           UINT message,
                           WPARAM wParam,
                           LPARAM lParam)
{
    switch (message) {
    case WM_COMMAND:
        EndDialog(hAbout, 0);
        return TRUE;
        break;
    }

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////

//--------subclassed wnd proc---------------------------//

///////////     Aimming Proc

LRESULT CALLBACK AimProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //varible
    static int ready;//if ready=1,start to capture window
    static HPEN hPen;
    static HBRUSH hBrush;
    LOGBRUSH LogBrush = {
        BS_NULL, DIB_RGB_COLORS, HS_HORIZONTAL
    };
    static HDC hdc;
    static HWND hPrevTarget;//if target changed,repaint the screen
    static BOOL NeedDraw = 1;

    //code start
    switch (message) {
    case WM_LBUTTONDOWN:
        //----start WM_LBUTTONDOWN---//

        //set cursor shape & setcapture

        hTarget = 0;
        SetCursor(LoadCursor(hInst, (LPCSTR) IDC_AIMMING));
        SetCapture(hWnd);

        //get DC
        hdc = GetDC(0);

        //set rop2 mode
        SetROP2(hdc, R2_NOT);

        //create red pen
        hPen = CreatePen(PS_INSIDEFRAME,  // pen style,solid pen
                         4,  // pen width
                         0x000000    // pen color
                        );

        //create brush

        hBrush = CreateBrushIndirect(&LogBrush);

        //use pen & brush
        SelectObject(hdc, hPen);
        SelectObject(hdc, hBrush);

        //can start to capture target
        ready = TRUE;

        break;
    //----end WM_LBUTTONDOWN-----//

    //---start WM_LBUTTONUP---//
    case WM_LBUTTONUP:
        //show all target's child windows
        SendMessage(hChildList, LB_RESETCONTENT, 0, 0);
        EnumChildWindows(hTarget, EnumChildWindowsProc, 0);

        //repaint screen
        InvalidateRect(0, NULL, TRUE);

        //recover cursor
        SetCursor(LoadCursor(NULL, (LPCSTR) IDC_ARROW));

        //stop capture mouse
        ReleaseCapture();

        //delete pen ,brush & release DC
        DeleteObject(hPen) ;
        DeleteObject(hBrush) ;
        ReleaseDC(0, hdc);

        //clear varible
        hPrevTarget = 0;

        //stop captureing window
        ready = FALSE;
        break;
    //---end WM_LBUTTONUP---//

    //-----start WM_MOUSEMOVE---//
    case WM_MOUSEMOVE:
        if (ready) {
            //start to capture window
            SetCursor(LoadCursor(hInst, (LPCSTR) IDC_AIMMING));

            //fet target from mouse point
            CaptureTarget();

            //if target hav already been drawn the line,skip
            if (NeedDraw) {
                //get target RECT
                GetWindowRect(hTarget, &TargetRect);

                //draw lines around target
                Rectangle(hdc,
                          TargetRect.left,
                          TargetRect.top,
                          TargetRect.right,
                          TargetRect.bottom);
                NeedDraw = 0;
            }


            //if target changed,wipe the lines
            if (hTarget != hPrevTarget) {
                //wipe line
                if (hPrevTarget) {
                    Rectangle(hdc,
                              TargetRect.left,
                              TargetRect.top,
                              TargetRect.right,
                              TargetRect.bottom);
                }

                //need draw
                NeedDraw = 1;
                hPrevTarget = hTarget;
            }

            //start show target's property
            ShowTargetProperty();
        }




        break;
    }


    return CallWindowProc(oldAimProc, hWnd, message, wParam, lParam);
}

//////// TargetList Proc
LRESULT CALLBACK TargetListProc(HWND hTargetList,
                                UINT ListMsg,
                                WPARAM wParam,
                                LPARAM lParam)
{
    switch (ListMsg) {
    //if close list box,hide it
    case WM_CLOSE:
        ShowWindow(hTargetList, SW_HIDE);
        return 0;

    //WM_LBUTTONDOWN & WM_KEYDOWN
    case WM_LBUTTONUP:
    case WM_KEYUP:
        char tmp[268];

        //get selected text
        SendMessage(hTargetList,
                    LB_GETTEXT,
                    SendMessage(hTargetList, LB_GETCURSEL, 0, 0),
                    (LPARAM) tmp);

        //cut thefirst 10 characters
        lstrcpyn(tmp, tmp, 11);

        //convert to integer
        hTarget = (HWND) atoi(tmp);


        //show target property
        GetWindowRect(hTarget, &TargetRect);
        ShowTargetProperty();

        //show all target's child windows
        SendMessage(hChildList, LB_RESETCONTENT, 0, 0);
        EnumChildWindows(hTarget, EnumChildWindowsProc, 0);

        break;
    }


    return CallWindowProc(oldTargetListProc,
                          hTargetList,
                          ListMsg,
                          wParam,
                          lParam);
}

/////ChildList Proc
LRESULT CALLBACK ChildListProc(HWND hChildList,
                               UINT ListMsg,
                               WPARAM wParam,
                               LPARAM lParam)
{
    switch (ListMsg) {
    //if close list box,hide it
    case WM_CLOSE:
        ShowWindow(hChildList, SW_HIDE);
        return 0;

    //WM_LBUTTONUP & WM_KEYUP
    case WM_LBUTTONUP:
    case WM_KEYUP:
        char tmp[268];

        //get selected text
        SendMessage(hChildList,
                    LB_GETTEXT,
                    SendMessage(hChildList, LB_GETCURSEL, 0, 0),
                    (LPARAM) tmp);

        //cut thefirst 10 characters
        lstrcpyn(tmp, tmp, 11);

        //convert to integer
        hTarget = (HWND) atoi(tmp);


        //show target property
        GetWindowRect(hTarget, &TargetRect);
        ShowTargetProperty();


        break;
    }


    return CallWindowProc(oldChildListProc,
                          hChildList,
                          ListMsg,
                          wParam,
                          lParam);
}
