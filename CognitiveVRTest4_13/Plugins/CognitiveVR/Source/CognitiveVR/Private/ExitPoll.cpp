// Fill out your copyright notice in the Description page of Project Settings.

#include "CognitiveVR.h"
#include "ExitPoll.h"
#include "CognitiveVRSettings.h"
#include "Util.h"

void ExitPoll::Initialize()
{

}

FCognitiveExitPollResponse r;

void ExitPoll::MakeQuestionSetRequest(const FString Hook, const FCognitiveExitPollResponse& response)
{
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	FString url = "https://api.cognitivevr.io/products/altimagegames59340-unitywanderdemo-test/questionSetHooks/"+ Hook+"/questionSet";

	//HttpRequest->SetURL("https://s3.amazonaws.com/cvr-test/sdkversion.txt");
	HttpRequest->SetURL(url);
	//HttpRequest->SetHeader("Content-Type", "");
	//HttpRequest->SetHeader("Accept-Encoding", "identity");
	HttpRequest->SetVerb("GET");
	
	//s->exitpoll

	r = response;
	//ResponseDelegate = response;

	//HttpRequest->OnProcessRequestComplete().BindRaw(this, &FExitPoll::OnResponseReceivedAsync, callback);
	//HttpRequest->OnProcessRequestComplete().BindRaw(v, &FExitPoll::OnResponseReceivedAsync);
	HttpRequest->OnProcessRequestComplete().BindStatic(ExitPoll::OnResponseReceivedAsync);

	HttpRequest->ProcessRequest();

	//save response
	//call response in that static function

	//HttpRequest->SetContent(AllBytes);

	//response.Execute(FExitPollQuestionSet());
}

void ExitPoll::OnResponseReceivedAsync(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful/*, FCognitiveExitPollResponse reponseDelegate*/)
{
	FString UE4Str = Response->GetContentAsString();
	std::string content(TCHAR_TO_UTF8(*UE4Str));
	CognitiveLog::Info("ExitPoll::OnResponseReceivedAsync - Response: " + content);

	CognitiveVRResponse response = Network::ParseResponse(content);
	//callback(response);

	//build the question set from json

	auto set = FExitPollQuestionSet();
	TSharedPtr<FJsonObject> jobject;
	TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(UE4Str);
	if (FJsonSerializer::Deserialize(Reader, jobject))
	{
		set.id = jobject->GetStringField(TEXT("id"));
		set.customerId = jobject->GetStringField(TEXT("customerId"));
		set.name = jobject->GetStringField(TEXT("name"));
		set.title = jobject->GetStringField(TEXT("title"));
		set.version = jobject->GetNumberField(TEXT("version"));

		TArray<TSharedPtr<FJsonValue>> questions = jobject->GetArrayField(TEXT("questions"));
		for (int32 i = 0; i < questions.Num(); i++)
		{
			//deserialize more stuff
			auto qobject = questions[i]->AsObject();
			FExitPollQuestion q = FExitPollQuestion();
			q.title = qobject->GetStringField(TEXT("title"));
			q.type = qobject->GetStringField(TEXT("type"));
			set.questions.Add(q);
		} 

		// Do stuff here
		r.Execute(set);
	}
	else
	{
		r.Execute(FExitPollQuestionSet());
	}
}

void ExitPoll::SendQuestionResponses(FExitPollResponses Responses)
{
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	FString url = "";

	//HttpRequest->SetURL("http://192.168.1.145:3000/api/scenes");
	HttpRequest->SetURL(url);
	HttpRequest->SetHeader("Content-Type", "multipart/form-data; boundary=\"cJkER9eqUVwt2tgnugnSBFkGLAgt7djINNHkQP0i\"");
	HttpRequest->SetHeader("Accept-Encoding", "identity");
	HttpRequest->SetVerb("POST");
	//HttpRequest->SetContent(AllBytes);
}

/*FExitPollQuestionSet FExitPoll::GetExitPollQuestionSet(EResponseValueReturn& Out, FString Hook)
{
	return FExitPollQuestionSet();
}

void FExitPoll::SendExitPollResponse(FExitPollResponses FExitPoll)
{

}*/