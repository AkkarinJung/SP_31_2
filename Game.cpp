#include "Main.h"
#include "Renderer.h"
#include "Manager.h"
#include "Sprite.h"
#include "Game.h"
#include "keyboard.h"
#include "Camera.h"
#include "texture.h"

#include	"Sprite2D.h"
#include	"Field3D.h"
#include	"PolygonModel.h"
#include	"VertexDirectionalLighting.h"
#include	"PixelDirectionalLighting.h"
#include	"PixelLightingBlinnPhong.h"
#include	"HemiSphereLighting.h"
#include	"PointPixelLighting.h"
#include	"LimLighting.h"
#include	"SpotLighting.h"
#include	"BumpField3D.h"
#include	"CookTorrance.h"
#include	"DisneyPBR.h"
#include	"Toon_1.h"
#include	"Toon_2.h"
#include	"Toon_3.h"

#include	"Mosaic.h"
#include	"RGBShift.h"
#include	"Horror.h"
#include	"posterise.h"
#include	"Gaussian.h"
#include	"Bloom.h"

#include	"DoF.h"

#include	"ProjectionShadow.h"
#include	"ProjectionShadowField3D.h"
#include	"DepthShadow.h"
#include	"DepthShadowField3D.h" 
#include	"GBufferField3D.h"
#include	"GBufferPolygon.h"
#include	"DeferredSprite2D.h"

//1 : Gバッファ（デファードレンダリング）表示 / 0 : 従来のデプスシャドウ表示
#define	USE_GBUFFER	(1)

//===============================================
//グローバル変数
 
Camera		 CameraObject;
Sprite2D	 test2D;
//Field3D		 Field;
BumpField3D  BumpField;
PolygonModel Model;
VertexDirectionalLighting VDLModel;
PixelDirectionalLighting PDLModel;
PixelLightingBlinnPhong PLBPModel;
HemiSphereLighting HSLModel;
PointPixelLighting PPLModel;
LimLighting LLModel;
SpotLighting SLModel;
CookTorrance CTModel;
DisneyPBR DPBRModel;
Toon_1 Toon_1Model;
Toon_2 Toon_2Model;
Toon_3 Toon_3Model;

Mosaic Mosaic_Model;
RGBShift RGBShift_Model;
Horror Horror_Model;
posterise posterise_Model;
Gaussian Gaussian_Model;
Bloom Bloom_Model;
DoF DoFSprite;

ProjectionShadow PSModel;
ProjectionShadowField3D PSField;
DepthShadow         DSModel;
DepthShadowField3D  DSField;
GBufferField3D      GBufferField;
GBufferPolygon      GBufferModel;
DeferredSprite2D    DeferredSprite;

static LIGHT Light;
float g_LightNear = 1.0f;
float g_LightFar = 10.0f;

//ポーズフラグ
static	bool	pause = false;

//===============================================
//ポーズフラグセット
void	SetPause(bool flg)
{
	pause = flg;
}
//===============================================
//ポーズフラグ取得
bool	GetPause()
{
	return pause;
}

//===============================================
//ゲームシーン初期化
void InitGame()
{
	TextureInitialize(GetDevice());
	InitCamera();

	test2D.Init();
	//Field.Init();
	//BumpField.Init();
	//Model.Init();
	//VDLModel.Init();
	//PDLModel.Init();
	//PLBPModel.Init();
	//HSLModel.Init();
	//PPLModel.Init();
	//LLModel.Init();
	//SLModel.Init();
	//CTModel.Init();
	//DPBRModel.Init();
	//Toon_1Model.Init();
	//Toon_2Model.Init();
	//Toon_3Model.Init();
	//Mosaic_Model.Init();
	//RGBShift_Model.Init();
	//Horror_Model.Init
	//posterise_Model.Init();
	//Gaussian_Model.Init();
	//Bloom_Model.Init();
	//DoFSprite.Init();
	//PSModel.Init();
	//PSField.Init();
#if USE_GBUFFER
	GBufferField.Init();
	GBufferModel.Init();
	DeferredSprite.Init();
#else
	DSModel.Init();
	DSField.Init();
#endif

	//XMVECTOR dir = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
	//dir = XMVector3Normalize(dir);
	//XMStoreFloat4(&Light.Direction, dir);

	//Light.Position = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	//Light.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//Light.Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);

	//Light.PointLightParam = XMFLOAT4(10.0f, 1.0f, 0.0f, 0.0f);
	//Light.Angle.x = XMConvertToRadians(30.0f);

	//dir = XMVector4Normalize(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	//XMStoreFloat4(&Light.GroundNormal, dir);

	//Light.SkyColor = XMFLOAT4(0.6f, 0.0f, 0.0f, 1.0f);
	//Light.GroundColor = XMFLOAT4(0.0f, 0.6f, 0.0f, 1.0f);

	{
		//disney

		XMVECTOR dir = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
		dir = XMVector4Normalize(dir);
		XMStoreFloat4(&Light.Direction, dir);

		Light.Position = XMFLOAT4(2.0f, 2.0f, -3.0f, 0.0f);
		Light.Diffuse = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
		Light.Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);

		Light.PointLightParam = XMFLOAT4(1000.0f, 5.0f, 0.0f, 0.5f);
	}

}

//===============================================
//ゲームシーン終了
void FinalizeGame()
{
	FinalizeCamera();
	test2D.Finalize();
	//Field.Finalize(); 
	//BumpField.Finalize();
	//Model.Finalize();
	//VDLModel.Finalize();
	//PDLModel.Finalize();
	//PLBPModel.Finalize();
	//HSLModel.Finalize();
	//PPLModel.Finalize();
	//LLModel.Finalize();
	//SLModel.Finalize();
	//CTModel.Finalize();
	//DPBRModel.Finalize();
	//Toon_1Model.Finalize();
	//Toon_2Model.Finalize();
	//Toon_3Model.Finalize();

	//Mosaic_Model.Finalize();
	//RGBShift_Model.Finalize();
	//Horror_Model.Finalize();
	//posterise_Model.Finalize();
	//Gaussian_Model.Finalize();
	//Bloom_Model.Finalize();
	//DoFSprite.Finalize();
	//PSModel.Finalize();
	//PSField.Finalize();
#if USE_GBUFFER
	DeferredSprite.Finalize();
	GBufferModel.Finalize();
	GBufferField.Finalize();
#else
	DSField.Finalize();
	DSModel.Finalize();
#endif

	TextureFinalize();
}

//===============================================
//ゲームシーン更新
void UpdateGame()
{
	//ImGui::Begin("ProjectionShadow");
	//{
	//	ImGui::SliderFloat("LightPositionX", &Light.Position.x, -3.0f, 3.0f, "%.4f");
	//	ImGui::SliderFloat("LightPositionY", &Light.Position.y, 1.0f, 5.0f, "%.4f");
	//	ImGui::SliderFloat("LightPositionZ", &Light.Position.z, -3.0f, 3.0f, "%.4f");
	//}
	//ImGui::End();

	//XMVECTOR   lpos = XMVectorSet(Light.Position.x, Light.Position.y, Light.Position.z, 0.0f);
	//XMVECTOR   lat = XMVectorSet(0.001f, 0.0f, 0.001f, 0.0f);
	//XMVECTOR   lup = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	//Light.ViewMatrix = XMMatrixLookAtLH(lpos, lat, lup);
	//Light.ProjectionMatrix = XMMatrixPerspectiveFovLH(
	//	XMConvertToRadians(45.0f), (float)SCREEN_WIDTH / SCREEN_HEIGHT, 1.0f, 1000.0f
	//);

#if !USE_GBUFFER
	ImGui::Begin("DepthShadow");                                                // ★
	{
		ImGui::SliderFloat("LightPositionX", &Light.Position.x, -10.0f, 10.0f); // ★
		ImGui::SliderFloat("LightPositionY", &Light.Position.y, 0.5f, 20.0f); // ★
		ImGui::SliderFloat("LightPositionZ", &Light.Position.z, -10.0f, 10.0f); // ★
		ImGui::SliderFloat("Near", &g_LightNear, 0.1f, 10.0f);                  // ★
		ImGui::SliderFloat("Far", &g_LightFar, 1.0f, 50.0f);                  // ★
		if (g_LightFar < g_LightNear + 0.1f) g_LightFar = g_LightNear + 0.1f;   // ★ FAR must be beyond NEAR
	}
	ImGui::End();
#endif

	if (GetPause() == false)//ポーズ中でなければ更新実行
	{
		UpdateCamera();
		//test2D.Update();
		//Field.Update();
		//BumpField.Update();
		//Model.Update();
		//VDLModel.Update();
		//PDLModel.Update();
		//PLBPModel.Update();
		//HSLModel.Update();
		//PPLModel.Update();
		//LLModel.Update();
		//SLModel.Update();
		//CTModel.Update();
		//DPBRModel.Update();
		//Toon_1Model.Update();
		//Toon_2Model.Update();
		//Toon_3Model.Update();

		//Mosaic_Model.Update();
		//RGBShift_Model.Update();
		//Horror_Model.Update();
		//posterise_Model.Update();
		//Gaussian_Model.Update();
		//Bloom_Model.Update();
		//DoFSprite.Update();
		//PSModel.Update();
		//PSField.Update();
#if USE_GBUFFER
		GBufferField.Update();
		GBufferModel.Update();
#else
		DSModel.Update();
		DSField.Update();
#endif
	}

	//ImGui::Begin("SPOT LIGHT");
	//{
	//	ImGui::SliderFloat("Diffuse-R", &Light.Diffuse.x, 0.0f, 1.0f, "%.1f");
	//	ImGui::SliderFloat("Diffuse-G", &Light.Diffuse.y, 0.0f, 1.0f, "%.1f");
	//	ImGui::SliderFloat("Diffuse-B", &Light.Diffuse.z, 0.0f, 1.0f, "%.1f");

	//	float angle = XMConvertToDegrees(Light.Angle.x); // 度に変換
	//	ImGui::SliderFloat("Cone Angle", &angle, 5.0f, 45.0f, "%.1f");
	//	Light.Angle.x = XMConvertToRadians(angle); // ラジアンに変換

	//	ImGui::SliderFloat("Attenuation", &Light.PointLightParam.x, 0.0f, 10.0f, "%.1f"); // 距離
	//	ImGui::SliderFloat("Pow", &Light.PointLightParam.y, 1.0f, 50.0f, "%0.2f"); // 調整用

	//	ImGui::SliderFloat("Position.x",
	//		&Light.Position.x, -2.0f, 2.0f, "%.1f");

	//	ImGui::SliderFloat("Position.y",
	//		&Light.Position.y, -2.0f, 2.0f, "%.1f");

	//	ImGui::SliderFloat("Position.z",
	//		&Light.Position.z, -2.0f, 2.0f, "%.1f");
	//}
	//ImGui::End();

}

//===============================================
//ゲームシーン描画
void DrawGame()
{
#if USE_GBUFFER
	BeginPe(0, 2);//レンダリングテクスチャ0と1の2つをレンダリングターゲットにする
	{//3D描画
		SetDepthEnable(true);
		DrawCamera();
		GBufferField.Draw();	//フィールド
		GBufferModel.Draw();	//モデル
	}
	Clear();//レンダリングターゲットをデフォルトに戻す
	{//2D描画
		SetWorldViewProjection2D();
		DeferredSprite.Draw();//DEBUGスプライト2枚表示
	}
	return;
#endif

	//ライトカメラ行列作成
	XMVECTOR lpos = XMVectorSet(Light.Position.x, Light.Position.y, Light.Position.z, 0.0f);
	XMVECTOR lat = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR lup = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	Light.ViewMatrix = XMMatrixLookAtLH(lpos, lat, lup);
	SetViewMatrix(Light.ViewMatrix);                         //セット時に転置される
	Light.ViewMatrix = XMMatrixTranspose(Light.ViewMatrix);  //転置しておく

	//ライトカメラのプロジェクション行列
	Light.ProjectionMatrix = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(45.0f), (float)SCREEN_WIDTH / SCREEN_HEIGHT,
		g_LightNear, g_LightFar);                                       // ★ was 1.0f, 10.0f
	SetProjectionMatrix(Light.ProjectionMatrix);                        //セット時に転置される
	Light.ProjectionMatrix = XMMatrixTranspose(Light.ProjectionMatrix); //転置しておく

	SetLight(Light);

	BeginDepth();   //デプスバッファをレンダリングターゲットにする
	{
		SetDepthEnable(true);
		DSModel.Draw(); //ドーナツ深度表示
		DSField.Draw(); //地面深度表示
	}
	Clear();        //レンダリングターゲットをデフォルトに戻す
	{
		DrawCamera();
		DSModel.Draw(); //普通にドーナツ表示
		DSField.Draw(); //普通に地面表示
	}
	{
		SetWorldViewProjection2D();
		test2D.Draw();
	}

	//BeginPe(0);
	//BeginDoF();
	//3D用マトリクス設定
	{
		//SetDepthEnable(true);		//奥行き処理有効
		//DrawCamera();
		////SetLight(Light);
		//SetViewMatrix(Light.ViewMatrix);
		//SetProjectionMatrix(Light.ProjectionMatrix);
		//PSModel.Draw(1);


		//Field.Draw();
		//BumpField.Draw();
		//Model.Draw();
		//VDLModel.Draw();
		//PDLModel.Draw();
		//PLBPModel.Draw();
		//HSLModel.Draw();
		//PPLModel.Draw();
		//LLModel.Draw();
		//SLModel.Draw();
		//CTModel.Draw();
		//DPBRModel.Draw();
		//Toon_1Model.Draw();
		//Toon_2Model.Draw();
		//Toon_3Model.Draw();
	}
	//BeginPe(1); //レンダリングテクスチャ1をレンダリングターゲットにする
	//{//2D描画
	//	SetWorldViewProjection2D();
	//	//Gaussian_Model.Draw(0); //横ブラーで描画
	//	//Bloom_Model.Draw(0);
	//}
	//BeginPe(0);
	//{
	//	SetWorldViewProjection2D();
	//	//test2D.Draw();
	//	//Mosaic_Model.Draw();
	//	//RGBShift_Model.Draw();
	//	//Horror_Mode//raw();
	//	//posterise_Model.Draw();
	//	//Gaussian_Model.Draw(1); //横ブラーで描画
	//	///Bloom_Model.Draw(1);
	//}
	//GenerateMipsPe(0);
	//Clear();
	{
		//DrawCamera();
		//SetLight(Light);
		//PSModel.Draw(0);
		//PSField.Draw();
	}
	{
		//SetWorldViewProjection2D();
		//DoFSprite.Draw();
		//test2D.Draw();
	}
}
