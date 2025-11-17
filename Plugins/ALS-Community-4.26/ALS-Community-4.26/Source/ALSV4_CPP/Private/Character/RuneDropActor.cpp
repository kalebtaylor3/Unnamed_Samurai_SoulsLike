// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/RuneDropActor.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Character/PlayerStatsComponent.h"
#include "Character/ALSBaseCharacter.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "MyGameInstance.h"
#include "Kismet/GameplayStatics.h"

ARuneDropActor::ARuneDropActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->SetSphereRadius(100.f);
	Collision->SetCollisionProfileName("OverlapAll");
	Collision->OnComponentBeginOverlap.AddDynamic(this, &ARuneDropActor::OnOverlapBegin);

	NiagaraEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraEffect"));
	NiagaraEffect->SetupAttachment(RootComponent);
	NiagaraEffect->SetAutoActivate(true); // Or manually activate if needed
}

void ARuneDropActor::BeginPlay()
{
	Super::BeginPlay();
}

void ARuneDropActor::OnOverlapBegin(UPrimitiveComponent* Overlapped, AActor* Other, UPrimitiveComponent* OtherComp,
	int32 BodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AALSBaseCharacter* Player = Cast<AALSBaseCharacter>(Other))
	{
		if (UPlayerStatsComponent* Stats = Player->FindComponentByClass<UPlayerStatsComponent>())
		{
			Stats->AddRunes(RuneAmount);

			if (UMyGameInstance* GI = Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(this)))
			{
				GI->bHasRuneDrop = false;
				GI->DroppedRuneAmount = 0;
			}

			Destroy();
		}
	}
}