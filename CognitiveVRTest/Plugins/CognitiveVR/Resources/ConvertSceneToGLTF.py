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

#export path folder (dynamics/scene level)
exportPath = args[3]

#scene/dynamic
#TODO CONSIDER eventually merging this python script with the dynamic python script
exportType = args[4]

#objects to work on (in dynamic object path or scene)
targetNames = args[5].split(',')

def ClearScene():
    #select all
    for ob in scene.objects:
     #ob.select = True
     #2.7 -> 2.8 change
     ob.select_set(True)
    ops.object.delete()
    print("clear default blender content")

def ImportGeometry(fullPathNoExtension):
    #import
    print("import obj")
    if os.path.isfile(fullPathNoExtension+".obj"):
        ops.import_scene.obj(filepath=fullPathNoExtension+".obj", use_edges=True, use_smooth_groups=True, use_split_objects=True, use_split_groups=True, use_groups_as_vgroups=False, use_image_search=True, split_mode='ON', axis_forward='-Z', axis_up='Y')
    else:
        print("couldn't find "+fullPathNoExtension+".obj to import")

    #import fbx geometry brushes
    print("import fbx geometry")
    if os.path.isfile(fullPathNoExtension+".fbx"):
        ops.import_scene.fbx(global_scale=100.0,filepath=fullPathNoExtension+".fbx")
    else:
        print("couldn't find "+fullPathNoExtension+".fbx to import")
        
    print("import complete")
    bpy.ops.wm.redraw_timer(type='DRAW_WIN_SWAP',iterations=1)

#should be exported as scene.gltf, if a scene
def ExportGeometry(fullPathWithExtension):
    #export
    ops.export_scene.gltf(export_format='GLTF_SEPARATE',export_animations=False,filepath=fullPathWithExtension)

#unused
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

def RebuildMaterials():
    materialListPath = exportPath + "/materiallist.txt"
    if os.path.exists(materialListPath):
        realTextures = []
        print("load materials from list")
        #just write lines. no fancy json stuff
        materialListStream = open(materialListPath)
        readString = materialListStream.read()
        for line2 in readString.splitlines():
            #print (line2)
            split = line2.split('|')
            if len(split) < 3:
                print("too short!")
                continue
            for texture in split:
                if texture.endswith('.bmp'):
                    realTextures.append(texture)
                    #print(split[1])
                    #print(texture)
        
        #remove textures not in this list
        #TODO do this as the final step. this isn't important to do now?        
        
        #rebuild the materials
        for mat in bpy.data.materials:
            print("blender data material "+mat.name)
            if mat.node_tree is None:
                print("---"+mat.name + "  has null node tree. skipping")
            #elif mat.name == split[1]: #found the material. now import/set textures from next parts of split
            elif split[1] in mat.name: #found the material. now import/set textures from next parts of split
                print("---"+mat.name + "  FOUND. UPDATE TEXTURE REFERENCE")
                prin = mat.node_tree.nodes["Principled BSDF"]
                imgnode = mat.node_tree.nodes.new("ShaderNodeTexImage")
                mat.node_tree.links.new(prin.inputs['Base Color'],imgnode.outputs[0])
                tex = bpy.data.images.load(split[2])
                imgnode.image = tex
                #TODO normal map, opacity/mask map
                #print(mat.name + " append image to shader tree")
                #set clipping mask
                for node in mat.node_tree.nodes:
                    #print('node.type '+node.type)
                    if node.type == "TEX_IMAGE":
                        #print('node image name ' + node.image.name)
                        if node.image.name.endswith('OM.bmp'):
                            #print("set material mask as clip "+ mat.name)
                            mat.blend_method = 'CLIP'
        materialListStream.close()


## doing stuff



ClearScene()
ImportGeometry(exportPath+"/"+targetNames[0])
print("pre rebuild maerials complete --------------------------------")
RebuildMaterials()
print("rebuild maerials complete --------------------------------")
ExportGeometry(exportPath+"/"+"scene.gltf")
print('export complete')

onlydirectories = [d for d in os.listdir(exportPath) if os.path.isdir(os.path.join(exportPath, d))]

#remove unused directories
for dir in onlydirectories: 
    if dir != "screenshot":
        print("removed directory "+exportPath+"/"+dir)
        shutil.rmtree(exportPath+'/'+dir)

#remove all bmps, mtl and obj
for tempPath, dirs, files in os.walk(exportPath):
    for name in files:
        print (tempPath + "  " + name)
        if name.endswith('.mtl'):
            os.remove(os.path.join(tempPath, name))
        if name.endswith('.obj'):
            os.remove(os.path.join(tempPath, name))
        if name.endswith('.fbx'):
            os.remove(os.path.join(tempPath, name))
        if name.endswith('.bmp'):
            os.remove(os.path.join(tempPath, name))
        if name.endswith('.txt'):
            os.remove(os.path.join(tempPath, name))
            
    for dir in dirs:
        print ("remove " + os.path.join(tempPath, dir))
        #os.remove(dir)
        shutil.rmtree(os.path.join(tempPath, dir))

print("ALL DONE. exit")