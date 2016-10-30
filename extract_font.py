#!/usr/bin/python

from PIL import Image
import glob, os

def ensure_dir(f):
    d = os.path.dirname(f)
    if not os.path.isdir(d):
        os.makedirs(d)
        
def isGap(image, x):
	pixels = image.load()
	width, height = image.size
	for y in range(0,height) :
			if pixels[x,y] == 1:
				return False
	return True

# create structure which describes expected gaps in different characters
# for default font, and any special cases
fontGaps = {
	"default":{ # for most fonts only the double quote is not continuous
		ord("\""):1
	},
	"sg05":{ # this font has no special cases - all characters are continuous
	},
	"bm_tube":{ # this font has plenty of special cases because of its weird design
		ord("\""):1,
		ord("#"):2,
		ord("*"):1,
		ord("4"):1,
		ord("H"):2,
		ord("K"):1,
		ord("h"):1,
		ord("k"):1			
	}
}

# used to substitute names for characters which are problematic in a filesystem
charSubstitutes = {"?":"QUESTION","!":"EXCLAMATION",".":"STOP","/":"FWDSLASH", "\\":"BACKSLASH", "`":"FWDAPOS"}

allImageFiles = glob.glob("*.png")
allImageFiles.sort()
for imageFilePath in allImageFiles:
	
	# derive a name and create a directory to export data
	fontImagePath, ext  = os.path.splitext(imageFilePath)
	fontName = fontImagePath
		
	# load information from image
	fontImage = Image.open(imageFilePath)
	fontImageInfo = fontImage.info
	fontPixels = fontImage.load()
	fontWidth, fontHeight = fontImage.size
	
	fontOffsets = []
	fontBytes = []
		
	bitCount = 0
	currentByte = 0
	
	asciiChar = 33
	charStart = 0
	gapCount = 0
	
	for x in range(0, fontWidth):
		
		# handle gap if present
		if isGap(fontImage,x):
			if x > 0 and isGap(fontImage, x - 1) : #previous column was also a gap
				if charStart == x: 
					charStart = x + 1
				continue
			else:
				gapCount += 1
			
		#figure out how many gaps to expect in this character
		if fontName in fontGaps: # check if this font is a special case
			gaps = fontGaps[fontName]
		else:
			gaps = fontGaps["default"]
			
		if asciiChar in gaps: # check if this character is a special case
			expectedGaps = gaps[asciiChar]
		else:
			expectedGaps = 0
		
		# if gaps indicate that character has ended, export character and reset
		if gapCount > expectedGaps:
										
			# handle overrun of ascii range if character matching by gaps failed
			if asciiChar >= 127 :
				print("Mismatch when extracting characters. Arrived at ASCII 127!")
				exit()
			else:
				# export the image
				charImage = fontImage.crop((charStart, 0, x, fontHeight))
				charName = chr(asciiChar)
				if charName in charSubstitutes:
					charName = charSubstitutes[charName]
				charFilePath = fontName + "/" + charName + ".png"
				ensure_dir(charFilePath)
				charImage.convert("RGBA").save(charFilePath)
				
				# print out the pixel information
				charPixels = charImage.load()
				charWidth, charHeight = charImage.size

				# record bit where this character started
				fontOffsets.append(bitCount)
								
				for charX in range(0,charWidth):
					for charY in range(0,charHeight):
						if charPixels[charX,charY] == 1:
							currentByte |= 1 << (bitCount % 8)
						bitCount += 1
						if bitCount % 8 == 0: # write next byte if complete
							fontBytes.append(currentByte)
							currentByte = 0	

				# reset state to handle next character
				asciiChar+= 1
				charStart = x + 1
				gapCount = 0
				
	# write remaining byte if partial byte remains
	if bitCount %8 != 0: 
		fontBytes.append(currentByte)

	# write final offset (column after last char)
	fontOffsets.append(bitCount)
		
	print( "// Font: " + fontName)	
	print( "// " + str(len(fontBytes)) + " bytes" )
	print( "byte fontHeight = " + str(fontHeight) + ";" )
	print( "byte fontBytes[] = {" + ",".join([str(byte) for byte in fontBytes]) + "};" )
	print( "int fontOffsets[] = {" + ",".join([str(offset) for offset in fontOffsets]) + "};" )
	print("")
