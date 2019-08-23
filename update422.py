import os
import shutil
import sys
import time

cwd = os.getcwd()
version = "0"
enginesubversion = "22"

def replaceline(file, linesrc, linedst):

	#mo = open(file, encoding='utf-8-sig')
	mo = open(file, "r")
	readString = mo.read()

	#replace mtl with references to pngs
	finalstrings=[]
	print("=============================================file loop start")
	for line in readString.splitlines():
		if line == linesrc:
			finalstrings.append(linedst+"\n")
			print("replaced line " + line)
		else:
			print("-------- line " + line)
			finalstrings.append(line+"\n")
			

	mo.close()

	#remove the original file
	os.remove(file)

	#write to new file
	nmo = open(file, 'w+')
	nmo.writelines(finalstrings)
	nmo.close()
	print("=============================================file loop end")
	return;

def insertline(file, targetline, insertline):

	#mo = open(file, encoding='utf-8-sig')
	mo = open(file, "r")
	readString = mo.read()

	#replace mtl with references to pngs
	finalstrings=[]
	print("=============================================file loop start")
	for line in readString.splitlines():
		if line == targetline:
			finalstrings.append(line+"\n")
			finalstrings.append(insertline+"\n")
			print("insert line " + insertline)
		else:
			print("-------- line " + line)
			finalstrings.append(line+"\n")
			

	mo.close()

	#remove the original file
	os.remove(file)

	#write to new file
	nmo = open(file, 'w+')
	nmo.writelines(finalstrings)
	nmo.close()
	print("=============================================file loop end")
	return;

def getpluginversion():
	plugin = open(cwd+"/Plugins/CognitiveVR/CognitiveVR.uplugin","r")
	pluginreadstring = plugin.read()
	print("=============================================get plugin version")
	for line in pluginreadstring.splitlines():
		if line.startswith('	"VersionName":'):
			vsplit = line.split(':')
			raw = vsplit[1][2:-2]
			return raw.replace('.','_')


#copy plugin folder to temp directory
print(cwd+"/CognitiveVRTest/Plugins/")
print(cwd+"/cognitiveVR_UnrealSDK_Dist/")

if os.path.exists(cwd+"/Plugins/"):
	shutil.rmtree(cwd+"/Plugins/")
	print("delete " + cwd+"/Plugins/")

shutil.copytree(cwd+"/CognitiveVRTest/Plugins/",cwd+"/Plugins/")

#delete binaries and intermediate if present
if os.path.exists(cwd+"/Plugins/CognitiveVR/Binaries/"):
	shutil.rmtree(cwd+"/Plugins/CognitiveVR/Binaries/")
	print("delete " + cwd+"/Plugins/CognitiveVR/Binaries/")
	
if os.path.exists(cwd+"/Plugins/CognitiveVR/Intermediate/"):
	shutil.rmtree(cwd+"/Plugins/CognitiveVR/Intermediate/")
	print("delete " + cwd+"/Plugins/CognitiveVR/Intermediate/")

#get the version
version = getpluginversion()
print (version)

#do all the update stuff

#12 add audio define to microphone header
insertline(cwd+"/Plugins\CognitiveVR\Source\CognitiveVR\Public\MicrophoneCaptureActor.h","#pragma once","#define NTDDI_THRESHOLD 0")

#14 replace editor selection code in dynamiccomponentdetails
replaceline(cwd+"/Plugins\CognitiveVR\Source\CognitiveVREditor\Private\SSceneSetupWidget.cpp","	UWorld* World = GEditor->LevelViewportClients[0]->GetWorld();","	UWorld* World = GEditor->GetLevelViewportClients()[0]->GetWorld();")
replaceline(cwd+"/Plugins\CognitiveVR\Source\CognitiveVREditor\Private\SSceneSetupWidget.cpp","		UWorld* World = GEditor->LevelViewportClients[0]->GetWorld();","		UWorld* World = GEditor->GetLevelViewportClients()[0]->GetWorld();")


# save to zip archive
output_filename = cwd+"/C3D_Plugin"+version+"_ue4"+enginesubversion
shutil.make_archive(output_filename, 'zip', cwd+"/Plugins/")

#delete the temp plugin directory
shutil.rmtree(cwd+"/Plugins/")
print("delete " + cwd+"/Plugins/")

print("complete!")

time.sleep(5)