using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Networking;

namespace ShahcoinPlugin
{
    /// <summary>
    /// Main manager class for Shahcoin integration in Unity games
    /// </summary>
    public class ShahcoinManager : MonoBehaviour
    {
        #region Singleton

        private static ShahcoinManager _instance;
        public static ShahcoinManager Instance
        {
            get
            {
                if (_instance == null)
                {
                    GameObject go = new GameObject("ShahcoinManager");
                    _instance = go.AddComponent<ShahcoinManager>();
                    DontDestroyOnLoad(go);
                }
                return _instance;
            }
        }

        #endregion

        #region Events

        public event Action<string> OnError;
        public event Action<string> OnWalletConnected;
        public event Action OnWalletDisconnected;
        public event Action<decimal> OnBalanceChanged;
        public event Action<string> OnTransactionReceived;

        #endregion

        #region Configuration

        [Header("Configuration")]
        [SerializeField] private string apiKey = "";
        [SerializeField] private ShahcoinNetwork network = ShahcoinNetwork.Mainnet;
        [SerializeField] private string customRpcUrl = "";
        [SerializeField] private float requestTimeout = 30f;

        private string _walletAddress = "";
        private bool _isConnected = false;
        private string _baseUrl = "";

        #endregion

        #region Network Configuration

        private static readonly Dictionary<ShahcoinNetwork, string> NetworkUrls = new Dictionary<ShahcoinNetwork, string>
        {
            { ShahcoinNetwork.Mainnet, "https://rpc.shah.vip" },
            { ShahcoinNetwork.Testnet, "https://testnet-rpc.shah.vip" },
            { ShahcoinNetwork.Regtest, "http://localhost:8332" }
        };

        #endregion

        #region Initialization

        /// <summary>
        /// Initialize the Shahcoin plugin with API key
        /// </summary>
        public static void Initialize(string apiKey)
        {
            Instance.apiKey = apiKey;
            Instance.SetupNetwork();
        }

        /// <summary>
        /// Set the network to use
        /// </summary>
        public static void SetNetwork(ShahcoinNetwork network)
        {
            Instance.network = network;
            Instance.SetupNetwork();
        }

        private void SetupNetwork()
        {
            _baseUrl = string.IsNullOrEmpty(customRpcUrl) ? NetworkUrls[network] : customRpcUrl;
            Debug.Log($"[Shahcoin] Network set to: {network} ({_baseUrl})");
        }

        #endregion

        #region Wallet Operations

        /// <summary>
        /// Connect to a Shahcoin wallet
        /// </summary>
        public void ConnectWallet(Action<bool, string> callback)
        {
            if (_isConnected)
            {
                callback?.Invoke(true, _walletAddress);
                return;
            }

            StartCoroutine(ConnectWalletCoroutine(callback));
        }

        private IEnumerator ConnectWalletCoroutine(Action<bool, string> callback)
        {
            // Simulate wallet connection (in real implementation, this would connect to Qt wallet or bridge)
            var request = CreateRequest($"{_baseUrl}/wallet/connect", "POST", new Dictionary<string, object>
            {
                { "network", network.ToString().ToLower() }
            });

            yield return request.SendWebRequest();

            if (request.result == UnityWebRequest.Result.Success)
            {
                var response = JsonUtility.FromJson<WalletConnectResponse>(request.downloadHandler.text);
                if (response.success)
                {
                    _isConnected = true;
                    _walletAddress = response.data.address;
                    OnWalletConnected?.Invoke(_walletAddress);
                    callback?.Invoke(true, _walletAddress);
                }
                else
                {
                    OnError?.Invoke(response.error ?? "Failed to connect wallet");
                    callback?.Invoke(false, "");
                }
            }
            else
            {
                OnError?.Invoke($"Connection failed: {request.error}");
                callback?.Invoke(false, "");
            }
        }

        /// <summary>
        /// Disconnect from wallet
        /// </summary>
        public void DisconnectWallet()
        {
            _isConnected = false;
            _walletAddress = "";
            OnWalletDisconnected?.Invoke();
        }

        /// <summary>
        /// Check if wallet is connected
        /// </summary>
        public bool IsWalletConnected()
        {
            return _isConnected;
        }

        /// <summary>
        /// Get current wallet address
        /// </summary>
        public string GetWalletAddress()
        {
            return _walletAddress;
        }

        #endregion

        #region Balance Operations

        /// <summary>
        /// Get SHAH balance
        /// </summary>
        public void GetBalance(Action<decimal> callback)
        {
            if (!_isConnected)
            {
                OnError?.Invoke("Wallet not connected");
                callback?.Invoke(0);
                return;
            }

            StartCoroutine(GetBalanceCoroutine(callback));
        }

        private IEnumerator GetBalanceCoroutine(Action<decimal> callback)
        {
            var request = CreateRequest($"{_baseUrl}/wallet/balance/{_walletAddress}", "GET");

            yield return request.SendWebRequest();

            if (request.result == UnityWebRequest.Result.Success)
            {
                var response = JsonUtility.FromJson<BalanceResponse>(request.downloadHandler.text);
                if (response.success)
                {
                    var balance = decimal.Parse(response.data.balance.ToString());
                    OnBalanceChanged?.Invoke(balance);
                    callback?.Invoke(balance);
                }
                else
                {
                    OnError?.Invoke(response.error ?? "Failed to get balance");
                    callback?.Invoke(0);
                }
            }
            else
            {
                OnError?.Invoke($"Balance request failed: {request.error}");
                callback?.Invoke(0);
            }
        }

        /// <summary>
        /// Get token balance
        /// </summary>
        public void GetTokenBalance(string contractAddress, Action<decimal> callback)
        {
            if (!_isConnected)
            {
                OnError?.Invoke("Wallet not connected");
                callback?.Invoke(0);
                return;
            }

            StartCoroutine(GetTokenBalanceCoroutine(contractAddress, callback));
        }

        private IEnumerator GetTokenBalanceCoroutine(string contractAddress, Action<decimal> callback)
        {
            var request = CreateRequest($"{_baseUrl}/tokens/balance/{contractAddress}/{_walletAddress}", "GET");

            yield return request.SendWebRequest();

            if (request.result == UnityWebRequest.Result.Success)
            {
                var response = JsonUtility.FromJson<BalanceResponse>(request.downloadHandler.text);
                if (response.success)
                {
                    var balance = decimal.Parse(response.data.balance.ToString());
                    callback?.Invoke(balance);
                }
                else
                {
                    OnError?.Invoke(response.error ?? "Failed to get token balance");
                    callback?.Invoke(0);
                }
            }
            else
            {
                OnError?.Invoke($"Token balance request failed: {request.error}");
                callback?.Invoke(0);
            }
        }

        /// <summary>
        /// Get NFT count for a contract
        /// </summary>
        public void GetNFTCount(string contractAddress, Action<int> callback)
        {
            if (!_isConnected)
            {
                OnError?.Invoke("Wallet not connected");
                callback?.Invoke(0);
                return;
            }

            StartCoroutine(GetNFTCountCoroutine(contractAddress, callback));
        }

        private IEnumerator GetNFTCountCoroutine(string contractAddress, Action<int> callback)
        {
            var request = CreateRequest($"{_baseUrl}/nfts/count/{contractAddress}/{_walletAddress}", "GET");

            yield return request.SendWebRequest();

            if (request.result == UnityWebRequest.Result.Success)
            {
                var response = JsonUtility.FromJson<NFTCountResponse>(request.downloadHandler.text);
                if (response.success)
                {
                    callback?.Invoke(response.data.count);
                }
                else
                {
                    OnError?.Invoke(response.error ?? "Failed to get NFT count");
                    callback?.Invoke(0);
                }
            }
            else
            {
                OnError?.Invoke($"NFT count request failed: {request.error}");
                callback?.Invoke(0);
            }
        }

        #endregion

        #region NFT Operations

        /// <summary>
        /// Check if wallet owns a specific NFT
        /// </summary>
        public void HasNFT(string contractAddress, string tokenId, Action<bool> callback)
        {
            if (!_isConnected)
            {
                OnError?.Invoke("Wallet not connected");
                callback?.Invoke(false);
                return;
            }

            StartCoroutine(HasNFTCoroutine(contractAddress, tokenId, callback));
        }

        private IEnumerator HasNFTCoroutine(string contractAddress, string tokenId, Action<bool> callback)
        {
            var request = CreateRequest($"{_baseUrl}/nfts/owner/{contractAddress}/{tokenId}", "GET");

            yield return request.SendWebRequest();

            if (request.result == UnityWebRequest.Result.Success)
            {
                var response = JsonUtility.FromJson<NFTOwnerResponse>(request.downloadHandler.text);
                if (response.success)
                {
                    var hasNFT = response.data.owner.ToLower() == _walletAddress.ToLower();
                    callback?.Invoke(hasNFT);
                }
                else
                {
                    OnError?.Invoke(response.error ?? "Failed to check NFT ownership");
                    callback?.Invoke(false);
                }
            }
            else
            {
                OnError?.Invoke($"NFT ownership check failed: {request.error}");
                callback?.Invoke(false);
            }
        }

        /// <summary>
        /// Get owned NFTs for a contract
        /// </summary>
        public void GetOwnedNFTs(string contractAddress, Action<NFTInfo[]> callback)
        {
            if (!_isConnected)
            {
                OnError?.Invoke("Wallet not connected");
                callback?.Invoke(new NFTInfo[0]);
                return;
            }

            StartCoroutine(GetOwnedNFTsCoroutine(contractAddress, callback));
        }

        private IEnumerator GetOwnedNFTsCoroutine(string contractAddress, Action<NFTInfo[]> callback)
        {
            var request = CreateRequest($"{_baseUrl}/nfts/user/{_walletAddress}?contract={contractAddress}", "GET");

            yield return request.SendWebRequest();

            if (request.result == UnityWebRequest.Result.Success)
            {
                var response = JsonUtility.FromJson<NFTListResponse>(request.downloadHandler.text);
                if (response.success)
                {
                    callback?.Invoke(response.data.nfts);
                }
                else
                {
                    OnError?.Invoke(response.error ?? "Failed to get owned NFTs");
                    callback?.Invoke(new NFTInfo[0]);
                }
            }
            else
            {
                OnError?.Invoke($"Get owned NFTs failed: {request.error}");
                callback?.Invoke(new NFTInfo[0]);
            }
        }

        /// <summary>
        /// Get NFT metadata
        /// </summary>
        public void GetNFTMetadata(string contractAddress, string tokenId, Action<NFTMetadata> callback)
        {
            StartCoroutine(GetNFTMetadataCoroutine(contractAddress, tokenId, callback));
        }

        private IEnumerator GetNFTMetadataCoroutine(string contractAddress, string tokenId, Action<NFTMetadata> callback)
        {
            var request = CreateRequest($"{_baseUrl}/nfts/{contractAddress}/{tokenId}", "GET");

            yield return request.SendWebRequest();

            if (request.result == UnityWebRequest.Result.Success)
            {
                var response = JsonUtility.FromJson<NFTMetadataResponse>(request.downloadHandler.text);
                if (response.success)
                {
                    callback?.Invoke(response.data);
                }
                else
                {
                    OnError?.Invoke(response.error ?? "Failed to get NFT metadata");
                    callback?.Invoke(null);
                }
            }
            else
            {
                OnError?.Invoke($"Get NFT metadata failed: {request.error}");
                callback?.Invoke(null);
            }
        }

        #endregion

        #region ShahSwap Integration

        /// <summary>
        /// Get token price from ShahSwap
        /// </summary>
        public void GetTokenPrice(string tokenSymbol, Action<decimal> callback)
        {
            StartCoroutine(GetTokenPriceCoroutine(tokenSymbol, callback));
        }

        private IEnumerator GetTokenPriceCoroutine(string tokenSymbol, Action<decimal> callback)
        {
            var request = CreateRequest($"{_baseUrl}/shahswap/price/{tokenSymbol}", "GET");

            yield return request.SendWebRequest();

            if (request.result == UnityWebRequest.Result.Success)
            {
                var response = JsonUtility.FromJson<PriceResponse>(request.downloadHandler.text);
                if (response.success)
                {
                    var price = decimal.Parse(response.data.price.ToString());
                    callback?.Invoke(price);
                }
                else
                {
                    OnError?.Invoke(response.error ?? "Failed to get token price");
                    callback?.Invoke(0);
                }
            }
            else
            {
                OnError?.Invoke($"Get token price failed: {request.error}");
                callback?.Invoke(0);
            }
        }

        /// <summary>
        /// Check if player can afford an item
        /// </summary>
        public void CanAffordItem(string tokenSymbol, decimal amount, Action<bool> callback)
        {
            if (!_isConnected)
            {
                callback?.Invoke(false);
                return;
            }

            if (tokenSymbol.ToUpper() == "SHAH")
            {
                GetBalance((balance) => callback?.Invoke(balance >= amount));
            }
            else
            {
                // For tokens, you would need the contract address
                // This is a simplified version
                callback?.Invoke(false);
            }
        }

        /// <summary>
        /// Get swap quote from ShahSwap
        /// </summary>
        public void GetSwapQuote(string inputToken, string outputToken, decimal amount, Action<SwapQuote> callback)
        {
            StartCoroutine(GetSwapQuoteCoroutine(inputToken, outputToken, amount, callback));
        }

        private IEnumerator GetSwapQuoteCoroutine(string inputToken, string outputToken, decimal amount, Action<SwapQuote> callback)
        {
            var request = CreateRequest($"{_baseUrl}/shahswap/quote", "POST", new Dictionary<string, object>
            {
                { "inputToken", inputToken },
                { "outputToken", outputToken },
                { "amount", amount.ToString() }
            });

            yield return request.SendWebRequest();

            if (request.result == UnityWebRequest.Result.Success)
            {
                var response = JsonUtility.FromJson<SwapQuoteResponse>(request.downloadHandler.text);
                if (response.success)
                {
                    callback?.Invoke(response.data);
                }
                else
                {
                    OnError?.Invoke(response.error ?? "Failed to get swap quote");
                    callback?.Invoke(null);
                }
            }
            else
            {
                OnError?.Invoke($"Get swap quote failed: {request.error}");
                callback?.Invoke(null);
            }
        }

        #endregion

        #region Utility Methods

        private UnityWebRequest CreateRequest(string url, string method, Dictionary<string, object> data = null)
        {
            var request = new UnityWebRequest(url, method);
            request.timeout = (int)requestTimeout;

            if (!string.IsNullOrEmpty(apiKey))
            {
                request.SetRequestHeader("Authorization", $"Bearer {apiKey}");
            }

            if (data != null)
            {
                var json = JsonUtility.ToJson(new RequestData { data = data });
                byte[] bodyRaw = System.Text.Encoding.UTF8.GetBytes(json);
                request.uploadHandler = new UploadHandlerRaw(bodyRaw);
                request.downloadHandler = new DownloadHandlerBuffer();
                request.SetRequestHeader("Content-Type", "application/json");
            }
            else
            {
                request.downloadHandler = new DownloadHandlerBuffer();
            }

            return request;
        }

        #endregion

        #region Response Classes

        [System.Serializable]
        private class WalletConnectResponse
        {
            public bool success;
            public WalletData data;
            public string error;
        }

        [System.Serializable]
        private class WalletData
        {
            public string address;
            public double balance;
        }

        [System.Serializable]
        private class BalanceResponse
        {
            public bool success;
            public BalanceData data;
            public string error;
        }

        [System.Serializable]
        private class BalanceData
        {
            public double balance;
        }

        [System.Serializable]
        private class NFTCountResponse
        {
            public bool success;
            public NFTCountData data;
            public string error;
        }

        [System.Serializable]
        private class NFTCountData
        {
            public int count;
        }

        [System.Serializable]
        private class NFTOwnerResponse
        {
            public bool success;
            public NFTOwnerData data;
            public string error;
        }

        [System.Serializable]
        private class NFTOwnerData
        {
            public string owner;
        }

        [System.Serializable]
        private class NFTListResponse
        {
            public bool success;
            public NFTListData data;
            public string error;
        }

        [System.Serializable]
        private class NFTListData
        {
            public NFTInfo[] nfts;
        }

        [System.Serializable]
        private class NFTMetadataResponse
        {
            public bool success;
            public NFTMetadata data;
            public string error;
        }

        [System.Serializable]
        private class PriceResponse
        {
            public bool success;
            public PriceData data;
            public string error;
        }

        [System.Serializable]
        private class PriceData
        {
            public double price;
        }

        [System.Serializable]
        private class SwapQuoteResponse
        {
            public bool success;
            public SwapQuote data;
            public string error;
        }

        [System.Serializable]
        private class RequestData
        {
            public Dictionary<string, object> data;
        }

        #endregion
    }

    /// <summary>
    /// Available Shahcoin networks
    /// </summary>
    public enum ShahcoinNetwork
    {
        Mainnet,
        Testnet,
        Regtest
    }
}
