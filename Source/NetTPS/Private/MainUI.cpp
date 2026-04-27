// Fill out your copyright notice in the Description page of Project Settings.


#include "MainUI.h"

#include "NetPlayerController.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/UniformGridPanel.h"

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

void UMainUI::NativeConstruct()
{
	Super::NativeConstruct();
	btn_retry->OnClicked.AddDynamic(this, &UMainUI::OnRetry);
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
		pc->ServerRPC_RespawnPlayer();
	}
}












