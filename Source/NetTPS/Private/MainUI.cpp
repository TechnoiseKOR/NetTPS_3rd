// Fill out your copyright notice in the Description page of Project Settings.


#include "MainUI.h"

#include "ChatWidget.h"
#include "NetGameInstance.h"
#include "NetPlayerController.h"
#include "NetTPSCharacter.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void UMainUI::ShowCrosshair(bool isShow)
{
	if ( isShow )
	{
		img_Crosshair->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		img_Crosshair->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMainUI::AddBullet()
{
	auto BulletWidget = CreateWidget(GetWorld(), BulletUIFactory);
	BulletPanel->AddChildToUniformGrid(BulletWidget, 1, BulletPanel->GetChildrenCount());
}

void UMainUI::PopBullet(int32 index)
{
	BulletPanel->RemoveChildAt(index);
}

void UMainUI::RemoveAllAmmo()
{
	for ( auto bulletWidget : BulletPanel->GetAllChildren() )
	{
		BulletPanel->RemoveChild(bulletWidget);
	}
}

void UMainUI::PlayDamageAnimation()
{
	PlayAnimation(DamageAnim);
}

void UMainUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	// 플레이어 리스트 출력
	TArray<APlayerState*> playerArr = GetWorld()->GetGameState()->PlayerArray;
	
	FString name;
	for ( APlayerState* pState : playerArr )
	{
		name.Append(FString::Printf(TEXT("%s : %d\n"), *(pState->GetPlayerName()), (int32)pState->GetScore()));
	}
	txt_users->SetText(FText::FromString(name));
}

void UMainUI::NativeConstruct()
{
	Super::NativeConstruct();
	btn_retry->OnClicked.AddDynamic(this, &UMainUI::OnRetry);
	btn_exit->OnClicked.AddDynamic(this, &UMainUI::OnExit);
	btn_send->OnClicked.AddDynamic(this, &UMainUI::SendMsg);
}

void UMainUI::OnRetry()
{
	// 게임종료 UI 안보이도록 처리
	GameoverUI->SetVisibility(ESlateVisibility::Hidden);
	auto pc = Cast<ANetPlayerController>(GetWorld()->GetFirstPlayerController());
	if ( pc )
	{
		// 마우스 커서를 안보이도록 처리
		pc->SetShowMouseCursor(false);
		//pc->ServerRPC_RespawnPlayer();
		pc->ServerRPC_ChangeToSpectator();
	}
}

void UMainUI::OnExit()
{
	auto gi = Cast<UNetGameInstance>(GetWorld()->GetGameInstance());
	if ( gi )
	{
		gi->ExitRoom();
	}
}

void UMainUI::SendMsg()
{
	FString msg = edit_input->GetText().ToString();
	edit_input->SetText(FText::GetEmpty());
	if ( msg.IsEmpty() == false )
	{
		auto pc = Cast<ANetPlayerController>(GetWorld()->GetFirstPlayerController());
		if ( pc )
		{
			auto player = Cast<ANetTPSCharacter>(pc->GetPawn());
			if ( player )
			{
				player->ServerRPC_SendMsg(msg);
			}
		}
	}
}

void UMainUI::ReceiveMsg(const FString& msg)
{	
	auto msgWidget = CreateWidget<UChatWidget>(GetWorld(), chatWidget);
	msgWidget->txt_msg->SetText(FText::FromString(msg));
	scroll_msgList->AddChild(msgWidget);
	scroll_msgList->ScrollToEnd();	
}












