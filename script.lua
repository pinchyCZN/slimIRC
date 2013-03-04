-- external C functions available:
-- irc_cmd_msg (session,nch,msg)
-- irc_cmd_me (session,nch,msg)
-- irc_send_raw (session,str)
-- post_message (session,nch,msg)
-- send_privmsg (session,origin,mynick,msg,type)


-- lua functions that get called after certain events
 -- session=irc session pointer
 -- origin=full nick (billy!~test@example.com)
 -- nch=nick or channel
 -- msg=message body

-- function check_ignore(session,origin,nch,msg)
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
