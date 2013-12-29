-- external C functions available:
-- irc_cmd_msg (session,nch,msg)
-- irc_cmd_me (session,nch,msg)
-- irc_send_raw (session,str)
-- irc_cmd_ctcp_reply (session,nick,reply)
-- post_message (session,nch,msg)
-- send_privmsg (session,origin,mynick,msg,type)
-- find_channel_window (session,channel)
-- add_line_mdi (win,str)
-- get_win_linecount (win)
-- get_win_line (win,line)


-- lua functions that get called after certain events
 -- session=irc session pointer
 -- origin=full nick (billy!~test@example.com)
 -- nch=nick or channel
 -- msg=message body

-- function check_ignore(session,origin,nch,msg)
-- end

-- function ctcp_event(session,origin,nch,msg)
-- end

-- function privmsg_event(session,origin,nch,msg)
-- end

-- function channel_event(session,origin,nch,msg)
-- end

-- function post_connect_event(session,origin,nch,msg)
-- end

-- function join_event(session,origin,nch,msg)
-- end

-- function numeric_event(session,origin,nch,msg)
-- end

-- function user_called_event(session,origin,nch,msg)
-- slap menu calls this with msg GET_SLAP_COUNT and DO_SLAP rnd#
-- end
