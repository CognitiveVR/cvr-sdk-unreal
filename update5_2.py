import os
import shutil
import time

cwd = os.getcwd()
version = "0"
engineversion = "5"
enginesubversion = "2"

def replaceline(file, linesrc, linedst):

	mo = open(file, "r")
	readString = mo.read()

	foundLine = False

	#replace mtl with references to pngs
	finalstrings=[]
	#print("=============================================file loop start")
	for line in readString.splitlines():
		if line == linesrc:
			finalstrings.append(linedst+"\n")
			print("replaced line " + linedst)
			foundLine = True
		else:
			#print("-------- line " + line)
			finalstrings.append(line+"\n")

	mo.close()

	if foundLine == False:
		print("couldnt find line to replace " + linesrc)
		#delete the temp plugin directory
		shutil.rmtree(cwd+"/Plugins/")
		print("delete " + cwd+"/Plugins/")
		input("Press Enter to continue...")

	#remove the original file
	os.remove(file)

	#write to new file
	nmo = open(file, 'w+')
	nmo.writelines(finalstrings)
	nmo.close()
	#print("=============================================file loop end")
	return;

def insertline(file, targetline, insertline):

	#mo = open(file, encoding='utf-8-sig')
	mo = open(file, "r")
	readString = mo.read()

	foundLine = False

	#replace mtl with references to pngs
	finalstrings=[]
	#print("=============================================file loop start")
	for line in readString.splitlines():
		if line == targetline:
			finalstrings.append(line+"\n")
			finalstrings.append(insertline+"\n")
			print("insert line " + insertline)
			foundLine = True
		else:
			#print("-------- line " + line)
			finalstrings.append(line+"\n")
			

	mo.close()

	if foundLine == False:
		print("couldnt find line to insert " + targetline)
		#delete the temp plugin directory
		shutil.rmtree(cwd+"/Plugins/")
		print("delete " + cwd+"/Plugins/")
		input("Press Enter to continue...")

	#remove the original file
	os.remove(file)

	#write to new file
	nmo = open(file, 'w+')
	nmo.writelines(finalstrings)
	nmo.close()
	#print("=============================================file loop end")
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

#1 comment out unsupport platform in build.cs
replaceline(cwd+"/Plugins\CognitiveVR\Source\CognitiveVR\CognitiveVR.Build.cs","			|| Target.Platform == UnrealTargetPlatform.Win32","//			|| Target.Platform == UnrealTargetPlatform.Win32")

#change definitions and oculus plugin include in build.cs
replaceline(cwd+"/Plugins\CognitiveVR\Source\CognitiveVR\CognitiveVR.Build.cs","			//Definitions.Add(\"INCLUDE_OCULUS_PLUGIN\");","			//PublicDefinitions.Add(\"INCLUDE_OCULUS_PLUGIN\");")

replaceline(cwd+"/Plugins\CognitiveVR\Source\CognitiveVR\CognitiveVR.Build.cs","			//PublicDependencyModuleNames.AddRange(new string[] { \"OnlineSubsystem\", \"OnlineSubsystemOculus\", \"LibOVRPlatform\", \"OculusHMD\" });","			//PublicDependencyModuleNames.AddRange(new string[] { \"OnlineSubsystem\", \"OnlineSubsystemOculus\", \"LibOVRPlatform\", \"OculusXRHMD\" });")


# save to zip archive
output_filename = cwd+"/C3D_Plugin"+version+"_ue"+engineversion+"_"+enginesubversion
shutil.make_archive(output_filename, 'zip', cwd+"/Plugins/")

#delete the temp plugin directory
shutil.rmtree(cwd+"/Plugins/")
print("delete " + cwd+"/Plugins/")

print("complete!")

time.sleep(1)
input("Press Enter to continue...")