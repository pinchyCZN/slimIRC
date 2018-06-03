from PIL import Image
import sys
import os
import math

colortable=[
	0xFFFFFF, #0 white
	0x000000, #1 black
	0x00007F, #2 blue (navy)
	0x009300, #3 green
	0xFF0000, #4 red
	0x7F0000, #5 brown (maroon)
	0x9C009C, #6 purple
	0xFC7F00, #7 orange (olive)
	0xFFFF00, #8 yellow
	0x00FC00, #9 light green (lime)
	0x009393, #10 teal (a green/blue cyan)
	0x00FFFF, #11 light cyan (cyan) (aqua)
	0x0000FC, #12 light blue (royal)
	0xFF00FF, #13 pink (light purple) (fuchsia)
	0x7F7F7F, #14 grey
	0xD2D2D2, #15 light grey (silver)
	0x470000, #extended colors http://anti.teamidiot.de/static/nei/*/extended_mirc_color_proposal.html
	0x472100,
	0x474700,
	0x324700,
	0x004700,
	0x00472C,
	0x004747,
	0x002747,
	0x000047,
	0x2E0047,
	0x470047,
	0x47002A,
	0x740000,
	0x743A00,
	0x747400,
	0x517400,
	0x007400,
	0x007449,
	0x007474,
	0x004074,
	0x000074,
	0x4B0074,
	0x740074,
	0x740045,
	0xB50000,
	0xB56300,
	0xB5B500,
	0x7DB500,
	0x00B500,
	0x00B571,
	0x00B5B5,
	0x0063B5,
	0x0000B5,
	0x7500B5,
	0xB500B5,
	0xB5006B,
	0xFF0000,
	0xFF8C00,
	0xFFFF00,
	0xB2FF00,
	0x00FF00,
	0x00FFA0,
	0x00FFFF,
	0x008CFF,
	0x0000FF,
	0xA500FF,
	0xFF00FF,
	0xFF0098,
	0xFF5959,
	0xFFB459,
	0xFFFF71,
	0xCFFF60,
	0x6FFF6F,
	0x65FFC9,
	0x6DFFFF,
	0x59B4FF,
	0x5959FF,
	0xC459FF,
	0xFF66FF,
	0xFF59BC,
	0xFF9C9C,
	0xFFD39C,
	0xFFFF9C,
	0xE2FF9C,
	0x9CFF9C,
	0x9CFFDB,
	0x9CFFFF,
	0x9CD3FF,
	0x9C9CFF,
	0xDC9CFF,
	0xFF9CFF,
	0xFF94D3,
	0x000000,
	0x131313,
	0x282828,
	0x363636,
	0x4D4D4D,
	0x656565,
	0x818181,
	0x9F9F9F,
	0xBCBCBC,
	0xE2E2E2,
	0xFFFFFF
]

def get_distance(r,g,b,c):
    r1=(c>>16)&0xFF;
    g1=(c>>8)&0xFF;
    b1=c&0xFF;
    d=math.sqrt(((b1-b)**2)+((g1-g)**2)+((r1-r)**2));
    return d;
  
def get_closest(r,g,b):
    dist=10000
    closest=-1
    dist=[]
    for index,c in enumerate(colortable):
        v=get_distance(r,g,b,c)
        dist.append([v,index])
    dist.sort(key = lambda x:x[0])
    closest=dist[0][1]
    #print("FF:",r,g,b,closest)
    #print("close",closest)
    return closest

fname="B:\\Clipboard01.bmp"
#for files in os.listdir("."):
#    if files.endswith(".bmp"):
#        fname=files
#        break

if not os.path.isfile(fname):
    print("cant find file"+fname)
    sys.exit(0)

print("using fname:"+fname)
im=Image.open(fname)
#im=im.transpose(Image.FLIP_LEFT_RIGHT)
#im=im.rotate(90)
im=im.convert('RGB')
width,height=im.size
print("width="+str(width)+" height="+str(height))
#print colortable

max_line=0
f=open("b:\\out.txt","w+");
for y in range(height):
    last=-1
    length=0
    for x in range(width):
        r,g,b=im.getpixel((x,y))
        k=get_closest(r,g,b)
        if(k!=last):
            s="\3"+"0,"+str(k);
            #sys.stdout.write(s)
            f.write(s);
            last=k
            length+=len(s)
        #sys.stdout.write(" ")
        f.write(" ");
        length+=1
        
    #sys.stdout.write("\n")
    f.write("\n");
    if(length>max_line):
        max_line=length

f.close();
print("maxlen=",max_line)
print("done");


