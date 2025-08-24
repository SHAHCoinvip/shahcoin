#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Http.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"
#include "ShahcoinTypes.h"
#include "ShahcoinManager.generated.h"

/**
 * Main manager class for Shahcoin integration in Unreal Engine
 * Provides wallet connectivity, NFT operations, and ShahSwap integration
 */
UCLASS(BlueprintType, Blueprintable)
class SHAHCOINPLUGIN_API UShahcoinManager : public UObject
{
	GENERATED_BODY()

public:
	UShahcoinManager();

	// Singleton instance
	static UShahcoinManager* GetInstance();

	// Static initialization
	UFUNCTION(BlueprintCallable, Category = "Shahcoin")
	static void Initialize(const FString& ApiKey);

	UFUNCTION(BlueprintCallable, Category = "Shahcoin")
	static void SetNetwork(EShahcoinNetwork Network);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Shahcoin")
	FShahcoinWalletCallback OnWalletConnected;

	UPROPERTY(BlueprintAssignable, Category = "Shahcoin")
	FShahcoinWalletCallback OnWalletDisconnected;

	UPROPERTY(BlueprintAssignable, Category = "Shahcoin")
	FShahcoinBalanceCallback OnBalanceChanged;

	UPROPERTY(BlueprintAssignable, Category = "Shahcoin")
	FShahcoinErrorCallback OnError;

	// Wallet Operations
	UFUNCTION(BlueprintCallable, Category = "Shahcoin")
	void ConnectWallet(const FShahcoinWalletCallback& Callback);

	UFUNCTION(BlueprintCallable, Category = "Shahcoin")
	void DisconnectWallet();

	UFUNCTION(BlueprintPure, Category = "Shahcoin")
	bool IsWalletConnected() const;

	UFUNCTION(BlueprintPure, Category = "Shahcoin")
	FString GetWalletAddress() const;

	// Balance Operations
	UFUNCTION(BlueprintCallable, Category = "Shahcoin")
	void GetBalance(const FShahcoinBalanceCallback& Callback);

	UFUNCTION(BlueprintCallable, Category = "Shahcoin")
	void GetTokenBalance(const FString& ContractAddress, const FShahcoinBalanceCallback& Callback);

	UFUNCTION(BlueprintCallable, Category = "Shahcoin")
	void GetNFTCount(const FString& ContractAddress, const FShahcoinNFTCountCallback& Callback);

	// NFT Operations
	UFUNCTION(BlueprintCallable, Category = "Shahcoin")
	void HasNFT(const FString& ContractAddress, const FString& TokenId, const FShahcoinNFTCallback& Callback);

	UFUNCTION(BlueprintCallable, Category = "Shahcoin")
	void GetOwnedNFTs(const FString& ContractAddress, const FShahcoinNFTArrayCallback& Callback);

	UFUNCTION(BlueprintCallable, Category = "Shahcoin")
	void GetNFTMetadata(const FString& ContractAddress, const FString& TokenId, const FShahcoinNFTMetadataCallback& Callback);

	// ShahSwap Integration
	UFUNCTION(BlueprintCallable, Category = "Shahcoin")
	void GetTokenPrice(const FString& TokenSymbol, const FShahcoinPriceCallback& Callback);

	UFUNCTION(BlueprintCallable, Category = "Shahcoin")
	void CanAffordItem(const FString& TokenSymbol, float Amount, const FShahcoinAffordabilityCallback& Callback);

	UFUNCTION(BlueprintCallable, Category = "Shahcoin")
	void GetSwapQuote(const FString& InputToken, const FString& OutputToken, float Amount, const FShahcoinSwapQuoteCallback& Callback);

	// Transaction Operations
	UFUNCTION(BlueprintCallable, Category = "Shahcoin")
	void GetTransactionHistory(int32 Limit = 50, int32 Offset = 0, const FShahcoinTransactionCallback& Callback = FShahcoinTransactionCallback());

	UFUNCTION(BlueprintCallable, Category = "Shahcoin")
	void SendTransaction(const FString& ToAddress, float Amount, float Fee = 0.0f, const FString& Memo = TEXT(""), const FShahcoinErrorCallback& Callback = FShahcoinErrorCallback());

	// Staking Operations
	UFUNCTION(BlueprintCallable, Category = "Shahcoin")
	void GetStakingInfo(const FShahcoinStakingInfoCallback& Callback);

	UFUNCTION(BlueprintCallable, Category = "Shahcoin")
	void Stake(float Amount, const FShahcoinErrorCallback& Callback = FShahcoinErrorCallback());

	// Token Operations
	UFUNCTION(BlueprintCallable, Category = "Shahcoin")
	void GetTokenInfo(const FString& ContractAddress, const FShahcoinTokenInfoCallback& Callback);

	UFUNCTION(BlueprintCallable, Category = "Shahcoin")
	void CreateToken(const FString& Name, const FString& Symbol, int32 Decimals, float TotalSupply, const FShahcoinTokenInfoCallback& Callback);

	UFUNCTION(BlueprintCallable, Category = "Shahcoin")
	void CreateNFT(const FString& Name, const FString& Description, const FString& ImageUrl, const TMap<FString, FString>& Attributes, const FShahcoinNFTMetadataCallback& Callback);

	// Network Information
	UFUNCTION(BlueprintPure, Category = "Shahcoin")
	EShahcoinNetwork GetCurrentNetwork() const;

	UFUNCTION(BlueprintPure, Category = "Shahcoin")
	FString GetNetworkName() const;

	UFUNCTION(BlueprintPure, Category = "Shahcoin")
	FString GetExplorerUrl() const;

protected:
	// Configuration
	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	FString ApiKey;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	EShahcoinNetwork CurrentNetwork;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	FString BaseUrl;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	FString ExplorerUrl;

	// Connection state
	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	FString WalletAddress;

	UPROPERTY(BlueprintReadOnly, Category = "Shahcoin")
	bool bIsConnected;

	// HTTP client
	TSharedPtr<IHttpRequest> HttpRequest;
	TSharedPtr<IHttpResponse> HttpResponse;

	// WebSocket client
	TSharedPtr<IWebSocket> WebSocket;

	// Network configurations
	static TMap<EShahcoinNetwork, FString> NetworkUrls;
	static TMap<EShahcoinNetwork, FString> ExplorerUrls;

	// Callback storage
	TArray<FShahcoinWalletCallback> WalletCallbacks;
	TArray<FShahcoinBalanceCallback> BalanceCallbacks;
	TArray<FShahcoinNFTCallback> NFTCallbacks;
	TArray<FShahcoinNFTArrayCallback> NFTArrayCallbacks;
	TArray<FShahcoinNFTMetadataCallback> NFTMetadataCallbacks;
	TArray<FShahcoinNFTCountCallback> NFTCountCallbacks;
	TArray<FShahcoinPriceCallback> PriceCallbacks;
	TArray<FShahcoinAffordabilityCallback> AffordabilityCallbacks;
	TArray<FShahcoinSwapQuoteCallback> SwapQuoteCallbacks;
	TArray<FShahcoinTransactionCallback> TransactionCallbacks;
	TArray<FShahcoinStakingInfoCallback> StakingInfoCallbacks;
	TArray<FShahcoinTokenInfoCallback> TokenInfoCallbacks;
	TArray<FShahcoinErrorCallback> ErrorCallbacks;

private:
	// HTTP request handling
	void OnHttpRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);
	void MakeHttpRequest(const FString& Endpoint, const FString& Method, const FString& Data, const FString& CallbackType);

	// WebSocket handling
	void SetupWebSocket();
	void OnWebSocketConnected();
	void OnWebSocketConnectionError(const FString& Error);
	void OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
	void OnWebSocketMessage(const FString& Message);

	// JSON parsing
	bool ParseWalletResponse(const FString& JsonString, bool& bSuccess, FString& Address);
	bool ParseBalanceResponse(const FString& JsonString, float& Balance);
	bool ParseNFTResponse(const FString& JsonString, TArray<FShahcoinNFTInfo>& NFTs);
	bool ParseNFTMetadataResponse(const FString& JsonString, FShahcoinNFTMetadata& Metadata);
	bool ParseSwapQuoteResponse(const FString& JsonString, FShahcoinSwapQuote& Quote);
	bool ParseTransactionResponse(const FString& JsonString, TArray<FShahcoinTransaction>& Transactions);
	bool ParseStakingInfoResponse(const FString& JsonString, FShahcoinStakingInfo& StakingInfo);
	bool ParseTokenInfoResponse(const FString& JsonString, FShahcoinTokenInfo& TokenInfo);

	// Utility functions
	FString GetNetworkString() const;
	void LogError(const FString& Error);
	void ExecuteCallbacks(const FString& CallbackType, const FString& Data);
};
