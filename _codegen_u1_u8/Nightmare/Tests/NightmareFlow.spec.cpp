#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "NightmareFailPopupState.h"
#include "UObject/Package.h"

BEGIN_DEFINE_SPEC(FNightmareFlowSpec,
	"Nightmare.Flow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FNightmareFlowSpec)

void FNightmareFlowSpec::Define()
{
	Describe("UNightmareFailPopupState", [this]()
	{
		It("opens with 10 second display countdown", [this]()
		{
			UNightmareFailPopupState* State = NewObject<UNightmareFailPopupState>(GetTransientPackage());
			State->SetCountdownDurationSeconds(10.0f);
			State->Open();

			TestTrue(TEXT("open"), State->IsOpen());
			TestEqual(TEXT("display"), State->GetDisplayCountdownSeconds(), 10);
			TestEqual(TEXT("remaining"), State->GetRemainingSeconds(), 10.0f);
		});

		It("counts down and expires after duration", [this]()
		{
			UNightmareFailPopupState* State = NewObject<UNightmareFailPopupState>(GetTransientPackage());
			State->SetCountdownDurationSeconds(10.0f);
			State->Open();

			TestFalse(TEXT("not yet"), State->TickCountdown(9.9f));
			TestEqual(TEXT("display 1"), State->GetDisplayCountdownSeconds(), 1);

			TestTrue(TEXT("expired"), State->TickCountdown(0.2f));
			TestFalse(TEXT("closed"), State->IsOpen());
		});

		It("Close hides popup and clears remaining time", [this]()
		{
			UNightmareFailPopupState* State = NewObject<UNightmareFailPopupState>(GetTransientPackage());
			State->Open();
			State->Close();

			TestFalse(TEXT("closed"), State->IsOpen());
			TestEqual(TEXT("remaining zero"), State->GetRemainingSeconds(), 0.0f);
		});
	});
}

#endif
