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
			
			mod = bpy.context.object.modifiers.new('Remesh','REMESH')
			mod.octree_depth = 6
			ops.object.modifier_apply(apply_as='DATA', modifier="Remesh")
		
print("=============================================decimate complete")



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
f.write('{"scale":100,"customerId":"'+productid+'","sdkVersion":"'+sdkVersion+'"}')
f.close()

print("=============================================json write complete")

exit()

