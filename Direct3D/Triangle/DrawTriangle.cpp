#include "DrawTriangle.h"
#include <d3dcompiler.h>
#pragma comment(lib,"d3dcompiler.lib")

void DrawTriangle::Initizlize(HINSTANCE hInstance, int widht, int heigth)
{
	D3DFramework::Initizlize(hInstance, widht, heigth);

	IniTriangle();
	InitPipeline();
}

void DrawTriangle::Destory()
{
	mspInputLayout.Reset();
	mspPixelShader.Reset();
	mspVertexShader.Reset();
	mspVertexBuffer.Reset();

	D3DFramework::Destory();
}

void DrawTriangle::Render()
{
	UINT stride = sizeof(VERTEX);
	UINT offest = 0;

	mspDeviceContext->IASetVertexBuffers(0, 1, mspVertexBuffer.GetAddressOf(),
		&stride, &offest
	);

	mspDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mspDeviceContext->Draw(3,0);

}

void DrawTriangle::IniTriangle()
{
	VERTEX vertices[]
	{
		{0.0f,0.5f,0.0f,{1.0f,0.0f,0.0f,1.0f}},
		{0.45f,-0.5f,0.0f,{0.0f,1.0f,0.0f,1.0f}},
		{-0.45f,-0.5f,0.0f,{0.0f,0.0f,1.0f,1.0f}}
	};

	CD3D11_BUFFER_DESC bd(sizeof(VERTEX) * 3, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
	mspDevice->CreateBuffer(&bd, nullptr, mspVertexBuffer.ReleaseAndGetAddressOf());

	D3D11_MAPPED_SUBRESOURCE ms;
	mspDeviceContext->Map(mspVertexBuffer.Get(),0,D3D11_MAP_WRITE_DISCARD,0,&ms);
	
	memcpy(ms.pData, vertices, sizeof(vertices));
	
	mspDeviceContext->Unmap(mspVertexBuffer.Get(),0);
}

void DrawTriangle::InitPipeline()
{
	Microsoft::WRL::ComPtr<ID3DBlob> spVS;
	Microsoft::WRL::ComPtr<ID3DBlob> spPS;

	D3DCompileFromFile(L"VertexShader.hlsl", nullptr, nullptr, "main","vs_4_0_level_9_3",0,0,spVS.GetAddressOf(),nullptr);

	D3DCompileFromFile(L"PixelShader.hlsl", nullptr, nullptr, "main", "ps_4_0_level_9_3", 0, 0, spPS.GetAddressOf(), nullptr);


	mspDevice->CreateVertexShader(spVS->GetBufferPointer(), spVS->GetBufferSize(), nullptr,
		mspVertexShader.ReleaseAndGetAddressOf()
	);
	mspDevice->CreatePixelShader(spPS->GetBufferPointer(),spPS->GetBufferSize(),nullptr,mspPixelShader.ReleaseAndGetAddressOf());
	mspDeviceContext->VSSetShader(mspVertexShader.Get(),nullptr,0);
	mspDeviceContext->PSSetShader(mspPixelShader.Get(), nullptr, 0);

	D3D11_INPUT_ELEMENT_DESC ied[]{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},{"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0}
	};

	mspDevice->CreateInputLayout(ied, 2, spVS->GetBufferPointer(),
		spVS->GetBufferSize(), mspInputLayout.ReleaseAndGetAddressOf());
	mspDeviceContext->IASetInputLayout(mspInputLayout.Get());


}
