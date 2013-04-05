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
int get_wide_path(const char *fname,char *out_path,int psize,char *out_fname,int fsize)
{
	char drive[_MAX_DRIVE],dir[_MAX_DIR],fn[_MAX_FNAME],ext[_MAX_EXT];
	char tmp[MAX_PATH];
	drive[0]=dir[0]=fn[0]=ext[0]=0;
	_splitpath(fname,drive,dir,fn,ext);
	_snprintf(tmp,sizeof(tmp),"%s%s",drive,dir);
	mbstowcs(out_path,tmp,psize);
	_snprintf(tmp,sizeof(tmp),"%s%s",fn,ext);
	mbstowcs(out_fname,tmp,fsize);
	return 0;
}
int get_higher_context(IContextMenu *c)
{
	IContextMenu2 *c2=0;
	IContextMenu3 *c3=0;
	c->lpVtbl->QueryInterface(c,&IID_IContextMenu3,c2);
//http://netez.com/2xExplorer/shellFAQ/bas_context.html
}
int GetContextMenu(HWND hwnd,char *fname)
{
	HRESULT hr;
	ITEMIDLIST *folder=0,*file=0;
	IShellFolder *shell=0,*parent=0;
	static init=TRUE;
	if(init){
		hr=CoInitialize(0);
		if(hr!=S_OK)
			return 0;
		init=FALSE;
	}
	CoCreateInstance(&CLSID_ShellDesktop,NULL,CLSCTX_INPROC,&IID_IShellFolder,&shell);

	if(shell!=0){
		char wpath[MAX_PATH*2]={0,0},wfname[MAX_PATH*2]={0,0};
		get_wide_path(fname,wpath,sizeof(wpath),wfname,sizeof(wfname));
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
						menu_id=TrackPopupMenu(hmenu,TPM_RETURNCMD,pt.x+10,pt.y+10,0,hwnd,NULL);
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
	//CoUninitialize();
    return 0;
}