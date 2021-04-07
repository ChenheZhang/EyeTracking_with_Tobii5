/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#include "TobiiGTOMBlueprintLibrary.h"
#include "TobiiGTOMModule.h"
#include "TobiiGazeFocusableComponent.h"
#include "TobiiGTOMInternalTypes.h"
#include "TobiiGTOMEngine.h"

#include "IEyeTracker.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "Slate/SceneViewport.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TWO_PI (6.28318530718)

UTobiiGTOMBlueprintLibrary::UTobiiGTOMBlueprintLibrary(const class FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
}

void UTobiiGTOMBlueprintLibrary::SetGTOMPlayerController(APlayerController* NewGTOMPlayerController)
{
	if (FTobiiGTOMModule::IsAvailable())
	{
		if (FTobiiGTOMModule::Get().GTOMInputDevice.IsValid())
		{
			FTobiiGTOMModule::Get().GTOMInputDevice->GTOMPlayerController = NewGTOMPlayerController;
		}
		if (GEngine && GEngine->EyeTrackingDevice.IsValid())
		{
			GEngine->EyeTrackingDevice->SetEyeTrackedPlayer(NewGTOMPlayerController);
		}
	}
}

bool UTobiiGTOMBlueprintLibrary::GetGazeFocusData(FTobiiGazeFocusData& OutFocusData)
{
	if (FTobiiGTOMModule::IsAvailable() && FTobiiGTOMModule::Get().GTOMInputDevice.IsValid())
	{
		const TArray<FTobiiGazeFocusData>& FocusData = FTobiiGTOMModule::Get().GTOMInputDevice->GetFocusData();
		if (FocusData.Num() > 0)
		{
			OutFocusData = FocusData[0];
			return true;
		}
	}

	OutFocusData.FocusedActor = nullptr;
	OutFocusData.FocusedPrimitiveComponent = nullptr;
	OutFocusData.FocusedWidget = nullptr;
	return false;
}

bool UTobiiGTOMBlueprintLibrary::GetAllGazeFocusData(TArray<FTobiiGazeFocusData>& OutFocusData)
{
	OutFocusData.Empty();

	if (FTobiiGTOMModule::IsAvailable() && FTobiiGTOMModule::Get().GTOMInputDevice.IsValid())
	{
		OutFocusData = FTobiiGTOMModule::Get().GTOMInputDevice->GetFocusData();
		return OutFocusData.Num() > 0;
	}

	return false;
}

bool UTobiiGTOMBlueprintLibrary::GetFilteredGazeFocusData(const TArray<FName>& FocusLayerFilterList, const bool bIsWhiteList, const bool bWantPrimitives, const bool bWantWidgets, FTobiiGazeFocusData& OutFocusData)
{
	if (FTobiiGTOMModule::IsAvailable() && FTobiiGTOMModule::Get().GTOMInputDevice.IsValid())
	{
		const TArray<FTobiiGazeFocusData>& AllFocusData = FTobiiGTOMModule::Get().GTOMInputDevice->GetFocusData();
		for (const FTobiiGazeFocusData& FocusData : AllFocusData)
		{
			if (bWantWidgets && FocusData.FocusedWidget.IsValid())
			{
				if (FTobiiGTOMUtils::ValidateFocusLayers(FocusLayerFilterList, bIsWhiteList, FocusData.FocusedWidget.Get()))
				{
					OutFocusData = FocusData;
					return true;
				}
			}
			else if(bWantPrimitives && FocusData.FocusedPrimitiveComponent.IsValid() && !FocusData.FocusedWidget.IsValid())
			{
				if (FTobiiGTOMUtils::ValidateFocusLayers(FocusLayerFilterList, bIsWhiteList, FocusData.FocusedPrimitiveComponent.Get()))
				{
					OutFocusData = FocusData;
					return true;
				}
			}
		}
	}

	OutFocusData.FocusedActor = nullptr;
	OutFocusData.FocusedPrimitiveComponent = nullptr;
	OutFocusData.FocusedWidget = nullptr;
	return false;
}

bool UTobiiGTOMBlueprintLibrary::GetAllFilteredGazeFocusData(const TArray<FName>& FocusLayerFilterList, const bool bIsWhiteList, const bool bWantPrimitives, const bool bWantWidgets, TArray<FTobiiGazeFocusData>& OutFocusData)
{
	OutFocusData.Empty();

	if (FTobiiGTOMModule::IsAvailable() && FTobiiGTOMModule::Get().GTOMInputDevice.IsValid())
	{
		const TArray<FTobiiGazeFocusData>& AllFocusData = FTobiiGTOMModule::Get().GTOMInputDevice->GetFocusData();
		for (const FTobiiGazeFocusData& FocusData : AllFocusData)
		{
			if (bWantWidgets && FocusData.FocusedWidget.IsValid())
			{
				if (FTobiiGTOMUtils::ValidateFocusLayers(FocusLayerFilterList, bIsWhiteList, FocusData.FocusedWidget.Get()))
				{
					OutFocusData.Add(FocusData);
				}
			}
			else if (bWantPrimitives && FocusData.FocusedPrimitiveComponent.IsValid() && !FocusData.FocusedWidget.IsValid())
			{
				if (FTobiiGTOMUtils::ValidateFocusLayers(FocusLayerFilterList, bIsWhiteList, FocusData.FocusedPrimitiveComponent.Get()))
				{
					OutFocusData.Add(FocusData);
				}
			}
		}
	}

	return OutFocusData.Num() > 0;
}

void UTobiiGTOMBlueprintLibrary::RegisterScreenSpaceGazeFocusableWidgets(UWidget* Root)
{
	UUserWidget* UserWidget = Cast<UUserWidget>(Root);
	if (UserWidget != nullptr)
	{
		RegisterScreenSpaceGazeFocusableWidgets(UserWidget->GetRootWidget());
	}
	else
	{
		UTobiiGazeFocusableWidget* GazeFocusableWidget = Cast<UTobiiGazeFocusableWidget>(Root);
		if (GazeFocusableWidget != nullptr)
		{
			GazeFocusableWidget->RegisterWidgetToGTOM(nullptr);
		}

		UPanelWidget* Panel = Cast<UPanelWidget>(Root);
		if (Panel != nullptr)
		{
			for (int32 ChildIdx = 0; ChildIdx < Panel->GetChildrenCount(); ChildIdx++)
			{
				RegisterScreenSpaceGazeFocusableWidgets(Panel->GetChildAt(ChildIdx));
			}
		}
	}
}

const FHitResult& UTobiiGTOMBlueprintLibrary::GetNaiveGazeHit()
{
	static FHitResult Dummy;
	if (FTobiiGTOMModule::IsAvailable() && FTobiiGTOMModule::Get().GTOMInputDevice.IsValid())
	{
		return FTobiiGTOMModule::Get().GTOMInputDevice->CombinedWorldGazeHitData;
	}
	else
	{
		return Dummy;
	}
}

bool UTobiiGTOMBlueprintLibrary::IsPrimitiveComponentGazeFocusable(UPrimitiveComponent* PrimitiveComponent)
{
	return UTobiiGazeFocusableComponent::IsPrimitiveFocusable(PrimitiveComponent);
}

bool UTobiiGTOMBlueprintLibrary::IsWidgetGazeFocusable(UWidget* Widget)
{
	return UTobiiGazeFocusableComponent::IsWidgetFocusable(Cast<UTobiiGazeFocusableWidget>(Widget));
}

bool UTobiiGTOMBlueprintLibrary::GetPrimitiveComponentFocusOffset(USceneComponent* Component, FVector& OutFocusOffset)
{
	OutFocusOffset = FVector::ZeroVector;
	if (Component == nullptr)
	{
		return false;
	}

	for (const FName& CurrentTag : Component->ComponentTags)
	{
		FString CurrentTagString = CurrentTag.ToString();
		if (CurrentTagString.StartsWith(FTobiiPrimitiveComponentGazeFocusTags::PrimitiveFocusOffsetXTag))
		{
			FString Argument = CurrentTagString.RightChop(FTobiiPrimitiveComponentGazeFocusTags::PrimitiveFocusOffsetXTag.Len());
			OutFocusOffset.X += FCString::Atof(*Argument);
		}
		if (CurrentTagString.StartsWith(FTobiiPrimitiveComponentGazeFocusTags::PrimitiveFocusOffsetYTag))
		{
			FString Argument = CurrentTagString.RightChop(FTobiiPrimitiveComponentGazeFocusTags::PrimitiveFocusOffsetYTag.Len());
			OutFocusOffset.Y += FCString::Atof(*Argument);
		}
		if (CurrentTagString.StartsWith(FTobiiPrimitiveComponentGazeFocusTags::PrimitiveFocusOffsetZTag))
		{
			FString Argument = CurrentTagString.RightChop(FTobiiPrimitiveComponentGazeFocusTags::PrimitiveFocusOffsetZTag.Len());
			OutFocusOffset.Z += FCString::Atof(*Argument);
		}
	}

	return true;
}

bool UTobiiGTOMBlueprintLibrary::GetPrimitiveComponentFocusLocation(USceneComponent* Component, FVector& OutFocusLocation)
{
	OutFocusLocation = FVector::ZeroVector;
	if (Component == nullptr)
	{
		return false;
	}

	OutFocusLocation = Component->GetComponentLocation();

	FVector Offset = FVector::ZeroVector;
	for (const FName& CurrentTag : Component->ComponentTags)
	{
		FString CurrentTagString = CurrentTag.ToString();
		if (CurrentTagString.StartsWith(FTobiiPrimitiveComponentGazeFocusTags::PrimitiveFocusOffsetXTag))
		{
			FString Argument = CurrentTagString.RightChop(FTobiiPrimitiveComponentGazeFocusTags::PrimitiveFocusOffsetXTag.Len());
			Offset.X += FCString::Atof(*Argument);
		}
		if (CurrentTagString.StartsWith(FTobiiPrimitiveComponentGazeFocusTags::PrimitiveFocusOffsetYTag))
		{
			FString Argument = CurrentTagString.RightChop(FTobiiPrimitiveComponentGazeFocusTags::PrimitiveFocusOffsetYTag.Len());
			Offset.Y += FCString::Atof(*Argument);
		}
		if (CurrentTagString.StartsWith(FTobiiPrimitiveComponentGazeFocusTags::PrimitiveFocusOffsetZTag))
		{
			FString Argument = CurrentTagString.RightChop(FTobiiPrimitiveComponentGazeFocusTags::PrimitiveFocusOffsetZTag.Len());
			Offset.Z += FCString::Atof(*Argument);
		}
	}

	OutFocusLocation += Component->GetComponentTransform().TransformVector(Offset);

	return true;
}

bool UTobiiGTOMBlueprintLibrary::MakeGazeFocusDataForWidgetInteractionComponent(UWidgetInteractionComponent* WidgetInteraction, ECollisionChannel CollisionChannel, FTobiiGazeFocusData& OutGazeFocusData)
{
	OutGazeFocusData = FTobiiGazeFocusData();
	if (WidgetInteraction == nullptr || !WidgetInteraction->bEnableHitTesting)
	{
		return false;
	}

	UWorld* CurrentWorld = WidgetInteraction->GetWorld();
	if (CurrentWorld == nullptr)
	{
		return false;
	}

	FHitResult HitResult;
	FVector StartPoint = WidgetInteraction->GetComponentLocation();
	FVector EndPoint = StartPoint + WidgetInteraction->GetForwardVector() * WidgetInteraction->InteractionDistance;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(WidgetInteraction->GetOwner());
	if (CurrentWorld->LineTraceSingleByChannel(HitResult, StartPoint, EndPoint, CollisionChannel, CollisionParams))
	{
		UPrimitiveComponent* HitComponent = HitResult.GetComponent();
		if (UTobiiGTOMBlueprintLibrary::IsPrimitiveComponentGazeFocusable(HitComponent))
		{
			OutGazeFocusData.FocusedActor = HitResult.GetActor();
			OutGazeFocusData.FocusedPrimitiveComponent = HitComponent;
			OutGazeFocusData.LastVisibleWorldLocation = HitResult.Location;
			OutGazeFocusData.FocusConfidence = 1.0f;

			for (const TWeakPtr<SWidget>& WidgetMaybe : WidgetInteraction->GetHoveredWidgetPath().Widgets)
			{
				if (WidgetMaybe.IsValid())
				{
					SWidget* HoveredWidgetPtr = WidgetMaybe.Pin().Get();
					STobiiGazeFocusableWidget* TobiiSlateWidget = STobiiGazeFocusableWidget::TryCastToTobiiGazeFocusable(HoveredWidgetPtr);
					if (TobiiSlateWidget != nullptr && TobiiSlateWidget->UMGWidget.IsValid())
					{
						OutGazeFocusData.FocusedWidget = TobiiSlateWidget->UMGWidget;
						break;
					}
				}
			}
		}
	}

	return OutGazeFocusData.FocusedActor != nullptr;
}

void UTobiiGTOMBlueprintLibrary::EmulateGazeFocusUsingWidgetInteractionComponent(UWidgetInteractionComponent* WidgetInteraction, ECollisionChannel CollisionChannel)
{
	if (FTobiiGTOMModule::IsAvailable())
	{
		FTobiiGazeFocusData GazeFocusData;
		MakeGazeFocusDataForWidgetInteractionComponent(WidgetInteraction, CollisionChannel, GazeFocusData);
		TArray<FTobiiGazeFocusData> SingleFocusData;
		SingleFocusData.Add(GazeFocusData);
		FTobiiGTOMModule::Get().GTOMInputDevice->EmulateGazeFocus(SingleFocusData);
	}
}

bool UTobiiGTOMBlueprintLibrary::FindLargestInscribedAlignedRect(float CircleSegmentAngleRad, float CircleRadius, FVector2D& LargestInscribedRectSize, float& DistanceToCenter)
{
	//https://math.stackexchange.com/questions/2829710/largest-inscribed-rectangle-in-sector

	if (CircleSegmentAngleRad > FLT_EPSILON && CircleRadius > FLT_EPSILON)
	{
		if (CircleSegmentAngleRad >= PI - FLT_EPSILON)
		{
			const float SqrtTwo = FMath::Sqrt(2.0f);
			const float SqrtTwoOverTwo = SqrtTwo / 2.0f;
			LargestInscribedRectSize.Set(SqrtTwo * CircleRadius, SqrtTwoOverTwo * CircleRadius);
			DistanceToCenter = CircleRadius / 2.0f;
		}
		else
		{
			const float Theta = CircleSegmentAngleRad / 4.0f;
			float ThetaSine, ThetaCosine;
			FMath::SinCos(&ThetaSine, &ThetaCosine, Theta);
			const float Height = CircleRadius * ThetaSine * 2.0f;
			const float AlongX = CircleRadius * ThetaCosine;

			const float HalfAlpha = CircleSegmentAngleRad / 2.0f;
			float HalfAlphaSine, HalfAlphaCosine;
			FMath::SinCos(&HalfAlphaSine, &HalfAlphaCosine, HalfAlpha);
			const float B = CircleRadius * ThetaSine * HalfAlphaCosine / HalfAlphaSine;

			LargestInscribedRectSize.Set(AlongX - B, Height);
			DistanceToCenter = B + FMath::Min(LargestInscribedRectSize.X, LargestInscribedRectSize.Y) / 2.0f;
		}

		return true;
	}

	LargestInscribedRectSize.Set(0.0f, 0.0f);
	DistanceToCenter = 0.0f;
	return false;
}

bool UTobiiGTOMBlueprintLibrary::TransformWidgetLocalPointToWorld(UWidgetComponent* Component, const FVector2D& LocalWidgetLocation, FVector& OutWorldLocation)
{
	if (Component == nullptr)
	{
		return false;
	}

	EWidgetGeometryMode GeometryMode = Component->GetGeometryMode();
	switch (GeometryMode)
	{
	case EWidgetGeometryMode::Plane:
	{
		//First let's calculate some measurements we will need
		const FVector2D DrawSize = Component->GetDrawSize();
		const FVector2D Pivot = Component->GetPivot();

		//Undo the pivot and transform to world space			
		OutWorldLocation = Component->GetComponentTransform().TransformPosition(FVector(0.0f, DrawSize.X * Pivot.X - LocalWidgetLocation.X, DrawSize.Y * Pivot.Y - LocalWidgetLocation.Y));

		return true;
	}

	case EWidgetGeometryMode::Cylinder:
	{
		//First let's calculate some measurements we will need
		const FVector2D DrawSize = Component->GetDrawSize();
		const FVector2D Pivot = Component->GetPivot();
		const float NormalizedLocationX = LocalWidgetLocation.X / DrawSize.X;
		const float ArcAngleRadians = FMath::DegreesToRadians(Component->GetCylinderArcAngle());
		const float Radius = DrawSize.X / ArcAngleRadians;
		const float Apothem = Radius * FMath::Cos(0.5f * ArcAngleRadians);
		const float ChordLength = 2.0f * Radius * FMath::Sin(0.5f * ArcAngleRadians);
		const float PivotOffsetX = ChordLength * (0.5 - Pivot.X);

		//Determine the endpoints of the UI surface in UI space radians. The projection is "reversed" here so the UI surface segment is actually centered around X-.
		const float Endpoint1 = FMath::Fmod(FMath::Atan2(-0.5f * ChordLength, -Apothem) + 2 * PI, 2 * PI);
		const float Endpoint2 = FMath::Fmod(FMath::Atan2(+0.5f * ChordLength, -Apothem) + 2 * PI, 2 * PI);

		//Figure out where on the circle segment our X is using our normalized coordinate 
		const float InterpolatedAngle = FMath::Lerp(Endpoint1, Endpoint2, NormalizedLocationX);

		//We can now determine our coordinate in circle space (the space where the origin is the center of the cylinder circle Y-segment the X-coordinate we are seeking is on) as well as the distance to the cylinder surface.
		const float CircleSpaceWidgetX = Radius * FMath::Cos(InterpolatedAngle);
		const float CircleSpaceWidgetDistanceY = Radius * FMath::Sin(InterpolatedAngle);

		//Figure out our Z coordinate in widget space. This is easy since it's a cylinder.
		const float WidgetSpaceZ = DrawSize.Y * Pivot.Y - LocalWidgetLocation.Y;

		//We now convert the circle space coordinates to widget space (The origin is located at the center of the corda, in the middle of the widget component plane) and insert the widget space Z.
		const FVector WidgetSpaceCoord(Apothem + CircleSpaceWidgetX, -CircleSpaceWidgetDistanceY + PivotOffsetX, WidgetSpaceZ);

		//Finally we can simply transform this location to world space and we're done.
		OutWorldLocation = Component->GetComponentTransform().TransformPosition(WidgetSpaceCoord);

		return true;
	}

	default:
		break;
	}

	return false;
}

bool UTobiiGTOMBlueprintLibrary::TransformWorldPointToWidgetLocal(UWidgetComponent* Component, const FVector& WorldLocation, const FVector& WorldDirection, FVector2D& OutLocalWidgetLocation)
{
	if (Component == nullptr)
	{
		return false;
	}

	EWidgetGeometryMode GeometryMode = Component->GetGeometryMode();
	switch (GeometryMode)
	{
	case EWidgetGeometryMode::Plane:
	{
		Component->GetLocalHitLocation(WorldLocation, OutLocalWidgetLocation);
		return true;
	}

	case EWidgetGeometryMode::Cylinder:
	{
		TTuple<FVector, FVector2D> Output = Component->GetCylinderHitLocation(WorldLocation, WorldDirection);
		OutLocalWidgetLocation = Output.Value;
		return true;
	}

	default:
		break;
	}

	return false;
}

bool UTobiiGTOMBlueprintLibrary::TestConeSphereIntersection(const FVector& ConeApex, const FVector& ConeDirection, const float ConeAngleDeg, const FVector& SphereCenter, const float SphereRadius)
{
	//http://blog.julien.cayzac.name/2009/12/frustum-culling-sphere-cone-test-with.html

	float ConeAngleSine, ConeAngleCosine;
	FMath::SinCos(&ConeAngleSine, &ConeAngleCosine, FMath::DegreesToRadians(ConeAngleDeg));

	const FVector DirectionTowardsSphere = SphereCenter - ConeApex;
	const float SelfDot = FVector::DotProduct(DirectionTowardsSphere, DirectionTowardsSphere);
	const float a = FVector::DotProduct(DirectionTowardsSphere, ConeDirection);
	const float p(a*ConeAngleSine);
	const float q(ConeAngleCosine * ConeAngleCosine * SelfDot - a * a);
	const bool tmp(q < 0);

	float lhs[2];
	lhs[0] = (SphereRadius*SphereRadius - q);
	lhs[1] = -lhs[0];
	int result = (lhs[(p < SphereRadius) || !tmp] < 2.0f * SphereRadius * p) ? -1 : tmp;
	return result != 0; // -1 means partially included. 1 means fully included
}

bool UTobiiGTOMBlueprintLibrary::TestRectEllipseIntersection(const FVector2D& RectangleCenter, const FVector2D& RectangleRightAxis, const FVector2D& RectangleUpAxis, const FVector2D& RectangleExtents, const FVector2D& EllipseCenter, const FVector2D& EllipseRadii, const float& EllipseRotationDeg)
{
	//https://www.geometrictools.com/Documentation/IntersectionRectangleEllipse.pdf :: Minkowski sum

	//Compute the increase in extents for R'.
	float EllipseSine, EllipseCosine;
	FMath::SinCos(&EllipseSine, &EllipseCosine, FMath::DegreesToRadians(EllipseRotationDeg));
	FMatrix2x2 V(EllipseCosine, -EllipseSine, EllipseSine, EllipseCosine);
	FMatrix2x2 VT(EllipseCosine, EllipseSine, -EllipseSine, EllipseCosine);
	FMatrix2x2 D(1.0f / EllipseRadii.X, 0.0f, 0.0f, 1.0f / EllipseRadii.Y);
	FMatrix2x2 M = V.Concatenate(D).Concatenate(D).Concatenate(VT);
	FMatrix2x2 MInv = M.Inverse();

	const float LRight = FMath::Sqrt(FVector2D::DotProduct(RectangleRightAxis, MInv.TransformVector(RectangleRightAxis)));
	const float LUp = FMath::Sqrt(FVector2D::DotProduct(RectangleUpAxis, MInv.TransformVector(RectangleUpAxis)));

	//Transform the ellipse center to the rectangle coordinate system.
	FVector2D KmC = EllipseCenter - RectangleCenter;
	float xi[2] = { FVector2D::DotProduct(RectangleRightAxis, KmC), FVector2D::DotProduct(RectangleUpAxis, KmC) };
	if (FMath::Abs(xi[0]) <= RectangleExtents.X + LRight && FMath::Abs(xi[1]) <= RectangleExtents.Y + LUp)
	{
		float s[2] = { (xi[0] >= 0.0f ? 1.0f : -1.0f), (xi[1] >= 0.0f ? 1.0f : -1.0f) };
		FVector2D PmC = s[0] * RectangleExtents.X * RectangleRightAxis + s[1] * RectangleExtents.X * RectangleUpAxis;
		FVector2D MDelta = M.TransformVector(KmC - PmC);

		if (s[0] * FVector2D::DotProduct(RectangleRightAxis, MDelta) <= 0.0f
			|| s[1] * FVector2D::DotProduct(RectangleUpAxis, MDelta) <= 0.0f)
		{
			return true;
		}

		return FVector2D::DotProduct(MDelta, M.TransformVector(MDelta)) <= 1.0f;
	}

	return false;
}

FVector2D UTobiiGTOMBlueprintLibrary::GetUniformlyDistributedRandomCirclePoint(float CircleRadius)
{
	float Angle = FMath::RandRange(0.0f, TWO_PI);
	float Radius = CircleRadius * FMath::Sqrt(FMath::RandRange(0.0f, 1.0f));
	return FVector2D(Radius * FMath::Cos(Angle), Radius * FMath::Sin(Angle));
}