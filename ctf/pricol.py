import zipfile
import os
import base64
from PIL import Image

FP = "/media/p1d/ctf/I_love/"

# with ZipFile('/media/p1d/ctf/I_love.rar', 'r') as zf:
# while True:
# 	for v in os.listdir(FP):
# 		fp = FP+v
# 		if fp.endswith(".zip"):
# 			z = zipfile.ZipFile(fp, 'r')
# 			z.extractall(FP)
# 			z.close()
# 			os.remove(fp)

# for v in os.listdir(FP):
# 	if v.endswith(".txt"):
# 		with open(FP+v) as f:
# 			content = f.read().decode()
# 			raw = eval(content)
# 			c2 = base64.b64decode(raw)
# 			file = open(FP+v+".jpg","wb")
# 			file.write(c2)
# 			os.remove(FP+v)

final = Image.new("RGB", (800,1066), color=0)

for v in os.listdir(FP):
	if v.endswith(".jpg"):
		with Image.open(FP+v) as i:
			tags = i._getexif()
			mypos = eval((tags[37510])[20:-2])
			x = int(base64.b64decode(mypos)[3:])
			for y in range(0, 1066):
				final.putpixel((x,y), i.getpixel((0,y)))

final.save(FP+"image.png")