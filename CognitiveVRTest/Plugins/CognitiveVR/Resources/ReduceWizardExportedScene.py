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

img_dirs=[]								# list of image directories in the root folder
files=[]                                 # list for files 
diffuse=[]                                 # list for imgage files 
originalImagePathes=[]                           # where to find the original bmps
mtlpath=''
objPath=''

debuglog = open (os.path.join(exportPath,'export.log'),'w')
sys.stdout = debuglog

print("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")
print("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++ locate files")
print("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")
#find all file paths
for tempPath, dirs, files in os.walk(exportPath):
	for name in files:
		if "dynamics" in tempPath:
			print("------skip dynamic file------ " + name)
			continue
		if name.endswith('_D.bmp'):
			diffuse.append(name)
			newpath = os.path.join(tempPath, name)
			originalImagePathes.append(newpath)
			print ("======image src: " + newpath)
			print ("======image dst: " + exportPath)
		if name.endswith('.mtl'):
			mtlpath = os.path.join(tempPath, name)
			print("======mtl path " + mtlpath)
		if name.endswith('.obj'):
			objPath = os.path.join(tempPath,name)
			print("======obj path " + objPath)

def DecimateMeshes():
	print("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")
	print("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++ decimate")
	print("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")
	
	#select all
	for ob in scene.objects:
	 #ob.select = True
	 #2.7 -> 2.8 change
	 ob.select_set(True)
	ops.object.delete()
	print("clear default blender content")


	#import
	print("import obj")
	ops.import_scene.obj(filepath=exportPath+"/"+fileName+".obj", use_edges=True, use_smooth_groups=True, use_split_objects=True, use_split_groups=True, use_groups_as_vgroups=False, use_image_search=True, split_mode='ON', axis_forward='-Z', axis_up='Y')

	#import fbx geometry brushes
	print("import fbx geometry")
	if os.path.isfile(exportPath+"/"+fileName+".fbx"):
		ops.import_scene.fbx(global_scale=100.0,filepath=exportPath+"/"+fileName+".fbx")
	else:
		print("couldn't find "+exportPath+"/"+fileName+".fbx to import")
		
	print("import complete")
	bpy.ops.wm.redraw_timer(type='DRAW_WIN_SWAP',iterations=1)

	bpy.ops.object.select_all(action='DESELECT')
	
	os.makedirs(os.path.join(exportPath,"mesh_delete/"))
	shutil.move(objPath,os.path.join(exportPath,"mesh_delete/"))

	bpy.ops.object.select_all(action='SELECT')

	#export
	ops.export_scene.gltf(export_format='GLTF_SEPARATE',export_animations=False,filepath=exportPath+"/"+"scene.gltf")
	print("export complete")
	
def ConvertTextures():
	print("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")
	print("+++++++++++++++++++++++++++++++++++++++++++++++++ convert textures")
	print("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")
	#remove bmp files
	#downsize pngs

	onlyfiles = [f for f in os.listdir(exportPath) if os.path.isfile(os.path.join(exportPath, f))]
	image=''
	image2=''
	pixels=[]

	for file in onlyfiles:
		if (file.endswith('.bmp')):
			print("convert "+file)
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
	print("all images converted from bmp to png")

#write json settings file
with open (os.path.join(exportPath,'settings.json'),'w') as the_file:
	the_file.write('{\"scale\":100,"sdkVersion":"'+sdkVersion+'","sceneName":"'+fileName+'"}')
print("settings.json write complete")

try:
	DecimateMeshes()
except:
	print("Exception in DecimateMeshes:", sys.exc_info()[0])

try:
#sets masked materials to clipped blend mode
	print('try to fix material masking based on exported textures')
	for mat in bpy.data.materials:
		print('MATERIAL ' + str(mat.name))
		if (mat.node_tree == None):
			print('material doesnt use node tree. skip')
			continue
		for node in mat.node_tree.nodes:
			print('node.type '+node.type)
			if node.type == "TEX_IMAGE":
				print('node image name ' + node.image.name)
				if node.image.name.endswith('OM.bmp'):
					print("set material mask as clip "+ mat.name)
					mat.blend_method = 'CLIP'
except:
	print("Exception when settings Material Mask:", sys.exc_info()[0])

try:
	ConvertTextures()
except:
	print("Exception in ConvertTextures:", sys.exc_info()[0])

print('export complete')

onlydirectories = [d for d in os.listdir(exportPath) if os.path.isdir(os.path.join(exportPath, d))]

#remove unused directories
for dir in onlydirectories:	
	if dir != "screenshot":
		print("removed directory "+exportPath+"/"+dir)
		shutil.rmtree(exportPath+'/'+dir)

os.remove(mtlpath)
print("removed mtl")

if os.path.isfile(exportPath+"/"+fileName+".fbx"):
	os.remove(exportPath+"/"+fileName+".fbx")
	print("removed fbx")

if os.path.isfile(exportPath+"/materiallist.txt"):
	os.remove(exportPath+"/materiallist.txt")
	print("removed fbx material list")


print("ALL DONE. exit")