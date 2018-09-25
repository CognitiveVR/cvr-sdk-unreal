import os
import shutil
import sys

cwd = os.getcwd()
targetdir = "cognitiveVR_UnrealSDK_temp"
version = "0"
enginesubversion = "20"

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
	plugin = open(cwd+"/"+targetdir+"/Plugins/CognitiveVR/CognitiveVR.uplugin","r")
	pluginreadstring = plugin.read()
	print("=============================================get plugin version")
	for line in pluginreadstring.splitlines():
		if line.startswith('	"VersionName":'):
			vsplit = line.split(':')
			raw = vsplit[1][2:-2]
			return raw.replace('.','_')

	
#if tree directory exists, don't try to copy it
if not os.path.exists(cwd+"/"+targetdir):
	shutil.copytree(cwd+"/cognitiveVR_UnrealSDK_Dist",cwd+"/"+targetdir)
	
#1 open and change uproject file
replaceline(cwd+"/"+targetdir+"/YourProjectName.uproject",'	"EngineAssociation": "4.13",',' 	"EngineAssociation": "4.'+enginesubversion+'",')

#2 get the version
version = getpluginversion()
print (version)

#3 add blueprinttype to dynamic data
replaceline(cwd+"/"+targetdir+"/Plugins\CognitiveVR\Source\CognitiveVR\Public\DynamicObject.h","USTRUCT()","USTRUCT(BlueprintType)")

#4 replace json attrvalue with json attrvaluestring
replaceline(cwd+"/"+targetdir+"/Plugins\CognitiveVR\Source\CognitiveVR\Private\CognitiveVR.cpp","			properties->SetStringField(Attr.AttrName, Attr.AttrValue);","			properties->SetStringField(Attr.AttrName, Attr.AttrValueString);")

#5 insert xr and hmd function headers
insertline(cwd+"/"+targetdir+"/Plugins\CognitiveVR\Source\CognitiveVR\Private\playertracker.h",'#include "DynamicObject.h"','#include "Runtime/HeadMountedDisplay/Public/IXRTrackingSystem.h"')
insertline(cwd+"/"+targetdir+"/Plugins\CognitiveVR\Source\CognitiveVR\Private\playertracker.h",'#include "DynamicObject.h"','#include "HeadMountedDisplayFunctionLibrary.h"')

#6 replace hmddevice with xrsystem
replaceline(cwd+"/"+targetdir+"/Plugins\CognitiveVR\Source\CognitiveVR\Private\playertracker.cpp","	if (GEngine->HMDDevice.IsValid())","	if (GEngine->XRSystem.IsValid())")
replaceline(cwd+"/"+targetdir+"/Plugins\CognitiveVR\Source\CognitiveVR\Private\playertracker.cpp","		DeviceName = GEngine->HMDDevice->GetDeviceName();","		DeviceName = GEngine->XRSystem->GetSystemName();")

#7 add workspacemenustructre to editor module
insertline(cwd+"/"+targetdir+"\Plugins\CognitiveVR\Source\CognitiveVREditor\Private\CognitiveVREditorModule.cpp",'#include "SSceneSetupWidget.h"','#include "WorkspaceMenuStructure.h"')

#8 add WorkspaceMenuStructure to module dependencies
insertline(cwd+"/"+targetdir+"\Plugins\CognitiveVR\Source\CognitiveVREditor\CognitiveVREditor.Build.cs",'				"LevelEditor",','				"WorkspaceMenuStructure",')

#9 readonlytargetinfo
replaceline(cwd+"/"+targetdir+"\Plugins\CognitiveVR\Source\CognitiveVREditor\CognitiveVREditor.Build.cs","	public CognitiveVREditor(TargetInfo Target)","	public CognitiveVREditor(ReadOnlyTargetRules Target):base(Target)")
replaceline(cwd+"/"+targetdir+"\Plugins\CognitiveVR\Source\CognitiveVR\CognitiveVR.Build.cs","		public CognitiveVR(TargetInfo Target)","		public CognitiveVR(ReadOnlyTargetRules Target): base(Target)")
replaceline(cwd+"/"+targetdir+"\Plugins\CognitiveVR\Source\CognitiveVR\CognitiveVR.Build.cs",'		string DirectXSDKDir = UEBuildConfiguration.UEThirdPartySourceDirectory + "Windows/DirectX";','		string DirectXSDKDir = Target.UEThirdPartySourceDirectory + "Windows/DirectX";')

#10 add core minimal to dynamiccomponentdetails
insertline(cwd+"/"+targetdir+"/Plugins\CognitiveVR\Source\CognitiveVREditor\Private\DynamicComponentDetails.h",'#include "BaseEditorTool.h"','#include "coreminimal.h"')

#11 replace editor selection code in dynamiccomponentdetails
replaceline(cwd+"/"+targetdir+"/Plugins\CognitiveVR\Source\CognitiveVREditor\Private\DynamicComponentDetails.cpp","	const TArray< TWeakObjectPtr<UObject> >& SelectedObjects = DetailLayout.GetDetailsView().GetSelectedObjects();","	const TArray< TWeakObjectPtr<UObject> >& SelectedObjects = DetailLayout.GetSelectedObjects();")

#12 add audio define to microphone header
insertline(cwd+"/"+targetdir+"/Plugins\CognitiveVR\Source\CognitiveVR\Public\MicrophoneCaptureActor.h","#pragma once","#define NTDDI_THRESHOLD 0")

print("made file changes")

#finally rename the folder to the correct sdk version

os.rename(cwd+"/"+targetdir,cwd+"/cognitiveVR_UnrealSDK_"+version+"_ue4"+enginesubversion)