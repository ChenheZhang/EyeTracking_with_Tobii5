/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#include "TobiiGazeFocusableComponent.h"
#include "TobiiGTOMBlueprintLibrary.h"
#include "TobiiGTOMModule.h"

#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"

static TAutoConsoleVariable<int32> CVarDrawCleanUIDebug(TEXT("tobii.debug.CleanUI"), 1, TEXT("0 - CleanUI debug visualizations are not displayed. 1 - CleanUI debug visualizations are displayed."));

static TMap<FTobiiFocusableUID, TWeakObjectPtr<UTobiiGazeFocusableComponent>> GRegisteredTobiiFocusableComponents;
static TSet<FPrimitiveComponentId>* GazeFocusPrioSetA = nullptr;
static TSet<FPrimitiveComponentId>* GazeFocusPrioSetB = nullptr;
static bool bUseGazeFocusPrioSetA = true;

void UTobiiGazeFocusableComponent::ClearFocusableComponents()
{
	GRegisteredTobiiFocusableComponents.Empty();
}

const TMap<FTobiiFocusableUID, TWeakObjectPtr<UTobiiGazeFocusableComponent>>& UTobiiGazeFocusableComponent::GetFocusableComponents()
{
	return GRegisteredTobiiFocusableComponents;
}

void UTobiiGazeFocusableComponent::UpdateGazeFocusPrio(FVector POVActorLocation, int32 MaxNrFocusables)
{
	TMap<FPrimitiveComponentId, float> PrioMap;
	TSet<FPrimitiveComponentId> NonPrioSet;

	//Determine which primitive components should be considered for gaze focus.
	for (auto& FocusableElement : GRegisteredTobiiFocusableComponents)
	{
		if (FocusableElement.Value.IsValid())
		{
			UTobiiGazeFocusableComponent* FocusableComponent = FocusableElement.Value.Get();
			AActor* FocusableActor = FocusableComponent->GetOwner();
			if (FocusableActor != nullptr)
			{
				TArray<UActorComponent*> PrimitiveComponents = FocusableActor->GetComponentsByClass(UPrimitiveComponent::StaticClass());
				for (UActorComponent* Comp : PrimitiveComponents)
				{
					UPrimitiveComponent* PrimitiveComponent = (UPrimitiveComponent*)Comp;
					if (PrimitiveComponent == nullptr)
					{
						continue;
					}

					bool bSkipComponent = false;
					float MaxDistance = 0.0f;
					float Priority = 0.0f;
					for (const FName& CurrentTag : PrimitiveComponent->ComponentTags)
					{
						if (CurrentTag == FTobiiPrimitiveComponentGazeFocusTags::NotGazeFocusableTag)
						{
							bSkipComponent = true;
							break;
						}

						FString CurrentTagString = CurrentTag.ToString();
						if (CurrentTagString.StartsWith(FTobiiPrimitiveComponentGazeFocusTags::GazeFocusableMaximumDistanceTag))
						{
							FString Argument = CurrentTagString.RightChop(FTobiiPrimitiveComponentGazeFocusTags::GazeFocusableMaximumDistanceTag.Len());
							MaxDistance = FCString::Atof(*Argument);
						}
						if (CurrentTagString.StartsWith(FTobiiPrimitiveComponentGazeFocusTags::GazeFocusablePriorityTag))
						{
							FString Argument = CurrentTagString.RightChop(FTobiiPrimitiveComponentGazeFocusTags::GazeFocusablePriorityTag.Len());
							Priority = FCString::Atof(*Argument);
						}
					}

					if (bSkipComponent)
					{
						continue;
					}

					if (MaxDistance == 0.0f)
					{
						MaxDistance = FocusableComponent->DefaultMaxFocusDistance;
					}
					if (Priority == 0.0f)
					{
						Priority = FocusableComponent->DefaultFocusPriority;
					}

					bool bAcceptComponent = true;
					if (MaxDistance != 0.0f)
					{
						float Distance = FVector::Distance(POVActorLocation, PrimitiveComponent->GetComponentLocation());
						if (Distance > MaxDistance)
						{
							bAcceptComponent = false;
						}
					}

					if (bAcceptComponent)
					{
						if (Priority == 0.0f)
						{
							NonPrioSet.Add(PrimitiveComponent->ComponentId);
						}
						else
						{
							PrioMap.Add(PrimitiveComponent->ComponentId, Priority);
						}
					}
				}
			}
		}
	}

	PrioMap.ValueSort(TGreater<float>());

	TSet<FPrimitiveComponentId>* NewGazeFocusPrioSet = new TSet<FPrimitiveComponentId>();
	MaxNrFocusables -= 1; //Since we test before add, take that into account
	for (auto& SortedElement : PrioMap)
	{
		if (NewGazeFocusPrioSet->Num() >= MaxNrFocusables)
		{
			break;
		}
		NewGazeFocusPrioSet->Add(SortedElement.Key);
	}
	for (auto NonPrioElement : NonPrioSet)
	{
		if (NewGazeFocusPrioSet->Num() >= MaxNrFocusables)
		{
			break;
		}
		NewGazeFocusPrioSet->Add(NonPrioElement);
	}
	
	//Destroy old set, set our new one and flip our record pointer so the one we swapped into becomes the active one.
	TSet<FPrimitiveComponentId>*& ListPtr = bUseGazeFocusPrioSetA ? GazeFocusPrioSetB : GazeFocusPrioSetA;
	if (ListPtr != nullptr)
	{
		delete ListPtr;
	}
	ListPtr = NewGazeFocusPrioSet;
	bUseGazeFocusPrioSetA = !bUseGazeFocusPrioSetA;
}

const TSet<FPrimitiveComponentId>* UTobiiGazeFocusableComponent::GetGazeFocusPrioSet()
{
	return bUseGazeFocusPrioSetA ? GazeFocusPrioSetA : GazeFocusPrioSetB;
}

bool UTobiiGazeFocusableComponent::IsPrimitiveFocusable(UPrimitiveComponent* Primitive)
{
	if (Primitive == nullptr || !Primitive->IsVisible() || Primitive->ComponentHasTag(FTobiiPrimitiveComponentGazeFocusTags::NotGazeFocusableTag))
	{
		//Override disabled
		return false;
	}
	else if (Primitive->ComponentHasTag(FTobiiPrimitiveComponentGazeFocusTags::GazeFocusableTag))
	{
		//Override enabled
		return true;
	}
	else if (Primitive->GetOwner())
	{
		UTobiiGazeFocusableComponent* GazeFocusable = (UTobiiGazeFocusableComponent*)Primitive->GetOwner()->GetComponentByClass(UTobiiGazeFocusableComponent::StaticClass());
		if (GazeFocusable != nullptr && GazeFocusable->bDefaultIsFocusable)
		{
			return true;
		}
	}

	return false;
}

bool UTobiiGazeFocusableComponent::GetMaxFocusDistanceForPrimitive(UPrimitiveComponent* Primitive, float& OutMaxDistance)
{
	if (Primitive == nullptr)
	{
		return false;
	}

	bool bMaxDistanceFound = false;

	for (const FName& CurrentTag : Primitive->ComponentTags)
	{
		FString CurrentTagString = CurrentTag.ToString();
		if (CurrentTagString.StartsWith(FTobiiPrimitiveComponentGazeFocusTags::GazeFocusableMaximumDistanceTag))
		{
			FString Argument = CurrentTagString.RightChop(FTobiiPrimitiveComponentGazeFocusTags::GazeFocusableMaximumDistanceTag.Len());
			OutMaxDistance = FCString::Atof(*Argument);
			bMaxDistanceFound = OutMaxDistance != 0.0f; //Atof returns 0.0 on failure
		}
	}

	if (!bMaxDistanceFound && Primitive->GetOwner())
	{
		UTobiiGazeFocusableComponent* GazeFocusableComponent = (UTobiiGazeFocusableComponent*)Primitive->GetOwner()->GetComponentByClass(UTobiiGazeFocusableComponent::StaticClass());
		if (GazeFocusableComponent != nullptr && GazeFocusableComponent->DefaultMaxFocusDistance > FLT_EPSILON)
		{
			OutMaxDistance = GazeFocusableComponent->DefaultMaxFocusDistance;
			bMaxDistanceFound = true;
		}
	}

	return bMaxDistanceFound;
}

float UTobiiGazeFocusableComponent::GetFocusPriorityForPrimitive(UPrimitiveComponent* Primitive)
{
	float Priority = 0.0f;
	bool bPriorityFound = false;
	if (Primitive == nullptr)
	{
		return Priority;
	}

	for (const FName& CurrentTag : Primitive->ComponentTags)
	{
		FString CurrentTagString = CurrentTag.ToString();
		if (CurrentTagString.StartsWith(FTobiiPrimitiveComponentGazeFocusTags::GazeFocusablePriorityTag))
		{
			FString Argument = CurrentTagString.RightChop(FTobiiPrimitiveComponentGazeFocusTags::GazeFocusablePriorityTag.Len());
			Priority = FCString::Atof(*Argument);
			bPriorityFound = Priority != 0.0f; //Atof returns 0.0 on failure
		}
	}

	if (!bPriorityFound && Primitive->GetOwner())
	{
		UTobiiGazeFocusableComponent* GazeFocusableComponent = (UTobiiGazeFocusableComponent*)Primitive->GetOwner()->GetComponentByClass(UTobiiGazeFocusableComponent::StaticClass());
		if (GazeFocusableComponent != nullptr)
		{
			Priority = GazeFocusableComponent->DefaultFocusPriority;
		}
	}

	return Priority;
}

FName UTobiiGazeFocusableComponent::GetFocusLayerForPrimitive(UPrimitiveComponent* Primitive)
{
	FName FocusLayer = FName("Default");
	bool bFocusLayerFound = false;
	if (Primitive == nullptr)
	{
		return FocusLayer;
	}

	for (const FName& CurrentTag : Primitive->ComponentTags)
	{
		FString CurrentTagString = CurrentTag.ToString();
		if (CurrentTagString.StartsWith(FTobiiPrimitiveComponentGazeFocusTags::GazeFocusableLayerTag))
		{
			FString Argument = CurrentTagString.RightChop(FTobiiPrimitiveComponentGazeFocusTags::GazeFocusableLayerTag.Len());
			Argument.TrimStartAndEndInline();
			if (!Argument.IsEmpty())
			{
				FocusLayer = FName(*Argument);
				bFocusLayerFound = true;
			}
		}
	}

	if (!bFocusLayerFound && Primitive->GetOwner())
	{
		UTobiiGazeFocusableComponent* GazeFocusableComponent = (UTobiiGazeFocusableComponent*)Primitive->GetOwner()->GetComponentByClass(UTobiiGazeFocusableComponent::StaticClass());
		if (GazeFocusableComponent != nullptr)
		{
			FocusLayer = GazeFocusableComponent->DefaultFocusLayer;
		}
	}

	return FocusLayer;
}


bool UTobiiGazeFocusableComponent::IsWidgetFocusable(UTobiiGazeFocusableWidget* GazeFocusableWidget)
{
	if (GazeFocusableWidget == nullptr || !GazeFocusableWidget->IsVisible() || !GazeFocusableWidget->bIsGazeFocusable)
	{
		return false;
	}
	else if (GazeFocusableWidget->GetWorldSpaceHostWidgetComponent() != nullptr && GazeFocusableWidget->GetWorldSpaceHostWidgetComponent()->GetOwner() != nullptr)
	{
		if (!GazeFocusableWidget->GetWorldSpaceHostWidgetComponent()->IsVisible())
		{
			return false;
		}
		
		UTobiiGazeFocusableComponent* GazeFocusable = (UTobiiGazeFocusableComponent*)GazeFocusableWidget->GetWorldSpaceHostWidgetComponent()->GetOwner()->GetComponentByClass(UTobiiGazeFocusableComponent::StaticClass());
		if (GazeFocusable != nullptr)
		{
			return GazeFocusable->bDefaultIsFocusable;
		}
	}

	return true;
}

bool UTobiiGazeFocusableComponent::GetMaxFocusDistanceForWidget(UTobiiGazeFocusableWidget* GazeFocusableWidget, float& OutMaxDistance)
{
	if (GazeFocusableWidget == nullptr)
	{
		return false;
	}

	bool bMaxDistanceFound = false;
	if (GazeFocusableWidget->MaxFocusDistance >= FLT_EPSILON)
	{
		OutMaxDistance = GazeFocusableWidget->MaxFocusDistance;
		bMaxDistanceFound = true;
	}
	else if (GazeFocusableWidget->GetWorldSpaceHostWidgetComponent() != nullptr && GazeFocusableWidget->GetWorldSpaceHostWidgetComponent()->GetOwner() != nullptr)
	{
		UTobiiGazeFocusableComponent* GazeFocusableComponent = (UTobiiGazeFocusableComponent*)GazeFocusableWidget->GetWorldSpaceHostWidgetComponent()->GetOwner()->GetComponentByClass(UTobiiGazeFocusableComponent::StaticClass());
		if (GazeFocusableComponent != nullptr && GazeFocusableComponent->DefaultMaxFocusDistance > FLT_EPSILON)
		{
			OutMaxDistance = GazeFocusableComponent->DefaultMaxFocusDistance;
			bMaxDistanceFound = true;
		}
	}

	return bMaxDistanceFound;
}

float UTobiiGazeFocusableComponent::GetFocusPriorityForWidget(UTobiiGazeFocusableWidget* GazeFocusableWidget)
{
	if (GazeFocusableWidget == nullptr)
	{
		return 0.0f;
	}

	if (GazeFocusableWidget->GazeFocusPriority >= 0.0f)
	{
		return GazeFocusableWidget->GazeFocusPriority;
	}
	else if (GazeFocusableWidget->GetWorldSpaceHostWidgetComponent() != nullptr && GazeFocusableWidget->GetWorldSpaceHostWidgetComponent()->GetOwner() != nullptr)
	{
		UTobiiGazeFocusableComponent* GazeFocusableComponent = (UTobiiGazeFocusableComponent*)GazeFocusableWidget->GetWorldSpaceHostWidgetComponent()->GetOwner()->GetComponentByClass(UTobiiGazeFocusableComponent::StaticClass());
		if (GazeFocusableComponent)
		{
			return GazeFocusableComponent->DefaultFocusPriority;
		}
	}

	return 0.0f;
}

FName UTobiiGazeFocusableComponent::GetFocusLayerForWidget(UTobiiGazeFocusableWidget* GazeFocusableWidget)
{
	FName FocusLayer = FName("Default");
	if (GazeFocusableWidget == nullptr)
	{
		return FocusLayer;
	}

	if (!GazeFocusableWidget->FocusLayer.IsEmpty())
	{
		FocusLayer = FName(*GazeFocusableWidget->FocusLayer);
	}
	else if (GazeFocusableWidget->GetWorldSpaceHostWidgetComponent() != nullptr && GazeFocusableWidget->GetWorldSpaceHostWidgetComponent()->GetOwner() != nullptr)
	{
		UTobiiGazeFocusableComponent* GazeFocusableComponent = (UTobiiGazeFocusableComponent*)GazeFocusableWidget->GetWorldSpaceHostWidgetComponent()->GetOwner()->GetComponentByClass(UTobiiGazeFocusableComponent::StaticClass());
		if (GazeFocusableComponent != nullptr)
		{
			FocusLayer = GazeFocusableComponent->DefaultFocusLayer;
		}
	}

	return FocusLayer;
}

//////////////////////////////////////////////

UTobiiGazeFocusableComponent::UTobiiGazeFocusableComponent()
	: bDefaultIsFocusable(true)
	, DefaultFocusPriority(0.0f)
	, DefaultMaxFocusDistance(0.0f)
	, DefaultFocusLayer("Default")

	, bWidgetsRefreshedOnce(false)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTobiiGazeFocusableComponent::PrimitiveReceivedGazeFocus(UPrimitiveComponent* FocusedComponent)
{
	OnPrimitiveReceivedGazeFocus.Broadcast(FocusedComponent);
}

void UTobiiGazeFocusableComponent::PrimitiveLostGazeFocus(UPrimitiveComponent* FocusedComponent)
{
	OnPrimitiveLostGazeFocus.Broadcast(FocusedComponent);
}

void UTobiiGazeFocusableComponent::WidgetReceivedGazeFocus( UTobiiGazeFocusableWidget* FocusedWidget)
{
	OnWidgetReceivedGazeFocus.Broadcast(FocusedWidget);
}

void UTobiiGazeFocusableComponent::WidgetLostGazeFocus(UTobiiGazeFocusableWidget* FocusedWidget)
{
	OnWidgetLostGazeFocus.Broadcast(FocusedWidget);
}

const TArray<TWeakObjectPtr<UTobiiGazeFocusableWidget>>& UTobiiGazeFocusableComponent::GetFocusableWidgetsForPrimitiveComponent(UPrimitiveComponent* PrimitiveComponent)
{
	if (PrimitiveComponent == nullptr || !PrimitiveMap.Contains(PrimitiveComponent->GetUniqueID()))
	{
		static TArray<TWeakObjectPtr<UTobiiGazeFocusableWidget>> DefaultArray;
		return DefaultArray;
	}

	return PrimitiveMap[PrimitiveComponent->GetUniqueID()];
}

void UTobiiGazeFocusableComponent::BeginPlay()
{
	Super::BeginPlay();

	bWidgetsRefreshedOnce = false;
	GRegisteredTobiiFocusableComponents.Add(GetUniqueID(), this);
}

void UTobiiGazeFocusableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GRegisteredTobiiFocusableComponents.Remove(GetUniqueID());

	Super::EndPlay(EndPlayReason);
}

void UTobiiGazeFocusableComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GEngine == nullptr
		|| GEngine->GameViewport == nullptr
		|| GEngine->GameViewport->GetGameViewport() == nullptr)
	{
		return;
	}

	if (!bWidgetsRefreshedOnce)
	{
		bWidgetsRefreshedOnce = true;
		RefreshOwnedWidgets();
	}
}

void UTobiiGazeFocusableComponent::RefreshOwnedWidgets()
{
	AllFocusableWidgets.Empty();
	PrimitiveMap.Empty();

	AActor* Owner = GetOwner();
 	if (Owner != nullptr)
 	{
 		TArray<UActorComponent*> WidgetComponents = Owner->GetComponentsByClass(UWidgetComponent::StaticClass());
 		for (UActorComponent* Component : WidgetComponents)
 		{
			UWidgetComponent* WidgetComponent = (UWidgetComponent*)Component;
			TArray<TWeakObjectPtr<UTobiiGazeFocusableWidget>>& NewFocusableArray = PrimitiveMap.Add(WidgetComponent->GetUniqueID(), TArray<TWeakObjectPtr<UTobiiGazeFocusableWidget>>());
 			GatherGazeFocusableWidgets(WidgetComponent->GetUserWidgetObject(), NewFocusableArray, WidgetComponent);
 		}
 	}
}

void UTobiiGazeFocusableComponent::GatherGazeFocusableWidgets(UWidget* Parent, TArray<TWeakObjectPtr<UTobiiGazeFocusableWidget>>& WidgetArray, UWidgetComponent* OptionalHostWidgetComponent)
{
	UUserWidget* UserWidget = Cast<UUserWidget>(Parent);
	if (UserWidget != nullptr)
	{
		GatherGazeFocusableWidgets(UserWidget->GetRootWidget(), WidgetArray, OptionalHostWidgetComponent);
	}
	else
	{
		UTobiiGazeFocusableWidget* CleanUIContainer = Cast<UTobiiGazeFocusableWidget>(Parent);
		if (CleanUIContainer != nullptr)
		{
			CleanUIContainer->RegisterWidgetToGTOM(OptionalHostWidgetComponent);
			AllFocusableWidgets.Add(CleanUIContainer->GetUniqueID(), CleanUIContainer);
			WidgetArray.Add(CleanUIContainer);
		}

		UPanelWidget* Panel = Cast<UPanelWidget>(Parent);
		if (Panel != nullptr)
		{
			for (int32 ChildIdx = 0; ChildIdx < Panel->GetChildrenCount(); ChildIdx++)
			{
				GatherGazeFocusableWidgets(Panel->GetChildAt(ChildIdx), WidgetArray, OptionalHostWidgetComponent);
			}
		}
	}
}
