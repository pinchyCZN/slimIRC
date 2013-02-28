

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

a,b,c=privmsg_event('sjdfsdfjh ig45nore','567','234234')
print("a=",a," b=",b," c=",c)
