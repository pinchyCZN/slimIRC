/*
 * Copyright (C) 2004-2009 Georgy Yunaev gyunaev@ulduzsoft.com
 *
 * This example is free, and not covered by LGPL license. There is no 
 * restriction applied to their modification, redistribution, using and so on.
 * You can study them, modify them, use them in your own program - either 
 * completely or partially. By using it you may give me some credits in your
 * program, but you don't have to.
 *
 *
 * This example tests most features of libirc. It can join the specific
 * channel, welcoming all the people there, and react on some messages -
 * 'help', 'quit', 'dcc chat', 'dcc send', 'ctcp'. Also it can reply to 
 * CTCP requests, receive DCC files and accept DCC chats.
 *
 * Features used:
 * - nickname parsing;
 * - handling 'channel' event to track the messages;
 * - handling dcc and ctcp events;
 * - using internal ctcp rely procedure;
 * - generating channel messages;
 * - handling dcc send and dcc chat events;
 * - initiating dcc send and dcc chat.
 *
 * $Id: irctest.c 94 2012-01-18 08:04:49Z gyunaev $
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "libircclient.h"


static int debug_level=0;
int set_irc_debug_level(int i){debug_level=i;return 0;}
int get_irc_debug_level(void){return debug_level;}

int dprintf(int level,char *fmt,...)
{
	va_list args;
	if(level<debug_level){
		va_start(args,fmt);
		vprintf(fmt,args);
	}
	return 0;
}
void dump_event(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	char buf[512];
	unsigned int cnt;

	if(debug_level<2)
		return;
	buf[0] = '\0';

	for(cnt=0;cnt<count;cnt++)
	{
		if (cnt)
			strcat(buf,"|");
		_snprintf(buf,sizeof(buf),"%s%s",buf,params[cnt]);
	}
	buf[sizeof(buf)-1]=0;
	printf("Event \"%s\", origin: \"%s\", params: %d [%s]\n", event, origin ? origin : "NULL", cnt, buf);
}
int get_new_nick(char *nick,int size)
{
	static int count=0;
	_snprintf(nick,size,"u%02i",count++);
	return TRUE;
}
void event_quit(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{	//Event "QUIT", origin: "p123!234234@127.0.0.1", params: 1 [ertert]
	char nick[20];
	echo_server_window(session,"%s has quit %s %s",origin,
		(params[0]!=0 && params[0][0]!=0)?"saying":"",
		(params[0]!=0 && params[0][0]!=0)?params[0]:"");
	extract_nick(origin,nick,sizeof(nick));
	quit_irc_event(session,nick);
	dump_event(session,event,origin,params,count);
}
void event_part(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	part_channel_event(session,origin,params[0],count>1?params[1]:"");
	echo_server_window(session,"%s %s %s %s",event,origin,params[0],count>1?params[1]:"");
	dump_event(session,event,origin,params,count);
}
void event_nick(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	//Event "NICK", origin: "QWER!Car@my.server.name", params: 1 [qwer]
	char old[20]={0};
	extract_nick(origin,old,sizeof(old));
	update_nick(session,old,params[0]);
	echo_server_window(session,"%s %s %s",event,origin,params[0]);
	dump_event(session,event,origin,params,count);
}

void event_join(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	join_channel_event(session,origin,params[0]);
	echo_server_window(session,"%s %s %s",event,origin,params[0]);
	lua_process_event(session,event,origin,params,count);
	dump_event(session,event,origin,params,count);
}


void event_connect(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	dump_event(session,event,origin,params,count);
	update_chat_sessions(session);
	autojoin_channels(session);
	echo_server_window(session,"%s %s %s %s",event,origin,count>0?params[0]:"",count>1?params[1]:"");
	lua_process_event(session,"POST_CONNECT",origin,params,count);
}

void event_channel(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	if(count<2)
		return;
	if(is_lua_active(session))
		if(lua_process_event(session,"CHECKIGNORE",origin,params,count))
			return;
	if(params[0][0]=='#')
		channel_msg_event(session,origin,params[0],params[1],0);
	else
		privmsg_event(session,origin,params[0],params[1],0);
	lua_process_event(session,event,origin,params,count);
	dprintf(1,"'%s' said in channel %s: %s\n",origin?origin:"someone",params[0],params[1]);
}
void event_privmsg(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	if(is_lua_active(session))
		if(lua_process_event(session,"CHECKIGNORE",origin,params,count))
			return;
	privmsg_event(session,origin,params[0],params[1],0);
	lua_process_event(session,event,origin,params,count);
	dprintf(1,"PRIVMSG '%s' said me (%s): %s\n", 
		origin ? origin : "someone",
		params[0], params[1] );
}

void dcc_event_callback(irc_session_t * session, irc_dcc_t id, int status, void * ctx, const char * data, unsigned int length)
{
	dcc_callback(session,status,ctx,data,length);
}
void irc_event_dcc_chat(irc_session_t * session, const char * nick, const char * addr, irc_dcc_t dccid)
{
	void *ctx=0;
	dprintf(0,"DCC chat [%d] requested from '%s' (%s)\n", dccid, nick, addr);
	echo_server_window(session,"DCC chat [%d] requested from '%s' (%s)", dccid, nick, addr);
	if(is_lua_active(session)){
		const char *params[2]={"DCC","ACCEPT"};
		if(lua_process_event(session,"CHECKIGNORE",nick,params,2)){
			irc_dcc_decline(session,dccid);
			return;
		}
	}
	if(dcc_chat_request(session,dccid,nick,addr,&ctx)){
		if(0!=irc_dcc_accept(session,dccid,ctx,dcc_event_callback)){
			echo_server_window(session,"DCC accept failed error:%i",irc_errno(session));
			add_line_mdi_nolog(ctx,"DCC accept failed error");
		}
	}
}
void irc_event_dcc_send(irc_session_t * session, const char * nick, const char * addr, const char * filename, unsigned long size, irc_dcc_t dccid)
{
	dprintf(0,"DCC send [%d] of %s requested from '%s' (%s)\n",dccid,filename,nick,addr);
	echo_server_window(session,"DCC send [%d] of %s requested from '%s' (%s)",dccid,filename,nick,addr);
	irc_dcc_decline(session,dccid);
}

void event_topic(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	char nick[20]={0};
	extract_nick(origin,nick,sizeof(nick));
	//[channel:topic]
	echo_server_window(session,"TOPIC %s %s",params[0],params[1]);
	update_channel_topic(session,nick,params[0],params[1],TRUE);
}
void event_kick(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	IRC_WINDOW *win;
	win=find_channel_window(session,params[0]);
	if(win!=0){
		char str[80],nick[20];
		extract_nick(origin,nick,sizeof(nick));
		_snprintf(str,sizeof(str),"* %s kicked %s from channel %s %s",nick,params[1],params[0],
			count>2?params[2]:"");
		add_line_mdi(win,str);
		update_nick_in_list(win->hlist,params[1],0);
	}
	dump_event(session,event,origin,params,count);
}
void event_numeric (irc_session_t * session, unsigned int event, const char * origin, const char ** params, unsigned int count)
{
	IRC_WINDOW *win=0;
	char buf[24];
	sprintf(buf, "%d", event);
	dump_event(session, buf, origin, params, count);
	echo_server_window(session,"%s %s %s %s %s %s %s",buf,
		count>1?params[1]:"",count>2?params[2]:"",count>3?params[3]:"",count>4?params[4]:"",count>5?params[5]:"",
		count>6?params[6]:"");
	lua_process_event(session,"NUMERIC",buf,params,count);
	switch(event){
//	case 461: //need more parameters
//		break;
	case 317: //nick seconds signon :info
		{
			time_t timer;
			int seconds;
			int d,h,m,s;
			seconds=atoi(params[2]);
			d=seconds/60/60/24;
			h=(seconds/60/60)-(d*24);
			m=(seconds/60)-(d*60*24)-(h*60);
			s=seconds%60;
			echo_server_window(session,"%s has been idle for %i days,%i hours,%i minutes,%i seconds",params[1],d,h,m,s);
			timer=atoi(params[3]);
			echo_server_window(session,"signon %s",asctime(localtime(&timer)));
		}
		break;
	case 324: //returns mode is set
		update_chan_mode_dlg(event,origin,params,count);
		break;
	case 331: //no topic set
		update_chan_mode_dlg(event,origin,params,count);
		break;
	case 332: //[nick:channel:topic]
		update_chan_mode_dlg(event,origin,params,count);
		update_channel_topic(session,params[0],params[1],params[2],FALSE);
		break;
	case 333: //topic set at time [channel nickname time]
		break;
	case 353://list all names on channel
		if(count>=4){
			list_names_event(session,params[2],params[3]);
		}
		break;
	case 366: //end of names
		break;
	case 367: //ban entry
		update_chan_mode_dlg(event,origin,params,count);
		break;
	case 474: //cannot join channel (ban)
		if(count>=3)
			win=find_channel_window(session,params[1]);
		if(win!=0){
			char str[255];
			_snprintf(str,sizeof(str),"* %s",params[2]);
			add_line_mdi(win,str);
		}
		break;
	case 401: //no such nick channel (netsplit)
	case 404: //cant send to channel (kick/ban)
		if(count>=3)
			win=find_channel_window(session,params[1]);
		if(win!=0){
			char str[255];
			_snprintf(str,sizeof(str),"* %s",params[2]);
			irc_cmd_join(session,win->channel,win->password);
			add_line_mdi(win,str);
		}
		break;
	case 432: //erroneous nick
	case 433: //nick allready used
		{
			char nick[20]={0};
			if(strcmp(params[0],"*")==0){
				get_new_nick(nick,sizeof(nick));
				irc_send_raw(session,"NICK %s",nick);
				update_user_nick(session,nick);
			}
		}
		break;
	default:
		break;
	}

}
void event_ctcp_action(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	dump_event(session,event,origin,params,count);
	if(strnicmp(event,"ACTION",6)==0){
		if(is_lua_active(session))
			if(lua_process_event(session,"CHECKIGNORE",origin,params,count))
				return;
		if(params[0][0]=='#')
			channel_msg_event(session,origin,params[0],params[1],'N');
		else
			privmsg_event(session,origin,params[0],params[1],'N');
	}
}

void event_mode(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	IRC_WINDOW *win=0;
	unsigned int i,echo=TRUE;
	char nick[20]={0};
	char str[255]={0};
	dump_event(session,event,origin,params,count);
	if(is_lua_active(session))
		if(lua_process_event(session,"CHECKIGNORE",origin,params,count))
			echo=FALSE;
	if(echo){
		_snprintf(str,sizeof(str),"%s %s",event,origin);
		for(i=0;i<count;i++)
			_snprintf(str,sizeof(str),"%s %s",str,params[i]);
		echo_server_window(session,str);
	}
	extract_nick(origin,nick,sizeof(nick));
	if(count>=2){
		win=find_channel_window(session,params[0]);
		if(win!=0){
			if(echo){
				_snprintf(str,sizeof(str),"* %s sets mode",nick);
				for(i=0;i<count;i++)
					_snprintf(str,sizeof(str),"%s %s",str,params[i]);
				add_line_mdi(win,str);
			}
			if((strstri(params[1],"v")!=0) || (strstri(params[1],"o")!=0)){
				if(strstr(params[1],"-")!=0){
					SendMessage(win->hlist,LB_RESETCONTENT,0,0);
					irc_cmd_names(win->session,win->channel);
				}
				else
					update_nick_list(win,params,count);
			}
		}
	}
	//Event "MODE", origin: "p123!~234324@127.0.0.1", params: 3 [#1|+o|u05]
}
void event_notice(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
	IRC_WINDOW *win=0;
	char nick[20]={0};
	char str[768];
	unsigned int i;
	dump_event(session,event,origin,params,count);
	_snprintf(str,sizeof(str),"%s %s",event,origin);
	for(i=0;i<count;i++)
		_snprintf(str,sizeof(str),"%s %s",str,params[i]);
	echo_server_window(session,str);
	extract_nick(origin,nick,sizeof(nick));
	win=find_msg_window(session,nick);
	if(win!=0){
		_snprintf(str,sizeof(str),"<%s>",nick);
		for(i=0;i<count;i++)
			_snprintf(str,sizeof(str),"%s %s",str,params[i]);
		str[sizeof(str)-1]=0;
		add_line_mdi(win,str);
	}
}
void report_server_window(irc_session_t *session,const char * event,const char *origin,const char **params,unsigned int count)
{
	unsigned int i;
	char str[768];
	dump_event(session,event,origin,params,count);
	_snprintf(str,sizeof(str),"%s %s",event,origin);
	for(i=0;i<count;i++){
		_snprintf(str,sizeof(str),"%s %s",str,params[i]);
	}
	str[sizeof(str)-1]=0;
	echo_server_window(session,"%s",str);
}
int irc_slap(irc_session_t *session,char *channel,char *nick)
{
	extern int lua_script_enable;
	char slap[256]={0};
	char msg[256]={0};
	IRC_WINDOW *win;
	static int rnd=0,donce=TRUE;
	static char *randoms[]={
	/*0*/"around with higgs bosons made from north korean missile research",
	/*1*/"around with the famous purple stuffed worm in flap-jaw space",
	/*2*/"around with the wheres veeeeeeeeeeebeeeeeeeeeeeeeeeeeeeeee",
	/*3*/"around with real dolls assembled in china",
	/*4*/"around with tertiary parameters",
	/*5*/"up and down the sidewalk talking like a mofo",
	/*6*/"with emo kids from the new world order",
	/*7*/"around with some plastic rings from Great Pacific Garbage Patch",
	/*8*/"around with DRM laden ubisoft games",
	/*9*/"around with big ol sack of pride n joy dog food: http://www.pridenjoy.net/food.html",
	/*0*/"around with partially solidified tar from the le brea tar pits",
	/*1*/"around with the chared remains of celine dions gardener that got caught in an organic grass mulching experiment gone wrong",
	/*2*/"around with partially torn 3DO game boxes",
	/*3*/"around with a hardback copy of The Pragmatic Brogrammer",
	/*4*/"around with 4 chan greeting cards",
	/*5*/"around with vhs tv recordings of tales from the crypt",
	/*6*/"around with one of dem big ol trouts yeeeehaawww",
	};
	static char prev[sizeof(randoms)/sizeof(char *)];
	if(donce){
		srand(GetTickCount());
		donce=FALSE;
	}
	win=find_channel_window(session,channel);
	get_ini_str("SETTINGS","SLAPMSG",msg,sizeof(msg));
	if(msg[0]!=0)
		_snprintf(slap,sizeof(slap),"slaps %s %s",nick,msg);
	else if(lua_script_enable){
		char *params[3]={channel,"GET_SLAP_COUNT",(char*)win};
		int count=0;
		if(is_lua_active(session))
			count=lua_process_event(session,"USER_CALLED",nick,&params,3);
		if(count>=1){
			int i,index,used=0;
			char list[255];
			char tmp[255];
			memset(list,0,sizeof(list));
			get_ini_str("SETTINGS","SLAPPED",list,sizeof(list));
			if(count>=sizeof(list))
				count=sizeof(list)-1;
			for(i=0;i<count;i++){
				if(list[i]=='1')
					used++;
				else if(list[i]!='0')
					list[i]='0';
			}
			list[count]=0;
			if(used>=count){
				memset(list,'0',sizeof(list));
				list[count]=0;
			}
			memset(tmp,0,sizeof(tmp));
			index=0;
			for(i=0;i<count;i++){
				if(list[i]=='0')
					tmp[index++]=i+1;
			}
			if(index<=0)
				index=count;
			if(index==0)
				index=1;
			rnd=rand()%index;
			index=(unsigned char)tmp[rnd];
			index--;
			if(index>=count)
				index=count-1;
			if(index<0)
				index=0;
			_snprintf(tmp,sizeof(tmp),"DO_SLAP %i",index);
			params[0]=channel;
			params[1]=tmp;
			lua_process_event(session,"USER_CALLED",nick,&params,3);
			list[index]='1';
			list[count]=0;
			write_ini_str("SETTINGS","SLAPPED",list);
			return TRUE;
		}
		else{
			goto USUAL;
		}
	}
	else{
		int i,index,count;
		char list[1+sizeof(prev)];
USUAL:
		count=0;
		memset(list,0,sizeof(list));
		get_ini_str("SETTINGS","SLAPPED",list,sizeof(list));
		if(list[0]!=0){
			for(i=0;i<sizeof(prev);i++){
				if(list[i]=='1')
					prev[i]=1;
				else if(list[i]==0)
					break;
			}
		}
		for(i=0;i<sizeof(prev);i++){
			if(prev[i]!=0)
				count++;
		}
		if(count>=sizeof(prev)){
			memset(prev,0,sizeof(prev));
			dprintf(1,"prev reset\n");
		}

		memset(list,0,sizeof(list));
		for(i=0,index=0;i<sizeof(prev);i++){
			if(prev[i]==0)
				list[index++]=i+1;
		}
		if(index==0){
			dprintf(1,"index is zero\n");
			index=sizeof(prev);
		}
		dprintf(1,"count=%i\n",index);
		rnd=rand();
		rnd=rnd%index;
		index=(unsigned char)list[rnd];
		index--;
		if(index<0 || index>=(sizeof(randoms)/sizeof(char *))){
			index=0;
			dprintf(1,"index out of range\n");
		}
		dprintf(1,"index=%i\n",index);
		_snprintf(slap,sizeof(slap),"slaps %s %s",nick,randoms[index]);
		prev[index]=1;
		for(i=0;i<sizeof(prev);i++){
			if(prev[i]!=0)
				list[i]='1';
			else
				list[i]='0';
		}
		list[i]=0;
		write_ini_str("SETTINGS","SLAPPED",list);
	}
	irc_cmd_me(session,channel,slap);
	if(win!=0){
		char str[sizeof(slap)+40];
		_snprintf(str,sizeof(str),"* %s %s",win->nick,slap);
		add_line_mdi(win,str);
	}
	return TRUE;
}
int create_session(irc_callbacks_t *callbacks)
{
	irc_session_t *s=0;
	if(callbacks==0)
		return 0;
	memset(callbacks, 0, sizeof(irc_callbacks_t));

	callbacks->event_connect = event_connect;
	callbacks->event_join = event_join;
	callbacks->event_nick = event_nick;
	callbacks->event_quit = event_quit;
	callbacks->event_part = event_part;
	callbacks->event_mode = event_mode;
	callbacks->event_topic = event_topic;
	callbacks->event_kick = event_kick;
	callbacks->event_channel = event_channel;
	callbacks->event_channel_notice = event_ctcp_action;
	callbacks->event_privmsg = event_privmsg;
	callbacks->event_notice = event_notice;
	callbacks->event_invite = report_server_window;
	callbacks->event_umode = report_server_window;
	callbacks->event_ctcp_rep = report_server_window;
	//callbacks->event_ctcp_req = libirc_event_ctcp_internal
	callbacks->event_ctcp_action = event_ctcp_action;
	callbacks->event_unknown = report_server_window;
	callbacks->event_numeric = event_numeric;
	
	callbacks->event_dcc_chat_req = irc_event_dcc_chat;
	callbacks->event_dcc_send_req = irc_event_dcc_send;
	s=irc_create_session(callbacks);
	if(!s)
		return 0;
	else
		return (int)s;
}
int irc_connect_run(irc_session_t *s,char *server,int port,char *nick,char *password,char *user)
{
	char real_name[80]={0},user_name[80]={0},pw[40]={0};
	char *srv=server;
	int (*irc_connect_ptr) (irc_session_t * session,
				const char * server, 
				unsigned short port,
				const char * server_password,
				const char * nick,
				const char * username,
				const char * realname)=irc_connect;

	if(strnicmp(srv,"IPV6:",sizeof("IPV6:")-1)==0){
		irc_connect_ptr=irc_connect6;
		srv+=sizeof("IPV6:")-1;
	}
#if defined (ENABLE_DEBUG)
	irc_option_set(s,LIBIRC_OPTION_DEBUG);
#endif
	// To handle the "SSL certificate verify failed" from command line we allow passing ## in front 
	// of the server name, and in this case tell libircclient not to verify the cert
	if(srv[0]=='#' && srv[1]=='#')
	{
		irc_option_set(s,LIBIRC_OPTION_SSL_NO_VERIFY);
		srv=srv+1;
	}
	get_ini_str("SETTINGS","user_name",user_name,sizeof(user_name));
	if(user[0]!=0){
		strncpy(user_name,user,sizeof(user_name));
		user_name[sizeof(user_name)-1]=0;
	}
	strncpy(pw,password,sizeof(pw));
	pw[sizeof(pw)-1]=0;

	get_ini_str("SETTINGS","real_name",real_name,sizeof(real_name));
	if(irc_connect_ptr(s,srv,port,
		pw[0]==0?0:pw,
		nick,
		user_name[0]==0?0:user_name,
		real_name[0]==0?0:real_name))
	{
		dprintf(0,"Could not connect:%s\n",irc_strerror(irc_errno(s)));
		return FALSE;
	}
	irc_run(s);
	return TRUE;
}
int find_server_thread(char *network,char *server)
{
	int i;
	SERVER_THREAD *thread=0;
	for(i=0;i<sizeof(server_threads)/sizeof(SERVER_THREAD);i++){
		if(server_threads[i].server[0]!=0 && server_threads[i].network[0]!=0){
			thread=&server_threads[i];
			if(stricmp(thread->server,server)==0 && stricmp(thread->network,network)==0)
				return thread;
		}
	}
	return 0;
}
int acquire_network_thread(char *network,char *server,int port,char *password,char *user,char *nick)
{
	int i;
	SERVER_THREAD *thread=0;
	for(i=0;i<sizeof(server_threads)/sizeof(SERVER_THREAD);i++){
		if(server_threads[i].network[0]!=0)
			if(stricmp(server_threads[i].network,network)==0){
				thread=&server_threads[i];
				thread->port=port;
				strncpy(thread->network,network,sizeof(thread->network));
				strncpy(thread->server,server,sizeof(thread->server));
				strncpy(thread->password,password,sizeof(thread->password));
				strncpy(thread->user,user,sizeof(thread->user));
				strncpy(thread->nick,nick,sizeof(thread->nick));
				return thread;
			}
	}
	for(i=0;i<sizeof(server_threads)/sizeof(SERVER_THREAD);i++){
		if(server_threads[i].server[0]==0 && server_threads[i].network[0]==0){
			thread=&server_threads[i];
			thread->port=port;
			strncpy(thread->network,network,sizeof(thread->network));
			strncpy(thread->server,server,sizeof(thread->server));
			strncpy(thread->password,password,sizeof(thread->password));
			strncpy(thread->user,user,sizeof(thread->user));
			strncpy(thread->nick,nick,sizeof(thread->nick));
			return thread;
		}
	}
	return 0;
}
int disconnect_all_threads()
{
	int i;
	for(i=0;i<sizeof(server_threads)/sizeof(SERVER_THREAD);i++){
		if(server_threads[i].session!=0)
			if(!server_threads[i].disconnected)
				irc_disconnect(server_threads[i].session);
	}
	return TRUE;
}
int erase_server_thread(SERVER_THREAD *thread)
{
	memset(thread,0,sizeof(SERVER_THREAD));
	return TRUE;
}