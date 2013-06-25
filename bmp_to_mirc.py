from PIL import Image
import sys
colortable=[
	0xFFFFFF, #0 white
	0x000000, #1 black
	0x7F0000, #2 blue (navy)
	0x009300, #3 green
	0x0000FF, #4 red
	0x00007F, #5 brown (maroon)
	0x9C009C, #6 purple
	0x007FFC, #7 orange (olive)
	0x00FFFF, #8 yellow
	0x00FC00, #9 light green (lime)
	0x939300, #10 teal (a green/blue cyan)
	0xFFFF00, #11 light cyan (cyan) (aqua)
	0xFC0000, #12 light blue (royal)
	0xFF00FF, #13 pink (light purple) (fuchsia)
	0x7F7F7F, #14 grey
	0xD2D2D2 #15 light grey (silver)
]

def get_closest(r,g,b):
    dist=10000
    closest=-1
    for index,c in enumerate(colortable):
        rv=c&0xFF
        gv=(c>>8)&0xFF
        bv=(c>>16)&0xFF
        rv=abs(r-rv)
        gv=abs(g-gv)
        bv=abs(b-bv)
        x=rv+gv+bv
        if(x<dist):
            dist=x
            closest=index
#    print closest
    return closest
im=Image.open("e:\\filexfer\\clip1.bmp")
im=im.transpose(Image.FLIP_LEFT_RIGHT)
im=im.rotate(90)
im=im.convert('RGB')
width,height=im.size

#print colortable
for x in range(width):
    last=-1
    for y in range(height):
        r,g,b=im.getpixel((x,y))
        k=get_closest(r,g,b)
        if(k!=last):
            sys.stdout.write("\3"+"1,"+str(k))
            last=k
        sys.stdout.write(" ")
        
    sys.stdout.write("\n")


for i in range(256):
    t=i
#    sys.stdout.write(chr(t))

