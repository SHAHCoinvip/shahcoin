#include "SHAHCOINWallet.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Engine/Engine.h"

USHAHCOINWallet::USHAHCOINWallet()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    RPCUrl = TEXT("http://localhost:8368");
    Network = TEXT("mainnet");
    AutoConnect = true;
    BalanceUpdateInterval = 30.0f;
    
    // Initialize HTTP module
    HttpModule = &FHttpModule::Get();
}

void USHAHCOINWallet::BeginPlay()
{
    Super::BeginPlay();
    
    if (AutoConnect)
    {
        ConnectWallet();
    }
}

void USHAHCOINWallet::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Handle balance update timer
    if (IsConnected && BalanceUpdateInterval > 0.0f)
    {
        BalanceUpdateTimer += DeltaTime;
        if (BalanceUpdateTimer >= BalanceUpdateInterval)
        {
            BalanceUpdateTimer = 0.0f;
            GetBalance();
        }
    }
}

void USHAHCOINWallet::ConnectWallet()
{
    if (IsConnected)
    {
        UE_LOG(LogTemp, Warning, TEXT("SHAHCOIN Wallet already connected"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Connecting to SHAHCOIN wallet..."));
    
    // In a real implementation, this would connect to a wallet provider
    // For now, we'll simulate a connection
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
    {
        // Simulate getting wallet address
        CurrentAddress = FString::Printf(TEXT("SHAH%d"), FMath::RandRange(1000000000, 9999999999));
        IsConnected = true;
        
        // Broadcast connection event
        OnWalletConnected.Broadcast(CurrentAddress);
        
        UE_LOG(LogTemp, Log, TEXT("SHAHCOIN Wallet connected: %s"), *CurrentAddress);
        
        // Get initial balance
        GetBalance();
        
    }, 1.0f, false);
}

void USHAHCOINWallet::DisconnectWallet()
{
    if (!IsConnected)
    {
        return;
    }
    
    IsConnected = false;
    CurrentAddress = TEXT("");
    CurrentBalance = 0.0f;
    
    OnWalletDisconnected.Broadcast(TEXT(""));
    UE_LOG(LogTemp, Log, TEXT("SHAHCOIN Wallet disconnected"));
}

void USHAHCOINWallet::GetBalance()
{
    if (!IsConnected)
    {
        OnError.Broadcast(TEXT("Wallet not connected"));
        return;
    }
    
    // Create RPC request
    TSharedPtr<FJsonObject> RequestObject = MakeShareable(new FJsonObject);
    RequestObject->SetStringField(TEXT("jsonrpc"), TEXT("2.0"));
    RequestObject->SetNumberField(TEXT("id"), ++RequestId);
    RequestObject->SetStringField(TEXT("method"), TEXT("getbalance"));
    
    TArray<TSharedPtr<FJsonValue>> Params;
    Params.Add(MakeShareable(new FJsonValueString(CurrentAddress)));
    Params.Add(MakeShareable(new FJsonValueNumber(1)));
    RequestObject->SetArrayField(TEXT("params"), Params);
    
    // Convert to string
    FString RequestString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestString);
    FJsonSerializer::Serialize(RequestObject.ToSharedRef(), Writer);
    
    // Send HTTP request
    SendRPCRequest(RequestString, [this](const FString& Response)
    {
        ParseBalanceResponse(Response);
    });
}

void USHAHCOINWallet::SendSHAH(const FString& ToAddress, float Amount)
{
    if (!IsConnected)
    {
        OnError.Broadcast(TEXT("Wallet not connected"));
        return;
    }
    
    if (Amount <= 0.0f)
    {
        OnError.Broadcast(TEXT("Amount must be greater than 0"));
        return;
    }
    
    if (Amount > CurrentBalance)
    {
        OnError.Broadcast(TEXT("Insufficient balance"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Sending %.2f SHAH to %s"), Amount, *ToAddress);
    
    // In a real implementation, this would create and sign a transaction
    // For now, we'll simulate the transaction
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, ToAddress, Amount]()
    {
        // Simulate transaction
        FString TxId = FString::Printf(TEXT("tx_%d"), FMath::RandRange(1000000000, 9999999999));
        CurrentBalance -= Amount;
        
        OnTransactionSent.Broadcast(TxId);
        OnBalanceUpdated.Broadcast(CurrentBalance);
        
        UE_LOG(LogTemp, Log, TEXT("Transaction sent: %s, Amount: %.2f SHAH to %s"), 
               *TxId, Amount, *ToAddress);
        
    }, 2.0f, false);
}

void USHAHCOINWallet::GetNFTBalance()
{
    if (!IsConnected)
    {
        OnError.Broadcast(TEXT("Wallet not connected"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Getting NFT balance for wallet: %s"), *CurrentAddress);
    
    // In a real implementation, this would query the NFT registry
    // For now, we'll return mock data
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
    {
        // Simulate NFT data
        FNFTInfo NFTInfo;
        NFTInfo.TokenId = TEXT("1");
        NFTInfo.ContractAddress = TEXT("SHAH_NFT_001");
        NFTInfo.Name = TEXT("Game Achievement #1");
        NFTInfo.Description = TEXT("First victory in the arena");
        NFTInfo.Image = TEXT("https://example.com/nft1.png");
        
        // You can add an event to handle NFT data
        UE_LOG(LogTemp, Log, TEXT("Found NFT: %s"), *NFTInfo.Name);
        
    }, 0.5f, false);
}

void USHAHCOINWallet::SendRPCRequest(const FString& RequestData, TFunction<void(const FString&)> Callback)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule->CreateRequest();
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetURL(RPCUrl);
    Request->SetContentAsString(RequestData);
    
    Request->OnProcessRequestComplete().BindLambda([Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
    {
        if (bSuccess && Response.IsValid())
        {
            Callback(Response->GetContentAsString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("RPC request failed"));
        }
    });
    
    Request->ProcessRequest();
}

void USHAHCOINWallet::ParseBalanceResponse(const FString& Response)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response);
    
    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        // Check for error
        if (JsonObject->HasField(TEXT("error")) && !JsonObject->GetObjectField(TEXT("error"))->IsEmpty())
        {
            FString ErrorMessage = JsonObject->GetObjectField(TEXT("error"))->GetStringField(TEXT("message"));
            OnError.Broadcast(FString::Printf(TEXT("RPC Error: %s"), *ErrorMessage));
            return;
        }
        
        // Parse result
        if (JsonObject->HasField(TEXT("result")))
        {
            float Balance = JsonObject->GetNumberField(TEXT("result"));
            CurrentBalance = Balance;
            OnBalanceUpdated.Broadcast(Balance);
        }
    }
    else
    {
        OnError.Broadcast(TEXT("Failed to parse RPC response"));
    }
}

// Blueprint exposed functions
void USHAHCOINWallet::ConnectWalletBP()
{
    ConnectWallet();
}

void USHAHCOINWallet::DisconnectWalletBP()
{
    DisconnectWallet();
}

void USHAHCOINWallet::GetBalanceBP()
{
    GetBalance();
}

void USHAHCOINWallet::SendSHAHBP(const FString& ToAddress, float Amount)
{
    SendSHAH(ToAddress, Amount);
}

void USHAHCOINWallet::GetNFTBalanceBP()
{
    GetNFTBalance();
}

// Getters for Blueprint
bool USHAHCOINWallet::GetIsConnected() const
{
    return IsConnected;
}

FString USHAHCOINWallet::GetCurrentAddress() const
{
    return CurrentAddress;
}

float USHAHCOINWallet::GetCurrentBalance() const
{
    return CurrentBalance;
}
