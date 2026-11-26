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
#include	"ShadowDiorama.h"

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

ShadowDiorama       Diorama;    //課題07 ジオラマ「影の町の一日」

static LIGHT Light;
float g_LightNear = 1.0f;
float g_LightFar = 10.0f;

//ジオラマ用
static float    g_Time = 0.0f;          //一日の中の時刻（0～LOOP_TIME秒）
static float    g_TimeSpeed = 1.0f;     //時間の進む速さ
static bool     g_AutoPlay = true;      //時間を自動で進める（Pキーで切り替え）
static bool     g_AutoCamera = true;    //カメラ自動演出（OFFでA/Dキー操作）
static bool     g_ManualLight = false;  //ライト位置をスライダーで操作
static bool     g_AutoNearFar = true;   //ライトのNear/Farを自動調整
static bool     g_ShowDepthMap = false; //シャドウマップを左上に表示
static bool     g_ShowUI = true;        //ImGui表示（F1キーで切り替え、録画時はOFF）
static XMFLOAT3 g_SkyColor(0.5f, 0.7f, 0.95f);
static XMFLOAT3 g_ShadowTarget(0.0f, 0.3f, 0.2f);  //昼のライトカメラの注視点（ジオラマの中心）
static XMFLOAT3 g_TheatreTarget(0.0f, 0.75f, 1.4f); //夜のライトカメラの注視点（奥の壁＝スクリーン）
static float    g_Night = 0.0f;                     //夜の割合（0=昼, 1=夜の影絵劇場）

static float SmoothStepF(float e0, float e1, float x)
{
	float t = (x - e0) / (e1 - e0);
	t = (t < 0.0f) ? 0.0f : ((t > 1.0f) ? 1.0f : t);
	return t * t * (3.0f - 2.0f * t);
}
static float LerpF(float a, float b, float t) { return a + (b - a) * t; }
static XMFLOAT3 LerpF3(const XMFLOAT3& a, const XMFLOAT3& b, float t)
{
	return XMFLOAT3(LerpF(a.x, b.x, t), LerpF(a.y, b.y, t), LerpF(a.z, b.z, t));
}

//夜の割合（0=昼, 1=夜）
static float NightRate(float t)
{
	return ShadowDiorama::NightRate(t);   //38～43秒で夜へ、86～90秒で朝へ
}

//===============================================
//時刻からライト（太陽・ランタン）を決める
//  0～40秒 : 太陽が東(+X)から西(-X)へ弧を描いて動く（20秒で真昼）
//  40～86秒: 影絵劇場。ランタンが奥の壁をスクリーンのように照らす
//            （位置は ShadowDiorama::NightLightPosition。隠し絵はこの位置から見たときだけ完成する）
static void UpdateSceneLight(float t)
{
	//太陽（86秒以降は次の朝の日の出へつなぐため t-90 で計算）
	float sunT = (t > 80.0f) ? t - ShadowDiorama::LOOP_TIME : t;
	float theta = XMConvertToRadians(8.0f + 164.0f * (sunT / 40.0f));
	XMFLOAT3 sun(7.0f * cosf(theta), 7.0f * sinf(theta), -1.2f);
	float e = sinf(theta);                         //太陽の高さ
	float k = SmoothStepF(0.15f, 0.75f, e);        //0=地平線付近, 1=真昼
	bool  morning = (sunT < 20.0f);

	XMFLOAT3 horizon = morning ? XMFLOAT3(1.0f, 0.6f, 0.35f) : XMFLOAT3(1.0f, 0.42f, 0.25f);
	XMFLOAT3 noon(1.0f, 0.96f, 0.88f);
	XMFLOAT3 sunDiffuse = LerpF3(horizon, noon, k);
	float    power = 0.75f + 0.35f * k;
	sunDiffuse = XMFLOAT3(sunDiffuse.x * power, sunDiffuse.y * power, sunDiffuse.z * power);
	XMFLOAT3 sunAmbient = LerpF3(XMFLOAT3(0.22f, 0.17f, 0.2f), XMFLOAT3(0.32f, 0.34f, 0.4f), k);
	XMFLOAT3 skyLow = morning ? XMFLOAT3(0.95f, 0.6f, 0.45f) : XMFLOAT3(0.85f, 0.4f, 0.35f);
	XMFLOAT3 sunSky = LerpF3(skyLow, XMFLOAT3(0.45f, 0.68f, 0.95f), k);

	//影絵劇場のランタン
	XMFLOAT3 moon = ShadowDiorama::NightLightPosition(t);
	XMFLOAT3 moonDiffuse(0.38f, 0.46f, 0.8f);
	XMFLOAT3 moonAmbient(0.06f, 0.07f, 0.14f);
	XMFLOAT3 moonSky(0.03f, 0.04f, 0.1f);

	float n = NightRate(t);
	g_Night = n;
	XMFLOAT3 pos = LerpF3(sun, moon, n);
	XMFLOAT3 dif = LerpF3(sunDiffuse, moonDiffuse, n);
	XMFLOAT3 amb = LerpF3(sunAmbient, moonAmbient, n);
	g_SkyColor = LerpF3(sunSky, moonSky, n);

	Light.Enable = TRUE;
	Light.Position = XMFLOAT4(pos.x, pos.y, pos.z, 1.0f);
	Light.Diffuse = XMFLOAT4(dif.x, dif.y, dif.z, 1.0f);
	Light.Ambient = XMFLOAT4(amb.x, amb.y, amb.z, 1.0f);
	//元のDepthShadowPSはライトの色を使わず、明るさ = ハーフランバート × 減衰(1 - 距離 / PointLightParam.x)。
	//そこで減衰距離を変えて、朝夕と夜を暗くする
	//昼 : farBright = ジオラマの一番奥（ライトから 距離+3.6）での明るさ
	//夜 : スクリーン（ランタンから約4.6）の明るさを0.7にして、影とのコントラストを出す
	float farBright = LerpF(0.55f, 0.95f, k);
	float dx = sun.x - g_ShadowTarget.x, dy = sun.y - g_ShadowTarget.y, dz = sun.z - g_ShadowTarget.z;
	float dist = sqrtf(dx * dx + dy * dy + dz * dz);
	float dayRange = (dist + 3.6f) / (1.0f - farBright);
	float nightRange = 4.6f / (1.0f - 0.7f);
	float range = LerpF(dayRange, nightRange, n);
	Light.PointLightParam = XMFLOAT4(range, 5.0f, 0.0f, 0.5f);
}

//===============================================
//時刻からカメラを決める
//  普段 : 手前側をゆっくり左右に回り込む
//  真昼 : 上から見下ろして五輪マークの影を見せる
//  夜   : 客席（ランタンの後ろ）から影絵のスクリーンを見る。
//         少し左右に動くので、隠し絵のブロックがバラバラなのに影だけ絵になるのが分かる
static void UpdateSceneCamera(float t)
{
	Camera* cam = GetCamera();
	float n = NightRate(t);

	float az = XMConvertToRadians(-90.0f + 40.0f * sinf(XM_2PI * t / 45.0f));
	XMFLOAT3 orbitPos(4.3f * cosf(az), 2.3f, 4.3f * sinf(az));
	XMFLOAT3 orbitAt(0.0f, 0.35f, 0.4f);

	float top = SmoothStepF(13.0f, 17.0f, t) * (1.0f - SmoothStepF(23.0f, 27.0f, t));
	XMFLOAT3 topPos(0.0f, 4.6f, -2.6f);
	XMFLOAT3 topAt(0.0f, 0.2f, 0.3f);

	XMFLOAT3 dayPos = LerpF3(orbitPos, topPos, top);
	XMFLOAT3 dayAt = LerpF3(orbitAt, topAt, top);

	XMFLOAT3 nightPos(1.5f * sinf(XM_2PI * (t - 40.0f) / 24.0f), 2.0f, -3.9f);
	XMFLOAT3 nightAt(0.0f, 0.8f, 1.3f);

	cam->Position = LerpF3(dayPos, nightPos, n);
	cam->AtPosition = LerpF3(dayAt, nightAt, n);
	cam->UpVector = XMFLOAT3(0.0f, 1.0f, 0.0f);
	cam->Nearclip = 0.1f;
}

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
	//DSModel.Init();   //課題07ではジオラマ(ShadowDiorama)に置き換え
	DSField.Init();
	Diorama.Init();

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
	DSField.Finalize();
	//DSModel.Finalize();
	Diorama.Finalize();

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

	//キー操作
	if (Keyboard_IsKeyDownTrigger(KK_F1)) g_ShowUI = !g_ShowUI;       //UI表示切り替え（録画時は消す）
	if (Keyboard_IsKeyDownTrigger(KK_P))  g_AutoPlay = !g_AutoPlay;   //時間の再生/停止

	if (g_ShowUI)
	{
		ImGui::Begin("Shadow Diorama");
		{
			ImGui::Text("F1: hide UI   P: play / pause");
			ImGui::SliderFloat("Time", &g_Time, 0.0f, ShadowDiorama::LOOP_TIME, "%.1f s");
			ImGui::SliderFloat("Speed", &g_TimeSpeed, 0.0f, 4.0f, "%.2f");
			ImGui::Checkbox("Auto Play", &g_AutoPlay);
			ImGui::Checkbox("Auto Camera (off: A/D keys)", &g_AutoCamera);
			ImGui::Checkbox("Manual Light", &g_ManualLight);
			if (g_ManualLight)
			{
				ImGui::SliderFloat("LightPositionX", &Light.Position.x, -10.0f, 10.0f);
				ImGui::SliderFloat("LightPositionY", &Light.Position.y, 0.5f, 20.0f);
				ImGui::SliderFloat("LightPositionZ", &Light.Position.z, -10.0f, 10.0f);
			}
			ImGui::Checkbox("Auto Near/Far", &g_AutoNearFar);
			if (!g_AutoNearFar)
			{
				ImGui::SliderFloat("Near", &g_LightNear, 0.1f, 10.0f);
				ImGui::SliderFloat("Far", &g_LightFar, 1.0f, 50.0f);
				if (g_LightFar < g_LightNear + 0.1f) g_LightFar = g_LightNear + 0.1f; //FARはNEARより奥
			}
			ImGui::Checkbox("Show Shadow Map", &g_ShowDepthMap);
			ImGui::SliderFloat("Cube Pivot Z", &Diorama.CubePivotZ, -1.0f, 1.0f, "%.2f");
		}
		ImGui::End();
	}

	if (GetPause() == false)//ポーズ中でなければ更新実行
	{
		//時間を進める
		if (g_AutoPlay)
		{
			g_Time += g_TimeSpeed / 60.0f;
			if (g_Time >= ShadowDiorama::LOOP_TIME) g_Time -= ShadowDiorama::LOOP_TIME;
		}

		if (g_AutoCamera) UpdateSceneCamera(g_Time);
		else              UpdateCamera();
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
		//DSModel.Update();
		DSField.Update();
	}

	//ライトとオブジェクトの動きは時刻から決める（ポーズ中でもTimeスライダーで確認できる）
	if (!g_ManualLight)
	{
		UpdateSceneLight(g_Time);
	}
	Diorama.Update(g_Time);

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
	SetClearColor(g_SkyColor.x, g_SkyColor.y, g_SkyColor.z);   //空の色

	//ライトカメラ行列作成
	XMVECTOR lpos = XMVectorSet(Light.Position.x, Light.Position.y, Light.Position.z, 0.0f);
	XMFLOAT3 target = LerpF3(g_ShadowTarget, g_TheatreTarget, g_Night);
	XMVECTOR lat = XMLoadFloat3(&target);
	XMVECTOR ldir = XMVector3Normalize(XMVectorSubtract(lat, lpos));
	//真上から照らすときは上方向ベクトルが視線と平行になるのでZ方向を使う
	XMVECTOR lup = (fabsf(XMVectorGetY(ldir)) > 0.98f) ? XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f)
	                                                   : XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	Light.ViewMatrix = XMMatrixLookAtLH(lpos, lat, lup);
	SetViewMatrix(Light.ViewMatrix);                         //セット時に転置される
	Light.ViewMatrix = XMMatrixTranspose(Light.ViewMatrix);  //転置しておく

	//ライトカメラのプロジェクション行列
	//Auto時
	//  昼 : ジオラマ全体（半径約3.6）がちょうど入るように画角とNear/Farを決める
	//  夜 : ランタンから奥の壁（スクリーン）全体が入るようにする
	//（範囲を狭くするほど深度の精度が上がり、影がきれいになる）
	float lightFov = 45.0f;
	if (g_AutoNearFar)
	{
		const float sceneRadius = 3.6f;
		float dist = XMVectorGetX(XMVector3Length(XMVectorSubtract(lat, lpos)));
		if (dist < sceneRadius + 0.5f) dist = sceneRadius + 0.5f;
		float dayFov = XMConvertToDegrees(2.0f * asinf(sceneRadius / dist));
		float dayNear = (dist - sceneRadius > 0.3f) ? dist - sceneRadius : 0.3f;
		float dayFar = dist + sceneRadius;

		lightFov = LerpF(dayFov, 70.0f, g_Night);
		g_LightNear = LerpF(dayNear, 1.0f, g_Night);
		g_LightFar = LerpF(dayFar, 6.5f, g_Night);
	}
	Light.ProjectionMatrix = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(lightFov), (float)SCREEN_WIDTH / SCREEN_HEIGHT,
		g_LightNear, g_LightFar);
	SetProjectionMatrix(Light.ProjectionMatrix);                        //セット時に転置される
	Light.ProjectionMatrix = XMMatrixTranspose(Light.ProjectionMatrix); //転置しておく

	SetLight(Light);

	BeginDepth();   //デプスバッファをレンダリングターゲットにする
	{
		SetDepthEnable(true);
		Diorama.Draw(true); //ジオラマ深度表示
		DSField.Draw();     //地面深度表示
	}
	Clear();        //レンダリングターゲットをデフォルトに戻す
	{
		DrawCamera();
		Diorama.Draw(false); //普通にジオラマ表示
		DSField.Draw();      //普通に地面表示
	}
	if (g_ShowDepthMap)
	{
		SetWorldViewProjection2D();
		test2D.Draw();       //シャドウマップ確認用
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
