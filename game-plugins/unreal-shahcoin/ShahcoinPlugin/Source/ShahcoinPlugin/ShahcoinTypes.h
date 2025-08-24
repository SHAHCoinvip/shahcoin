#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "ShahcoinTypes.generated.h"

/**
 * Shahcoin network types
 */
UENUM(BlueprintType)
enum class EShahcoinNetwork : uint8
{
	Mainnet UMETA(DisplayName = "Mainnet"),
	Testnet UMETA(DisplayName = "Testnet"),
	Regtest UMETA(DisplayName = "Regtest")
};

/**
 * Shahcoin NFT information structure
 */
USTRUCT(BlueprintType)
struct FShahcoinNFTInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	FString TokenId;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	FString Name;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	FString Description;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	FString ImageUrl;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	TMap<FString, FString> Attributes;

	FShahcoinNFTInfo()
	{
		TokenId = TEXT("");
		Name = TEXT("");
		Description = TEXT("");
		ImageUrl = TEXT("");
		Attributes = TMap<FString, FString>();
	}
};

/**
 * Shahcoin NFT metadata structure
 */
USTRUCT(BlueprintType)
struct FShahcoinNFTMetadata
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	FString Name;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	FString Description;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	FString Image;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	TMap<FString, FString> Attributes;

	FShahcoinNFTMetadata()
	{
		Name = TEXT("");
		Description = TEXT("");
		Image = TEXT("");
		Attributes = TMap<FString, FString>();
	}
};

/**
 * Shahcoin swap quote structure
 */
USTRUCT(BlueprintType)
struct FShahcoinSwapQuote
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	FString InputToken;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	FString OutputToken;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	float InputAmount;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	float OutputAmount;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	float PriceImpact;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	float Fee;

	FShahcoinSwapQuote()
	{
		InputToken = TEXT("");
		OutputToken = TEXT("");
		InputAmount = 0.0f;
		OutputAmount = 0.0f;
		PriceImpact = 0.0f;
		Fee = 0.0f;
	}
};

/**
 * Shahcoin transaction information
 */
USTRUCT(BlueprintType)
struct FShahcoinTransaction
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	FString TxId;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	FString FromAddress;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	FString ToAddress;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	float Amount;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	float Fee;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	FString Status;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	int64 Timestamp;

	FShahcoinTransaction()
	{
		TxId = TEXT("");
		FromAddress = TEXT("");
		ToAddress = TEXT("");
		Amount = 0.0f;
		Fee = 0.0f;
		Status = TEXT("");
		Timestamp = 0;
	}
};

/**
 * Shahcoin staking information
 */
USTRUCT(BlueprintType)
struct FShahcoinStakingInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	float StakedAmount;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	float RewardsEarned;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	float APY;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	int64 StakingStartTime;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	bool IsStaking;

	FShahcoinStakingInfo()
	{
		StakedAmount = 0.0f;
		RewardsEarned = 0.0f;
		APY = 0.0f;
		StakingStartTime = 0;
		IsStaking = false;
	}
};

/**
 * Shahcoin token information
 */
USTRUCT(BlueprintType)
struct FShahcoinTokenInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	FString ContractAddress;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	FString Name;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	FString Symbol;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	int32 Decimals;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	float TotalSupply;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	float Price;

	FShahcoinTokenInfo()
	{
		ContractAddress = TEXT("");
		Name = TEXT("");
		Symbol = TEXT("");
		Decimals = 18;
		TotalSupply = 0.0f;
		Price = 0.0f;
	}
};

// Callback delegates
DECLARE_DYNAMIC_DELEGATE_TwoParams(FShahcoinWalletCallback, bool, bSuccess, const FString&, Address);
DECLARE_DYNAMIC_DELEGATE_OneParam(FShahcoinBalanceCallback, float, Balance);
DECLARE_DYNAMIC_DELEGATE_OneParam(FShahcoinNFTCallback, bool, bHasNFT);
DECLARE_DYNAMIC_DELEGATE_OneParam(FShahcoinNFTArrayCallback, const TArray<FShahcoinNFTInfo>&, NFTs);
DECLARE_DYNAMIC_DELEGATE_OneParam(FShahcoinNFTMetadataCallback, const FShahcoinNFTMetadata&, Metadata);
DECLARE_DYNAMIC_DELEGATE_OneParam(FShahcoinNFTCountCallback, int32, Count);
DECLARE_DYNAMIC_DELEGATE_OneParam(FShahcoinPriceCallback, float, Price);
DECLARE_DYNAMIC_DELEGATE_OneParam(FShahcoinAffordabilityCallback, bool, bCanAfford);
DECLARE_DYNAMIC_DELEGATE_OneParam(FShahcoinSwapQuoteCallback, const FShahcoinSwapQuote&, Quote);
DECLARE_DYNAMIC_DELEGATE_OneParam(FShahcoinTransactionCallback, const TArray<FShahcoinTransaction>&, Transactions);
DECLARE_DYNAMIC_DELEGATE_OneParam(FShahcoinStakingInfoCallback, const FShahcoinStakingInfo&, StakingInfo);
DECLARE_DYNAMIC_DELEGATE_OneParam(FShahcoinTokenInfoCallback, const FShahcoinTokenInfo&, TokenInfo);
DECLARE_DYNAMIC_DELEGATE_OneParam(FShahcoinErrorCallback, const FString&, ErrorMessage);
