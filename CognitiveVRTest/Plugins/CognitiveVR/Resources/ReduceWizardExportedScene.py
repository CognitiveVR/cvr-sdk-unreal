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
if (len(args))<8:
 args.append('')
productid = args[7]
if (len(args))<9:
 args.append('')
sdkVersion = args[8]
if (len(args))<10:
 args.append('ABSOLUTELYNOTHING3475236148265')
excludeMeshes = args[9]

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
excludeMeshArray = excludeMeshes.split(',')

copymtlstring='\n'

print("---------------exclude meshes " + excludeMeshes);

for temp in os.listdir(exportPath):
	print("============found file/dir: "+os.path.join(exportPath,temp))
	if os.path.isdir(os.path.join(exportPath,temp)):
		img_dirs.append(os.path.join(exportPath,temp))
		print("============found dir: "+temp)

#copy image files into root
for tempPath, dirs, files in os.walk(exportPath):
	for name in files:
		if name.endswith(diffuse_ext):
			diffuse.append(name)
			newpath = os.path.join(tempPath, name)
			originalImagePathes.append(newpath)
			print ("======image src: " + newpath)
			print ("======image dst: " + exportPath)
			#check that this file exists
			if os.path.isfile(newpath):
				shutil.copy(newpath,exportPath)
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

#	if line.endswith(diffuse_ext):
#		#remove references to bmp images that dont exist
#		for sline in line.split(" "):
#			#print (sline)
#			if sline.endswith(".bmp"):
#				print("full path to image "+ exportPath+"/"+sline)
#				if os.path.isfile(exportPath+"/"+sline):
#					outstrings.append(line.replace(".bmp",".png")+'\n')
#					print ("========>FOUND this image file " + sline)
#				else:
#					print ("------------->image file doesnt exist " + sline)
#	elif not line.endswith(other_img_ext):
#		#remove references to bmp images that dont exist
#		for sline in line.split(" "):
#			#print (sline)
#			if sline.endswith(".bmp"):
#				print("full path to image "+ exportPath+"/"+sline)
#				if os.path.isfile(exportPath+"/"+sline):
#					outstrings.append(line.replace(".bmp",".png")+'\n'+'\n')
#					print ("========>FOUND this image file " + sline)
#				else:
#					print ("------------->image file doesnt exist " + sline)
#		outstrings.append(line.replace(".bmp",".png")+'\n'+'\n')

		

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

#decimate. remesh bsp
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
		ops.object.modifier_apply(apply_as='DATA', modifier="Decimate")
		if obj.name == 'BSP':
			print("found bsp, recalc normals")			
			ops.object.mode_set(mode='EDIT')
			ops.mesh.remove_doubles(threshold=0.0001)
			ops.mesh.normals_make_consistent(inside=False)
			ops.object.mode_set(mode='OBJECT')
			#TODO box uv unwrap and put some checker pattern on it
			
			mod = bpy.context.object.modifiers.new('Remesh','REMESH')
			ops.object.modifier_apply(apply_as='DATA', modifier="Remesh")


print("=============================================decimate complete")

bpy.ops.object.select_all(action='DESELECT')

# select objects by type
for o in scene.objects:
	for excludeMesh in excludeMeshArray:
		print("=============================================checking if " + o.name + " contained in " + excludeMesh)
	
		if excludeMesh in o.name:
			print("=============================================delete " + o.name)
			o.select = True
		
# call the operator once
bpy.ops.object.delete()
print("=============================================delete all exclude meshes")

#move this obj into a new folder called 'raw_model_old' or something
if not os.path.exists(os.path.join(exportPath,"raw_model_old/")):
    os.makedirs(os.path.join(exportPath,"raw_model_old/"))

shutil.move(objPath,os.path.join(exportPath,"raw_model_old/"))
shutil.move(mtlpath,os.path.join(exportPath,"raw_model_old/"))




#export
bpy.ops.object.join()
ops.export_scene.obj(filepath=exportPath+"/"+fileName+"_unrealdec.obj", use_edges=False, path_mode='RELATIVE')
print("=============================================export complete")



for tempPath, dirs, files in os.walk(exportPath):
	for name in files:
		if name.endswith("_unrealdec.mtl"):
			mtlpath = os.path.join(tempPath, name)

#does nothing
for line in readString.splitlines():
	if line.endswith(diffuse_ext):
		outstrings.append(line.replace(".bmp",".png")+'\n')
	elif not line.endswith(other_img_ext):
		outstrings.append(line.replace(".bmp",".png")+'\n'+'\n')

mo = open(mtlpath, encoding='utf-8-sig')
readString = mo.read()





finalmtlstrings=[]
print("=============================================mtl start")
#==========================replace mtl with png references to textures
for line in readString.splitlines():
	print("original line " + line)
	if line.startswith("map_Kd"):
		temppath = line.replace("map_Kd ","")
		head, tail = os.path.split(temppath)
		
		#before actually appending this to the mtl, check if the .bmp image file exists
		
		print(">>>>>is this a file? "+exportPath+"/"+tail[:-4]+".bmp")
		
		if os.path.isfile(exportPath+"/"+tail[:-4]+".bmp"):
			finalmtlstrings.append("map_Kd " + tail+"\n")
			print(">>>>>append map_kd as " + tail)
		else:
			print("^^^^^^^^^^^ couldn't find file " + tail)
	#	for sline in line.split(" "):
	#		#print (sline)
	#		if sline.endswith(".png"):
	#			print("full path to image "+ exportPath+"/"+sline[:-4]+".bmp")
	#			if os.path.isfile(exportPath+"/"+sline[:-4]+".bmp"): #at this point, it references a png and the file is bmp
	#				print ("========>FOUND this image file " + sline)
    #
	#			else:
	#				print ("------------->image file doesnt exist " + sline)
	elif line.startswith("Ka"):
		finalmtlstrings.append("Ka 0 0 0"+"\n")
	elif line.startswith("Ks"):
		finalmtlstrings.append("Ks 0 0 0"+"\n")
	else:
		finalmtlstrings.append(line+"\n")
		#print("????????unknown line as " + line)
		

mo.close()

#remove the mtl
os.remove(mtlpath)

#write to new file (BMP)
nmo = open(mtlpath, 'w+', encoding='utf-8-sig')
nmo.writelines(finalmtlstrings)
nmo.close()
print("=============================================mtl fixed")

f = open (os.path.join(exportPath,'settings.json'),'w')

productid = productid[:-5]

f.write('{\"scale\":100,"sdkVersion":"'+sdkVersion+'","sceneName":"'+fileName+'"}')
f.close()

print("=============================================json write complete")





##============================CONVERT TEXTURE SECTION






#remove bmp files
#downsize pngs

onlyfiles = [f for f in os.listdir(exportPath) if os.path.isfile(os.path.join(exportPath, f))]
onlydirectories = [d for d in os.listdir(exportPath) if os.path.isdir(os.path.join(exportPath, d))]


#bpy.context.window.screen = bpy.data.screens['UV Editing'] #uv window needs to be open?


for file in onlyfiles:
	print("open"+file)
	if (file.endswith(other_img_ext)):
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
		image2.scale(image.size[0]//1,image.size[1]//1)
		image2.save()
		os.remove(exportPath+"/"+file)
		#i2 = bpy.data.images.new()
print("==================================all files done")

for dir in onlydirectories:
	print("==========remove directory "+exportPath+"/"+dir)
	#shutil.rmtree(os.path.join(exportPath,dir))
	shutil.rmtree(exportPath+'/'+dir)

print("ALL DONE")
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
