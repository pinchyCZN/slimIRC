#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <shlwapi.h>
#include <Shlobj.h>


#define INI_FNAME "SlimIRC.ini"

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
	char str[255];
	int result=FALSE;
	str[0]=0;
	result=GetPrivateProfileString(section,key,"",str,sizeof(str),ini_file);
	if(str[0]!=0)
		*val=atoi(str);
	return result>0;
}
int get_ini_str(char *section,char *key,char *str,int size)
{
	int result=FALSE;
	char tmpstr[1024];
	tmpstr[0]=0;
	result=GetPrivateProfileString(section,key,"",tmpstr,sizeof(tmpstr),ini_file);
	if(result>0)
		strncpy(str,tmpstr,size);
	return result>0;
}
int delete_ini_key(char *section,char *key)
{
	return WritePrivateProfileString(section,key,NULL,ini_file);
}
int delete_ini_section(char *section)
{
	return WritePrivateProfileString(section,NULL,NULL,ini_file);
}
int write_ini_value(char *section,char *key,int val)
{
	char str[20]={0};
	_snprintf(str,sizeof(str),"%i",val);
	if(WritePrivateProfileString(section,key,str,ini_file)!=0)
		return TRUE;
	else
		return FALSE;
}
int write_ini_str(char *section,char *key,char *str)
{
	if(WritePrivateProfileString(section,key,str,ini_file)!=0)
		return TRUE;
	else
		return FALSE;
}
int add_trail_slash(char *path)
{
	int i;
	i=strlen(path);
	if(i>0 && path[i-1]!='\\')
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
int get_appdata_folder(char *path)
{
	int found=FALSE;
	ITEMIDLIST *pidl;
	IMalloc	*palloc;
	HWND hwindow=0;
	if(SHGetSpecialFolderLocation(hwindow,CSIDL_APPDATA,&pidl)==NOERROR){
		if(SHGetPathFromIDList(pidl,path)){
			found=TRUE;
		}
		if(SHGetMalloc(&palloc)==NOERROR){
			palloc->lpVtbl->Free(palloc,pidl);
			palloc->lpVtbl->Release(palloc);
		}
	}
	return found;
}
int create_portable_file()
{
	FILE *f;
	f=fopen("SlimIRC_portable","wb");
	if(f!=0){
		fclose(f);
		return TRUE;
	}
	return FALSE;
}
int init_ini_file()
{
	char path[MAX_PATH],str[MAX_PATH];
	FILE *f;
	memset(ini_file,0,sizeof(ini_file));
	memset(path,0,sizeof(path));
	memset(str,0,sizeof(str));
	GetCurrentDirectory(sizeof(path),path);
	_snprintf(str,sizeof(str)-1,"%s\\SlimIRC_portable",path);
	if(does_file_exist(str)){
		if(!does_file_exist(INI_FNAME))
			goto install;
	}
	else{
		if(get_appdata_folder(path)){
			add_trail_slash(path);
			strcat(path,"SlimIRC\\");
			_snprintf(str,sizeof(str)-1,"%s%s",path,INI_FNAME);
			if((!is_path_directory(path)) || (!does_file_exist(str))){
				char str[MAX_PATH*3],cdir[MAX_PATH];
				memset(str,0,sizeof(str));memset(cdir,0,sizeof(cdir));
				GetCurrentDirectory(sizeof(cdir),cdir);
				_snprintf(str,sizeof(str)-1,"YES=Create directory %s to put INI file in\r\n\r\n"
					"NO=Make installation portable by putting INI in current directory %s\r\n\r\n"
					"CANCEL=Abort installation",path,cdir);
				switch(MessageBox(NULL,str,"Install",MB_YESNOCANCEL|MB_SYSTEMMODAL)){
				default:
				case IDCANCEL:
					exit(-1);
					break;
				case IDYES:
					CreateDirectory(path,NULL);
					break;
				case IDNO:
					GetCurrentDirectory(sizeof(path),path);
					create_portable_file();
					break;
				}
			}
		}else{
			char str[MAX_PATH*3];
install:
			GetCurrentDirectory(sizeof(path),path);
			memset(str,0,sizeof(str));
			_snprintf(str,sizeof(str)-1,
				"OK=Install INI in current directory %s\r\n\r\n"
				"CANCEL=Abort installation",path);
			switch(MessageBox(NULL,str,"Install",MB_OKCANCEL|MB_SYSTEMMODAL)){
			default:
			case IDCANCEL:
				exit(-1);
				break;
			case IDOK:
				create_portable_file();
				break;
			}
		}

	}
	add_trail_slash(path);
	_snprintf(ini_file,sizeof(ini_file)-1,"%s%s",path,INI_FNAME);
	f=fopen(ini_file,"rb");
	if(f==0){
		f=fopen(ini_file,"wb");
	}
	if(f!=0){
		char str[255];
		fclose(f);
		str[0]=0;
		get_ini_str("SlimIRC","installed",str,sizeof(str));
		if(str[0]==0)
			write_new_list_ini();
	}
	return 0;
}

int open_ini(HWND hwnd,int explore)
{
	WIN32_FIND_DATA fd;
	HANDLE h;
	char str[MAX_PATH+80];
	if(h=FindFirstFile(ini_file,&fd)!=INVALID_HANDLE_VALUE){
		FindClose(h);
		if(explore)
		{
			char drive[_MAX_DRIVE],dir[_MAX_DIR];
			_splitpath(ini_file,drive,dir,NULL,NULL);
			_snprintf(str,sizeof(str),"%s%s",drive,dir);
			ShellExecute(hwnd,"explore",str,NULL,NULL,SW_SHOWNORMAL);
		}
		else
			ShellExecute(hwnd,"open","notepad.exe",ini_file,NULL,SW_SHOWNORMAL);
	}
	else if(hwnd!=0){
		memset(str,0,sizeof(str));
		_snprintf(str,sizeof(str)-1,"cant locate ini file:\r\n%s",ini_file);
		MessageBox(hwnd,str,"Error",MB_OK);
	}
	return TRUE;
}

int write_new_list_ini()
{
	write_ini_str("SlimIRC","installed","TRUE");
	return TRUE;
}