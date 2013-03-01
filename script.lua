

function privmsg_event(origin,nick,msg)
	process=true;
	replace=false;
	newstr="nope";
	io.write("privmsg_event\n");
	io.write(" origin=",origin," nick=",nick," msg=",msg,"\n");
	if(string.find(msg,"ignore"))then
		io.write("ignoring\n");
		process=false;
	end
	return process,replace,newstr
	--return 1,2,3
end

function channel_event(origin,channel,msg)
	process=true;
	replace=false;
	newstr="nope";
	io.write("channel_event\n");
	io.write(" origin=",origin," channel=",channel," msg=",msg,"\n");
	if(string.find(origin,"621366"))then
		io.write("ignoring\n");
		process=false;
	end
	return process,replace,newstr
	--return 1,2,3
end

io.write("file load\n");
