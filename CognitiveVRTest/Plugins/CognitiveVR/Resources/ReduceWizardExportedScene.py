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

#textures
#diffuse_ext='_D.bmp'
#other_img_ext='.bmp'

img_dirs=[]								# list of image directories in the root folder
files=[]                                 # list for files 
diffuse=[]                                 # list for imgage files 
originalImagePathes=[]                           # where to find the original bmps
mtlpath=''
objPath=''


#find all file paths
for tempPath, dirs, files in os.walk(exportPath):
	for name in files:
		print (tempPath + "  " + name)
		if "dynamics" in tempPath:
			print("------skip dynamic file------ " + name)
			continue
		if name.endswith('_D.bmp'):
			diffuse.append(name)
			newpath = os.path.join(tempPath, name)
			originalImagePathes.append(newpath)
			print ("======image src: " + newpath)
			print ("======image dst: " + exportPath)
			if os.path.isfile(newpath):
				shutil.copy(newpath,exportPath)
		if name.endswith('.mtl'):
			mtlpath = os.path.join(tempPath, name)
			print("found mtl path " + mtlpath)
		if name.endswith('.obj'):
			objPath = os.path.join(tempPath,name)

#add empty line in beginning of mtlpath
image_count=len(diffuse)
if image_count > 0:
	mo = open(mtlpath, encoding='utf-8-sig')
	readString = mo.read()
	outstrings=[]
	outstrings.append('\n\n')
	#==========================replace mtl with png references to textures
	for line in readString.splitlines():
		outstrings.append(line+'\n')
	mo.close()

	#remove the mtl
	os.remove(mtlpath)

	#write to new file (pngs)
	nmo = open(mtlpath, 'w+', encoding='utf-8-sig')
	nmo.writelines(outstrings)
	nmo.close()


#import meshes
#decimate
#export meshes

#copy textures to root
#set mtl to root texture paths

#convert bmp textures to pngs

def DecimateMeshes():
	print("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")
	print("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++decimate")
	print("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")
	
	#select all
	for ob in scene.objects:
	 #ob.select = True
	 #2.7 -> 2.8 change
	 ob.select_set(True)
	ops.object.delete()
	print("=============================================deleted stuff")


	#import
	#ops.import_scene.obj(filepath=exportPath+"/"+fileName+".obj", use_edges=True, use_smooth_groups=True, use_split_objects=True, use_split_groups=True, use_groups_as_vgroups=False, use_image_search=True, split_mode='ON', global_clamp_size=0, axis_forward='-Z', axis_up='Y')
	ops.import_scene.obj(filepath=exportPath+"/"+fileName+".obj", use_edges=True, use_smooth_groups=True, use_split_objects=True, use_split_groups=True, use_groups_as_vgroups=False, use_image_search=True, split_mode='ON', axis_forward='-Z', axis_up='Y')

	#import fbx geometry brushes
	if os.path.isfile(exportPath+"/"+fileName+".fbx"):
		ops.import_scene.fbx(filepath=exportPath+"/"+fileName+".fbx")
	else:
		print("couldn't find "+exportPath+"/"+fileName+".fbx")
		
	print("=============================================import complete")

	#decimate. remesh bsp
	#for obj in scene.objects:
	#	if obj.type == 'MESH':
	#		scene.objects.active = obj #HERE
	#		
	#		bpy.ops.object.scale_clear()
	#		
	#		mod = bpy.context.object.modifiers.new('Decimate','DECIMATE')
    #
	#		faceCount = len(bpy.context.object.data.polygons)
	#		ratio = 1.0
    #
	#		ratio = (faceCount-minFaces)/maxFaces
    #
	#		ratio = 1-ratio
    #
	#		if ratio >= 1.0:
	#			ratio = 1.0
	#		if ratio <= 0.1:
	#			ratio = 0.1
    #
	#		mod.ratio = ratio
	#		ops.object.modifier_apply(apply_as='DATA', modifier="Decimate")
	#		#ops.mesh.customdata_custom_splitnormals_clear() #TEST should fix custom normal issue on bsp


	print("=============================================decimate complete")

	print(bpy.context.area)

	bpy.ops.object.select_all(action='DESELECT')
	
	os.makedirs(os.path.join(exportPath,"mesh_delete/"))
	shutil.move(objPath,os.path.join(exportPath,"mesh_delete/"))

	#bpy.context.active_object = bpy.ops.objects[0]
	bpy.ops.object.select_all(action='SELECT')

	#what is the context currently? can this be printed to the screen?

	print(bpy.context.area)

	#export
	#bpy.ops.object.join()
	#ops.export_scene.obj(filepath=exportPath+"/"+fileName+".obj", use_edges=False, path_mode='RELATIVE')
	ops.export_scene.gltf(export_format='GLTF_SEPARATE',export_animations=False,filepath=exportPath+"/"+"scene.gltf")
	print("=============================================export complete")

def FlattenTexturePaths(mtlpath):
	print("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")
	print("++++++++++++++++++++++++++++++++++++++++++++flatten texture paths")
	print("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")
	#get all subdirectories with images
	for temp in os.listdir(exportPath):
		tempdir = os.path.join(exportPath,temp)
		print("============found file/dir: "+tempdir)
		if os.path.isdir(tempdir):
			if tempdir.endswith("dynamics"):
				continue
			img_dirs.append(tempdir)
			print("============found dir: "+temp)

	#mark directories for deletion
	for dir in img_dirs:
		print ("---------- dirs: " + dir)
		os.rename(dir,dir+"_delete")

	image_count=len(diffuse)                        # count of diffuse 
	print("image_count " + str(image_count))       

	#replace mtl bmps with pngs
	readString = ''
	#if image_count > 0:
	#	print("============================ replace mtl bmps with pngs")
	#	#==========================add a space at the beginning of the mtl
	#	mo = open(mtlpath, encoding='utf-8-sig')
	#	readString = mo.read()
    #
	#	outstrings=[]
	#	outstrings.append('\n\n')
    #
	#	#==========================replace mtl with png references to textures
	#	for line in readString.splitlines():
	#		print("original line " + line)
	#		if line.endswith('_D.bmp'):
	#			outstrings.append(line.replace(".bmp",".png")+'\n')
	#		elif not line.endswith('.bmp'):
	#			outstrings.append(line.replace(".bmp",".png")+'\n')
    #
	#	mo.close()
    #
	#	#remove the mtl
	#	os.remove(mtlpath)
    #
	#	#write to new file (pngs)
	#	nmo = open(mtlpath, 'w+', encoding='utf-8-sig')
	#	nmo.writelines(outstrings)
	#	nmo.close()
	#	print("=============================================mtl fixed")
	#else:
	#	print("============================ no bmp images to fix!")
	#	for tempPath, dirs, files in os.walk(exportPath):
	#		for name in files:
	#			if name.endswith(".mtl"):
	#				mtlpath = os.path.join(tempPath, name)

	if image_count > 0:
		mo = open(mtlpath, encoding='utf-8-sig')
		readString = mo.read()
		finalmtlstrings=[]
		print("=============================================mtl start")
		#==========================replace mtl with png references to textures. remove path, since images will be copied to root in step 3
		for line in readString.splitlines():
			print("original line " + line)
			if line.startswith("map_Kd"):
				temppath = line.replace("map_Kd ","").replace(".bmp",".png")
				head, tail = os.path.split(temppath)
				tail = tail.replace(" ","")
				finalmtlstrings.append("map_Kd " + tail+"\n")
				
				#if os.path.isfile(exportPath+"/"+tail[:-4]+".bmp"):
				#	print(">>>>>append map_kd as " + tail)
				#else:
				#	print("^^^^^^^^^^^ couldn't find file " + tail)
			elif line.startswith("Ka"):
				finalmtlstrings.append("Ka 0 0 0"+"\n")
			elif line.startswith("Ks"):
				finalmtlstrings.append("Ks 0 0 0"+"\n")
			elif line.endswith('.bmp'):
				continue
			else:
				finalmtlstrings.append(line+"\n")
				#print("????????unknown line as " + line)
		mo.close()

		#remove the mtl
		os.remove(mtlpath)

		#write to new file (BMP)
		#nmo = open(mtlpath, 'w+', encoding='utf-8-sig')
		#nmo.writelines(finalmtlstrings)
		#nmo.close()
		print("=============================================mtl fixed")
	
def ConvertTextures():
	print("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")
	print("+++++++++++++++++++++++++++++++++++++++++++++++++convert textures")
	print("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")
	#remove bmp files
	#downsize pngs

	onlyfiles = [f for f in os.listdir(exportPath) if os.path.isfile(os.path.join(exportPath, f))]

	for file in onlyfiles:
		print("open"+file)
		if (file.endswith('.bmp')):
			print("THIS IS AN IMAGE: "+file)
			image = bpy.data.images.load(exportPath+"/"+file)
			
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


DecimateMeshes()
FlattenTexturePaths(mtlpath)
ConvertTextures()

#write json settings file
f = open (os.path.join(exportPath,'settings.json'),'w')

productid = productid[:-5]

f.write('{\"scale\":100,"sdkVersion":"'+sdkVersion+'","sceneName":"'+fileName+'"}')
f.close()

print("=============================================json write complete")

onlydirectories = [d for d in os.listdir(exportPath) if os.path.isdir(os.path.join(exportPath, d))]

#remove unused directories
for dir in onlydirectories:
	print (dir)

	if dir.endswith("dynamics"):
		print("====skip dynamics directory")
		continue
	if dir.endswith("_delete"):
		print("==========remove directory "+exportPath+"/"+dir)
		shutil.rmtree(exportPath+'/'+dir)

if os.path.isfile(exportPath+"/"+fileName+".fbx"):
	os.remove(exportPath+"/"+fileName+".fbx")

print("ALL DONE")
#exit()