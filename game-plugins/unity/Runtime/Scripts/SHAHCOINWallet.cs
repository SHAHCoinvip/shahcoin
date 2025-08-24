using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Networking;
using Newtonsoft.Json;

namespace SHAHCOIN
{
    /// <summary>
    /// SHAHCOIN Wallet Integration for Unity
    /// Handles wallet connection, balance queries, and basic transactions
    /// </summary>
    public class SHAHCOINWallet : MonoBehaviour
    {
        [Header("Network Configuration")]
        [SerializeField] private string rpcUrl = "http://localhost:8368";
        [SerializeField] private string network = "mainnet"; // mainnet, testnet, regtest
        
        [Header("Wallet Settings")]
        [SerializeField] private bool autoConnect = true;
        [SerializeField] private float balanceUpdateInterval = 30f;
        
        [Header("Events")]
        public UnityEvent<string> OnWalletConnected;
        public UnityEvent<string> OnWalletDisconnected;
        public UnityEvent<decimal> OnBalanceUpdated;
        public UnityEvent<string> OnTransactionSent;
        public UnityEvent<string> OnError;
        
        // Wallet state
        private bool isConnected = false;
        private string currentAddress = "";
        private decimal currentBalance = 0;
        private Coroutine balanceUpdateCoroutine;
        
        // RPC request ID counter
        private int requestId = 0;
        
        /// <summary>
        /// Wallet connection status
        /// </summary>
        public bool IsConnected => isConnected;
        
        /// <summary>
        /// Current wallet address
        /// </summary>
        public string CurrentAddress => currentAddress;
        
        /// <summary>
        /// Current wallet balance in SHAH
        /// </summary>
        public decimal CurrentBalance => currentBalance;
        
        private void Start()
        {
            if (autoConnect)
            {
                StartCoroutine(ConnectWallet());
            }
        }
        
        private void OnDestroy()
        {
            DisconnectWallet();
        }
        
        /// <summary>
        /// Connect to SHAHCOIN wallet
        /// </summary>
        public IEnumerator ConnectWallet()
        {
            try
            {
                // In a real implementation, this would connect to a wallet provider
                // For now, we'll simulate a connection
                yield return new WaitForSeconds(1f);
                
                // Simulate getting wallet address
                currentAddress = "SHAH" + UnityEngine.Random.Range(1000000000, 9999999999).ToString();
                isConnected = true;
                
                OnWalletConnected?.Invoke(currentAddress);
                
                // Start balance updates
                StartBalanceUpdates();
                
                Debug.Log($"SHAHCOIN Wallet connected: {currentAddress}");
            }
            catch (Exception e)
            {
                OnError?.Invoke($"Failed to connect wallet: {e.Message}");
                Debug.LogError($"SHAHCOIN Wallet connection failed: {e.Message}");
            }
        }
        
        /// <summary>
        /// Disconnect from wallet
        /// </summary>
        public void DisconnectWallet()
        {
            if (balanceUpdateCoroutine != null)
            {
                StopCoroutine(balanceUpdateCoroutine);
                balanceUpdateCoroutine = null;
            }
            
            isConnected = false;
            currentAddress = "";
            currentBalance = 0;
            
            OnWalletDisconnected?.Invoke("");
            Debug.Log("SHAHCOIN Wallet disconnected");
        }
        
        /// <summary>
        /// Get wallet balance
        /// </summary>
        public IEnumerator GetBalance()
        {
            if (!isConnected)
            {
                OnError?.Invoke("Wallet not connected");
                yield break;
            }
            
            var request = new RPCRequest
            {
                jsonrpc = "2.0",
                id = ++requestId,
                method = "getbalance",
                @params = new object[] { currentAddress, 1 }
            };
            
            yield return StartCoroutine(SendRPCRequest(request, (response) =>
            {
                if (response.error != null)
                {
                    OnError?.Invoke($"RPC Error: {response.error.message}");
                    return;
                }
                
                if (decimal.TryParse(response.result.ToString(), out decimal balance))
                {
                    currentBalance = balance;
                    OnBalanceUpdated?.Invoke(balance);
                }
            }));
        }
        
        /// <summary>
        /// Send SHAH to another address
        /// </summary>
        public IEnumerator SendSHAH(string toAddress, decimal amount)
        {
            if (!isConnected)
            {
                OnError?.Invoke("Wallet not connected");
                yield break;
            }
            
            if (amount <= 0)
            {
                OnError?.Invoke("Amount must be greater than 0");
                yield break;
            }
            
            if (amount > currentBalance)
            {
                OnError?.Invoke("Insufficient balance");
                yield break;
            }
            
            // In a real implementation, this would create and sign a transaction
            // For now, we'll simulate the transaction
            yield return new WaitForSeconds(2f);
            
            // Simulate transaction
            string txid = "tx_" + UnityEngine.Random.Range(1000000000, 9999999999).ToString();
            currentBalance -= amount;
            
            OnTransactionSent?.Invoke(txid);
            OnBalanceUpdated?.Invoke(currentBalance);
            
            Debug.Log($"Transaction sent: {txid}, Amount: {amount} SHAH to {toAddress}");
        }
        
        /// <summary>
        /// Get NFT balance for the connected wallet
        /// </summary>
        public IEnumerator GetNFTBalance()
        {
            if (!isConnected)
            {
                OnError?.Invoke("Wallet not connected");
                yield break;
            }
            
            // In a real implementation, this would query the NFT registry
            // For now, we'll return mock data
            yield return new WaitForSeconds(0.5f);
            
            var nfts = new List<NFTInfo>
            {
                new NFTInfo
                {
                    tokenId = "1",
                    contractAddress = "SHAH_NFT_001",
                    name = "Game Achievement #1",
                    description = "First victory in the arena",
                    image = "https://example.com/nft1.png"
                }
            };
            
            // You can add an event to handle NFT data
            Debug.Log($"Found {nfts.Count} NFTs in wallet");
        }
        
        /// <summary>
        /// Start automatic balance updates
        /// </summary>
        private void StartBalanceUpdates()
        {
            if (balanceUpdateCoroutine != null)
            {
                StopCoroutine(balanceUpdateCoroutine);
            }
            
            balanceUpdateCoroutine = StartCoroutine(BalanceUpdateLoop());
        }
        
        /// <summary>
        /// Balance update loop
        /// </summary>
        private IEnumerator BalanceUpdateLoop()
        {
            while (isConnected)
            {
                yield return StartCoroutine(GetBalance());
                yield return new WaitForSeconds(balanceUpdateInterval);
            }
        }
        
        /// <summary>
        /// Send RPC request to SHAHCOIN node
        /// </summary>
        private IEnumerator SendRPCRequest(RPCRequest request, System.Action<RPCResponse> callback)
        {
            string json = JsonConvert.SerializeObject(request);
            
            using (UnityWebRequest webRequest = new UnityWebRequest(rpcUrl, "POST"))
            {
                byte[] bodyRaw = System.Text.Encoding.UTF8.GetBytes(json);
                webRequest.uploadHandler = new UploadHandlerRaw(bodyRaw);
                webRequest.downloadHandler = new DownloadHandlerBuffer();
                webRequest.SetRequestHeader("Content-Type", "application/json");
                
                yield return webRequest.SendWebRequest();
                
                if (webRequest.result == UnityWebRequest.Result.Success)
                {
                    try
                    {
                        var response = JsonConvert.DeserializeObject<RPCResponse>(webRequest.downloadHandler.text);
                        callback?.Invoke(response);
                    }
                    catch (Exception e)
                    {
                        OnError?.Invoke($"Failed to parse RPC response: {e.Message}");
                    }
                }
                else
                {
                    OnError?.Invoke($"RPC request failed: {webRequest.error}");
                }
            }
        }
    }
    
    /// <summary>
    /// RPC request structure
    /// </summary>
    [Serializable]
    public class RPCRequest
    {
        public string jsonrpc;
        public int id;
        public string method;
        public object[] @params;
    }
    
    /// <summary>
    /// RPC response structure
    /// </summary>
    [Serializable]
    public class RPCResponse
    {
        public object result;
        public RPCError error;
        public int id;
    }
    
    /// <summary>
    /// RPC error structure
    /// </summary>
    [Serializable]
    public class RPCError
    {
        public int code;
        public string message;
    }
    
    /// <summary>
    /// NFT information structure
    /// </summary>
    [Serializable]
    public class NFTInfo
    {
        public string tokenId;
        public string contractAddress;
        public string name;
        public string description;
        public string image;
        public Dictionary<string, object> attributes;
    }
    
    /// <summary>
    /// Unity event for string parameters
    /// </summary>
    [Serializable]
    public class UnityEvent<T> : UnityEngine.Events.UnityEvent<T> { }
}
