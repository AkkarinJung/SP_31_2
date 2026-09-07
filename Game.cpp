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

static LIGHT Light;
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

	//test2D.Init();
	//Field.Init();
	BumpField.Init();
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
	RGBShift_Model.Init();

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
		dir = XMVector3Normalize(dir);
		XMStoreFloat4(&Light.Direction, dir);

		Light.Position = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);
		Light.Diffuse = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
		Light.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);

		Light.PointLightParam = XMFLOAT4(2000.0f, 1.5f, 0.0f, 0.0f);
	}

}

//===============================================
//ゲームシーン終了
void FinalizeGame()
{
	FinalizeCamera();
	//test2D.Finalize();
	//Field.Finalize(); 
	BumpField.Finalize();
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
	RGBShift_Model.Finalize();

	TextureFinalize();
}

//===============================================
//ゲームシーン更新
void UpdateGame()
{

	if (GetPause() == false)//ポーズ中でなければ更新実行
	{
		UpdateCamera();
		//test2D.Update();
		//Field.Update();
		BumpField.Update();
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
		RGBShift_Model.Update();
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
	BeginPe();
	//3D用マトリクス設定
	{
		SetDepthEnable(true);		//奥行き処理有効
		DrawCamera();
		SetLight(Light);

		//Field.Draw();
		BumpField.Draw();
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
	GenerateMipsPe();
	Clear();
	// 2D用マトリクス設定
	{
		SetWorldViewProjection2D();
		//test2D.Draw();
		//Mosaic_Model.Draw();
		RGBShift_Model.Draw();
	}
}
