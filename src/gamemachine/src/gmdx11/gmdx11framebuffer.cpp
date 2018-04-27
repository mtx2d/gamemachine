﻿#include "stdafx.h"
#include "gmdx11framebuffer.h"
#include "foundation/gamemachine.h"
#include <gmimagebuffer.h>
#include "gmengine/gmgraphicengine.h"
#include "gmdx11helper.h"

BEGIN_NS

GM_PRIVATE_OBJECT(GMDx11DefaultFramebuffers)
{
	GMComPtr<ID3D11RenderTargetView> defaultRenderTargetView;
};

class GMDx11DefaultFramebuffers : public GMDx11Framebuffers
{
	DECLARE_PRIVATE_AND_BASE(GMDx11DefaultFramebuffers, GMDx11Framebuffers);

public:
	GMDx11DefaultFramebuffers()
	{
		D(d);
		D_BASE(db, Base);
		GM.getGraphicEngine()->getInterface(GameMachineInterfaceID::D3D11DepthStencilView, (void**)&db->depthStencilView);
		GM_ASSERT(db->depthStencilView);
		GM.getGraphicEngine()->getInterface(GameMachineInterfaceID::D3D11DepthStencilTexture, (void**)&db->depthStencilTexture);
		GM_ASSERT(db->depthStencilTexture);
		GM.getGraphicEngine()->getInterface(GameMachineInterfaceID::D3D11RenderTargetView, (void**)&d->defaultRenderTargetView);
		GM_ASSERT(d->defaultRenderTargetView);
		db->renderTargetViews.push_back(d->defaultRenderTargetView);
	}

public:
	virtual bool init(const GMFramebuffersDesc& desc) override
	{
		return false;
	}

	virtual void addFramebuffer(AUTORELEASE IFramebuffer* framebuffer) override
	{
	}

	void bind()
	{
		D_BASE(d, Base);
		d->deviceContext->OMSetRenderTargets(d->renderTargetViews.size(), d->renderTargetViews.data(), d->depthStencilView);
	}

	virtual void unbind() override
	{
	}

	virtual GMuint count() override
	{
		return 1;
	}

	virtual IFramebuffer* getFramebuffer(GMuint) override
	{
		return nullptr;
	}

	virtual void clear() override
	{
	}
};

GM_PRIVATE_OBJECT(GMDx11FramebufferTexture)
{
	GMFramebufferDesc desc;
};

class GMDx11FramebufferTexture : public GMDx11Texture
{
	DECLARE_PRIVATE_AND_BASE(GMDx11FramebufferTexture, GMDx11Texture);

public:
	GMDx11FramebufferTexture(const GMFramebufferDesc& desc);

public:
	virtual void init() override;

public:
	ID3D11Resource* getTexture();
};

GMDx11FramebufferTexture::GMDx11FramebufferTexture(const GMFramebufferDesc& desc)
	: GMDx11Texture(nullptr)
{
	D(d);
	d->desc = desc;
}

void GMDx11FramebufferTexture::init()
{
	D(d);
	D_BASE(db, Base);
	DXGI_FORMAT format;
	if (d->desc.framebufferFormat == GMFramebufferFormat::R8G8B8A8_UNORM)
	{
		format = DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	else if (d->desc.framebufferFormat == GMFramebufferFormat::R32G32B32A32_FLOAT)
	{
		format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	}
	else
	{
		format = DXGI_FORMAT_R8G8B8A8_UNORM;
		GM_ASSERT(!"Unsupported format.");
		gm_error("Unsupported format.");
	}

	const GMGameMachineRunningStates& runningState = GM.getGameMachineRunningStates();
	GMComPtr<ID3D11Texture2D> texture;
	D3D11_TEXTURE2D_DESC texDesc = { 0 };
	texDesc.Width = d->desc.rect.width;
	texDesc.Height = d->desc.rect.height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = format;
	texDesc.SampleDesc.Count = runningState.sampleCount;
	texDesc.SampleDesc.Quality = runningState.sampleQuality;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	GM_DX_HR(db->device->CreateTexture2D(&texDesc, NULL, &texture));
	db->resource = texture;

	GM_ASSERT(db->resource);

	GM_DX_HR(db->device->CreateShaderResourceView(
		db->resource,
		NULL,
		&db->shaderResourceView
	));

	if (!db->samplerState)
	{
		// 创建采样器
		D3D11_SAMPLER_DESC desc = GMDx11Helper::GMGetDx11DefaultSamplerDesc();
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		GM_DX_HR(db->device->CreateSamplerState(&desc, &db->samplerState));
	}
}

ID3D11Resource* GMDx11FramebufferTexture::getTexture()
{
	D_BASE(db, Base);
	return db->resource;
}
END_NS

GMDx11Framebuffer::~GMDx11Framebuffer()
{
	D(d);
	GM_delete(d->renderTexture);
}

bool GMDx11Framebuffer::init(const GMFramebufferDesc& desc)
{
	D(d);
	GMComPtr<ID3D11Device> device;
	GM.getGraphicEngine()->getInterface(GameMachineInterfaceID::D3D11Device, (void**)&device);
	GM_ASSERT(device);
	GMComPtr<ID3D11Texture2D> depthStencilBuffer;

	GM_ASSERT(!d->renderTexture);
	GMDx11FramebufferTexture* renderTexture = new GMDx11FramebufferTexture(desc);
	d->renderTexture = renderTexture;
	d->renderTexture->init();

	if (!d->name.isEmpty())
		GM_DX11_SET_OBJECT_NAME_A(renderTexture->getTexture(), d->name.toStdString().c_str());
	GM_DX_HR_RET(device->CreateRenderTargetView(renderTexture->getTexture(), NULL, &d->renderTargetView));

	return true;
}

void GMDx11Framebuffer::setName(const GMString& name)
{
	D(d);
	d->name = name;
	GM_ASSERT(!d->renderTexture && "Please call setName before init.");
}

ITexture* GMDx11Framebuffer::getTexture()
{
	D(d);
	GM_ASSERT(d->renderTexture);
	return d->renderTexture;
}

GMDx11Framebuffers::GMDx11Framebuffers()
{
	D(d);
	GM.getGraphicEngine()->getInterface(GameMachineInterfaceID::D3D11DeviceContext, (void**)&d->deviceContext);
	GM_ASSERT(d->deviceContext);
	d->engine = gm_cast<GMGraphicEngine*>(GM.getGraphicEngine());
}

GMDx11Framebuffers::~GMDx11Framebuffers()
{
	D(d);
	for (auto& framebuffer : d->framebuffers)
	{
		GM_delete(framebuffer);
	}
}

bool GMDx11Framebuffers::init(const GMFramebuffersDesc& desc)
{
	D(d);
	GMComPtr<ID3D11Device> device;
	GM.getGraphicEngine()->getInterface(GameMachineInterfaceID::D3D11Device, (void**)&device);
	GM_ASSERT(device);

	const GMGameMachineRunningStates& runningState = GM.getGameMachineRunningStates();
	GMComPtr<ID3D11DepthStencilState> depthStencilState;

	// 创建深度缓存模板
	D3D11_TEXTURE2D_DESC depthStencilTextureDesc = { 0 };
	depthStencilTextureDesc.Width = desc.rect.width;
	depthStencilTextureDesc.Height = desc.rect.height;
	depthStencilTextureDesc.MipLevels = 1;
	depthStencilTextureDesc.ArraySize = 1;
	depthStencilTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilTextureDesc.SampleDesc.Count = runningState.sampleCount;
	depthStencilTextureDesc.SampleDesc.Quality = runningState.sampleQuality;
	depthStencilTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilTextureDesc.CPUAccessFlags = 0;
	depthStencilTextureDesc.MiscFlags = 0;

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = { 0 };
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = FALSE;
	depthStencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	GM_DX_HR_RET(device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState));
	GM_DX_HR_RET(device->CreateTexture2D(&depthStencilTextureDesc, NULL, &d->depthStencilTexture));
	GM_DX_HR_RET(device->CreateDepthStencilView(d->depthStencilTexture, NULL, &d->depthStencilView));
	return true;
}

void GMDx11Framebuffers::addFramebuffer(AUTORELEASE IFramebuffer* framebuffer)
{
	D(d);
	GM_ASSERT(dynamic_cast<GMDx11Framebuffer*>(framebuffer));
	GMDx11Framebuffer* dxFramebuffer = static_cast<GMDx11Framebuffer*>(framebuffer);
	d->framebuffers.push_back(dxFramebuffer);
	d->renderTargetViews.push_back(dxFramebuffer->getRenderTargetView());
}

void GMDx11Framebuffers::bind()
{
	D(d);
	d->deviceContext->OMSetRenderTargets(d->renderTargetViews.size(), d->renderTargetViews.data(), d->depthStencilView);
	d->engine->getFramebuffersStack().push(this);
}

void GMDx11Framebuffers::unbind()
{
	D(d);
	GMFramebuffersStack& stack = d->engine->getFramebuffersStack();
	IFramebuffers* currentFramebuffers = stack.pop();
	if (currentFramebuffers != this)
	{
		GM_ASSERT(false);
		gm_error("Cannot unbind framebuffer because current framebuffer isn't this framebuffer.");
	}
	else
	{
		IFramebuffers* lastFramebuffers = stack.peek();
		if (lastFramebuffers)
			lastFramebuffers->bind();
		else
			getDefaultFramebuffers()->bind();
	}
}

void GMDx11Framebuffers::clear()
{
	static constexpr GMfloat clear[4] = { 0, 0, 0, 1 };
	D(d);
	for (auto renderTargetView : d->renderTargetViews)
	{
		d->deviceContext->ClearRenderTargetView(renderTargetView, clear);
	}
	d->deviceContext->ClearDepthStencilView(d->depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0U);
}

IFramebuffer* GMDx11Framebuffers::getFramebuffer(GMuint index)
{
	D(d);
	GM_ASSERT(index < d->framebuffers.size());
	return d->framebuffers[index];
}

GMuint GMDx11Framebuffers::count()
{
	D(d);
	return d->framebuffers.size();
}

void GMDx11Framebuffers::copyDepthStencilFramebuffer(IFramebuffers* dest)
{
	GMDx11Framebuffers* destFramebuffers = gm_cast<GMDx11Framebuffers*>(dest);
	D(d);
	D_OF(d_dest, destFramebuffers);
	d->deviceContext->CopyResource(d_dest->depthStencilTexture, d->depthStencilTexture);
}

IFramebuffers* GMDx11Framebuffers::getDefaultFramebuffers()
{
	static GMDx11DefaultFramebuffers s_defaultFramebuffers;
	return &s_defaultFramebuffers;
}