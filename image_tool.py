import json
import numpy as np
from PIL import Image

def rgb565_to_image(data, w, h):
    data = np.asarray(data, dtype=np.uint16).reshape((h, w))

    r = (data >> 11) & 0x1F
    g = (data >> 5)  & 0x3F
    b = data & 0x1F

    # espansione a 8 bit
    r = (r * 255 // 31).astype(np.uint8)
    g = (g * 255 // 63).astype(np.uint8)
    b = (b * 255 // 31).astype(np.uint8)

    rgb = np.dstack((r, g, b))
    return Image.fromarray(rgb, 'RGB')

def image_to_rgb565_bytes(img):
    img = img.convert('RGB')
    arr = np.array(img, dtype=np.uint8)

    outbytes = []
    for line in arr:
        for pixel in line:
            r,g,b = int(pixel[0]),int(pixel[1]),int(pixel[2])
            rgb565 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
            
            outbytes.append( (rgb565 >> 8) & 0xFF )
            outbytes.append( rgb565 & 0xFF )
            
            

    return outbytes


# esempio



if True:
    img = Image.open("header.bmp")
    name = "header"
    _bytes = [ format(b,"02X") for b in image_to_rgb565_bytes(img)]
    textout = f"const unsigned char {name}[{len(_bytes)}] = {{\n"
    cnt = 0
    for _byte in _bytes:
        textout += f"0X{_byte},"
        cnt += 1
        if cnt == 16:
            textout += "\n"
            cnt = 0
    if textout[-1] != "\n": textout += "\n};"
    else: textout += "};"
    open(f"{name}.h","w").write(textout)
    exit(0)
    print( len( data) )

    data = json.loads(open("test.json").read())["image"]
    i = 0
    l = len(data)
    raw = []
    while i < l:
        chunk = (int(data[i+1],16)) + (int(data[i],16) << 8)
        raw.append(chunk)
        i += 2
    print(l)
    print(len(raw))
    img = rgb565_to_image(raw, 248, 245)
    img.show()
    img.save("out2.png")
