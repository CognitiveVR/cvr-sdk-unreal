
#include "CognitiveVREditorPrivatePCH.h"
#include "BaseEditorToolCustomization.h"


#define LOCTEXT_NAMESPACE "BaseToolEditor"

void FBaseEditorToolCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TSet<UClass*> Classes;

	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);
	



	for (auto WeakObject : ObjectsBeingCustomized)
	{
		if (UObject* Instance = WeakObject.Get())
		{
			Classes.Add(Instance->GetClass());
			BlenderPathProperty = DetailBuilder.GetProperty("BlenderPath", Instance->GetClass());
		}
	}


	// Create a commands category
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(TEXT("Commands"));

	//go through all the functions. hand code functions into an order

	TArray<UFunction*> Functions;
	Functions.SetNum(7, false);

	for (UClass* Class : Classes)
	{
		for (TFieldIterator<UFunction> FuncIt(Class); FuncIt; ++FuncIt)
		{
			UFunction* Function = *FuncIt;
			if (Function->HasAnyFunctionFlags(FUNC_Exec) && (Function->NumParms == 0))
			{
				const FString FunctionName = Function->GetName();

				if (FunctionName == "Select_Blender")
				{
					Functions[0] = Function;
				}
				if (FunctionName == "Select_Export_Meshes")
				{
					Functions[1] = Function;
				}
				if (FunctionName == "Export_Selected")
				{
					Functions[2] = Function;
				}
				if (FunctionName == "Export_All")
				{
					Functions[3] = Function;
				}
				if (FunctionName == "Reduce_Meshes")
				{
					Functions[4] = Function;
				}
				if (FunctionName == "Reduce_Textures")
				{
					Functions[5] = Function;
				}
				if (FunctionName == "Http_Request")
				{
					Functions[6] = Function;
				}

				/*const FText ButtonCaption = FText::FromString(FunctionName);
				const FString FilterString = FunctionName;

				Category.AddCustomRow(FText::FromString(FilterString))
				.ValueContent()
				[
				SNew(SButton)
				.Text(ButtonCaption)
				.OnClicked(FOnClicked::CreateStatic(&FBaseEditorToolCustomization::ExecuteToolCommand, &DetailBuilder, Function))
				];*/
			}
		}

		BlenderPathProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, BlenderPath)); //i assume this is just fundamentally wrong

		FText p = GetBlenderPath();
		if (p.EqualTo(FText::FromString("")) && !HasSearchedForBlender)
		{

			HasSearchedForBlender = true;
			UE_LOG(LogTemp, Warning, TEXT("blender path is empty. search for blender"));
			SearchForBlender();
		}


		static FText propertyTextName;
		Category.AddCustomRow(propertyTextName)
		.WholeRowContent()
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(this, &FBaseEditorToolCustomization::GetBlenderPath)
		];
		
		FName someName;

		for (auto& Function : Functions)
		{
			const FString FunctionName = Function->GetName();
			//const FText ButtonCaption = FText::FromString(FunctionName);

			FString title = FunctionName;
			const FText ButtonCaption = FText::FromString(title.Replace(TEXT("_"), TEXT(" ")));

			const FString FilterString = FunctionName;

			if (ButtonCaption.EqualTo(FText::FromString("Select Blender")))
			{
				Category.AddCustomRow(FText::FromString(FilterString))
					.ValueContent()
					[
						SNew(SButton)
						//.IsEnabled(&FBaseEditorToolCustomization::HasFoundBlender.Get() || ButtonCaption.EqualTo(FText::FromString("Select Blender")))
					.IsEnabled(true)
					.Text(ButtonCaption)
					.OnClicked(FOnClicked::CreateStatic(&FBaseEditorToolCustomization::ExecuteToolCommand, &DetailBuilder, Function))
					];
			}
			else
			{
				Category.AddCustomRow(FText::FromString(FilterString))
					.ValueContent()
					[
						SNew(SButton)
						//.IsEnabled(&FBaseEditorToolCustomization::HasFoundBlender.Get() || ButtonCaption.EqualTo(FText::FromString("Select Blender")))
					.IsEnabled(this, &FBaseEditorToolCustomization::HasFoundBlender)
					.Text(ButtonCaption)
					.OnClicked(FOnClicked::CreateStatic(&FBaseEditorToolCustomization::ExecuteToolCommand, &DetailBuilder, Function))
					];
			}
		}
	}
}

bool FBaseEditorToolCustomization::HasFoundBlender() const
{
	return FBaseEditorToolCustomization::GetBlenderPath().ToString().Contains("blender.exe");
}

FText FBaseEditorToolCustomization::GetBlenderPath() const
{
	FString blendPath = "";
	BlenderPathProperty.Get()->GetValue(blendPath);
	
	return FText::FromString(blendPath);
}

void FBaseEditorToolCustomization::SearchForBlender()
{
	//try to find blender in program files
	FString testApp = "C:/Program Files/Blender Foundation/Blender/blender.exe";

	if (VerifyFileExists(testApp))
	{
		UE_LOG(LogTemp, Warning, TEXT("found blender at program files"));
		BlenderPathProperty.Get()->SetValue(testApp);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("didnt find blender at program files"));
	}
}

TSharedRef<IDetailCustomization> FBaseEditorToolCustomization::MakeInstance()
{
	return MakeShareable(new FBaseEditorToolCustomization);
}

FReply FBaseEditorToolCustomization::ExecuteToolCommand(IDetailLayoutBuilder* DetailBuilder, UFunction* MethodToExecute)
{
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder->GetObjectsBeingCustomized(ObjectsBeingCustomized);

	for (auto WeakObject : ObjectsBeingCustomized)
	{
		if (UObject* Instance = WeakObject.Get())
		{
			Instance->CallFunctionByNameWithArguments(*MethodToExecute->GetName(), *GLog, nullptr, true);
		}
	}


	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE