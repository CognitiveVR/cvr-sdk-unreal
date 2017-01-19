import bpy
import mathutils
import math
import bmesh
import sys
import os
import shutil

#arguments
scene = bpy.context.scene
ops = bpy.ops
args = sys.argv
exportPath = args[3]
sizeFactor = int(args[4])
ignore = int(args[5])
if (len(args))<7:
 args.append('')
fileName = args[6]


#remove bmp files
#downsize pngs

img_ext='.bmp'
onlyfiles = [f for f in os.listdir(exportPath) if os.path.isfile(os.path.join(exportPath, f))]


#bpy.context.window.screen = bpy.data.screens['UV Editing'] #uv window needs to be open?


for file in onlyfiles:
	print("open"+file)
	if (file.endswith(img_ext)):
		print("THIS IS AN IMAGE: "+file)
		image = bpy.data.images.load(exportPath+"/"+file)
		#bpy.ops.image.open(filepath=exportPath+"/"+file)
		#bpy.ops.image.save_as(override,file_format='PNG',filepath=exportPath+"/"+file.replace(".bmp",".png"))
		#newName = file.replace(".bmp",".png")
		#image.name = newName
		#image.filepath = exportPath+"/"+file.replace(".bmp",".png")
		#image.filepath_raw = exportPath+"/"+file.replace(".bmp",".png")
		#image.file_format = 'PNG'
		#image.save()
		
		#display pixel count
		#print (len(image.pixels)/4)
		
		pixels = list(image.pixels)
		
		image2 = bpy.data.images.new(image.name, width=image.size[0], height=image.size[1])
		image2.filepath = exportPath+"/"+file.replace(".bmp",".png")
		#num_pixels = len(image.pixels)
		#dm = [(0.2) for cp in range(num_pixels)]
		image2.pixels = pixels
		image2.file_format = 'PNG'
		#image2.depth = '8'
		image2.scale(image.size[0]//sizeFactor,image.size[1]//sizeFactor)
		image2.save()
		os.remove(exportPath+"/"+file)
		#i2 = bpy.data.images.new()
print("all files done")
exit()


#imgs = bpy.data.images


#for image in imgs:
	#settings = bpy.context.scene.render.image_settings
	#format = settings.file_format
	#mode = settings.color_mode
	#depth = settings.color_depth

	#settings.file_format = 'PNG'
	#settings.color_mode = find_color_mode(image)
	#settings.color_depth = '8'

	#image.save_render(exportPath+"/"+file.replace(".bmp",".png"))


#for image in imgs:
	#image.filepath_raw = exportPath+"/"+file.replace(".bmp",".png")
	#image.file_format = 'PNG'
	#bpy.ops.image.save_as(override, filepath=exportPath+"/"+file.replace(".bmp",".png"),file_format='PNG')
	#image.save()
print("ALL DONE")



#textures

#
#img_dirs=[]								# list of image directories in the root folder
#files=[]                                 # list for files 
#diffuse=[]                                 # list for imgage files 
#originalImagePathes=[]                           # where to find the original bmps
#mtl_ext='.mtl'
#mtlpath=''
#objPath=''
#obj_ext='.obj'
#
#copymtlstring='\n'
#
#
#
#for temp in os.listdir(exportPath):
#	print("============found file/dir: "+os.path.join(exportPath,temp))
#	if os.path.isdir(os.path.join(exportPath,temp)):
#		img_dirs.append(os.path.join(exportPath,temp))
#		print("============found dir: "+temp)
#
#for tempPath, dirs, files in os.walk(exportPath):
#	for name in files:
#		if name.endswith(diffuse_ext):
#			diffuse.append(name)
#			newpath = os.path.join(tempPath, name)
#			originalImagePathes.append(newpath)
#			print ("======image src: " + newpath)
#			print ("======image dst: " + exportPath)
#			shutil.copy(newpath,exportPath) #need to export this
#		if name.endswith(mtl_ext):
#			mtlpath = os.path.join(tempPath, name)
#		if name.endswith(obj_ext):
#			objPath = os.path.join(tempPath,name)
#
#for dir in img_dirs:
#	print ("---------- dirs: " + dir)
#	os.rename(dir,dir+"_old")
#
#image_count=len(diffuse)                        # count of diffuse 
#print("image_count " + str(image_count))       
#
#
##==========================add a space at the beginning of the mtl
#mo = open(mtlpath, encoding='utf-8-sig')
#readString = mo.read()
#
#outstrings=[]
#outstrings.append('\n\n')
#
##==========================replace mtl with png references to textures
#for line in readString.splitlines():
#	if line.endswith(diffuse_ext):
#		outstrings.append(line+'\n')
#	elif not line.endswith(other_img_ext):
#		outstrings.append(line+'\n'+'\n')
#		
#
#mo.close()
#
##remove the mtl
#os.remove(mtlpath)
#
##write to new file (BMP)
#nmo = open(mtlpath, 'w+', encoding='utf-8-sig')
#nmo.writelines(outstrings)
#nmo.close()
#print("=============================================mtl fixed")
#
#

#bpy.context.window.screen = bpy.data.screens['UV Editing'] #uv window needs to be open?



#oContextOverride = AssembleOverrideContextForImageOps()    # Get an override context suitable for bpy.ops operators
#bpy.ops.mesh.knife_project(oContextOverride)






#exporting textures


#original_type = bpy.context.area.type
#imgs = bpy.data.images

#for image in imgs:
#	print("--->found image: " + image.name)
#	if (image.name.endswith(diffuse_ext)):
#		bpy.ops.image.save_as(oContextOverride, file_format='PNG',filepath=exportPath+"/"+image.name)
		#originalpath = image.filepath_raw
#		print("--->export image: " + exportPath+"/" + image.name)
		#image.file_format = 'PNG'
		#image.save()


#bpy.ops.image.save_as
##names of images, not the actual images
#for dtex in diffuse:
	#export
	#image = bpy.data.images.new("somename", width=size[0], height=size[1])
	## For white image
	# pixels = [1.0] * (4 * size[0] * size[1])

	#pixels = [None] * size[0] * size[1]
	#for x in range(size[0]):
	#	for y in range(size[1]):
			# assign RGBA to something useful
	#		r = x / size[0]
	#		g = y / size[1]
	#		b = (1 - r) * g
	#		a = 1.0

	#		pixels[(y * size[0]) + x] = [r, g, b, a]

	# flatten list
	#pixels = [chan for px in pixels for chan in px]

	# assign pixels
	#image.pixels = pixels

	# write image
	#image.filepath_raw = exportPath
	#image.file_format = 'PNG'
	#image.save()




#downsize textures

#for a in range(0,image_count):                     # for each image 
#   print("========================")
#   print('loop count: '+str(a)) 
#   bpy.ops.mesh.primitive_plane_add()
#   plane=bpy.context.scene.objects.active
#   
#   mat=bpy.data.materials.new('mat'+str(a))
#   bpy.context.object.data.materials.append(mat)
#   
#   tex=bpy.data.textures.new('ColorTex', type = 'IMAGE')
#   imgpath=imgdir+'\\'+images[a]                  # make string with path ti image 
#   img = bpy.data.images.load(imgpath)       # load image
#   tex.image = img
#   mtex = mat.texture_slots.add()
#   mtex.texture = tex
#   
#   imgX = img.size[0]/1000.0                   # calculate dimensions 
#   imgY = img.size[1]/1000.0
#   
#   plane.scale[0] = (imgX)                          # set x plane dimensions to match image 
#   plane.scale[1] = (imgY)                          # set y plane dimensions to match image    

