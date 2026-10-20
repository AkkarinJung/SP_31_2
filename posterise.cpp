/*==============================================================================

   ホラー演出 [posterise.cpp]
														 Author :
														 Date   :
--------------------------------------------------------------------------------

==============================================================================*/
#include "posterise.h"
#include "Camera.h"
#include "texture.h"
#include "sprite.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************

static float Lerp(float a, float b, float t) 
{ return a + (b - a) * t; }

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT posterise::Init(void)
{
	// テクスチャ読み込み
	//int Texture = TextureLoad(L"asset\\texture\\4.png");

	//シェーダー読み込み
	CreateVertexShader(&VertexShader, &VertexLayout, "poterizeVS.cso");
	CreatePixelShader(&PixelShader, "poterizePS.cso");

	//2Dオブジェクト初期化
	Position = XMFLOAT3(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0.0f);
	Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Scale = XMFLOAT2(1.0f, 1.0f);
	Size = XMFLOAT2(SCREEN_WIDTH, SCREEN_HEIGHT);
	Rotate = 0.0f;

	// テクスチャ読み込み（2.png → 3.png → 4.png）
	TexID[0] = TextureLoad(L"asset\\texture\\2.png");
	TexID[1] = TextureLoad(L"asset\\texture\\3.png");
	TexID[2] = TextureLoad(L"asset\\texture\\4.png");

	TexIndex = 0;
	Timer = 0;

	//Parameter = XMFLOAT4(64.0f, 0.2f, 0.7f, 1.0f);

	StartParameter = XMFLOAT4(1.0f, 0.916f, 0.588f, 1.0f);
	EndParameter = XMFLOAT4(64.0f, 0.2f, 0.3f, 1.0f); 
	Parameter = StartParameter;
	AutoParameter = true;

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void posterise::Finalize(void)
{
	VertexLayout->Release();
	VertexShader->Release();
	PixelShader->Release();

}

//=============================================================================
// 更新処理
//=============================================================================
void posterise::Update(void)
{
	float dt = 1.0f / 60.0f;
	// 2秒ごとに次の画像へ切り替え
	Timer += dt;
	if (Timer >= SWITCH_TIME)
	{
		Timer -= SWITCH_TIME;
		TexIndex++;

		if (TexIndex >= TEXTURE_NUM)
		{
			//TexIndex = TEXTURE_NUM - 1;  // 4.png で停止
			TexIndex = 0;              // ループさせたい場合はこっち
		}
	}

	if (AutoParameter)
	{
		float total = SWITCH_TIME * TEXTURE_NUM;
		float elapsed = SWITCH_TIME * TexIndex + Timer;

		float t = elapsed / total;
		if (t > 1.0f) t = 1.0f;
		if (t < 0.0f) t = 0.0f;

		Parameter.x = Lerp(StartParameter.x, EndParameter.x, t);
		Parameter.y = Lerp(StartParameter.y, EndParameter.y, t);
		Parameter.z = Lerp(StartParameter.z, EndParameter.z, t);
		Parameter.w = Lerp(StartParameter.w, EndParameter.w, t);
	}

	ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_FirstUseEver);
	ImGui::Begin("Posterize");
	{
		ImGui::SliderFloat("Gradation", &Parameter.x, 1.0f, 64.0f, "%.0f"); // 1~64段階
		ImGui::SliderFloat("MIN", &Parameter.y, 0.0f, 1.0f, "%.3f");
		ImGui::SliderFloat("MAX", &Parameter.z, 0.0f, 1.0f, "%.3f");
		ImGui::SliderFloat("POW", &Parameter.w, 1.0f, 30.0f, "%.0f");
		ImGui::Checkbox("Auto", &AutoParameter);
	}
	ImGui::End();
}

//=============================================================================
// 描画処理
//=============================================================================
void posterise::Draw(void)
{

	// 頂点レイアウト設定
	GetDeviceContext()->IASetInputLayout(VertexLayout);
	//頂点シェーダーをセット
	GetDeviceContext()->VSSetShader(VertexShader, NULL, 0);
	//ピクセルシェーダーをセット
	GetDeviceContext()->PSSetShader(PixelShader, NULL, 0);

	//奥行き処理をOFF
	SetDepthEnable(false);

	SetParameter(Parameter);

	// マテリアル設定
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	{//2Dポリゴン1枚ずつで必要な処理

		//テクスチャをセット（レンダリングテクスチャ＝直前に描画した3Dシーン）
		ID3D11ShaderResourceView* tex = GetTexture(TexID[TexIndex]);
		GetDeviceContext()->PSSetShaderResources(0, 1, &tex);

		//平行移動行列の作成（表示座標を決める）
		XMMATRIX	TranslationMatrix = XMMatrixTranslation(
			Position.x, Position.y, 0.0f);

		//回転行列（Z回転）行列の作成
		XMMATRIX	RotationMatrix = XMMatrixRotationZ(XMConvertToRadians(Rotate));

		//スケーリング行列作成（倍率1.0が等倍、0倍はダメ！）
		XMMATRIX	ScalingMatrix = XMMatrixScaling(Scale.x, Scale.y, 1.0f);

		//ワールド行列の作成（ポリゴンの表示の仕方を指定する最終的な行列
		XMMATRIX	WorldMatrix = ScalingMatrix * RotationMatrix * TranslationMatrix;

		//ワールド行列をDirectXへセット
		SetWorldMatrix(WorldMatrix);

		// ポリゴン描画
		DrawSprite(Size, Color);
	}

}
