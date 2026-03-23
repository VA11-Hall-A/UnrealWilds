#pragma once
#include "CoreMinimal.h"

struct FGravitySourceData
{
	FVector Location;
	double MassDotG;
	double PlanetRadius;
	double HollowRadius;
	bool bUseInverseSquare;
};

struct UNREALWILDS_API FGravityAsyncInput : public Chaos::FSimCallbackInput
{
	TArray<FGravitySourceData> GravitySourceData;

	void Reset()
	{
		GravitySourceData.Empty();
	}
};

class UNREALWILDS_API FGravityAsyncCallback : public Chaos::TSimCallbackObject<
		FGravityAsyncInput,
		Chaos::FSimCallbackNoOutput,
		Chaos::ESimCallbackOptions::PreIntegrate | Chaos::ESimCallbackOptions::Presimulate>
{
public:
	FGravityAsyncCallback();
	virtual ~FGravityAsyncCallback() override;

	virtual void OnPreSimulate_Internal() override;
	virtual void OnPreIntegrate_Internal() override;
	
protected:
	static double GravitationalConstant;  
};
