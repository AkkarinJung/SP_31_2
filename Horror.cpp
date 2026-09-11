/*==============================================================================

   ホラー演出 [Horror.cpp]
														 Author :
														 Date   :
--------------------------------------------------------------------------------

==============================================================================*/
#include "Horror.h"
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

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT Horror::Init(void)
{
	// テクスチャ読み込み
	int Texture = TextureLoad(L"asset\\texture\\Toon2.bmp");

	//シェーダー読み込み
	CreateVertexShader(&VertexShader, &VertexLayout, "horrorVS.cso");
	CreatePixelShader(&PixelShader, "horrorPS.cso");

	//2Dオブジェクト初期化
	Position = XMFLOAT3(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0.0f);
	Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Scale = XMFLOAT2(1.0f, 1.0f);
	Size = XMFLOAT2(SCREEN_WIDTH, SCREEN_HEIGHT);
	Rotate = 0.0f;
	TexID = Texture;

	//Parameter : x=Seed値  y=MIN  z=MAX  w=POW
	Parameter = XMFLOAT4(0.0f, 0.2f, 0.7f, 1.0f);

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void Horror::Finalize(void)
{
	VertexLayout->Release();
	VertexShader->Release();
	PixelShader->Release();

}

//=============================================================================
// 更新処理
//=============================================================================
void Horror::Update(void)
{
	//シェーダーへ渡すSeed値を毎フレーム加算する（0.0～100.0でループさせる）
	Parameter.x += ((float)rand() / RAND_MAX);
	if (Parameter.x > 100.0f)
	{
		Parameter.x -= 100.0f;
	}

	ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_FirstUseEver);
	ImGui::Begin("Horror");
	{
		ImGui::SliderFloat("MIN", &Parameter.y, 0.0f, 1.0f, "%.3f");
		ImGui::SliderFloat("MAX", &Parameter.z, 0.0f, 1.0f, "%.3f");
		ImGui::SliderFloat("POW", &Parameter.w, 1.0f, 30.0f, "%.0f");
	}
	ImGui::End();
}

//=============================================================================
// 描画処理
//=============================================================================
void Horror::Draw(void)
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
		ID3D11ShaderResourceView* tex = GetPeTexture();
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
