#include "Private/api/coreutilities.h"

using namespace cognitivevrapi;

CoreUtilities::CoreUtilities(FAnalyticsProviderCognitiveVR* sp)
{
	s = sp;
}

void CoreUtilities::NewUser(FString nuser_id)
{
	/*TArray< TSharedPtr<FJsonValue> > ObjArray;
	TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

	FString fs = FString::SanitizeFloat(Util::GetTimestamp());
	FString empty;

	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, nuser_id);
	Util::AppendToJsonArray(jsonArray, empty);

	//s->thread_manager->PushTask(NULL, "datacollector_newUser", jsonArray);
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject());
	jsonObject.Get()->SetStringField("method", "datacollector_newUser");
	jsonObject.Get()->SetField("args", jsonArray);

	s->thread_manager->AddJsonToBatch(jsonObject);*/
}

void CoreUtilities::NewDevice(FString ndevice_id)
{
	/*TArray< TSharedPtr<FJsonValue> > ObjArray;
	TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

	FString fs = FString::SanitizeFloat(Util::GetTimestamp());
	FString empty;

	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, empty);
	Util::AppendToJsonArray(jsonArray, ndevice_id);

	//s->thread_manager->PushTask(NULL, "datacollector_newDevice", jsonArray);
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject());
	jsonObject.Get()->SetStringField("method", "datacollector_newDevice");
	jsonObject.Get()->SetField("args", jsonArray);

	s->thread_manager->AddJsonToBatch(jsonObject);*/
}

void CoreUtilities::NewUserChecked(FString nuser_id)
{
	/*TArray< TSharedPtr<FJsonValue> > ObjArray;
	TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

	FString fs = FString::SanitizeFloat(Util::GetTimestamp());
	FString empty;

	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, nuser_id);
	Util::AppendToJsonArray(jsonArray, empty);

	//s->thread_manager->PushTask(NULL, "datacollector_newUserChecked", jsonArray);
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject());
	jsonObject.Get()->SetStringField("method", "datacollector_newUserChecked");
	jsonObject.Get()->SetField("args", jsonArray);

	s->thread_manager->AddJsonToBatch(jsonObject);*/
}

void CoreUtilities::NewDeviceChecked(FString ndevice_id)
{
	/*TArray< TSharedPtr<FJsonValue> > ObjArray;
	TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

	FString fs = FString::SanitizeFloat(Util::GetTimestamp());
	FString empty;

	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, empty);
	Util::AppendToJsonArray(jsonArray, ndevice_id);

	//s->thread_manager->PushTask(NULL, "datacollector_newDeviceChecked", jsonArray);
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject());
	jsonObject.Get()->SetStringField("method", "datacollector_newDeviceChecked");
	jsonObject.Get()->SetField("args", jsonArray);

	s->thread_manager->AddJsonToBatch(jsonObject);*/
}

void CoreUtilities::UpdateUserState(FString nuser_id, TSharedPtr<FJsonObject> nproperties)
{
	/*TArray< TSharedPtr<FJsonValue> > ObjArray;
	TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

	FString fs = FString::SanitizeFloat(Util::GetTimestamp());
	FString empty;

	if (nproperties.Get() == NULL)
	{
		nproperties = MakeShareable(new FJsonObject);
	}

	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, nuser_id);
	Util::AppendToJsonArray(jsonArray, empty);
	Util::AppendToJsonArray(jsonArray, nproperties);

	//s->thread_manager->PushTask(NULL, "datacollector_updateUserState", jsonArray);
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject());
	jsonObject.Get()->SetStringField("method", "datacollector_updateUserState");
	jsonObject.Get()->SetField("args", jsonArray);

	s->thread_manager->AddJsonToBatch(jsonObject);*/
}

void CoreUtilities::UpdateDeviceState(FString ndevice_id, TSharedPtr<FJsonObject> nproperties)
{
	/*TArray< TSharedPtr<FJsonValue> > ObjArray;
	TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

	FString fs = FString::SanitizeFloat(Util::GetTimestamp());
	FString empty;

	if (nproperties.Get() == NULL)
	{
		nproperties = MakeShareable(new FJsonObject);
	}
	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, empty); //user
	Util::AppendToJsonArray(jsonArray, ndevice_id); //device
	Util::AppendToJsonArray(jsonArray, nproperties);

	//s->thread_manager->PushTask(NULL, "datacollector_updateDeviceState", jsonArray);
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject());
	jsonObject.Get()->SetStringField("method", "datacollector_updateDeviceState");
	jsonObject.Get()->SetField("args", jsonArray);

	s->thread_manager->AddJsonToBatch(jsonObject);*/
}