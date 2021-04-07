/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#include "TobiiGTOMEngine.h"
#include "TobiiGazeFocusableComponent.h"
#include "TobiiGazeFocusableWidget.h"
#include "TobiiGTOMBlueprintLibrary.h"
#include "TobiiGTOMInternalTypes.h"

#include "Engine/Engine.h"
#include "IEyeTracker.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerController.h"
#include "Runtime/Engine/Public/Slate/SceneViewport.h"
#include "Misc/Paths.h"

static TAutoConsoleVariable<int32> CVarDebugDisplayGTOMVisibility(TEXT("tobii.debug.DisplayGTOMVisibility"), 0, TEXT("1 means we visualize which objects are visible to G2OM"));
static TAutoConsoleVariable<int32> CVarDebugDisplayG2OMCandidateSet(TEXT("tobii.debug.DisplayG2OMCandidateSet"), 1, TEXT("1 will visualize all bounds calculated in G2OM. This is useful to test for math errors."));

FTobiiGTOMEngine::FTobiiGTOMEngine()
{
	g2om_context_create(&G2OMContext);
}

FTobiiGTOMEngine::~FTobiiGTOMEngine()
{
	if (G2OMContext != nullptr)
	{
		g2om_context_destroy(&G2OMContext);
		G2OMContext = nullptr;
	}
}

void FTobiiGTOMEngine::Tick(float DeltaTimeSecs)
{
	static const auto DrawDebugCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.debug"));
	static const auto MaximumTraceDistanceCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.MaximumTraceDistance"));
	static const auto FocusTraceChannelCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.FocusTraceChannel"));

	if (GEngine == nullptr 
		|| GEngine->GameViewport == nullptr
		|| GEngine->GameViewport->GetWorld() == nullptr
		|| GEngine->GameViewport->GetGameViewport() == nullptr
		|| !GEngine->EyeTrackingDevice.IsValid())
	{
		return;
	}

	//Gaze data
	FEyeTrackerGazeData CombinedGazeData;
	GEngine->EyeTrackingDevice->GetEyeTrackerGazeData(CombinedGazeData);

	FVector2D CombinedGazePtUNorm = FVector2D::ZeroVector;
	FVector2D ScreenGazePointPx;
	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);
	GTOMPlayerController->ProjectWorldLocationToScreen(CombinedGazeData.GazeOrigin + CombinedGazeData.GazeDirection * 10.0f, ScreenGazePointPx);
	if (ViewportSize.X > 0.0f && ViewportSize.Y > 0.0f)
	{
		CombinedGazePtUNorm = FVector2D(ScreenGazePointPx.X / ViewportSize.X, ScreenGazePointPx.Y / ViewportSize.Y);
	}

	if (!GTOMPlayerController.IsValid())
	{
		GTOMPlayerController = GEngine->GameViewport->GetWorld()->GetFirstPlayerController();
		GEngine->EyeTrackingDevice->SetEyeTrackedPlayer(GTOMPlayerController.Get());
	}

	//Main G2OM processing
	if (G2OMContext == nullptr 
		|| !GTOMPlayerController.IsValid()
		|| GTOMPlayerController->PlayerCameraManager == nullptr) 
	{
		return;
	}
	
	QUICK_SCOPE_CYCLE_COUNTER(STAT_TobiiEyetracking_GTOM);
	
	FRotator CameraRotation = GTOMPlayerController->PlayerCameraManager->GetCameraRotation();
	G2OMGazeData.timestamp_in_s = GTOMPlayerController->GetWorld()->GetTimeSeconds();
	G2OMGazeData.camera_up_direction_world_space = FTobiiGTOMUtils::UE4VectorToG2OMVector(CameraRotation.RotateVector(FVector::UpVector));
	G2OMGazeData.camera_right_direction_world_space = FTobiiGTOMUtils::UE4VectorToG2OMVector(CameraRotation.RotateVector(FVector::RightVector));
	G2OMGazeData.gaze_ray_world_space.is_valid = CombinedGazeData.ConfidenceValue > 0.5f; // Sigh. Why is this still a thing?
	G2OMGazeData.gaze_ray_world_space.ray.origin.x = CombinedGazeData.GazeOrigin.X;
	G2OMGazeData.gaze_ray_world_space.ray.origin.y = CombinedGazeData.GazeOrigin.Y;
	G2OMGazeData.gaze_ray_world_space.ray.origin.z = CombinedGazeData.GazeOrigin.Z;
	G2OMGazeData.gaze_ray_world_space.ray.direction.x = CombinedGazeData.GazeDirection.X;
	G2OMGazeData.gaze_ray_world_space.ray.direction.y = CombinedGazeData.GazeDirection.Y;
	G2OMGazeData.gaze_ray_world_space.ray.direction.z = CombinedGazeData.GazeDirection.Z;

	//Raycasts
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(GTOMPlayerController.Get());
	CollisionQueryParams.AddIgnoredActor(GTOMPlayerController->GetPawn());
	const float MaximumTraceDistance = FMath::Max(MaximumTraceDistanceCVar->GetFloat(), 0.0f);
	const FVector CombinedGazeFarLocation = CombinedGazeData.GazeOrigin + (CombinedGazeData.GazeDirection * MaximumTraceDistance);
	if (CombinedGazeData.ConfidenceValue < 0.5f ||
		!GTOMPlayerController->GetWorld()->LineTraceSingleByChannel(CombinedWorldGazeHitData, CombinedGazeData.GazeOrigin
			, CombinedGazeFarLocation, (ECollisionChannel)FocusTraceChannelCVar->GetInt(), CollisionQueryParams))
	{
		CombinedWorldGazeHitData.Actor = nullptr;
		CombinedWorldGazeHitData.Component = nullptr;
		CombinedWorldGazeHitData.Distance = MaximumTraceDistance;
		CombinedWorldGazeHitData.Location = CombinedGazeFarLocation;
		CombinedWorldGazeHitData.bBlockingHit = false;
	}
	FillG2OMRaycast(CombinedWorldGazeHitData, CombinedGazePtUNorm, CombinedGazeData.GazeDirection, G2OMRaycastResults.raycast);

	//Candidate generation
	TArray<g2om_candidate> Candidates;
	TArray<g2om_candidate_result> CandidateResults;
	TMap<FEngineFocusableUID, TWeakObjectPtr<UTobiiGazeFocusableComponent>> FocusableComponentsWithWidgets;
	const TMap<FEngineFocusableUID, FTobiiGTOMOcclusionData>& VisibleSet = OcclusionTester.Tick(DeltaTimeSecs, GTOMPlayerController.Get(), CombinedGazeData, G2OMGazeData, G2OMContext);
	TMap<FTobiiFocusableUID, TWeakObjectPtr<UTobiiGazeFocusableWidget>>& ScreenSpaceWidgets = UTobiiGazeFocusableWidget::GetTobiiScreenSpaceFocusableWidgets();

	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_TobiiEyetracking_GTOM_BuildCandidates);

		//We must do this every frame since any of these properties might have changed since the last tick.
		for (auto OcclusionDataIterator = VisibleSet.CreateConstIterator(); OcclusionDataIterator; ++OcclusionDataIterator)
		{
			const FTobiiGTOMOcclusionData& OcclusionData = OcclusionDataIterator.Value();
			const FEngineFocusableUID FocusableID = OcclusionDataIterator.Key();

			if (OcclusionData.PrimitiveComponent.IsValid() && OcclusionData.PrimitiveComponent->IsVisible())
			{
				bool bShouldAddPrimitive = true;
				if (OcclusionData.FocusableComponent.IsValid())
				{
					const TArray<TWeakObjectPtr<UTobiiGazeFocusableWidget>>& FocusableWidgets = OcclusionData.FocusableComponent->GetFocusableWidgetsForPrimitiveComponent(OcclusionData.PrimitiveComponent.Get());
					if (FocusableWidgets.Num() > 0)
					{
						//If our primitive contains widgets, then only add the widgets
						bShouldAddPrimitive = false;

						g2om_candidate NewCandidate;
						FTransform LocalToWorldTranform = OcclusionData.PrimitiveComponent->GetComponentTransform();
						FMatrix LocalToWorldMatrix = LocalToWorldTranform.ToMatrixWithScale();
						FMatrix WorldToLocalMatrix = LocalToWorldMatrix.InverseFast();
						FMemory::Memcpy(NewCandidate.local_to_world_matrix.data, LocalToWorldMatrix.M, 16 * sizeof(float));
						FMemory::Memcpy(NewCandidate.world_to_local_matrix.data, WorldToLocalMatrix.M, 16 * sizeof(float));

						for (const auto& Widget : FocusableWidgets)
						{
							if (Widget.IsValid() && !FocusableComponentsWithWidgets.Contains(Widget->GetUniqueID())
								&& UTobiiGazeFocusableComponent::IsWidgetFocusable(Widget.Get())
								&& Widget->GetWorldSpaceHostWidgetComponent() != nullptr)
							{
								STobiiGazeFocusableWidget* SlateContainer = (STobiiGazeFocusableWidget*)&Widget->TakeWidget().Get();
								FSlateRect WidgetRenderBounds = SlateContainer->GetCachedGeometry().GetRenderBoundingRect();
								FVector WidgetTopLeft, WidgetBottomRight;
								UTobiiGTOMBlueprintLibrary::TransformWidgetLocalPointToWorld(Widget->GetWorldSpaceHostWidgetComponent(), WidgetRenderBounds.GetTopLeft(), WidgetTopLeft);
								UTobiiGTOMBlueprintLibrary::TransformWidgetLocalPointToWorld(Widget->GetWorldSpaceHostWidgetComponent(), WidgetRenderBounds.GetBottomRight(), WidgetBottomRight);

								//Range test
								bool bIsInRange = true;
								float MaxDistance;
								const float DistanceToWidget = FVector::Distance(CombinedGazeData.GazeOrigin, (WidgetTopLeft + WidgetBottomRight) / 2.0f);
								if (UTobiiGazeFocusableComponent::GetMaxFocusDistanceForWidget(Widget.Get(), MaxDistance))
								{
									bIsInRange = DistanceToWidget <= MaxDistance;
								}

								if (bIsInRange)
								{
									WidgetTopLeft = WorldToLocalMatrix.TransformPosition(WidgetTopLeft);
									WidgetBottomRight = WorldToLocalMatrix.TransformPosition(WidgetBottomRight);

									NewCandidate.id = Widget->GetUniqueID();
									NewCandidate.min_local_space.x = WidgetTopLeft.X;
									NewCandidate.min_local_space.y = WidgetTopLeft.Y;
									NewCandidate.min_local_space.z = WidgetTopLeft.Z;
									NewCandidate.max_local_space.x = WidgetBottomRight.X;
									NewCandidate.max_local_space.y = WidgetBottomRight.Y;
									NewCandidate.max_local_space.z = WidgetBottomRight.Z;

									Candidates.Add(NewCandidate);
									CandidateResults.Add(g2om_candidate_result());
									FocusableComponentsWithWidgets.Add(Widget->GetUniqueID(), OcclusionData.FocusableComponent);
								}
							}
						}
					}
				}

				if(bShouldAddPrimitive)
				{
					//If there were no widgets, just add the primitive.
					g2om_candidate NewCandidate;
					NewCandidate.id = FocusableID;

					FTransform LocalToWorldTranform = OcclusionData.PrimitiveComponent->GetComponentTransform();
					FMatrix LocalToWorldMatrix = LocalToWorldTranform.ToMatrixWithScale();
					FMatrix WorldToLocalMatrix = LocalToWorldMatrix.InverseFast();
					FMemory::Memcpy(NewCandidate.local_to_world_matrix.data, LocalToWorldMatrix.M, 16 * sizeof(float));
					FMemory::Memcpy(NewCandidate.world_to_local_matrix.data, WorldToLocalMatrix.M, 16 * sizeof(float));

					FBox BoxBounds = OcclusionData.PrimitiveComponent->CalcBounds(FTransform::Identity).GetBox();
					NewCandidate.min_local_space.x = BoxBounds.Min.X;
					NewCandidate.min_local_space.y = BoxBounds.Min.Y;
					NewCandidate.min_local_space.z = BoxBounds.Min.Z;
					NewCandidate.max_local_space.x = BoxBounds.Max.X;
					NewCandidate.max_local_space.y = BoxBounds.Max.Y;
					NewCandidate.max_local_space.z = BoxBounds.Max.Z;

					Candidates.Add(MoveTemp(NewCandidate));
					CandidateResults.Add(g2om_candidate_result());
				}				
			}
		}

		//Finally add registered screen space widgets as candidates.
		TArray<FTobiiFocusableUID> StaleWidgetIds;
		for (auto& WidgetPair : ScreenSpaceWidgets)
		{
			TWeakObjectPtr<UTobiiGazeFocusableWidget> Widget = WidgetPair.Value;
			if (Widget.IsValid())
			{
				if (!FocusableComponentsWithWidgets.Contains(Widget->GetUniqueID()) && UTobiiGazeFocusableComponent::IsWidgetFocusable(Widget.Get()))
				{
					STobiiGazeFocusableWidget* SlateContainer = (STobiiGazeFocusableWidget*)&Widget->TakeWidget().Get();
					FSlateRect WidgetRenderBounds = SlateContainer->GetCachedGeometry().GetRenderBoundingRect();
					FVector2D ScreenTopLeft = GEngine->GameViewport->GetGameViewport()->VirtualDesktopPixelToViewport(FIntPoint(WidgetRenderBounds.GetTopLeft().X, WidgetRenderBounds.GetTopLeft().Y));
					FVector2D ScreenBottomRight = GEngine->GameViewport->GetGameViewport()->VirtualDesktopPixelToViewport(FIntPoint(WidgetRenderBounds.GetBottomRight().X, WidgetRenderBounds.GetBottomRight().Y));
					ScreenTopLeft.X *= ViewportSize.X;
					ScreenTopLeft.Y *= ViewportSize.Y;
					ScreenBottomRight.X *= ViewportSize.X;
					ScreenBottomRight.Y *= ViewportSize.Y;
					FVector2D ScreenBottomLeft(ScreenTopLeft.X, ScreenBottomRight.Y);

					FVector WorldTopLeftLocation, WorldBottomRightLocation, WorldBottomLeftLocation;
					FVector WorldTopLeftDir, WorldBottomRightDir, WorldBottomLeftDir;

					GTOMPlayerController->DeprojectScreenPositionToWorld(ScreenTopLeft.X, ScreenTopLeft.Y, WorldTopLeftLocation, WorldTopLeftDir);
					GTOMPlayerController->DeprojectScreenPositionToWorld(ScreenBottomRight.X, ScreenBottomRight.Y, WorldBottomRightLocation, WorldBottomRightDir);
					GTOMPlayerController->DeprojectScreenPositionToWorld(ScreenBottomLeft.X, ScreenBottomLeft.Y, WorldBottomLeftLocation, WorldBottomLeftDir);
					WorldTopLeftLocation += WorldTopLeftDir;
					WorldBottomRightLocation += WorldBottomRightDir;
					WorldBottomLeftLocation += WorldBottomLeftDir;

					FVector WorldTranslation = (WorldTopLeftLocation + WorldBottomRightLocation) / 2.0f;
					FVector WorldRightY = WorldBottomRightLocation - WorldBottomLeftLocation;
					FVector WorldUpZ = WorldTopLeftLocation - WorldBottomLeftLocation;
					FVector WorldForwardX = FVector::CrossProduct(WorldRightY, WorldUpZ);
					bool bCouldBuildOrthoNormalBasis = WorldForwardX.Normalize();
					bCouldBuildOrthoNormalBasis = bCouldBuildOrthoNormalBasis && WorldRightY.Normalize();
					bCouldBuildOrthoNormalBasis = bCouldBuildOrthoNormalBasis && WorldUpZ.Normalize();

					if (bCouldBuildOrthoNormalBasis)
					{
						FMatrix LocalToWorldMatrix = FMatrix(WorldForwardX, WorldRightY, WorldUpZ, WorldTranslation);
						FMatrix WorldToLocalMatrix = LocalToWorldMatrix.InverseFast();
						WorldTopLeftLocation = WorldToLocalMatrix.TransformPosition(WorldTopLeftLocation + WorldTopLeftDir);
						WorldBottomRightLocation = WorldToLocalMatrix.TransformPosition(WorldBottomRightLocation + WorldBottomRightDir);

						//Screen space widgets are always in range, so we can skip the distance test
						g2om_candidate NewCandidate;
						NewCandidate.id = Widget->GetUniqueID();
						FMemory::Memcpy(NewCandidate.local_to_world_matrix.data, LocalToWorldMatrix.M, 16 * sizeof(float));
						FMemory::Memcpy(NewCandidate.world_to_local_matrix.data, WorldToLocalMatrix.M, 16 * sizeof(float));
						NewCandidate.min_local_space.x = WorldTopLeftLocation.X;
						NewCandidate.min_local_space.y = WorldTopLeftLocation.Y;
						NewCandidate.min_local_space.z = WorldTopLeftLocation.Z;
						NewCandidate.max_local_space.x = WorldBottomRightLocation.X;
						NewCandidate.max_local_space.y = WorldBottomRightLocation.Y;
						NewCandidate.max_local_space.z = WorldBottomRightLocation.Z;

						Candidates.Add(NewCandidate);
						CandidateResults.Add(g2om_candidate_result());
					}
				}
			}
			else
			{
				StaleWidgetIds.Add(WidgetPair.Key);
			}

			for (FTobiiFocusableUID Id : StaleWidgetIds)
			{
				ScreenSpaceWidgets.Remove(Id);
			}
		}
	}

	UPrimitiveComponent* NewTopFocusPrimitive = nullptr;
	UTobiiGazeFocusableWidget* NewTopFocusWidget = nullptr;

	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_TobiiEyetracking_GTOM_G2OMProcess);

		g2om_process(G2OMContext, &G2OMGazeData, &G2OMRaycastResults, Candidates.Num(), Candidates.GetData(), CandidateResults.GetData());

		G2OMFocusResults.Empty();
		for (const g2om_candidate_result& ResultCandidate : CandidateResults)
		{
			if (ResultCandidate.score > FLT_EPSILON)
			{
				if(ScreenSpaceWidgets.Contains(ResultCandidate.id))
				{
					//Screen space widget
					TWeakObjectPtr<UTobiiGazeFocusableWidget> Widget = ScreenSpaceWidgets[ResultCandidate.id];
					if (Widget.IsValid())
					{
						FTobiiGazeFocusData NewFocusData;
						NewFocusData.FocusedWidget = Widget;
						NewFocusData.FocusConfidence = ResultCandidate.score;
						NewFocusData.FocusedActor = nullptr;
						NewFocusData.FocusedPrimitiveComponent = nullptr;
						NewFocusData.LastVisibleWorldLocation = FVector::ZeroVector;

						G2OMFocusResults.Add(MoveTemp(NewFocusData));

						if (NewTopFocusWidget == nullptr)
						{
							NewTopFocusWidget = Widget.Get();
						}
					}
				}
				else if (FocusableComponentsWithWidgets.Contains(ResultCandidate.id))
				{
					//World space widget
					TWeakObjectPtr<UTobiiGazeFocusableComponent> Focusable = FocusableComponentsWithWidgets[ResultCandidate.id];
					if (Focusable.IsValid())
					{
						const TMap<uint32, TWeakObjectPtr<UTobiiGazeFocusableWidget>>& FocusableWidgets = Focusable->GetAllFocusableWidgets();
						if (FocusableWidgets.Contains(ResultCandidate.id))
						{
							TWeakObjectPtr<UTobiiGazeFocusableWidget> Widget = FocusableWidgets[ResultCandidate.id];
							if (Widget.IsValid())
							{
								FTobiiGazeFocusData NewFocusData;
								NewFocusData.FocusedWidget = Widget;
								NewFocusData.FocusConfidence = ResultCandidate.score;

								if (Widget->GetWorldSpaceHostWidgetComponent() != nullptr)
								{
									NewFocusData.FocusedActor = Widget->GetWorldSpaceHostWidgetComponent()->GetOwner();
									NewFocusData.FocusedPrimitiveComponent = Widget->GetWorldSpaceHostWidgetComponent();
									NewFocusData.LastVisibleWorldLocation = Widget->GetWorldSpaceHostWidgetComponent()->GetComponentLocation();
								}

								G2OMFocusResults.Add(MoveTemp(NewFocusData));

								if (NewTopFocusWidget == nullptr)
								{
									NewTopFocusWidget = Widget.Get();
								}
							}
						}
					}
				}
				else if(VisibleSet.Contains(ResultCandidate.id))
				{
					//Primitive Component
					TWeakObjectPtr<UPrimitiveComponent> FocusedPrimitivePtr = VisibleSet[ResultCandidate.id].PrimitiveComponent;

					if (FocusedPrimitivePtr.IsValid())
					{
						FTobiiGazeFocusData NewFocusData;
						NewFocusData.FocusedActor = FocusedPrimitivePtr->GetOwner();
						NewFocusData.FocusedPrimitiveComponent = FocusedPrimitivePtr;
						NewFocusData.FocusedWidget = nullptr;
						UTobiiGTOMBlueprintLibrary::GetPrimitiveComponentFocusLocation(FocusedPrimitivePtr.Get(), NewFocusData.LastVisibleWorldLocation); // We want this to be taken care of by GXOM, but the current system does not support it.
						NewFocusData.FocusConfidence = ResultCandidate.score;

						G2OMFocusResults.Add(MoveTemp(NewFocusData));

						if (NewTopFocusPrimitive == nullptr)
						{
							NewTopFocusPrimitive = FocusedPrimitivePtr.Get();
						}
					}
				}
			}
		}
	}

	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_TobiiEyetracking_GTOM_G2OMNotify);
		UpdateWinners(NewTopFocusPrimitive, NewTopFocusWidget);
	}

	if (DrawDebugCVar->GetInt() && CVarDebugDisplayG2OMCandidateSet.GetValueOnGameThread() != 0)
	{
		for (g2om_candidate& Candidate : Candidates)
		{
			bool bIsFocused = false;
			float FocusAlpha = 1.0f;
			for (const g2om_candidate_result& ResultCandidate : CandidateResults)
			{
				if (ResultCandidate.id == Candidate.id && ResultCandidate.score > FLT_EPSILON)
				{
					bIsFocused = true;
					FocusAlpha = ResultCandidate.score;
					break;
				}
			}

			g2om_vector Corners[G2OM_CORNERS_COUNT];
			g2om_get_worldspace_corner_of_candidate(&Candidate, G2OM_CORNERS_COUNT, Corners);
				
			TArray<FVector> UCorners;
			for (int32 Idx = 0; Idx < G2OM_CORNERS_COUNT; Idx++)
			{
				UCorners.Add(FTobiiGTOMUtils::G2OMVectorToUE4Vector(Corners[Idx]));
			}

			UWorld* World = GTOMPlayerController->GetWorld();
			const float DistanceToFirstCorner = FVector::Distance(UCorners[G2OM_CORNERS_FLL], GTOMPlayerController->GetPawn()->GetActorLocation());
			const float Thickness = (bIsFocused && DistanceToFirstCorner > 100.0f) ? 3.0f : 0.0f;
			const FColor BoxColor = bIsFocused ? FColor(0, 255, 0, FocusAlpha * 255)
				: (FocusableComponentsWithWidgets.Contains(Candidate.id) ? FColor::Cyan : FColor::Orange);

			//FRONT
			DrawDebugLine(World, UCorners[G2OM_CORNERS_FLL], UCorners[G2OM_CORNERS_FLR], BoxColor, false, 0.0f, 0, Thickness);
			DrawDebugLine(World, UCorners[G2OM_CORNERS_FLR], UCorners[G2OM_CORNERS_FUR], BoxColor, false, 0.0f, 0, Thickness);
			DrawDebugLine(World, UCorners[G2OM_CORNERS_FUR], UCorners[G2OM_CORNERS_FUL], BoxColor, false, 0.0f, 0, Thickness);
			DrawDebugLine(World, UCorners[G2OM_CORNERS_FUL], UCorners[G2OM_CORNERS_FLL], BoxColor, false, 0.0f, 0, Thickness);
																												  
			//BACK																							  
			DrawDebugLine(World, UCorners[G2OM_CORNERS_BLL], UCorners[G2OM_CORNERS_BLR], BoxColor, false, 0.0f, 0, Thickness);
			DrawDebugLine(World, UCorners[G2OM_CORNERS_BLR], UCorners[G2OM_CORNERS_BUR], BoxColor, false, 0.0f, 0, Thickness);
			DrawDebugLine(World, UCorners[G2OM_CORNERS_BUR], UCorners[G2OM_CORNERS_BUL], BoxColor, false, 0.0f, 0, Thickness);
			DrawDebugLine(World, UCorners[G2OM_CORNERS_BUL], UCorners[G2OM_CORNERS_BLL], BoxColor, false, 0.0f, 0, Thickness);

			//SPOKES																						
			DrawDebugLine(World, UCorners[G2OM_CORNERS_FLL], UCorners[G2OM_CORNERS_BLL], BoxColor, false, 0.0f, 0, Thickness);
			DrawDebugLine(World, UCorners[G2OM_CORNERS_FLR], UCorners[G2OM_CORNERS_BLR], BoxColor, false, 0.0f, 0, Thickness);
			DrawDebugLine(World, UCorners[G2OM_CORNERS_FUR], UCorners[G2OM_CORNERS_BUR], BoxColor, false, 0.0f, 0, Thickness);
			DrawDebugLine(World, UCorners[G2OM_CORNERS_FUL], UCorners[G2OM_CORNERS_BUL], BoxColor, false, 0.0f, 0, Thickness);
		}
	}
}

void FTobiiGTOMEngine::UpdateWinners(UPrimitiveComponent* NewTopFocusPrimitive, UTobiiGazeFocusableWidget* NewTopFocusWidget)
{
	//Primitive components
	{
		if (NewTopFocusPrimitive == nullptr && PreviouslyFocusedPrimitiveComponent.IsValid())
		{
			//We lost focus completely
			NotifyPrimitiveComponentGazeFocusLost(*PreviouslyFocusedPrimitiveComponent.Get());
		}
		else if (NewTopFocusPrimitive != nullptr && !PreviouslyFocusedPrimitiveComponent.IsValid())
		{
			//We gained focus from no focus
			NotifyPrimitiveComponentGazeFocusReceived(*NewTopFocusPrimitive);
		}
		else if (NewTopFocusPrimitive != nullptr && PreviouslyFocusedPrimitiveComponent.IsValid() && NewTopFocusPrimitive != PreviouslyFocusedPrimitiveComponent.Get())
		{
			//Focus shifted from one object to another
			NotifyPrimitiveComponentGazeFocusLost(*PreviouslyFocusedPrimitiveComponent.Get());
			NotifyPrimitiveComponentGazeFocusReceived(*NewTopFocusPrimitive);
		}

		PreviouslyFocusedPrimitiveComponent = TWeakObjectPtr<UPrimitiveComponent>(NewTopFocusPrimitive);
	}

	//Widgets
	{
		if (NewTopFocusWidget == nullptr && PreviouslyFocusedWidget.IsValid())
		{
			//We lost focus completely
			NotifyWidgetGazeFocusLost(*PreviouslyFocusedWidget.Get());
		}
		else if (NewTopFocusWidget != nullptr && !PreviouslyFocusedWidget.IsValid())
		{
			//We gained focus from no focus
			NotifyWidgetGazeFocusReceived(*NewTopFocusWidget);
		}
		else if (NewTopFocusWidget != nullptr && PreviouslyFocusedWidget.IsValid() && NewTopFocusWidget != PreviouslyFocusedWidget.Get())
		{
			//Focus shifted from one object to another
			NotifyWidgetGazeFocusLost(*PreviouslyFocusedWidget.Get());
			NotifyWidgetGazeFocusReceived(*NewTopFocusWidget);
		}

		PreviouslyFocusedWidget = TWeakObjectPtr<UTobiiGazeFocusableWidget>(NewTopFocusWidget);
	}
}

void FTobiiGTOMEngine::FillG2OMRaycast(const FHitResult& HitResult, const FVector2D& ScreenGazePointUNorm, const FVector& GazeDirection, g2om_raycast& RaycastToModify)
{
	RaycastToModify.is_valid = false;
	RaycastToModify.id = -1;

	if (GEngine == nullptr
		|| GEngine->GameViewport == nullptr
		|| GEngine->GameViewport->GetWorld() == nullptr
		|| GEngine->GameViewport->GetGameViewport() == nullptr
		|| !GEngine->EyeTrackingDevice.IsValid())
	{
		return;
	}
	
	//First test screen space widgets
	FVector2D VirtualDesktopGazePoint = GEngine->GameViewport->GetGameViewport()->ViewportToVirtualDesktopPixel(ScreenGazePointUNorm);
	TMap<FTobiiFocusableUID, TWeakObjectPtr<UTobiiGazeFocusableWidget>>& ScreenSpaceWidgets = UTobiiGazeFocusableWidget::GetTobiiScreenSpaceFocusableWidgets();
	for (auto& WidgetPair : ScreenSpaceWidgets)
	{
		TWeakObjectPtr<UTobiiGazeFocusableWidget> Widget = WidgetPair.Value;
		if (Widget.IsValid())
		{
			if (UTobiiGazeFocusableComponent::IsWidgetFocusable(Widget.Get()))
			{
				STobiiGazeFocusableWidget* SlateContainer = (STobiiGazeFocusableWidget*)&Widget->TakeWidget().Get();
				FSlateRect WidgetRenderBounds = SlateContainer->GetCachedGeometry().GetRenderBoundingRect();
				if (WidgetRenderBounds.ContainsPoint(VirtualDesktopGazePoint))
				{
					RaycastToModify.is_valid = true;
					RaycastToModify.id = Widget->GetUniqueID();
					return;
				}
			}
		}
	}

	UPrimitiveComponent* Primitive = HitResult.GetComponent();
	if (Primitive != nullptr)
	{
		RaycastToModify.is_valid = UTobiiGazeFocusableComponent::IsPrimitiveFocusable(Primitive);

		if (Primitive->GetOwner())
		{
			UTobiiGazeFocusableComponent* FocusableComponent = (UTobiiGazeFocusableComponent*)Primitive->GetOwner()->GetComponentByClass(UTobiiGazeFocusableComponent::StaticClass());
			if (FocusableComponent != nullptr)
			{
				const TArray<TWeakObjectPtr<UTobiiGazeFocusableWidget>>& FocusableWidgets = FocusableComponent->GetFocusableWidgetsForPrimitiveComponent(Primitive);

				for (const auto& Widget : FocusableWidgets)
				{
					if (Widget.IsValid() && UTobiiGazeFocusableComponent::IsWidgetFocusable(Widget.Get()))
					{
						FVector2D WidgetLocation;
						if (UTobiiGTOMBlueprintLibrary::TransformWorldPointToWidgetLocal(Widget->GetWorldSpaceHostWidgetComponent(), HitResult.Location, GazeDirection, WidgetLocation))
						{
							STobiiGazeFocusableWidget* SlateContainer = (STobiiGazeFocusableWidget*)&Widget->TakeWidget().Get();
							FSlateRect WidgetRenderBounds = SlateContainer->GetCachedGeometry().GetRenderBoundingRect();

							if (WidgetRenderBounds.ContainsPoint(WidgetLocation))
							{
								//We found a hit!
								RaycastToModify.id = Widget->GetUniqueID();
								return;
							}
						}
					}
				}
			}
		}

		//Default
		RaycastToModify.id = Primitive->GetUniqueID();
	}
}

void FTobiiGTOMEngine::NotifyPrimitiveComponentGazeFocusReceived(UPrimitiveComponent& PrimitiveComponentToReceiveFocus)
{
	PrimitiveComponentToReceiveFocus.ComponentTags.AddUnique(FTobiiPrimitiveComponentGazeFocusTags::HasGazeFocusTag);

	if (PrimitiveComponentToReceiveFocus.GetOwner() != nullptr)
	{
		UTobiiGazeFocusableComponent* GazeFocusableComponent = (UTobiiGazeFocusableComponent*)PrimitiveComponentToReceiveFocus.GetOwner()->GetComponentByClass(UTobiiGazeFocusableComponent::StaticClass());
		if (GazeFocusableComponent != nullptr)
		{
			GazeFocusableComponent->PrimitiveReceivedGazeFocus(&PrimitiveComponentToReceiveFocus);
		}
	}
}

void FTobiiGTOMEngine::NotifyPrimitiveComponentGazeFocusLost(UPrimitiveComponent& PrimitiveComponentToLoseFocus)
{
	PrimitiveComponentToLoseFocus.ComponentTags.Remove(FTobiiPrimitiveComponentGazeFocusTags::HasGazeFocusTag);

	if (PrimitiveComponentToLoseFocus.GetOwner() != nullptr)
	{
		UTobiiGazeFocusableComponent* GazeFocusableComponent = (UTobiiGazeFocusableComponent*)PrimitiveComponentToLoseFocus.GetOwner()->GetComponentByClass(UTobiiGazeFocusableComponent::StaticClass());
		if (GazeFocusableComponent != nullptr)
		{
			GazeFocusableComponent->PrimitiveLostGazeFocus(&PrimitiveComponentToLoseFocus);
		}
	}
}

void FTobiiGTOMEngine::NotifyWidgetGazeFocusReceived(UTobiiGazeFocusableWidget& WidgetToReceiveFocus)
{
	WidgetToReceiveFocus.bHasGazeFocus = true;
	WidgetToReceiveFocus.ReceiveFocus();

	if (WidgetToReceiveFocus.GetWorldSpaceHostWidgetComponent() != nullptr && WidgetToReceiveFocus.GetWorldSpaceHostWidgetComponent()->GetOwner() != nullptr)
	{
		UTobiiGazeFocusableComponent* GazeFocusableComponent = (UTobiiGazeFocusableComponent*)WidgetToReceiveFocus.GetWorldSpaceHostWidgetComponent()->GetOwner()->GetComponentByClass(UTobiiGazeFocusableComponent::StaticClass());
		if (GazeFocusableComponent != nullptr)
		{
			GazeFocusableComponent->WidgetReceivedGazeFocus(&WidgetToReceiveFocus);
		}
	}
}

void FTobiiGTOMEngine::NotifyWidgetGazeFocusLost(UTobiiGazeFocusableWidget& WidgetToLoseFocus)
{
	WidgetToLoseFocus.bHasGazeFocus = false;
	WidgetToLoseFocus.LoseFocus();

	if (WidgetToLoseFocus.GetWorldSpaceHostWidgetComponent() != nullptr && WidgetToLoseFocus.GetWorldSpaceHostWidgetComponent()->GetOwner() != nullptr)
	{
		UTobiiGazeFocusableComponent* GazeFocusableComponent = (UTobiiGazeFocusableComponent*)WidgetToLoseFocus.GetWorldSpaceHostWidgetComponent()->GetOwner()->GetComponentByClass(UTobiiGazeFocusableComponent::StaticClass());
		if (GazeFocusableComponent != nullptr)
		{
			GazeFocusableComponent->WidgetLostGazeFocus(&WidgetToLoseFocus);
		}
	}
}

void FTobiiGTOMEngine::EmulateGazeFocus(TArray<FTobiiGazeFocusData>& EmulatedFocusData)
{
	G2OMFocusResults.Empty();
	G2OMFocusResults = EmulatedFocusData;
	UPrimitiveComponent* TopPrimitive = nullptr;
	UTobiiGazeFocusableWidget* TopWidget = nullptr;
	if (EmulatedFocusData.Num() > 0)
	{
		TopPrimitive = EmulatedFocusData[0].FocusedPrimitiveComponent.IsValid() ? EmulatedFocusData[0].FocusedPrimitiveComponent.Get() : nullptr;
		TopWidget = EmulatedFocusData[0].FocusedWidget.IsValid() ? EmulatedFocusData[0].FocusedWidget.Get() : nullptr;
	}

	UpdateWinners(TopPrimitive, TopWidget);
}
