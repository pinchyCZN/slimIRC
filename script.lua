
function check_ignore(session,origin,nick,msg)
	io.write("checking ignore list for:",origin,"\n")
	if(string.find(origin,"billy!"))then
		io.write("ignoring ",origin)
		return 1
	--elseif(string.find(origin,"u0"))then
	--	io.write("ignoring ",origin)
	--	return 1
	else
		return 0
	end
end

function privmsg_event(session,origin,nick,msg)
	io.write("privmsg_event\n")
	io.write(" origin=",origin," nick=",nick," msg=",msg,"\n")
	return 1
end

function channel_event(session,origin,channel,msg)
	process=true
	replace=false
	newstr="nope"
	io.write("channel_event\n")
	io.write(" origin=",origin," channel=",channel," msg=",msg,"\n")
	if(string.find(msg,"test"))then
		io.write("doing test\n")
		post_message(session,channel,"jesus is mah niggah")

	end
	if(string.find(msg,"xx"))then
		io.write("doing test\n")
		a="there was xx in "..msg
		post_message(session,channel,a)

	end
	return 1
end

io.write("file load\n")
