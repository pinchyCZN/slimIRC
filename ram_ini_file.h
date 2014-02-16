typedef struct{
	char *key;
	char *str;
	void *next;
	void *prev;
}INI_KEY;

typedef struct{
	char *section;
	void *next;
	void *prev;
	INI_KEY *keys;
}INI_SECTION;

typedef struct{
	INI_SECTION *sections;
}RAM_INI;

RAM_INI ram_ini={0};
static int add_section(RAM_INI *ini,char *section,INI_SECTION **s_new)
{
	int result=FALSE;
	if(ini!=0 && section!=0 && section[0]!=0){
		INI_SECTION *s=0;
		s=malloc(sizeof(INI_SECTION));
		if(s){
			int len;
			memset(s,0,sizeof(INI_SECTION));
			len=strlen(section)+1;
			s->section=malloc(len);
			if(s->section){
				strncpy(s->section,section,len);
				result=TRUE;
			}
		}
		if(result){
			if(ini->sections==0)
				ini->sections=s;
			else{
				INI_SECTION *n=ini->sections;
				while(TRUE){
					if(n->next==0){
						n->next=s;
						s->prev=n;
						break;
					}else{
						n=n->next;
					}
				};
			}
			if(s_new)
				*s_new=s;
		}else{
			if(s)
				free(s);
		}
	}
	return result;
}
static int add_key(INI_SECTION *section,char *key,char *val,INI_KEY **k_new)
{
	int result=FALSE;
	if(section!=0 && section->section!=0 && key!=0 && key[0]!=0 && val!=0){
		INI_KEY *k=0;
		k=malloc(sizeof(INI_KEY));
		if(k){
			int len;
			memset(k,0,sizeof(INI_KEY));
			len=strlen(key)+1;
			k->key=malloc(len);
			if(k->key){
				strncpy(k->key,key,len);
				len=strlen(val)+1;
				k->str=malloc(len);
				if(k->str){
					strncpy(k->str,val,len);
					result=TRUE;
				}
			}
			if(!result){
				if(k->key)
					free(k->key);
				if(k->str)
					free(k->str);
				if(k)
					free(k);
			}
		}
		if(result){
			if(section->keys==0)
				section->keys=k;
			else{
				INI_KEY *keys=section->keys;
				while(TRUE){
					if(keys->next==0){
						keys->next=k;
						k->prev=keys;
						break;
					}else{
						keys=keys->next;
					}
				};
			}
			if(k_new)
				*k_new=k;
		}
	}
	return result;
}
static int delete_key(INI_SECTION *section,INI_KEY *key)
{
	int result=FALSE;
	if(section==0 || key==0)
		return result;
	if(section->keys){
		INI_KEY *p,*n;
		p=key->prev;
		n=key->next;
		if(p)
			p->next=n;
		if(n)
			n->prev=p;
		if(p==0)
			section->keys=n;
		if(key->str)
			free(key->str);
		if(key->key)
			free(key->key);
		free(key);
		result=TRUE;
	}
	return result;
}
static int delete_section(RAM_INI *ini,INI_SECTION *section)
{
	int result=FALSE;
	if(ini==0 || section==0)
		return result;
	if(ini->sections){
		INI_SECTION *s=ini->sections;
		while(s!=0){
			if(s->section!=0){
				if(stricmp(s->section,section->section)==0){
					INI_SECTION *p,*n;
					while(s->keys){
						if(!delete_key(s,s->keys))
							break;
					};
					p=s->prev;
					n=s->next;
					if(p)
						p->next=n;
					if(n)
						n->prev=p;
					if(p==0) //top node
						ini->sections=n;
					free(s->section);
					free(s);
					result=TRUE;
					break;
				}
			}
			s=s->next;
		};
	}
	return result;
}

int find_section(RAM_INI *ini,char *section,INI_SECTION **sfound)
{
	int result=FALSE;
	if(ini==0 || section==0 || section[0]==0)
		return result;
	if(ini->sections){
		INI_SECTION *s=ini->sections;
		while(s){
			if(s->section){
				if(stricmp(s->section,section)==0){
					if(sfound!=0){
						*sfound=s;
					}
					result=TRUE;
					break;
				}
			}
			s=s->next;
		};
	}
	return result;
}
int find_key(INI_SECTION *section,char *key,INI_KEY **kfound)
{
	int result=FALSE;
	if(section==0 || key==0 || key[0]==0)
		return result;
	if(section->keys){
		INI_KEY *k=section->keys;
		while(k){
			if(k->key){
				if(stricmp(k->key,key)==0){
					if(kfound!=0){
						*kfound=k;
					}
					result=TRUE;
					break;
				}
			}
			k=k->next;
		};
	}
	return result;
}

int write_private_profile_string(char *section,char *key,char *str,RAM_INI *ini)
{
	int result=FALSE;
	if(section!=0 && section[0]==0)
		return result;
	if(key!=0 && key[0]==0)
		return result;
	if(ini==0)
		return result;
	if(section){
		INI_SECTION *s=0;
		if(!find_section(ini,section,&s)){
			if(key)
				add_section(ini,section,&s);
		}
		if(s){
			if(key==0)
				result=delete_section(ini,s);
			else{
				INI_KEY *k=0;
				if(!find_key(s,key,&k)){
					if(str)
						result=add_key(s,key,str,&k);
				}
				if(k!=0 && result==FALSE){
					if(str==0)
						result=delete_key(s,k);
					else{
						int len;
						if(k->str)
							free(k->str);
						len=strlen(str)+1;
						k->str=malloc(len);
						if(k->str){
							strncpy(k->str,str,len);
							result=TRUE;
						}
					}
				}
			}
		}
	}
	return result;
}
int get_private_profile_string(char *section,char *key,char *def,char *str,int size,RAM_INI *ini)
{
	int result=0;
	if(section!=0 && section[0]==0)
		return result;
	if(key!=0 && key[0]==0)
		return result;
	if(str==0 || size<=0)
		return result;
	if(ini==0)
		return result;
	if(section){
		INI_SECTION *s=0;
		find_section(ini,section,&s);
		if(s){
			INI_KEY *k=0;
			find_key(s,key,&k);
			if(k){
				if(k->str){
					strncpy(str,k->str,size);
					str[size-1]=0;
					result=size;
				}
			}

		}
	}
	return result;
}
/*
int dump_ramini()
{
	INI_SECTION *s=ram_ini.sections;
	while(s){
		INI_KEY *k;
		printf("[%s]\n",s->section);
		k=s->keys;
		while(k){
			printf("%s=%s\n",k->key,k->str);
			k=k->next;
		};
		s=s->next;
	};
	printf("------\n");
	return 0;
}

int test_ramini()
{
	char section[40];
	char key[40];
	char val[40];
	int i,x=80,y=50;
	open_console();
	move_console(100,0);
	get_max_console(&x,&y);
	resize_console(120,y-2);
	for(i=0;i<5;i++){
		int j;
		_snprintf(section,sizeof(section),"S%04i",i);
		for(j=0;j<5;j++){
			_snprintf(key,sizeof(key),"K%04i",j);
			_snprintf(val,sizeof(val),"V%04i",j);
			write_private_profile_string(section,key,val,&ram_ini);
		}

	}
	dump_ramini();
	printf("test set\n");
	getch();
	for(i=0;i<5;i++){
		int j;
		_snprintf(section,sizeof(section),"S%04i",i);
		for(j=5-1;j>=0;j--){
			_snprintf(key,sizeof(key),"K%04i",j);
			write_private_profile_string(section,key,0,&ram_ini);
			dump_ramini();
			printf("deleted key %s\n",key);
			getch();
		}
	}
	for(i=5-1;i>=0;i--){
		_snprintf(section,sizeof(section),"S%04i",i);
		write_private_profile_string(section,0,0,&ram_ini);
		dump_ramini();
		printf("deleted section %s\n",section);
		getch();
	}
	printf("tests done\n");
	getch();
	exit(0);
	return 0;
}
*/