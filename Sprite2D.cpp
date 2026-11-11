/*==============================================================================

   頂点管理 [Sprite2D.cpp]
														 Author :
														 Date   :
--------------------------------------------------------------------------------

==============================================================================*/
#include "Sprite2D.h"
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
HRESULT Sprite2D::Init(void)
{
	// テクスチャ読み込み
	int Texture = TextureLoad(L"asset\\texture\\Toon2_2.png");

	//シェーダー読み込み
	//CreateVertexShader(&VertexShader, &VertexLayout, "ZukeiVS.cso");
	//CreatePixelShader(&PixelShader, "ZukeiPS.cso");
	CreateVertexShader(&VertexShader, &VertexLayout, "unlitTextureVS.cso");
	CreatePixelShader(&PixelShader, "unlitTexturePS.cso");

	//2Dオブジェクト初期化
<<<<<<< HEAD
	Position = XMFLOAT3(SCREEN_WIDTH , SCREEN_HEIGHT , 0.0f);
	Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Scale = XMFLOAT2(1.0f, 1.0f);
	Size = XMFLOAT2(SCREEN_WIDTH / 6, SCREEN_HEIGHT / 6);
=======
	Position = XMFLOAT3(SCREEN_WIDTH / 3 / 2, SCREEN_HEIGHT / 3 / 2, 0.0f);
	Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Scale = XMFLOAT2(1.0f, 1.0f);
	Size = XMFLOAT2(SCREEN_WIDTH / 3, SCREEN_HEIGHT / 3);
>>>>>>> 4cabf6289a03468186dab4fe4a96fb17fe2544b6
	Rotate = 0.0f;
	TexID = Texture;

	Parameter = XMFLOAT4(0.3f, 0.05f, 0, 0);

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void Sprite2D::Finalize(void)
{
	VertexLayout->Release();
	VertexShader->Release();
	PixelShader->Release();

}

//=============================================================================
// 更新処理
//=============================================================================
void Sprite2D::Update(void)
{
	ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_FirstUseEver);
	ImGui::Begin("ZUKEI");
	{
		ImGui::SliderFloat("Parameter.x", &Parameter.x, 0.0f, 1.0f, "%.4f");
		ImGui::SliderFloat("Parameter.y", &Parameter.y, 0.0f, 1.0f, "%.4f");
		ImGui::SliderFloat("Parameter.z", &Parameter.z, 0.0f, 1.0f, "%.4f");
		ImGui::SliderFloat("Parameter.w", &Parameter.w, 0.0f, 1.0f, "%.4f");

	}
	ImGui::End();
}

//=============================================================================
// 描画処理
//=============================================================================
void Sprite2D::Draw(void)
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

		//テクスチャをセット
		ID3D11ShaderResourceView* tex = GetDepthStencil();
		GetDeviceContext()->PSSetShaderResources(0, 1, &tex);

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
