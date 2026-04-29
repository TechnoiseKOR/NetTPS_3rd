// Fill out your copyright notice in the Description page of Project Settings.


#include "NetGameInstance.h"

#include "NetTPS.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"

void UNetGameInstance::Init()
{
	Super::Init();
	
	if ( auto subsys = IOnlineSubsystem::Get() )
	{
		// 서브시스템으로부터 세션인터페이스를 가져온다.
		sessionInterface = subsys->GetSessionInterface();
		
		sessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UNetGameInstance::OnCreateSessionComplete);
		sessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UNetGameInstance::OnFindSessionsComplete);
		
		
		FTimerHandle handle;
		GetWorld()->GetTimerManager().SetTimer(handle,
			FTimerDelegate::CreateLambda([&]
			{
				//CreateSession(mySessionName, 10);
				FindOtherSessions();
			}			
			), 2, false	);
		
	}
}

void UNetGameInstance::CreateSession(FString roomName, int32 playerCount)
{
	// 세션설정변수
	FOnlineSessionSettings sessionSettings;
	
	// 1. Dedicated Server 접속여부
	sessionSettings.bIsDedicated = false;
	
	// 2. 랜선(로컬)매칭을 할지 Steam 매칭을 사용할지 여부
	FName subsysName = IOnlineSubsystem::Get()->GetSubsystemName();
	sessionSettings.bIsLANMatch = subsysName == "NULL";
	
	// 3. 매칭이 온라인을 통해 노출될지 여부
	// false 이면 초대를 통해서만 입장이 가능하다.
	sessionSettings.bShouldAdvertise = true;
	
	// 4. 온라인 상태(presence) 정보를 활용할지 여부
	sessionSettings.bUsesPresence = true;
	sessionSettings.bUseLobbiesIfAvailable = true;
	
	// 5. 게임진행중에 참여를 허가할지 여부
	sessionSettings.bAllowJoinViaPresence = true;
	sessionSettings.bAllowJoinInProgress = true;
	
	// 6. 세션에 참여할 수 있는 공개(public) 연결의 최대 허용 수
	sessionSettings.NumPublicConnections = playerCount;
	
	// 7. 커스텀 룸네임 설정
	sessionSettings.Set(FName("ROOM_NAME"), roomName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	
	// 8. 호스트네임 설정	
	sessionSettings.Set(FName("HOST_NAME"), mySessionName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	
	
	// netID
	FUniqueNetIdPtr netID = GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().GetUniqueNetId();
		
	PRINTLOG(TEXT("Create Session Start : %s"), *mySessionName);
	sessionInterface->CreateSession(*netID, FName(mySessionName), sessionSettings);
	
	
}

void UNetGameInstance::OnCreateSessionComplete(FName SessionName,
	bool bWasSuccessful)
{
	PRINTLOG(TEXT("Session Name : %s, bWasSuccessful : %d"), *mySessionName, bWasSuccessful);
}

void UNetGameInstance::FindOtherSessions()
{
	sessionSearch = MakeShareable(new FOnlineSessionSearch());
	
	// 1. 세션 검색 조건 설정
	sessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	// 2. Lan 여부
	sessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == FName("NULL");
	
	// 3. 최대 검색 세션 수
	sessionSearch->MaxSearchResults = 10;
	
	// 4. 세션검색	
	sessionInterface->FindSessions(0, sessionSearch.ToSharedRef());
}

void UNetGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	// 찾기 실패시
	if (bWasSuccessful == false)
	{
		PRINTLOG(TEXT("Session search failed..."));
		return;
	}
	
	//  세션검색결과 배열
	auto results = sessionSearch->SearchResults;
	PRINTLOG(TEXT("Search Result Count : %d"), results.Num());
	
	// 유효성 체크
	for ( auto sr : results )
	{
		if ( sr.IsValid() == false )
		{
			continue;
		}
		
		FString roomName;
		sr.Session.SessionSettings.Get(FName("ROOM_NAME"), roomName);
		FString hostName;
		sr.Session.SessionSettings.Get(FName("HOST_NAME"), hostName);
		// 세션주인(방장) 이름
		FString userName = sr.Session.OwningUserName;
		// 입장가능한 플레이어 수
		int32 maxPlayerCount = sr.Session.SessionSettings.NumPublicConnections;
		// 현재 입장한 플레이어 수 ( 최대인원수 - 현재입장가능한 수 )
		int32 currentPlayerCount = maxPlayerCount - sr.Session.NumOpenPublicConnections;
		
		// 핑 정보
		int32 pingSpeed = sr.PingInMs;
		
		PRINTLOG(TEXT("%s : %s(%s) - (%d/%d), %dms"), *roomName, *hostName, *userName, currentPlayerCount, maxPlayerCount, pingSpeed);
		
	}
	
}

















