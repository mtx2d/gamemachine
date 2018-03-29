﻿#ifndef __GMPRIMITIVECREATOR_H__
#define __GMPRIMITIVECREATOR_H__
#include <gmcommon.h>
#include <gmmodel.h>
BEGIN_NS

class GMModel;
class GMShader;

GM_INTERFACE(IPrimitiveCreatorShaderCallback)
{
	virtual void onCreateShader(GMShader& shader) = 0;
};

struct GMPrimitiveCreator
{
	enum GMCreateAnchor
	{
		TopLeft,
		Center,
	};

	static GMfloat* unitExtents();
	static GMfloat* origin();

	static void createCube(GMfloat extents[3], OUT GMModel** obj, IPrimitiveCreatorShaderCallback* shaderCallback = nullptr, GMModelType type = GMModelType::Model3D);
	static void createQuad(GMfloat extents[3], GMfloat position[3], OUT GMModel** obj, IPrimitiveCreatorShaderCallback* shaderCallback = nullptr, GMModelType type = GMModelType::Model3D, GMCreateAnchor anchor = Center, GMfloat (*customUV)[12] = nullptr);
	static void createQuad3D(GMfloat extents[3], GMfloat position[12], OUT GMModel** obj, IPrimitiveCreatorShaderCallback* shaderCallback = nullptr, GMModelType type = GMModelType::Model3D, GMfloat(*customUV)[8] = nullptr);
};

struct GMPrimitiveUtil
{
	static void translateModelTo(REF GMModel& model, const GMfloat(&trans)[3]);
	static void scaleModel(REF GMModel& model, const GMfloat(&scaling)[3]);
};

//! 引擎提供的各种繁琐操作的工具类。
/*!
  此工具类其实是对一些固有流程的方法调用进行了封装，和单独调用那些方法效果一样。
*/
struct GMToolUtil
{
	//! 创建一个纹理，它来源于某路径。
	/*!
	  此方法封装了纹理读取和添加纹理等方法。请在调用前确认GMGamePackage已经初始化。
	  \param filename 需要读取纹理的路径。它是一个相对于纹理目录的相对路径。
	  \param texture 得到的纹理将通过此指针返回。
	  \sa GMGamePackage
	*/
	static void createTexture(const GMString& filename, ITexture** texture);

	//! 将一个纹理添加到一个模型中。
	/*!
	  此方法会将模型添加到纹理动画列表的第1帧中。
	  \param shader 目标模型的着色器设置。
	  \param texture 待添加纹理。
	  \param type 纹理类型。
	  \param index 需要添加到第几个纹理。每种类型的纹理有纹理数量的限制。
	*/
	static void addTextureToShader(gm::GMShader& shader, ITexture* texture, GMTextureType type, GMuint index = 0);

	//! 创建一个光标图形。
	/*!
	  用户应该先用工厂类创建一个ICursor的实例，然后调用此方法为光标设置一个图形。请在调用前确认GMGamePackage已经初始化。
	  \param cursor 传入的已经创建好的空光标实例。
	  \param cursorDesc 光标的描述。
	  \param filename 需要读取光标的路径。它是一个相对于纹理目录的相对路径。
	  \sa GMCursorDesc
	*/
	static void createCursor(ICursor* cursor, const GMCursorDesc& cursorDesc, const GMString& filename);
};

END_NS
#endif