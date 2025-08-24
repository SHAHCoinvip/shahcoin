import { SHI20Builder, SHI30Builder, ContractCreator } from '../smart-contract-creator/index.js';

const creator = new ContractCreator();

const $ = (id) => document.getElementById(id);

$('btnGen20').onclick = () => {
	const token = new SHI20Builder(creator).createToken({
		name: $('tName').value || 'My Token',
		symbol: $('tSymbol').value || 'MTK',
		totalSupply: $('tSupply').value || '1000000000000000000000000',
		features: ['mintable', 'burnable']
	});
	$('out20').textContent = token.sourceCode;
};

$('btnGen30').onclick = () => {
	const nft = new SHI30Builder(creator).createNFT({
		name: $('nName').value || 'My NFT',
		symbol: $('nSymbol').value || 'MNFT',
		baseURI: $('nBase').value || 'https://example.com/metadata/'
	});
	$('out30').textContent = nft.sourceCode;
};
