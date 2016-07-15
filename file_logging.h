typedef struct{
	FILE *f;
	char name[MAX_PATH];
	DWORD tick,lastflush;
}LOG_FILE;

LOG_FILE log_files[sizeof(irc_windows)/sizeof(IRC_WINDOW)];
int log_enable=FALSE;
static CRITICAL_SECTION log_mutex;
static int log_mutex_init=FALSE;

int init_log_files()
{
	memset(log_files,0,sizeof(log_files));
	get_ini_value("SETTINGS","ENABLE_LOG",&log_enable);
	return TRUE;
}
int find_existing_log(char *name,LOG_FILE **logf)
{
	int i,result=FALSE;
	for(i=0;i<sizeof(log_files)/sizeof(LOG_FILE);i++){
		if(log_files[i].name[0]!=0 && stricmp(log_files[i].name,name)==0){
			if(logf!=0){
				*logf=&log_files[i];
				result=TRUE;
			}
			break;
		}
	}
	return result;
}
int acquire_log_file(char *name,LOG_FILE **logf)
{
	int i,result=FALSE;
	result=find_existing_log(name,logf);
	if(!result){
		for(i=0;i<sizeof(log_files)/sizeof(LOG_FILE);i++){
			if(log_files[i].f==0 && log_files[i].name[0]==0){
				strncpy(log_files[i].name,name,sizeof(log_files[i].name));
				if(logf!=0){
					*logf=&log_files[i];
					result=TRUE;
				}
				break;
			}
		}
	}
	return result;
}
int close_log(char *chan,char *network)
{
	int result=FALSE;
	char name[MAX_PATH]={0};
	LOG_FILE *log;
	_snprintf(name,sizeof(name),"%s.%s.log",chan,network);
	log=0;
	EnterCriticalSection(&log_mutex);
	find_existing_log(name,&log);
	if(log!=0){
		if(log->f!=0)
			fclose(log->f);
		memset(log,0,sizeof(LOG_FILE));
	}
	LeaveCriticalSection(&log_mutex);
	return result;
}
int close_all_logs()
{
	int i;
	EnterCriticalSection(&log_mutex);
	for(i=0;i<sizeof(log_files)/sizeof(LOG_FILE);i++){
		if(log_files[i].f!=0){
			fclose(log_files[i].f);
			memset(&log_files[i],0,sizeof(LOG_FILE));
		}
	}
	LeaveCriticalSection(&log_mutex);
	return TRUE;
}
int flush_all_logs()
{
	int i;
	EnterCriticalSection(&log_mutex);
	for(i=0;i<sizeof(log_files)/sizeof(LOG_FILE);i++){
		if(log_files[i].f!=0)
			fflush(log_files[i].f);
	}
	LeaveCriticalSection(&log_mutex);
	return TRUE;
}
int close_old_files()
{
#define CLOSE_DELAY (60*60*1000)
#define FLUSH_DELAY (10*1000)
	int i;
	DWORD tick;
	tick=GetTickCount();
	for(i=0;i<sizeof(log_files)/sizeof(LOG_FILE);i++){
		if(log_files[i].f!=0){
			if(tick>(log_files[i].tick+CLOSE_DELAY)){
				fclose(log_files[i].f);
				memset(&log_files[i],0,sizeof(LOG_FILE));
			}else if(tick>(log_files[i].lastflush+FLUSH_DELAY)){
				fflush(log_files[i].f);
				log_files[i].lastflush=tick;
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
int init_log_mutex()
{
	if(!log_mutex_init){
		InitializeCriticalSection(&log_mutex);
		log_mutex_init=TRUE;
	}
	return log_mutex_init;
}
int sanitize_fname(char *str,int len)
{
	const char exclude[]={'<','>',':','\"','/','\\','|','?','*'};
	int i;
	for(i=0;i<len;i++){
		unsigned char a=str[i];
		if(0==a)
			break;
		if(a<' ' || a>=0x7F)
			a='_';
		else{
			int j;
			for(j=0;j<sizeof(exclude);j++){
				if(exclude[j]==a)
					a='_';
			}
		}
		str[i]=a;
	}
	if(len>0)
		str[len-1]=0;
	return TRUE;
}
int log_str(char *chan,char *network,char *str)
{
	LOG_FILE *log;
	int result=FALSE;
	static int files_open=FALSE;
	char name[MAX_PATH]={0};

	assert(log_mutex_init);

	EnterCriticalSection(&log_mutex);
	if(!log_enable){
		if(files_open)
			close_all_logs();
		files_open=FALSE;
	}
	else{
		_snprintf(name,sizeof(name),"%s.%s.log",chan,network);
		log=0;
		acquire_log_file(name,&log);
		if(log!=0){
			if(log->f==0){
				char fullname[MAX_PATH]={0};
				if(create_log_directory(fullname,sizeof(fullname))){
					_snprintf(fullname,sizeof(fullname),"%s\\%s",fullname,name);
					log->f=fopen(fullname,"a");
					log->lastflush=GetTickCount();
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
	}
	LeaveCriticalSection(&log_mutex);
	return result;
}