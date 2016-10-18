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
minFaces = int(args[4])
maxFaces = int(args[5])
if (len(args))<7:
 args.append('')
fileName = args[6]

#copy textures
#fix mtl
#reimport
#decimate
#export

#textures
diffuse_ext='_D.bmp'
other_img_ext='.bmp'

img_dirs=[]								# list of image directories in the root folder
files=[]                                 # list for files 
diffuse=[]                                 # list for imgage files 
originalImagePathes=[]                           # where to find the original bmps
mtl_ext='.mtl'
mtlpath=''
objPath=''
obj_ext='.obj'

copymtlstring='\n'

for temp in os.listdir(exportPath):
	print("============found file/dir: "+os.path.join(exportPath,temp))
	if os.path.isdir(os.path.join(exportPath,temp)):
		img_dirs.append(os.path.join(exportPath,temp))
		print("============found dir: "+temp)

for tempPath, dirs, files in os.walk(exportPath):
	for name in files:
		if name.endswith(diffuse_ext):
			diffuse.append(name)
			newpath = os.path.join(tempPath, name)
			originalImagePathes.append(newpath)
			print ("======image src: " + newpath)
			print ("======image dst: " + exportPath)
			shutil.copy(newpath,exportPath) #need to export this
		if name.endswith(mtl_ext):
			mtlpath = os.path.join(tempPath, name)
		if name.endswith(obj_ext):
			objPath = os.path.join(tempPath,name)

for dir in img_dirs:
	print ("---------- dirs: " + dir)
	os.rename(dir,dir+"_old")

image_count=len(diffuse)                        # count of diffuse 
print("image_count " + str(image_count))       


#==========================add a space at the beginning of the mtl
mo = open(mtlpath, encoding='utf-8-sig')
readString = mo.read()

outstrings=[]
outstrings.append('\n\n')

#==========================replace mtl with png references to textures
for line in readString.splitlines():
	if line.endswith(diffuse_ext):
		outstrings.append(line.replace(".bmp",".png")+'\n')
	elif not line.endswith(other_img_ext):
		outstrings.append(line.replace(".bmp",".png")+'\n'+'\n')
		

mo.close()

#remove the mtl
os.remove(mtlpath)

#write to new file (BMP)
nmo = open(mtlpath, 'w+', encoding='utf-8-sig')
nmo.writelines(outstrings)
nmo.close()
print("=============================================mtl fixed")



#select all
for ob in scene.objects:
 ob.select = True
ops.object.delete()

print("=============================================deleted stuff")

#import
ops.import_scene.obj(filepath=exportPath+"/"+fileName+".obj", use_edges=True, use_smooth_groups=True, use_split_objects=True, use_split_groups=True, use_groups_as_vgroups=False, use_image_search=True, split_mode='ON', global_clamp_size=0, axis_forward='-Z', axis_up='Y')
print("=============================================import complete")



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




#decimate
for obj in scene.objects:
 if obj.type == 'MESH':
  scene.objects.active = obj
  mod = bpy.context.object.modifiers.new('Decimate','DECIMATE')
  
  faceCount = len(bpy.context.object.data.polygons)
  ratio = 1.0
  
  ratio = (faceCount-minFaces)/maxFaces
  
  ratio = 1-ratio
  
  if ratio >= 1.0:
   ratio = 1.0
  if ratio <= 0.1:
   ratio = 0.1
  
  mod.ratio = ratio
  ops.object.modifier_apply(apply_as='DATA')
print("=============================================decimate complete")






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





#move this obj into a new folder called 'raw_model_old' or something
if not os.path.exists(os.path.join(exportPath,"raw_model_old/")):
    os.makedirs(os.path.join(exportPath,"raw_model_old/"))

shutil.move(objPath,os.path.join(exportPath,"raw_model_old/"))
shutil.move(mtlpath,os.path.join(exportPath,"raw_model_old/"))




#export
bpy.ops.object.join()
ops.export_scene.obj(filepath=exportPath+"/"+fileName+"_unrealdec.obj", use_edges=False, path_mode='RELATIVE')
print("=============================================export complete")
#exit()




#find a line with map_kd. replace entire line with map_kd + " " + diffusename
#'unpath' textures in mtl file





for tempPath, dirs, files in os.walk(exportPath):
	for name in files:
		if name.endswith("_unrealdec.mtl"):
			mtlpath = os.path.join(tempPath, name)

for line in readString.splitlines():
	if line.endswith(diffuse_ext):
		outstrings.append(line.replace(".bmp",".png")+'\n')
	elif not line.endswith(other_img_ext):
		outstrings.append(line.replace(".bmp",".png")+'\n'+'\n')

mo = open(mtlpath, encoding='utf-8-sig')
readString = mo.read()





finalmtlstrings=[]
#==========================replace mtl with png references to textures
for line in readString.splitlines():
	if "map_Kd" in line:
		tempfinalline = "map_Kd "
		temppath = line.replace("map_Kd ","")
		head, tail = os.path.split(temppath)
		finalmtlstrings.append("map_Kd " + tail+"\n")
	if "Ka" in line:
		finalmtlstrings.append("Ka 0 0 0"+"\n")
	if "Ks" in line:
		finalmtlstrings.append("Ks 0 0 0"+"\n")
	else:
		finalmtlstrings.append(line+"\n")
		

mo.close()

#remove the mtl
os.remove(mtlpath)

#write to new file (BMP)
nmo = open(mtlpath, 'w+', encoding='utf-8-sig')
nmo.writelines(finalmtlstrings)
nmo.close()
print("=============================================mtl fixed")
exit()

