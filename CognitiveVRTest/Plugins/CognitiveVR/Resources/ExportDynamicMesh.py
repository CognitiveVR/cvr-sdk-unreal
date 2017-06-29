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

#objPath=''
dynamics=[]
obj_ext='.obj'

#get obj path
for tempPath, dirs, files in os.walk(exportPath):
	for name in files:
		if name.endswith(obj_ext):
			#dynamics.append(os.path.join(tempPath,name)
			#objPath = os.path.join(tempPath,name)
			
			#select all
			for ob in scene.objects:
			 ob.select = True
			ops.object.delete()
			print("=============================================deleted stuff")


			#import
			importPathName = os.path.join(tempPath,name)
			print ("import " + importPathName)
			
			ops.import_scene.obj(filepath=importPathName, use_edges=True, use_smooth_groups=True, use_split_objects=True, use_split_groups=True, use_groups_as_vgroups=False, use_image_search=True, split_mode='ON', global_clamp_size=0, axis_forward='-Z', axis_up='Y')
			print("=============================================import complete")

			#bpy.ops.object.select_all(action='DESELECT')

			#export
			#bpy.ops.object.join()
			ops.export_scene.obj(filepath=importPathName, use_edges=False, path_mode='RELATIVE')
			print("=============================================export complete")

exit()

