/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#include "TobiiGTOMTypes.h"
#include "STobiiRadialMenuWidget.h"

#include "CoreMinimal.h"
#include "ObjectEditorUtils.h"
#include "Components/SizeBox.h"
#include "Components/WidgetComponent.h"

#include "TobiiRadialMenuWidget.generated.h"

UCLASS()
class TOBIIGTOM_API UTobiiRadialMenuSlot : public UPanelSlot
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layout|Radial Menu Slot")
	FVector2D Offset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layout|Radial Menu Slot")
	float Scale;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layout|Radial Menu Slot")
	float Alpha;

public:
#if WITH_EDITOR
	virtual bool NudgeByDesigner(const FVector2D& NudgeDirection, const TOptional<int32>& GridSnapSize) override;
	virtual bool DragDropPreviewByDesigner(const FVector2D& LocalCursorPosition, const TOptional<int32>& XGridSnapSize, const TOptional<int32>& YGridSnapSize) override;
	virtual void SynchronizeFromTemplate(const UPanelSlot* const TemplateSlot) override;
#endif //WITH_EDITOR

	UFUNCTION(BlueprintCallable, Category = "Layout|Radial Menu Slot")
	void SetOffset(FVector2D InOffset);
	UFUNCTION(BlueprintCallable, Category = "Layout|Radial Menu Slot")
	FVector2D GetOffset() const;

	UFUNCTION(BlueprintCallable, Category = "Layout|Radial Menu Slot")
	void SetScale(float InScale);
	UFUNCTION(BlueprintCallable, Category = "Layout|Radial Menu Slot")
	float GetScale() const;

	UFUNCTION(BlueprintCallable, Category = "Layout|Radial Menu Slot")
	void SetAlpha(float InAlpha);
	UFUNCTION(BlueprintCallable, Category = "Layout|Radial Menu Slot")
	float GetAlpha() const;

public:
	STobiiRadialMenuWidget::FSlot* GetSlateSlot() { return Slot; }
	void BuildSlot(TSharedRef<STobiiRadialMenuWidget> RadialMenu);

	// UPanelSlot interface
	virtual void SynchronizeProperties() override;
	// End of UPanelSlot interface

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
	// UObject interface
	virtual void PreEditChange(class FEditPropertyChain& PropertyAboutToChange) override;
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
	// End of UObject interface
#endif

private:
	STobiiRadialMenuWidget::FSlot* Slot;
};

/**
  */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent, DisplayName= "Tobii Radial Menu"))
class TOBIIGTOM_API UTobiiRadialMenuWidget : public UPanelWidget
{
	GENERATED_UCLASS_BODY()

public:
	//A hard border will not be blended with the panel, while a soft border will be blended
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radial Menu")
	bool bUseHardBorder;
	//The color of the borders around each panel
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radial Menu")
	FColor BorderColor;
	//The color of the panels themselves
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radial Menu")
	FColor PanelColor;
	//The thickness of the border area
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radial Menu")
	float BorderThicknessPx;
	//The amount of pixels separating each segment
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radial Menu")
	float SegmentSeparationPx;
	//This is how rotated the panel is. Useful if you want another orientation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radial Menu")
	float AngularDisplacementDeg;
	//This is the preferred radius. This will inform the desired size of the widget
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radial Menu")
	float RadiusPx;
	//The amount of vertices that will be used for the outside of the circle segments in total.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radial Menu")
	int32 VertexCount;

public:
	UFUNCTION(BlueprintCallable, Category = "Radial Menu")
	void SetUseHardBorder(bool bNewUseHardBorder);
	UFUNCTION(BlueprintCallable, Category = "Radial Menu")
	void SetBorderColor(FColor NewBorderColor);
	UFUNCTION(BlueprintCallable, Category = "Radial Menu")
	void SetPanelColor(FColor NewPanelColor);
	UFUNCTION(BlueprintCallable, Category = "Radial Menu")
	void SetBorderThicknessPx(float NewBorderThicknessPx);
	UFUNCTION(BlueprintCallable, Category = "Radial Menu")
	void SetSegmentSeparationPx(float NewSegmentSeparationPx);
	UFUNCTION(BlueprintCallable, Category = "Radial Menu")
	void SetAngularDisplacementDeg(float NewAngularDisplacementDeg);
	UFUNCTION(BlueprintCallable, Category = "Radial Menu")
	void SetRadiusPx(float NewRadiusPx);
	UFUNCTION(BlueprintCallable, Category = "Radial Menu")
	void SetVertexCount(int32 NewVertexCount);

public:
	/**  */
	UFUNCTION(BlueprintCallable, Category = "Radial Menu")
	UTobiiRadialMenuSlot* AddRadialMenuChild(UWidget* Content);

	/** Computes the geometry for a particular slot based on the current geometry of the canvas. */
	bool GetGeometryForSlot(int32 SlotIndex, FGeometry& ArrangedGeometry) const;
	bool GetGeometryForSlot(UTobiiRadialMenuSlot* InSlot, FGeometry& ArrangedGeometry) const;

	void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
	// UWidget interface
	virtual const FText GetPaletteCategory() override;
	// End UWidget interface

	// UWidget interface
	virtual bool LockToPanelOnDrag() const override
	{
		return true;
	}
	// End UWidget interface
#endif

protected:

	// UPanelWidget
	virtual UClass* GetSlotClass() const override;
	virtual void OnSlotAdded(UPanelSlot* Slot) override;
	virtual void OnSlotRemoved(UPanelSlot* Slot) override;
	// End UPanelWidget

protected:

	TSharedPtr<class STobiiRadialMenuWidget> MyRadialMenu;

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface
};
