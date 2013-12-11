#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include "resource.h"


enum{
	CONTROL_ID,
	XPOS,YPOS,
	WIDTH,HEIGHT,
	HUG_L,
	HUG_R,
	HUG_T,
	HUG_B,
	HUG_CTRL_X,
	HUG_CTRL_Y,
	HUG_HEIGHT,
	HUG_WIDTH,
	HUG_CTRL_TXT_X,
	HUG_CTRL_TXT_Y,
	HUG_CTRL_TXT_X_,
	HUG_CTRL_TXT_Y_,
	SIZE_HEIGHT_OFF,
	SIZE_WIDTH_OFF,
	SIZE_HEIGHT_PER,
	SIZE_WIDTH_PER,
	SIZE_TEXT_CTRL,
	CONTROL_FINISH,
	RESIZE_FINISH
};

int process_anchor_list(HWND hwnd,short *list)
{
	int limit=9999;
	int i=0,j,x,y,width,height;
	HWND dlg_item;
	HDC	 hdc;
	RECT crect;
	SIZE text_size;
	char str[255];
	double f;
	int done=FALSE;
	int last_text=0;

	memset(&crect,0,sizeof(crect));
	hdc=GetDC(hwnd);
	GetClientRect(hwnd, &crect);
	do{
		switch(list[i]){
		case CONTROL_ID:
			x=y=width=height=0;
			dlg_item=GetDlgItem(hwnd,list[i+1]);
			if(dlg_item==NULL)
				done=TRUE;
			break;
		case XPOS:
			x+=list[i+1];
			break;
		case YPOS:
			y+=list[i+1];
			break;
		case WIDTH:
			width+=list[i+1];
			break;
		case HEIGHT:
			height+=list[i+1];
			break;
		case HUG_L:
			x+=crect.left+list[i+1];
			break;
		case HUG_R:
			x+=crect.right+list[i+1];
			break;
		case HUG_T:
			y+=crect.top+list[i+1];
			break;
		case HUG_B:
			y+=crect.bottom+list[i+1];
			break;
		case HUG_CTRL_X:
			break;
		case HUG_CTRL_Y:
			break;
		case HUG_HEIGHT:
			j=crect.bottom-crect.top;
			f=(double)list[i+1]/1000.0;
			y+=j*f;
			break;
		case HUG_WIDTH:
			j=crect.right-crect.left;
			f=(double)list[i+1]/1000.0;
			x+=j*f;
			break;
		case HUG_CTRL_TXT_X:
			if(last_text!=list[i+1]){
				GetDlgItemText(hwnd,list[i+1],str,sizeof(str)-1);
				GetTextExtentPoint32(hdc,str,strlen(str),&text_size);
				last_text=list[i+1];
			}
			x+=text_size.cx;
			break;
		case HUG_CTRL_TXT_X_:
			if(last_text!=list[i+1]){
				GetDlgItemText(hwnd,list[i+1],str,sizeof(str)-1);
				GetTextExtentPoint32(hdc,str,strlen(str),&text_size);
				last_text=list[i+1];
			}
			x-=text_size.cx;
			break;
		case HUG_CTRL_TXT_Y:
			if(last_text!=list[i+1]){
				GetDlgItemText(hwnd,list[i+1],str,sizeof(str)-1);
				GetTextExtentPoint32(hdc,str,strlen(str),&text_size);
				last_text=list[i+1];
			}
			y+=text_size.cy;
			break;
		case HUG_CTRL_TXT_Y_:
			if(last_text!=list[i+1]){
				GetDlgItemText(hwnd,list[i+1],str,sizeof(str)-1);
				GetTextExtentPoint32(hdc,str,strlen(str),&text_size);
				last_text=list[i+1];
			}
			y-=text_size.cy;
			break;
		case SIZE_HEIGHT_OFF:
			height+=crect.bottom-crect.top+list[i+1];
			break;
		case SIZE_WIDTH_OFF:
			width+=crect.right-crect.left+list[i+1];
			break;
		case SIZE_HEIGHT_PER:
			j=crect.bottom-crect.top;
			f=(double)list[i+1]/1000.0;
			height+=f*j;
			break;
		case SIZE_WIDTH_PER:
			j=crect.right-crect.left;
			f=(double)list[i+1]/1000.0;
			width+=f*j;
			break;
		case SIZE_TEXT_CTRL:
			if(last_text!=list[i+1]){
				GetDlgItemText(hwnd,list[i+1],str,sizeof(str)-1);
				GetTextExtentPoint32(hdc,str,strlen(str),&text_size);
				last_text=list[i+1];
			}
			width+=text_size.cx;
			height+=text_size.cy;
			break;
		case CONTROL_FINISH:
			SetWindowPos(dlg_item,NULL,x,y,width,height,SWP_NOZORDER);
			break;
		case RESIZE_FINISH:
			done=TRUE;
			break;
		default:
			printf("bad command %i\n",list[i]);
			break;
		}
		i+=2;
		if(i>limit)
			done=TRUE;
	}while(!done);
	ReleaseDC(hwnd,hdc);
	return TRUE;
}
short server_list_anchors[]={
	CONTROL_ID,IDC_ADD,
		HUG_R,-75,YPOS,0,
		WIDTH,75,HEIGHT,23,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDC_EDIT,
		HUG_R,-75,YPOS,34,
		WIDTH,75,HEIGHT,23,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDC_DELETE,
		HUG_R,-75,YPOS,73,
		WIDTH,75,HEIGHT,23,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDC_JOIN,
		HUG_R,-75,YPOS,111,
		WIDTH,75,HEIGHT,23,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDCANCEL,
		HUG_R,-75,YPOS,163,
		WIDTH,75,HEIGHT,23,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDC_LISTVIEW,
		XPOS,2,YPOS,2,
		SIZE_WIDTH_OFF,-85,SIZE_HEIGHT_OFF,-4,
		CONTROL_FINISH,-1,
	RESIZE_FINISH
};
short add_server_anchors[]={
	CONTROL_ID,IDC_NETWORK,
		XPOS,78,YPOS,0,
		SIZE_WIDTH_OFF,-78,HEIGHT,28,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDC_SERVER,
		XPOS,78,YPOS,29,
		SIZE_WIDTH_OFF,-78,HEIGHT,28,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDC_PORTS,
		XPOS,78,YPOS,59,
		SIZE_WIDTH_OFF,-78,HEIGHT,28,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDC_PASSWORD,
		XPOS,78,YPOS,124,
		SIZE_WIDTH_OFF,-78,HEIGHT,23,
		CONTROL_FINISH,-1,
	RESIZE_FINISH
};
short channel_list_anchors[]={
	CONTROL_ID,IDC_CHANNEL,
		XPOS,87,YPOS,3,
		SIZE_WIDTH_OFF,-87,HEIGHT,23,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDC_NETWORK,
		XPOS,87,YPOS,44,
		SIZE_WIDTH_OFF,-87,SIZE_HEIGHT_OFF,30,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDC_PASSWORD,
		XPOS,87,YPOS,98,
		SIZE_WIDTH_OFF,-87,HEIGHT,23,
		CONTROL_FINISH,-1,
	RESIZE_FINISH
};
short settings_list_anchors[]={
	CONTROL_ID,IDC_NICK,
		XPOS,63,YPOS,2,
		SIZE_WIDTH_OFF,-63,HEIGHT,23,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDC_QUIT_MSG,
		XPOS,65,YPOS,28,
		SIZE_WIDTH_OFF,-63,HEIGHT,23,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDC_USERNAME,
		XPOS,65,YPOS,60,
		SIZE_WIDTH_OFF,-63,HEIGHT,23,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDC_REALNAME,
		XPOS,65,YPOS,90,
		SIZE_WIDTH_OFF,-63,HEIGHT,23,
		CONTROL_FINISH,-1,
	RESIZE_FINISH
};
short chat_anchor_list[]={
	CONTROL_ID,MDI_STATIC, //static
		XPOS,0,YPOS,0,
		SIZE_WIDTH_OFF,-63,SIZE_HEIGHT_OFF,-25,
		CONTROL_FINISH,-1,
	CONTROL_ID,MDI_EDIT, //edit
		XPOS,0,HUG_B,-25,
		SIZE_WIDTH_OFF,0,HEIGHT,25,
		CONTROL_FINISH,-1,
	CONTROL_ID,MDI_LIST, //list
		HUG_R,-60,YPOS,0,
		WIDTH,60,SIZE_HEIGHT_OFF,-25,
		CONTROL_FINISH,-1,
	RESIZE_FINISH
};
short server_anchor_list[]={
	CONTROL_ID,MDI_STATIC, //static
		XPOS,0,YPOS,0,
		SIZE_WIDTH_OFF,0,SIZE_HEIGHT_OFF,-25,
		CONTROL_FINISH,-1,
	CONTROL_ID,MDI_EDIT, //edit
		XPOS,0,HUG_B,-25,
		SIZE_WIDTH_OFF,0,HEIGHT,25,
		CONTROL_FINISH,-1,
	/*
	CONTROL_ID,MDI_SCROLL, //scroll
		HUG_R,-80,YPOS,0,
		WIDTH,20,SIZE_HEIGHT_OFF,-25,
		CONTROL_FINISH,-1,
	*/
	RESIZE_FINISH
};
short chan_mode_list[]={
	CONTROL_ID,IDC_TOPIC,
		XPOS,0,YPOS,18,
		SIZE_WIDTH_OFF,0,HEIGHT,23, /*304 228*/
		CONTROL_FINISH,-1,
	CONTROL_ID,IDC_BANLIST,
		XPOS,0,YPOS,59,
		SIZE_WIDTH_OFF,0,SIZE_HEIGHT_OFF,-177, //,54, /*304 228*/
		CONTROL_FINISH,-1,

	CONTROL_ID,IDC_ONLYOPS,
        XPOS,0,HUG_B,-114, /*x.off=-300 y.off=-114*/
        WIDTH,108,HEIGHT,16, /*w.off=-192 h.off=-215*/
		CONTROL_FINISH,-1,
	CONTROL_ID,IDC_NOEXTERNAL,
		XPOS,0,HUG_B,-91,
		WIDTH,128,HEIGHT,16,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDC_INVITEONLY,
		XPOS,0,HUG_B,-68,
		WIDTH,72,HEIGHT,16,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDC_MODERATED,
		XPOS,0,HUG_B,-46,
		WIDTH,75,HEIGHT,16,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDOK,
		XPOS,0,HUG_B,-23,
		WIDTH,75,HEIGHT,23,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDC_REQUIRESKEY,
		XPOS,135,HUG_B,-112,
		WIDTH,42,HEIGHT,16,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDC_KEY,
		XPOS,174,HUG_B,-116,
		WIDTH,93,HEIGHT,23,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDC_LIMITTO,
		XPOS,135,HUG_B,-90,
		WIDTH,57,HEIGHT,16,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDC_USERS,
		XPOS,191,HUG_B,-91,
		WIDTH,60,HEIGHT,23,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDC_PRIVATE,
		XPOS,135,HUG_B,-67,
		WIDTH,57,HEIGHT,16,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDC_SECRET,
		XPOS,135,HUG_B,-44,
		WIDTH,56,HEIGHT,16,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDCANCEL,
		XPOS,168,HUG_B,-23,
		WIDTH,75,HEIGHT,23,
		CONTROL_FINISH,-1,
	CONTROL_ID,IDC_STATIC1,
		XPOS,257,HUG_B,-90,
		WIDTH,27,HEIGHT,13,
		CONTROL_FINISH,-1,

	RESIZE_FINISH
};
short install_dlg_anchors[]={
	CONTROL_ID,IDC_INSTALL_INFO,
			XPOS,0,YPOS,10,
			WIDTH,0,HEIGHT,23,
			SIZE_WIDTH_OFF,0,
			CONTROL_FINISH,-1,
	CONTROL_ID,IDC_TXT_LOCAL,
			XPOS,92,YPOS,49,
			WIDTH,0,HEIGHT,23,
			SIZE_WIDTH_OFF,-92,
			CONTROL_FINISH,-1,
	CONTROL_ID,IDC_TXT_APPDATA,
			XPOS,92,YPOS,85,
			WIDTH,0,HEIGHT,23,
			SIZE_WIDTH_OFF,-92,
			CONTROL_FINISH,-1,
	RESIZE_FINISH
};
int get_word(unsigned char *str,int num,char *out,int olen)
{
	int i,len,count=0,index=0;
	int start_found=FALSE;
	len=strlen(str);
	for(i=0;i<len;i++){
		if(str[i]==','){
			count++;
			if(count>num){
				out[index++]=0;
				break;
			}
		}
		if(!start_found){
			if(count==num && str[i]>' ' && str[i]!=',')
				start_found=TRUE;
		}
		if(start_found){
			if(str[i]<' ' || str[i]=='/'){
				out[index++]=0;
				break;
			}
			if(str[i]>' ')
				out[index++]=str[i];
			if(i==len-1){
				out[index++]=0;
				break;
			}
		}
		if(index>=olen-2){
			out[index++]=0;
			break;
		}
	}
	return TRUE;
}
int modify_list(short *list)
{
	FILE *f;
	char str[1024];
	char *tags[]={
	"CONTROL_ID",
	"XPOS","YPOS",
	"WIDTH","HEIGHT",
	"HUG_L",
	"HUG_R",
	"HUG_T",
	"HUG_B",
	"HUG_CTRL_X",
	"HUG_CTRL_Y",
	"HUG_HEIGHT",
	"HUG_WIDTH",
	"HUG_CTRL_TXT_X",
	"HUG_CTRL_TXT_Y",
	"HUG_CTRL_TXT_X_",
	"HUG_CTRL_TXT_Y_",
	"SIZE_HEIGHT_OFF",
	"SIZE_WIDTH_OFF",
	"SIZE_HEIGHT_PER",
	"SIZE_WIDTH_PER",
	"SIZE_TEXT_CTRL",
	"CONTROL_FINISH",
	"RESIZE_FINISH",
	0
	};
	if(list==0)
		return 0;
	f=fopen("rc.txt","rb");
	if(f!=0){
		int result=FALSE;
		int index=0;
		int done=FALSE;
		printf("-------start---------\n");
		do{
			char *s;
			int i,j;
			str[0]=0;
			result=fgets(str,sizeof(str),f);
			s=strstr(str,"/");
			if(s!=0)
				s[0]=0;
			for(i=0;i<10;i+=2){
				char word[40]={0};
				if(done)
					break;
				get_word(str,i,word,sizeof(word));
				if(strlen(word)==0)
					break;
				if(strlen(word)>0){
					for(j=0;j<100;j++){
						if(tags[j]==0)
							break;
						if(strcmp(word,tags[j])==0){
							int val;
							if(strcmp(word,"CONTROL_ID")==0){
								char w2[40]={0};
								get_word(str,i+1,w2,sizeof(w2));
								printf("%s,%s\n",word,w2);
								index+=2;
								break;
							}
							if(strcmp(word,"CONTROL_FINISH")==0){
								index+=2;
								break;
							}
							if(strcmp(word,"RESIZE_FINISH")==0){
								done=TRUE;
								break;
							}
							if(list[index]==RESIZE_FINISH){
								done=TRUE;
								break;
							}
							list[index++]=j;
							printf("%s,",word);
							word[0]=0;
							get_word(str,i+1,word,sizeof(word));
							printf("%s\n",word);
							if(strlen(word)>0){
								val=atoi(word);
								list[index++]=val;
							}
							else
								index=index;
						}
					}
				}
			}
			if(done)
				break;
		}while(result);
		fclose(f);
	}
	return TRUE;
}

int dump_sizes(HWND hwnd,short *IDC)
{
	int i;
	RECT client,win;
	GetClientRect(hwnd,&client);
	for(i=0;i<1000;i++){
		POINT p={0};
		int width,height;
		int cw,ch;
		if(IDC[i]==RESIZE_FINISH)
			break;
		if(IDC[i]==CONTROL_ID)
			i++;
		else{
			i++;
			continue;
		}
		printf("CONTROL_ID,%i,\n",IDC[i]);
		GetWindowRect(GetDlgItem(hwnd,IDC[i]),&win);
		p.x=win.left;
		p.y=win.top;
		ScreenToClient(hwnd,&p);
		width=win.right-win.left;
		height=win.bottom-win.top;
		cw=client.right-client.left;
		ch=client.bottom-client.top;
		printf("\tXPOS,%i,YPOS,%i, /*x.off=%i y.off=%i*/\n",
			p.x,p.y,
			-client.right+p.x,-client.bottom+p.y);
		printf("\tWIDTH,%i,HEIGHT,%i, /*w.off=%i h.off=%i*/\n\tCONTROL_FINISH,-1,\n",
			width,height,
			-cw+width,-ch+height);

	}
	printf("\n");return 0;
}
int set_list_width(int width)
{
	int i;
	for(i=0;i<sizeof(chat_anchor_list)/sizeof(short);i++){
		if(chat_anchor_list[i]==RESIZE_FINISH)
			break;
		if(chat_anchor_list[i]==CONTROL_ID && chat_anchor_list[i+1]==MDI_STATIC){
			chat_anchor_list[i+7]=-width-3;
		}
		if(chat_anchor_list[i]==CONTROL_ID && chat_anchor_list[i+1]==MDI_LIST){
			chat_anchor_list[i+3]=-width;
			chat_anchor_list[i+7]=width;
			break;
		}
	}
	return TRUE;
}
int main_dlg_anchors[]={

	RESIZE_FINISH

/*
	CONTROL_ID,IDC_SLIDER1,
				HUG_L,5,
				HUG_B,-35,
				SIZE_WIDTH_OFF,-25,HEIGHT,30,CONTROL_FINISH,-1,
	CONTROL_ID,IDCANCEL,
				XPOS,5,HUG_WIDTH,870,
				HUG_T,5,
				SIZE_WIDTH_PER,125,SIZE_HEIGHT_PER,166,CONTROL_FINISH,-1,
	CONTROL_ID,ID_SYNCTIME,
				XPOS,5,HUG_WIDTH,870,
				HUG_T,10,HUG_HEIGHT,166,
				SIZE_WIDTH_PER,125,SIZE_HEIGHT_PER,166,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_ONTOP1, XPOS,-95,HUG_WIDTH,870,HUG_T,5,SIZE_TEXT_CTRL,IDC_ONTOP1,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_FREEZE, XPOS,-95,HUG_WIDTH,870,HUG_T,22,SIZE_TEXT_CTRL,IDC_ONTOP1,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_UTC,    XPOS,-95,HUG_WIDTH,870,HUG_T,39,SIZE_TEXT_CTRL,IDC_ONTOP1,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_NTPSERVER,
				HUG_R,-5,HUG_CTRL_TXT_X_,IDC_NTPSERVER,
				HUG_HEIGHT,333,YPOS,14,
				SIZE_TEXT_CTRL,IDC_NTPSERVER,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_HELPSTATIC,HUG_L,15,HUG_B,-55,SIZE_TEXT_CTRL,IDC_HELPSTATIC,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_STATIC_CURRENT_TIME,
				HUG_L,15,
				HUG_HEIGHT,21,
				SIZE_TEXT_CTRL,IDC_STATIC_CURRENT_TIME,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_CTIME,  
				HUG_L,15,
				HUG_HEIGHT,21,HUG_CTRL_TXT_Y,IDC_STATIC_CURRENT_TIME,
				SIZE_TEXT_CTRL,IDC_CTIME,HEIGHT,10,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_STATIC_TARGET_TIME,
				HUG_WIDTH,333,
				HUG_HEIGHT,21,
				SIZE_TEXT_CTRL,IDC_STATIC_TARGET_TIME,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_TARGET,
				HUG_WIDTH,333,
				HUG_HEIGHT,21,HUG_CTRL_TXT_Y,IDC_STATIC_TARGET_TIME,
				SIZE_TEXT_CTRL,IDC_TARGET,HEIGHT,10,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_CLRHOUR,
				HUG_WIDTH,333,
				YPOS,10,HUG_HEIGHT,21,HUG_CTRL_TXT_Y,IDC_STATIC_TARGET_TIME,HUG_CTRL_TXT_Y,IDC_TARGET,
				WIDTH,20,HEIGHT,20,SIZE_TEXT_CTRL,IDC_CLRHOUR,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_CLRMIN,
				XPOS,20,HUG_WIDTH,333,HUG_CTRL_TXT_X,IDC_CLRHOUR,
				YPOS,10,HUG_HEIGHT,21,HUG_CTRL_TXT_Y,IDC_STATIC_TARGET_TIME,HUG_CTRL_TXT_Y,IDC_TARGET,
				WIDTH,20,HEIGHT,20,SIZE_TEXT_CTRL,IDC_CLRHOUR,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_CLRSEC,
				XPOS,40,HUG_WIDTH,333,HUG_CTRL_TXT_X,IDC_CLRHOUR,HUG_CTRL_TXT_X,IDC_CLRHOUR,
				YPOS,10,HUG_HEIGHT,21,HUG_CTRL_TXT_Y,IDC_STATIC_TARGET_TIME,HUG_CTRL_TXT_Y,IDC_TARGET,
				WIDTH,20,HEIGHT,20,SIZE_TEXT_CTRL,IDC_CLRHOUR,CONTROL_FINISH,-1,
	CONTROL_ID,IDC_LIST1,
				XPOS,20,HUG_WIDTH,500,HUG_HEIGHT,500,SIZE_WIDTH_PER,100,SIZE_HEIGHT_PER,100,

				CONTROL_FINISH,-1,

	RESIZE_FINISH
*/
};
int reposition_controls(HWND hwnd, short *list)
{
	RECT	rect;
	GetClientRect(hwnd, &rect);
//modify_list(list);
	process_anchor_list(hwnd,list);
	InvalidateRect(hwnd,&rect,TRUE);
	return TRUE;
}
int resize_install_dlg(HWND hwnd)
{
	return reposition_controls(hwnd,install_dlg_anchors);
}
#define GRIPPIE_SQUARE_SIZE 15
int create_grippy(HWND hwnd)
{
	RECT client_rect;
	GetClientRect(hwnd,&client_rect);
	
	return CreateWindow("Scrollbar",NULL,WS_CHILD|WS_VISIBLE|SBS_SIZEGRIP,
		client_rect.right-GRIPPIE_SQUARE_SIZE,
		client_rect.bottom-GRIPPIE_SQUARE_SIZE,
		GRIPPIE_SQUARE_SIZE,GRIPPIE_SQUARE_SIZE,
		hwnd,NULL,NULL,NULL);
}

int grippy_move(HWND hwnd,HWND grippy)
{
	RECT client_rect;
	GetClientRect(hwnd,&client_rect);
	if(grippy!=0)
	{
		SetWindowPos(grippy,NULL,
			client_rect.right-GRIPPIE_SQUARE_SIZE,
			client_rect.bottom-GRIPPIE_SQUARE_SIZE,
			GRIPPIE_SQUARE_SIZE,GRIPPIE_SQUARE_SIZE,
			SWP_NOZORDER|SWP_SHOWWINDOW);
	}
	return 0;
}