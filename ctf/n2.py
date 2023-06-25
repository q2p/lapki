from PIL import Image

final = Image.new("RGB", (102,102), color=0)

with open("./file.txt") as f:
	y = 0
	for l in f.readlines():
		sum = 0
		x = 0
		for p in l.split("+"):
			s = p.split("x")
			pc = int(s[0])
			len = int(s[1])
			for i in range(0, len):
				sum = sum + 1
				print(pc, end ="")
				final.putpixel((x,y), pc*255)
				x += 1
		print(" "+str(sum))
		y += 1

final.save("prec2.png")