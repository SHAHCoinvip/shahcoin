@tool
extends RefCounted
class_name ShahcoinPlugin

## Shahcoin Plugin for Godot
## Provides wallet connectivity, ShahSwap integration, and blockchain interactions

# Network configurations
enum ShahcoinNetwork {
	MAINNET,
	TESTNET,
	REGTEST
}

# Configuration
var api_key: String = ""
var network: ShahcoinNetwork = ShahcoinNetwork.MAINNET
var custom_rpc_url: String = ""
var request_timeout: float = 30.0

# Connection state
var wallet_address: String = ""
var is_connected: bool = false
var base_url: String = ""

# Network URLs
const NETWORK_URLS = {
	ShahcoinNetwork.MAINNET: "https://rpc.shah.vip",
	ShahcoinNetwork.TESTNET: "https://testnet-rpc.shah.vip",
	ShahcoinNetwork.REGTEST: "http://localhost:8332"
}

# HTTP client
var http_client: HTTPRequest

# Signals
signal wallet_connected(address: String)
signal wallet_disconnected()
signal balance_changed(balance: float)
signal transaction_received(tx_data: Dictionary)
signal error_occurred(error: String)

func _init():
	http_client = HTTPRequest.new()
	http_client.timeout = request_timeout
	add_child(http_client)
	http_client.request_completed.connect(_on_request_completed)
	setup_network()

## Initialize the plugin with API key
static func initialize(api_key: String) -> void:
	ShahcoinPlugin.api_key = api_key
	ShahcoinPlugin.setup_network()

## Set the network to use
static func set_network(network: ShahcoinNetwork) -> void:
	ShahcoinPlugin.network = network
	ShahcoinPlugin.setup_network()

## Setup network configuration
func setup_network() -> void:
	base_url = custom_rpc_url if custom_rpc_url != "" else NETWORK_URLS[network]
	print("[Shahcoin] Network set to: ", network, " (", base_url, ")")

## Connect to a Shahcoin wallet
func connect_wallet(callback: Callable) -> void:
	if is_connected:
		callback.call(true, wallet_address)
		return
	
	var headers = ["Content-Type: application/json"]
	if api_key != "":
		headers.append("Authorization: Bearer " + api_key)
	
	var data = {
		"network": network.keys()[network].to_lower()
	}
	
	var json = JSON.stringify(data)
	var url = base_url + "/wallet/connect"
	
	http_client.request(url, headers, HTTPClient.METHOD_POST, json)

## Disconnect from wallet
func disconnect_wallet() -> void:
	is_connected = false
	wallet_address = ""
	wallet_disconnected.emit()

## Check if wallet is connected
func is_wallet_connected() -> bool:
	return is_connected

## Get current wallet address
func get_wallet_address() -> String:
	return wallet_address

## Get SHAH balance
func get_balance(callback: Callable) -> void:
	if not is_connected:
		error_occurred.emit("Wallet not connected")
		callback.call(0.0)
		return
	
	var headers = []
	if api_key != "":
		headers.append("Authorization: Bearer " + api_key)
	
	var url = base_url + "/wallet/balance/" + wallet_address
	http_client.request(url, headers, HTTPClient.METHOD_GET)

## Get token balance
func get_token_balance(contract_address: String, callback: Callable) -> void:
	if not is_connected:
		error_occurred.emit("Wallet not connected")
		callback.call(0.0)
		return
	
	var headers = []
	if api_key != "":
		headers.append("Authorization: Bearer " + api_key)
	
	var url = base_url + "/tokens/balance/" + contract_address + "/" + wallet_address
	http_client.request(url, headers, HTTPClient.METHOD_GET)

## Get NFT count for a contract
func get_nft_count(contract_address: String, callback: Callable) -> void:
	if not is_connected:
		error_occurred.emit("Wallet not connected")
		callback.call(0)
		return
	
	var headers = []
	if api_key != "":
		headers.append("Authorization: Bearer " + api_key)
	
	var url = base_url + "/nfts/count/" + contract_address + "/" + wallet_address
	http_client.request(url, headers, HTTPClient.METHOD_GET)

## Check if wallet owns a specific NFT
func has_nft(contract_address: String, token_id: String, callback: Callable) -> void:
	if not is_connected:
		error_occurred.emit("Wallet not connected")
		callback.call(false)
		return
	
	var headers = []
	if api_key != "":
		headers.append("Authorization: Bearer " + api_key)
	
	var url = base_url + "/nfts/owner/" + contract_address + "/" + token_id
	http_client.request(url, headers, HTTPClient.METHOD_GET)

## Get owned NFTs for a contract
func get_owned_nfts(contract_address: String, callback: Callable) -> void:
	if not is_connected:
		error_occurred.emit("Wallet not connected")
		callback.call([])
		return
	
	var headers = []
	if api_key != "":
		headers.append("Authorization: Bearer " + api_key)
	
	var url = base_url + "/nfts/user/" + wallet_address + "?contract=" + contract_address
	http_client.request(url, headers, HTTPClient.METHOD_GET)

## Get NFT metadata
func get_nft_metadata(contract_address: String, token_id: String, callback: Callable) -> void:
	var headers = []
	if api_key != "":
		headers.append("Authorization: Bearer " + api_key)
	
	var url = base_url + "/nfts/" + contract_address + "/" + token_id
	http_client.request(url, headers, HTTPClient.METHOD_GET)

## Get token price from ShahSwap
func get_token_price(token_symbol: String, callback: Callable) -> void:
	var headers = []
	if api_key != "":
		headers.append("Authorization: Bearer " + api_key)
	
	var url = base_url + "/shahswap/price/" + token_symbol
	http_client.request(url, headers, HTTPClient.METHOD_GET)

## Check if player can afford an item
func can_afford_item(token_symbol: String, amount: float, callback: Callable) -> void:
	if not is_connected:
		callback.call(false)
		return
	
	if token_symbol.to_upper() == "SHAH":
		get_balance(func(balance):
			callback.call(balance >= amount)
		)
	else:
		# For tokens, you would need the contract address
		# This is a simplified version
		callback.call(false)

## Get swap quote from ShahSwap
func get_swap_quote(input_token: String, output_token: String, amount: float, callback: Callable) -> void:
	var headers = ["Content-Type: application/json"]
	if api_key != "":
		headers.append("Authorization: Bearer " + api_key)
	
	var data = {
		"inputToken": input_token,
		"outputToken": output_token,
		"amount": str(amount)
	}
	
	var json = JSON.stringify(data)
	var url = base_url + "/shahswap/quote"
	
	http_client.request(url, headers, HTTPClient.METHOD_POST, json)

## Handle HTTP request completion
func _on_request_completed(result: int, response_code: int, headers: PackedStringArray, body: PackedByteArray) -> void:
	if result != HTTPRequest.RESULT_SUCCESS:
		error_occurred.emit("Request failed: " + str(result))
		return
	
	var response_text = body.get_string_from_utf8()
	var json = JSON.new()
	var parse_result = json.parse(response_text)
	
	if parse_result != OK:
		error_occurred.emit("Failed to parse response")
		return
	
	var response_data = json.data
	
	# Handle different response types based on URL or response structure
	# This is a simplified version - in a real implementation, you'd track request types
	if response_data.has("success"):
		if response_data.success:
			if response_data.has("data"):
				var data = response_data.data
				
				# Handle wallet connection
				if data.has("address"):
					is_connected = true
					wallet_address = data.address
					wallet_connected.emit(wallet_address)
				
				# Handle balance response
				if data.has("balance"):
					var balance = float(data.balance)
					balance_changed.emit(balance)
				
				# Handle NFT count response
				if data.has("count"):
					var count = int(data.count)
				
				# Handle NFT owner response
				if data.has("owner"):
					var owner = data.owner
					var has_nft = owner.to_lower() == wallet_address.to_lower()
				
				# Handle NFT list response
				if data.has("nfts"):
					var nfts = data.nfts
				
				# Handle price response
				if data.has("price"):
					var price = float(data.price)
				
				# Handle swap quote response
				if data.has("inputToken") and data.has("outputToken"):
					var quote = SwapQuote.new()
					quote.input_token = data.inputToken
					quote.output_token = data.outputToken
					quote.input_amount = float(data.inputAmount)
					quote.output_amount = float(data.outputAmount)
					quote.price_impact = float(data.priceImpact)
					quote.fee = float(data.fee)
		else:
			var error_msg = response_data.get("error", "Unknown error")
			error_occurred.emit(error_msg)

## SwapQuote class for Godot
class SwapQuote:
	var input_token: String
	var output_token: String
	var input_amount: float
	var output_amount: float
	var price_impact: float
	var fee: float

## NFTInfo class for Godot
class NFTInfo:
	var token_id: String
	var name: String
	var description: String
	var image_url: String
	var attributes: Dictionary

## NFTMetadata class for Godot
class NFTMetadata:
	var name: String
	var description: String
	var image: String
	var attributes: Dictionary
