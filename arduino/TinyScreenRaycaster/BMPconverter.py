import os
from PIL import Image
import binascii

'''
Most of this code is from https://gist.github.com/BigglesZX/4016539

Determined that analysing the GIF to see if there are transparent update frames was implemented
incorrectly- full frame updates not detected, so for now we'll just start with the last frame,
if there's no transparency, the whole thing will be overwritten anyway. Seems to work fine.
'''

'''
Output is 8 bit by default, remove # below for 16 bit
'''

bitMode16=False
bitMode16=True
 
def processImage(path,out):
    '''
    Iterate the GIF, extracting each frame.
    '''
    
    im = Image.open(path)

    out.write('const uint16_t %s[64*64] = {' % os.path.basename(path).split('.')[0])
    try:
        #print "saving %s (%s) frame %d, %s %s" % (path, mode, i, im.size, im.tile)
        #new_frame.save('%s-%d.png' % (''.join(os.path.basename(path).split('.')[:-1]), i), 'PNG')
        
        #new_frame = Image.new('RGBA', im.size)
        
        data = list(im.convert('RGB').getdata())
        raw=[]
        if bitMode16:
            for j in range(64*64):
                out.write( hex((((((data[j][2]>>0)&0xF8)|((data[j][1]>>5)&0x07))<<8)|(((data[j][1]<<3)&0xE0)|((data[j][0]>>3)&0x1F)))) );
                if(j<64*64-1):
                    out.write(', ')
                else:
                    out.write('};\n\n')
                #if(j>1 and j%64 == 0):
                #    out.write('\n')
                    
        #out.write(bytearray(raw))
        
        '''
        do this last
        '''
        #im.seek(im.tell() + 1)
    except EOFError:
        pass
    #out.close()
 
def main():
    out = open('zOutput.txt','wb')
    for file in os.listdir('.'):
        if file.endswith('.png'):
            processImage(file,out)
    out.close()
    
 
if __name__ == "__main__":
    main()
