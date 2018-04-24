#include <windows.h>
#include <Shlobj.h>
#include <shlwapi.h>

int invoke_command(IContextMenu *c,int id,HWND hwnd)
{
	CMINVOKECOMMANDINFO ci;
	memset(&ci,0,sizeof(ci));
	ci.cbSize=sizeof(ci);
	ci.hwnd=hwnd;
	ci.lpVerb=MAKEINTRESOURCE(id-1);
	ci.nShow=SW_SHOWNORMAL;
	c->lpVtbl->InvokeCommand(c,&ci);
	return 0;
}
int get_wide_path(const char *fname,wchar_t *out_path,int pcount,wchar_t *out_fname,int fcount)
{
	char drive[_MAX_DRIVE],dir[_MAX_DIR],fn[_MAX_FNAME],ext[_MAX_EXT];
	char tmp[MAX_PATH];
	drive[0]=dir[0]=fn[0]=ext[0]=0;
	_splitpath(fname,drive,dir,fn,ext);
	_snprintf(tmp,sizeof(tmp),"%s%s",drive,dir);
	mbstowcs((wchar_t*)out_path,tmp,pcount);
	_snprintf(tmp,sizeof(tmp),"%s%s",fn,ext);
	mbstowcs((wchar_t*)out_fname,tmp,fcount);
	return 0;
}
static WNDPROC old_win_proc=0;
static IContextMenu2 *context2=0;
int get_higher_context(IContextMenu *c)
{
	int result=FALSE;
	context2=0;
	c->lpVtbl->QueryInterface(c,&IID_IContextMenu2,&context2);
	if(context2!=0)
		result=TRUE;
	return result;
}
LRESULT CALLBACK subclass_proc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	switch(msg){
	case WM_DRAWITEM:
	case WM_MEASUREITEM:
		if(wparam)break; // not menu related
	case WM_INITMENUPOPUP:
		if(context2!=0){
			context2->lpVtbl->HandleMenuMsg(context2,msg,wparam,lparam);
			return (msg==WM_INITMENUPOPUP ? 0 : TRUE); // handled
		}
		break;
	}
	return CallWindowProc(old_win_proc,hwnd,msg,wparam,lparam);
}
int subclass_window(IContextMenu *c,HWND hwnd,int release)
{
	if(release){
		if(old_win_proc!=0){
			SetWindowLong(hwnd,GWL_WNDPROC,old_win_proc);
			old_win_proc=0;
		}
		if(context2!=0){
			context2->lpVtbl->Release(context2);
			context2=0;
		}
		return 0;
	}
	else{
		get_higher_context(c);
		if(context2!=0)
			old_win_proc=SetWindowLong(hwnd,GWL_WNDPROC,subclass_proc);
	}
	return 0;
}
int GetContextMenu(HWND hwnd,char *fname)
{
	HRESULT hr;
	ITEMIDLIST *folder=0,*file=0;
	IShellFolder *shell=0,*parent=0;

	CoCreateInstance(&CLSID_ShellDesktop,NULL,CLSCTX_INPROC,&IID_IShellFolder,&shell);

	if(shell!=0){
		wchar_t wpath[MAX_PATH]={0},wfname[MAX_PATH]={0};
		get_wide_path(fname,wpath,sizeof(wpath)/sizeof(wchar_t),wfname,sizeof(wfname)/sizeof(wchar_t));
		hr=shell->lpVtbl->ParseDisplayName(shell,NULL,NULL,wpath,NULL,&folder,NULL);
		if(hr==S_OK){
			shell->lpVtbl->BindToObject(shell,folder,NULL,&IID_IShellFolder,&parent);
			if(parent!=0){
				IContextMenu *context=0;
				hr=parent->lpVtbl->ParseDisplayName(parent,NULL,NULL,wfname,NULL,&file,NULL);
				if(hr==S_OK)
					parent->lpVtbl->GetUIObjectOf(parent,NULL,1,&file,&IID_IContextMenu,NULL,&context);
				parent->lpVtbl->Release(parent);
				if(context!=0){
					HMENU hmenu=0;
					hmenu=CreatePopupMenu();
					hr=context->lpVtbl->QueryContextMenu(context,hmenu,0,1,0x7FFF,CMF_NORMAL);
					if(hr>1)//returns largest menu identifier + 1
					{
						POINT pt;
						int menu_id=0;
						GetCursorPos(&pt);
						subclass_window(context,hwnd,FALSE);
						menu_id=TrackPopupMenu(hmenu,TPM_RETURNCMD,pt.x+10,pt.y+10,0,hwnd,NULL);
						subclass_window(context,hwnd,TRUE);
						if(menu_id>=1 && menu_id<=0x7FFF)
							invoke_command(context,menu_id,hwnd);
					}
					context->lpVtbl->Release(context);
					if(hmenu!=0)
						DestroyMenu(hmenu);
				}
			}
		}
		shell->lpVtbl->Release(shell);
	}
	if(folder!=0)
		CoTaskMemFree(folder);
	if(file!=0)
		CoTaskMemFree(file);
   return 0;
}