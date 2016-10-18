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
sc = bpy.data.scenes[0]					# get current scene 
imgdir=exportPath	 					# set image directory
diffuse_ext='_D.bmp'
other_img_ext='.bmp'

img_dirs=[]								# list of image directories in the root folder
files=[]                                 # list for files 
diffuse=[]                                 # list for imgage files 
pathedImages=[]                           # list for imgage files 
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
			pathedImages.append(newpath)
			print ("======image src: " + newpath)
			print ("======image dst: " + exportPath)
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

#for strings in readString
	#if ends with other_img_ext and not end in diffuse_ext
		#write line to outstrings
for line in readString.splitlines():
	if line.endswith(diffuse_ext):
		outstrings.append(line+'\n')
	elif not line.endswith(other_img_ext):
		outstrings.append(line+'\n'+'\n')
		

mo.close()

#remove the mtl
os.remove(mtlpath)

#write to new file
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
exit()