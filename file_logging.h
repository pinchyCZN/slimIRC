typedef struct{
	FILE *f;
	char name[MAX_PATH];
	DWORD tick;
}LOG_FILE;

LOG_FILE log_files[sizeof(irc_windows)/sizeof(IRC_WINDOW)];
int log_enable=FALSE;

int init_log_files()
{
	memset(log_files,0,sizeof(log_files));
	get_ini_value("SETTINGS","ENABLE_LOG",&log_enable);
	return TRUE;
}
int acquire_log_file(char *name)
{
	int i;
	for(i=0;i<sizeof(log_files)/sizeof(LOG_FILE);i++){
		if(log_files[i].name[0]!=0 && stricmp(log_files[i].name,name)==0)
			return &log_files[i];
	}
	for(i=0;i<sizeof(log_files)/sizeof(LOG_FILE);i++){
		if(log_files[i].f==0 && log_files[i].name[0]==0){
			strncpy(log_files[i].name,name,sizeof(log_files[i].name));
			return &log_files[i];
		}
	}
	return 0;
}
int close_all_logs()
{
	int i;
	for(i=0;i<sizeof(log_files)/sizeof(LOG_FILE);i++){
		if(log_files[i].f!=0){
			fclose(log_files[i].f);
			memset(&log_files[i],0,sizeof(LOG_FILE));
		}
	}
	return TRUE;
}
int close_old_files()
{
	int i;
	DWORD tick;
	for(i=0;i<sizeof(log_files)/sizeof(LOG_FILE);i++){
		if(log_files[i].f!=0){
			tick=GetTickCount();
			if(tick>(log_files[i].tick+(60*60*1000))){
				fclose(log_files[i].f);
				memset(&log_files[i],0,sizeof(LOG_FILE));
			}
		}
	}
	return TRUE;
}
int create_log_directory(char *dir,int len)
{
	get_ini_path(dir,len);
	if(dir[0]!=0){
		_snprintf(dir,len,"%slogs",dir);
		if(is_path_directory(dir))
			return TRUE;
		return CreateDirectory(dir,NULL)!=0;
	}
	return FALSE;
}
int log_str(char *chan,char *network,char *str)
{
	LOG_FILE *log;
	int result=FALSE;
	static int files_open=FALSE;
	char name[MAX_PATH]={0};
	if(!log_enable){
		if(files_open)
			close_all_logs();
		files_open=FALSE;
		return FALSE;
	}
	_snprintf(name,sizeof(name),"%s.%s.log",chan,network);
	log=acquire_log_file(name);
	if(log!=0){
		if(log->f==0){
			char fullname[MAX_PATH]={0};
			if(create_log_directory(fullname,sizeof(fullname))){
				_snprintf(fullname,sizeof(fullname),"%s\\%s",fullname,name);
				log->f=fopen(fullname,"a");
			}
		}
		if(log->f!=0){
			DWORD tick;
			char date[16]={0},time[16]={0};
			tick=GetTickCount();
			GetDateFormat(LOCALE_SYSTEM_DEFAULT,NULL,NULL,"yy.MM.dd|",date,sizeof(date));
			GetTimeFormat(LOCALE_SYSTEM_DEFAULT,NULL,NULL,"HH:mm:ss",time,sizeof(time));
			fprintf(log->f,"%s%s %s\n",date,time,str);
			log->tick=tick;
			result=TRUE;
			files_open=TRUE;
		}
	}
	close_old_files();
	return result;
}