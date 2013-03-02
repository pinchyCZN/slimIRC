function get_nick(s)
	i=string.find(s,"!")
	if(i)then
		nick=string.sub(s,0,i-1)
	else
		return s
	end
	return nick
end


function check_ignore(session,origin,nick,msg)
	if(string.find(origin,"billy!"))then
--		io.write("ignoring ",origin)
--		return 1
	else
		return 0
	end
	return 0
end

function privmsg_event(session,origin,nick,msg)
	io.write("privmsg_event\n")
	io.write(" origin=",origin," nick=",nick," msg=",msg,"\n")
	return 1
end


function channel_event(session,origin,channel,msg)
	--io.write("channel_event\n")

	--io.write(" origin=",origin," channel=",channel," msg=",msg,"\n")
	if(string.find(msg,"http") or string.find(msg,"pinch"))then
		--post_message(session,channel,"jesus is mah niggah")
		m="* "..get_nick(origin).." "..channel.." "..msg
		send_privmsg(session,"word_watch",origin,m,0)
	end
	if(string.find(msg,"xx"))then
--		post_message(session,channel,a)

	end
	return 1
end


function post_connect_event(session,origin,t1,t2)
	irc_send_raw(session,"playtrafficlog last")
end

io.write("file loaded\n")

