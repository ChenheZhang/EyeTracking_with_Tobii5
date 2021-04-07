/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#include "tobii_g2om.h"

#include "CoreMinimal.h"

static class FTobiiGTOMUtils
{
public:
	static g2om_vector UE4VectorToG2OMVector(const FVector& Input)
	{
		return g2om_vector { Input.X, Input.Y, Input.Z };
	}

	static FVector G2OMVectorToUE4Vector(const g2om_vector& Input)
	{
		return FVector(Input.x, Input.y, Input.z);
	}

	static void UE4SpaceToUnitySpace(g2om_vector& Arg)
	{
		//Unity forward is Z, right is X, up is Y
		//UE4 forward is X, right is Y, up is Z

		const float Temp = Arg.z;
		Arg.z = Arg.x;
		Arg.x = Arg.y;
		Arg.y = Temp;
	}

	static void UnitySpaceToUE4Space(g2om_vector& Arg)
	{
		//Unity forward is Z, right is X, up is Y
		//UE4 forward is X, right is Y, up is Z

		const float Temp = Arg.x;
		Arg.x = Arg.z;
		Arg.z = Arg.y;
		Arg.y = Temp;
	}

	static bool QueryFocusableLayer(UPrimitiveComponent* Candidate, const FName& LayerToQuery)
	{
		FName FocusLayer = UTobiiGazeFocusableComponent::GetFocusLayerForPrimitive(Candidate);
		if (FocusLayer == FName("Default"))
		{
			return true;
		}

		return FocusLayer == LayerToQuery;
	}
	static bool QueryFocusableLayer(UTobiiGazeFocusableWidget* Candidate, const FName& LayerToQuery)
	{
		FName FocusLayer = UTobiiGazeFocusableComponent::GetFocusLayerForWidget(Candidate);
		if (FocusLayer == FName("Default"))
		{
			return true;
		}

		return FocusLayer == LayerToQuery;
	}

	template <class T>
	static bool ValidateFocusLayers(const TArray<FName>& FocusLayerFilterList, const bool bIsWhiteList, T* Candidate)
	{
		if (Candidate == nullptr)
		{
			return false;
		}

		if (bIsWhiteList)
		{
			//A white list only allows focusables that have a layer that is in the list. 
			for (auto& Layer : FocusLayerFilterList)
			{
				if (QueryFocusableLayer(Candidate, Layer))
				{
					return true;
				}
			}
		}
		else
		{
			//A black list will block any focusables whos layer is in the list.
			for (auto& Layer : FocusLayerFilterList)
			{
				if (QueryFocusableLayer(Candidate, Layer))
				{
					return false;
				}
			}

			//No filters hit, this object is ok.
			return true;
		}

		return false;
	}
};

DEFINE_LOG_CATEGORY_STATIC(LogTobiiGTOM, All, All);
