#include "CognitiveVREditorPrivatePCH.h"
#include "SetupCustomization.h"

#define LOCTEXT_NAMESPACE "BaseToolEditor"

TSharedRef<IDetailCustomization> FSetupCustomization::MakeInstance()
{
	GLog->Log("make instance of customization");
	return MakeShareable(new FSetupCustomization());
}
void FSetupCustomization::CustomizeDetails(IDetailLayoutBuilder & DetailBuilder)
{

}
#undef LOCTEXT_NAMESPACE