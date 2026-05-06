// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "NetGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FSessionInfo
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	FString roomName;	
	UPROPERTY(BlueprintReadOnly)
	FString hostName;
	UPROPERTY(BlueprintReadOnly)
	FString playerCount;
	UPROPERTY(BlueprintReadOnly)
	int32 pingSpeed;
	UPROPERTY(BlueprintReadOnly)
	int32 index;
	
	inline FString ToString()
	{
		return  FString::Printf(TEXT("[%d]%s : %s - %s, %dms"), index, *roomName, *hostName, *playerCount, pingSpeed);
	}
};



// 세션검색 끝났을 때 호출될 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSearchSignature, const FSessionInfo&, sessionInfo);


/**
 * 
 */
UCLASS()
class NETTPS_API UNetGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;	
	
	
public:
	IOnlineSessionPtr sessionInterface;
	
	void CreateSession(FString roomName, int32 playerCount);
	
	// 세션(호스트) 이름
	FString mySessionName = "Technoise";
	
	UFUNCTION()
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	
	
public:	// ------------- 방검색 -------------
	TSharedPtr<FOnlineSessionSearch> sessionSearch;
	void FindOtherSessions();
	
	void OnFindSessionsComplete(bool bWasSuccessful);
	
	// 방찾기완료 콜백을 등록할 델리게이트
	FSearchSignature onSearchCompleted;
	
	
};











