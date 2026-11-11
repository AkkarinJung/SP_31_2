/*==============================================================================

   頂点管理 [Sprite2D.cpp]
														 Author :
														 Date   :
--------------------------------------------------------------------------------

==============================================================================*/
#include "DoF.h"
#include "sprite.h"
#include "Camera.h"
#include "texture.h"

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
HRESULT DoF::Init(void)
{
	// テクスチャ読み込み
	int Texture = TextureLoad(L"asset\\texture\\Toon2_2.png");

	//シェーダー読み込み
	CreateVertexShader(&VertexShader, &VertexLayout, "DoFVS.cso");
	CreatePixelShader(&PixelShader, "DoFPS.cso");

	//2Dオブジェクト初期化
	Position = XMFLOAT3(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0.0f);
	Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Scale = XMFLOAT2(1.0f, 1.0f);
	Size = XMFLOAT2(SCREEN_WIDTH, SCREEN_HEIGHT);
	Rotate = 0.0f;
	TexID = Texture;

	Parameter = XMFLOAT4(0.6f, 0.15f, 0.0f, 0.0f);

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void DoF::Finalize(void)
{
	VertexLayout->Release();
	VertexShader->Release();
	PixelShader->Release();

}

//=============================================================================
// 更新処理
//=============================================================================
void DoF::Update(void)
{
	ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_FirstUseEver);
	ImGui::Begin("DepthOfField");
	{
		ImGui::SliderFloat("Focus", &Parameter.x, 0.01f, 3.0f, "%.4f");
		ImGui::SliderFloat("DoF", &Parameter.y, 0.01f, 1.0f, "%.4f");
	}
	ImGui::End();
}

//=============================================================================
// 描画処理
//=============================================================================
void DoF::Draw(void)
{

	// 頂点レイアウト設定
	GetDeviceContext()->IASetInputLayout(VertexLayout);
	//頂点シェーダーをセット
	GetDeviceContext()->VSSetShader(VertexShader, NULL, 0);
	//ピクセルシェーダーをセット
	GetDeviceContext()->PSSetShader(PixelShader, NULL, 0);

	//奥行き処理をOFF
	SetDepthEnable(false);

	// マテリアル設定
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	{//2Dポリゴン1枚ずつで必要な処理

		// テクスチャをセット　注意！
		ID3D11ShaderResourceView* tex = GetPeTexture(0);    // 0番テクスチャをセット
		GetDeviceContext()->PSSetShaderResources(0, 1, &tex);
		tex = GetPeTexture(2);                              // 2番テクスチャをセット
		GetDeviceContext()->PSSetShaderResources(1, 1, &tex);
		tex = GetDepthStencil();                            // 深度バッファをセット
		GetDeviceContext()->PSSetShaderResources(2, 1, &tex);

		SetParameter(Parameter);


		//平行移動行列の作成（表示座標を決める）
		XMMATRIX	TranslationMatrix = XMMatrixTranslation(
			Position.x, Position.y, 0.0f);

		//回転行列（Z回転）行列の作成
		XMMATRIX	RotationMatrix = XMMatrixRotationZ(XMConvertToRadians(Rotate));

		//スケーリング行列作成（倍率1.0が等倍、0倍はダメ！）
		XMMATRIX	ScalingMatrix = XMMatrixScaling(Scale.x,Scale.y, 1.0f);

		//ワールド行列の作成（ポリゴンの表示の仕方を指定する最終的な行列
		XMMATRIX	WorldMatrix = ScalingMatrix * RotationMatrix * TranslationMatrix;

		//ワールド行列をDirectXへセット
		SetWorldMatrix(WorldMatrix);

		//texのMIPMAP作成
		GetDeviceContext()->GenerateMips(tex);

		// ポリゴン描画
		DrawSprite(Size, Color);
	}

}
