/*==============================================================================

   ホラー演出 [Gaussian.cpp]
														 Author :
														 Date   :
--------------------------------------------------------------------------------

==============================================================================*/
#include "Gaussian.h"
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
{
	return a + (b - a) * t;
}

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT Gaussian::Init(void)
{
	// テクスチャ読み込み
	int Texture = TextureLoad(L"asset\\texture\\1.png");

	//シェーダー読み込み
	CreateVertexShader(&VertexShader, &VertexLayout, "gaussianVS.cso");
	CreatePixelShader(&PixelShader[0], "gaussianPS_H.cso");
	CreatePixelShader(&PixelShader[1], "gaussianPS_V.cso");


	//2Dオブジェクト初期化
	Position = XMFLOAT3(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0.0f);
	Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Scale = XMFLOAT2(1.0f, 1.0f);
	Size = XMFLOAT2(SCREEN_WIDTH, SCREEN_HEIGHT);
	Rotate = 0.0f;

	Parameter = XMFLOAT4(SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f);
	CalcGaussianWeight(0.0f);
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void Gaussian::Finalize(void)
{
	VertexLayout->Release();
	VertexShader->Release();
	PixelShader[0]->Release();
	PixelShader[1]->Release();

}

//=============================================================================
// 更新処理
//=============================================================================
void Gaussian::Update(void)
{

	ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_FirstUseEver);
	ImGui::Begin("Gaussian");
	{
		bool ret = ImGui::SliderFloat("Boke Str", &Parameter.z, 0.0f, 50.0f, "%.3f");
		if (ret == true)
		{
			CalcGaussianWeight(Parameter.z); //ウェイトを再計算
		}
	}
	ImGui::End();
}

//=============================================================================
// 描画処理
//=============================================================================
void Gaussian::Draw(int n)
{

	// 頂点レイアウト設定
	GetDeviceContext()->IASetInputLayout(VertexLayout);
	//頂点シェーダーをセット
	GetDeviceContext()->VSSetShader(VertexShader, NULL, 0);
	//ピクセルシェーダーをセット
	GetDeviceContext()->PSSetShader(PixelShader[n], NULL, 0);

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
		ID3D11ShaderResourceView* tex = GetPeTexture(n);
		GetDeviceContext()->PSSetShaderResources(0, 1, &tex);
		GetDeviceContext()->GenerateMips(tex);

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

//パラメータ作成関数
void    Gaussian::CalcGaussianWeight(float dispersion)
{
	Weight[0] = expf(0.0f); // 0番目（真ん中）は特殊処理
	float total = Weight[0];

	// i=1 からループを回し、左右対称なので2倍して total に足す
	for (int i = 1; i < 8; i++)
	{
		Weight[i] = expf(-0.5f * (float)(i * i) / dispersion);
		total += 2.0f * Weight[i];
	}

	// 重みの合計で除算することで、最終的な「全サンプリング点の合計」を1（正規化）にする
	for (int i = 0; i < 8; i++)
	{
		Weight[i] /= total;
	}
	return;
}