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
exportPath = args[3] #C:/Users/calder/Desktop/RootDynamics
sizeFactor = int(args[4])

#foreach dynamic object
#copy mtl
#import + export obj
#write mtl
#downsize textures
#move textures to base directory

diffuse_ext='_D.bmp'
other_img_ext='.bmp'
dynamics=[]

subdirectories = [d for d in os.listdir(exportPath) if os.path.isdir(os.path.join(exportPath, d))]
#onlyfiles = [f for f in os.listdir(exportPath+"/"+dir) if os.path.isfile(os.path.join(exportPath+"/"+dir, f))]
print("==================================ROOT " + exportPath)

for dir in subdirectories:
	
	workingdir = exportPath+"/"+dir
	print("==========working directory "+exportPath+"/"+dir)	
	mtlpath=""
	objPath=""
	
	#skip dynamic objects that don't have sub directories
	onlydirectories = [d for d in os.listdir(workingdir) if os.path.isdir(os.path.join(workingdir, d))]
	if (len(onlydirectories) == 0):
		continue
	
	
	#copy image files into root
	for tempPath, dirs, files in os.walk(workingdir):
		for name in files:
			if name.endswith(diffuse_ext):
				#diffuse.append(name)
				newpath = os.path.join(tempPath, name)
				#originalImagePathes.append(newpath)
				print ("======image src: " + newpath)
				print ("======image dst: " + workingdir)
				#check that this file exists
				if os.path.isfile(newpath):
					shutil.copy(newpath,workingdir)
			if name.endswith(".mtl"):
				mtlpath = os.path.join(tempPath, name)
			if name.endswith(".obj"):
				objPath = os.path.join(tempPath,name)
	
	#==========================add a space at the beginning of the mtl
	mo = open(mtlpath, encoding='utf-8-sig')
	readString = mo.read()

	outstrings=[]
	outstrings.append('\n\n')
	
	#replace .bmp with .png
	for line in readString.splitlines():
		if line.endswith(diffuse_ext):
			outstrings.append(line.replace(".bmp",".png")+'\n')
		elif not line.endswith(other_img_ext):
			outstrings.append(line.replace(".bmp",".png")+'\n'+'\n')
	
	mo.close()
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
	ops.import_scene.obj(filepath=objPath, use_edges=True, use_smooth_groups=True, use_split_objects=True, use_split_groups=True, use_groups_as_vgroups=False, use_image_search=True, split_mode='ON', global_clamp_size=0, axis_forward='-Z', axis_up='Y')
	print("=============================================import complete")
	
	bpy.ops.object.select_all(action='DESELECT')
	#bpy.ops.object.join()
	ops.export_scene.obj(filepath=objPath, use_edges=False, path_mode='RELATIVE')
	print("=============================================export complete")
	
	for tempPath, dirs, files in os.walk(workingdir):
		for name in files:
			if name.endswith(".mtl"):
				mtlpath = os.path.join(tempPath, name)
	
	mo = open(mtlpath, encoding='utf-8-sig')
	readString = mo.read()
	
	#replace mtl with references to pngs
	finalmtlstrings=[]
	print("=============================================mtl start")
	for line in readString.splitlines():
		print("original line " + line)
		if line.startswith("map_Kd"):
			temppath = line.replace("map_Kd ","")
			head, tail = os.path.split(temppath)
			
			#before actually appending this to the mtl, check if the .bmp image file exists
			
			print(">>>>>is this a file? "+workingdir+"/"+tail[:-4]+".bmp")
			
			if os.path.isfile(workingdir+"/"+tail[:-4]+".bmp"):
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

	#write to new file
	nmo = open(mtlpath, 'w+', encoding='utf-8-sig')
	nmo.writelines(finalmtlstrings)
	nmo.close()
	print("=============================================mtl fixed")
	
	for root, dirs, files in os.walk(workingdir):
		for file in files:
			print ("walk file name " + file)
			if (file.endswith(diffuse_ext)):
				image = bpy.data.images.load(root+"/"+file)				
				pixels = list(image.pixels)
				
				image2 = bpy.data.images.new(image.name, width=image.size[0], height=image.size[1])
				image2.filepath = workingdir+"/"+file.replace(".bmp",".png")
				#num_pixels = len(image.pixels)
				#dm = [(0.2) for cp in range(num_pixels)]
				image2.pixels = pixels
				image2.file_format = 'PNG'
				#image2.depth = '8'
				image2.scale(image.size[0]//sizeFactor,image.size[1]//sizeFactor)
				image2.save()
				#os.remove(root+"/"+file)
				#i2 = bpy.data.images.new()
				os.remove(root+"/"+file)
	print("=============================================images scaled")

	#remove subdirectories. defined up top
	#onlydirectories = [d for d in os.listdir(workingdir) if os.path.isdir(os.path.join(workingdir, d))]
	
	for filedir in onlydirectories:
		print("==========remove directory "+workingdir+"/"+filedir)
		shutil.rmtree(workingdir+"/"+filedir)

print("ALL DONE")
exit()