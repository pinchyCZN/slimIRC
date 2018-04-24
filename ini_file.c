#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <shlwapi.h>
#include <Shlobj.h>
#include <aclapi.h>
#ifndef CSIDL_COMMON_APPDATA
	#define  CSIDL_COMMON_APPDATA 0x23
#endif
#include "resource.h"

#define APP_NAME "slimIRC"
#include "ram_ini_file.h"
char ini_file[MAX_PATH]={0};
int is_path_directory(char *path)
{
	int attrib;
	attrib=GetFileAttributes(path);
	if((attrib!=0xFFFFFFFF) && (attrib&FILE_ATTRIBUTE_DIRECTORY))
		return TRUE;
	else
		return FALSE;
}
int get_ini_value(char *section,char *key,int *val)
{
	char str[255]={0};
	int result=0;
	if(ini_file[0]!=0){
		result=GetPrivateProfileString(section,key,"",str,sizeof(str),ini_file);
		if(str[0]!=0)
			*val=atoi(str);
	}
	else{
		result=get_private_profile_string(section,key,"",str,sizeof(str),&ram_ini);
		if(str[0]!=0)
			*val=atoi(str);
	}
	return result>0;
}
int get_ini_str(char *section,char *key,char *str,int size)
{
	int result=0;
	char tmpstr[1024]={0};
	if(ini_file[0]!=0){
		result=GetPrivateProfileString(section,key,"",tmpstr,sizeof(tmpstr),ini_file);
		if(result>0)
			strncpy(str,tmpstr,size);
	}else{
		result=get_private_profile_string(section,key,"",tmpstr,sizeof(tmpstr),&ram_ini);
		if(result>0)
			strncpy(str,tmpstr,size);
	}
	return result>0;
}
int delete_ini_key(char *section,char *key)
{
	if(ini_file[0]!=0)
		return WritePrivateProfileString(section,key,NULL,ini_file);
	else
		return write_private_profile_string(section,key,NULL,&ram_ini);
}
int delete_ini_section(char *section)
{
	if(ini_file[0]!=0)
		return WritePrivateProfileString(section,NULL,NULL,ini_file);
	else
		return write_private_profile_string(section,NULL,NULL,&ram_ini);
}
int write_ini_value(char *section,char *key,int val)
{
	char str[20]={0};
	_snprintf(str,sizeof(str),"%i",val);
	if(ini_file[0]!=0){
		if(WritePrivateProfileString(section,key,str,ini_file)!=0)
			return TRUE;
		else
			return FALSE;
	}
	else{
		if(write_private_profile_string(section,key,str,&ram_ini)!=0)
			return TRUE;
		else
			return FALSE;
	}
}
int write_ini_str(char *section,char *key,char *str)
{
	if(ini_file[0]!=0){
		if(WritePrivateProfileString(section,key,str,ini_file)!=0)
			return TRUE;
		else
			return FALSE;
	}
	else{
		if(write_private_profile_string(section,key,str,&ram_ini)!=0)
			return TRUE;
		else
			return FALSE;
	}
}
int add_trail_slash(char *path,int size)
{
	int i;
	i=strlen(path);
	if((i<(size-1)) && i>0 && path[i-1]!='\\')
		strcat(path,"\\");
	return TRUE;
}
int does_file_exist(char *fname)
{
	FILE *f;
	f=fopen(fname,"rb");
	if(f!=0){
		fclose(f);
		return TRUE;
	}
	return FALSE;
}
int get_appdata_folder(char *path,int size)
{
	int found=FALSE;
	ITEMIDLIST *pidl;
	IMalloc	*palloc;
	HWND hwindow=0;
	if(path==0 || size<MAX_PATH)
		return found;
	if(SHGetSpecialFolderLocation(hwindow,CSIDL_APPDATA,&pidl)==NOERROR){
		if(SHGetPathFromIDList(pidl,path)){
			_snprintf(path,size,"%s\\%s",path,APP_NAME);
			found=TRUE;
		}
		if(SHGetMalloc(&palloc)==NOERROR){
			palloc->lpVtbl->Free(palloc,pidl);
			palloc->lpVtbl->Release(palloc);
		}
	}
	return found;
}
//no trailing slash
int extract_folder(char *f,int size)
{
	int i,len;
	if(f==0)
		return FALSE;
	len=strlen(f);
	if(len>size)
		len=size;
	for(i=len-1;i>=0;i--){
		if(f[i]=='\\'){
			f[i]=0;
			break;
		}
		else
			f[i]=0;
	}
	return TRUE;
}
//returns no trailing slash
int set_module_dir()
{
	char path[MAX_PATH]={0};
	GetModuleFileName(NULL,path,sizeof(path));
	extract_folder(path,sizeof(path));
	if(path[0]!=0 && is_path_directory(path)){
		SetCurrentDirectory(path);
		return TRUE;
	}
	return FALSE;

}
LRESULT CALLBACK install_proc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	static char *path_param=0;
	static char local_path[MAX_PATH]={0};
	static char appdata_path[MAX_PATH]={0};
	static HWND grippy=0;
	switch(msg){
	case WM_INITDIALOG:
		{
			RECT rect={0};
			path_param=lparam;
			local_path[0]=0;
			GetCurrentDirectory(sizeof(local_path),local_path);
			SetWindowText(GetDlgItem(hwnd,IDC_TXT_LOCAL),local_path);
			appdata_path[0]=0;
			get_appdata_folder(appdata_path,sizeof(appdata_path));
			SetWindowText(GetDlgItem(hwnd,IDC_TXT_APPDATA),appdata_path);
			grippy=create_grippy(hwnd);
			resize_install_dlg(hwnd);
			if(GetWindowRect(GetDesktopWindow(),&rect)!=0){
				int cx,cy;
				cx=(rect.left+rect.right)/2;
				cy=(rect.top+rect.bottom)/2;
				if(GetWindowRect(hwnd,&rect)!=0){
					int w,h;
					w=rect.right-rect.left;
					h=rect.bottom-rect.top;
					SetWindowPos(hwnd,NULL,cx-w/2,cy-h/2,0,0,SWP_NOSIZE);
				}
			}
		}
		break;
	case WM_SIZE:
		resize_install_dlg(hwnd);
		grippy_move(hwnd,grippy);
		break;
	case WM_COMMAND:
		switch(LOWORD(wparam)){
		case IDC_USE_LOCAL:
			if(path_param!=0)
				strncpy(path_param,local_path,MAX_PATH);
			EndDialog(hwnd,1);
			break;
		case IDC_USE_APPDATA:
			if(path_param!=0){
				CreateDirectory(appdata_path,NULL);
				strncpy(path_param,appdata_path,MAX_PATH);
			}
			EndDialog(hwnd,2);
			break;
		case IDCANCEL:
		case IDC_NO_INI:
			if(path_param!=0)
				path_param[0]=0;
			EndDialog(hwnd,3);
			break;
		}
		break;
	}
	return 0;
}
int create_folder_all_access(char *path)
{
	int result=FALSE;
	SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
	PSID everyone_sid = NULL;
	if(path==0 || path[0]==0)
		return result;
	if(AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &everyone_sid)){
		EXPLICIT_ACCESS ea;
		PACL acl = NULL;
		ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
		ea.grfAccessPermissions = SPECIFIC_RIGHTS_ALL | STANDARD_RIGHTS_ALL;
		ea.grfAccessMode = SET_ACCESS;
		ea.grfInheritance = NO_INHERITANCE;
		ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
		ea.Trustee.ptstrName  = (LPTSTR)everyone_sid;

		if(ERROR_SUCCESS==SetEntriesInAcl(1, &ea, NULL, &acl)){
			PSECURITY_DESCRIPTOR sd = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
			if(sd && InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION)){
				if(SetSecurityDescriptorDacl(sd, TRUE, acl, FALSE)){
					SECURITY_ATTRIBUTES sa;
					sa.nLength = sizeof(SECURITY_ATTRIBUTES);
					sa.lpSecurityDescriptor = sd;
					sa.bInheritHandle = FALSE;
					if(CreateDirectory(path, &sa))
						result=TRUE;
				}
			}
			if(sd)
				LocalFree(sd);

			LocalFree(acl);
		}
		FreeSid(everyone_sid);
	}
	return result;
}

int do_install_dialog(char *path_param)
{
	extern HINSTANCE ghinstance;
	return DialogBoxParam(ghinstance,IDD_INSTALL_DIALOG,NULL,install_proc,path_param);
}
int init_ini_file()
{
	char path[MAX_PATH],str[MAX_PATH];
	FILE *f;
	memset(ini_file,0,sizeof(ini_file));
	path[0]=0;
	str[0]=0;
	set_module_dir();
	GetCurrentDirectory(sizeof(path),path);
	_snprintf(str,sizeof(str)-1,"%s\\" APP_NAME ".ini",path);
	if(!does_file_exist(str)){
		path[0]=0;
		get_appdata_folder(path,sizeof(path));
		_snprintf(str,sizeof(str)-1,"%s\\" APP_NAME ".ini",path);
		if(!does_file_exist(str)){
			path[0]=0;
			do_install_dialog(path);
		}
	}
	if(path[0]==0){
		ini_file[0]=0;
		return 0;
	}
	add_trail_slash(path,sizeof(path));
	_snprintf(ini_file,sizeof(ini_file)-1,"%s%s",path,APP_NAME ".ini");
	f=fopen(ini_file,"rb");
	if(f==0){
		f=fopen(ini_file,"wb");
	}
	if(f!=0){
		fclose(f);
	}
	else
	{
		char msg[MAX_PATH+80]={0};
		_snprintf(msg,sizeof(msg),"Unable to access ini file:\r\n%s",ini_file);
		MessageBox(NULL,msg,"Error",MB_OK|MB_SYSTEMMODAL);
	}
	return 0;
}
int get_ini_path(char *path,int size)
{
	char drive[_MAX_DRIVE],dir[_MAX_DIR];
	if((ini_file[0]==0) || (size<=0)){
		if(size>0)
			path[0]=0;
		return FALSE;
	}
	_splitpath(ini_file,drive,dir,NULL,NULL);
	_snprintf(path,size,"%s%s",drive,dir);
	add_trail_slash(path,size);
	path[size-1]=0;
	return TRUE;
}
int open_ini(HWND hwnd,int explore)
{
	WIN32_FIND_DATA fd;
	HANDLE h;
	char str[MAX_PATH+80];
	if(h=FindFirstFile(ini_file,&fd)!=INVALID_HANDLE_VALUE){
		FindClose(h);
		if(explore){
			if(get_ini_path(str,sizeof(str)))
				ShellExecute(hwnd,"explore",str,NULL,NULL,SW_SHOWNORMAL);
		}
		else{
			if(ini_file[0]!=0)
				if(ShellExecute(hwnd,"open","notepad.exe",ini_file,NULL,SW_SHOWNORMAL)<=32)
					ShellExecute(hwnd,"open",ini_file,NULL,NULL,SW_SHOWNORMAL);

		}
	}
	else if(hwnd!=0){
		memset(str,0,sizeof(str));
		_snprintf(str,sizeof(str)-1,"cant locate ini file:\r\n%s",ini_file);
		MessageBox(hwnd,str,"Error",MB_OK);
	}
	return TRUE;
}
