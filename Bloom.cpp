#include "Bloom.h"
#include "Camera.h"
#include "texture.h"
#include "sprite.h"

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT Bloom::Init(void)
{
	//シェーダー読み込み
	CreateVertexShader(&VertexShader, &VertexLayout, "BloomVS.cso");
	CreatePixelShader(&PixelShader[0], "BloomPS01.cso");	//輝度抽出
	CreatePixelShader(&PixelShader[1], "BloomPS02.cso");	//加算合成

	//2Dオブジェクト初期化
	Position = XMFLOAT3(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0.0f);
	Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Scale = XMFLOAT2(1.0f, 1.0f);
	Size = XMFLOAT2(SCREEN_WIDTH, SCREEN_HEIGHT);
	Rotate = 0.0f;

	//x:輝度しきい値 y,z,w:ブレンド率
	Parameter = XMFLOAT4(0.6f, 1.0f, 1.3f, 1.6f);
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void Bloom::Finalize(void)
{
	VertexLayout->Release();
	VertexShader->Release();
	PixelShader[0]->Release();
	PixelShader[1]->Release();
}

//=============================================================================
// 更新処理
//=============================================================================
void Bloom::Update(void)
{
	ImGui::Begin("BLOOM");
	{
		ImGui::SliderFloat("Luminance", &Parameter.x, 0.0f, 1.0f, "%.4f");
		ImGui::SliderFloat("Blend 1", &Parameter.y, 0.0f, 2.0f, "%.4f");
		ImGui::SliderFloat("Blend 2", &Parameter.z, 0.0f, 2.0f, "%.4f");
		ImGui::SliderFloat("Blend 3", &Parameter.w, 0.0f, 2.0f, "%.4f");
	}
	ImGui::End();
}

//=============================================================================
// 描画処理  n=0:輝度抽出(RT0→RT1)  n=1:ブルーム(RT0+RT1→バックバッファ)
//=============================================================================
void Bloom::Draw(int n)
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
		ID3D11ShaderResourceView* tex = GetPeTexture(0);
		GetDeviceContext()->PSSetShaderResources(0, 1, &tex);//通常シーン表示
		GetDeviceContext()->GenerateMips(tex);				//ミップマップ作成 ★

		if (n == 1)	//RT1は輝度抽出(n=0)の時は描画先なので、ブルーム時だけセットする
		{
			tex = GetPeTexture(1);
			GetDeviceContext()->PSSetShaderResources(1, 1, &tex);//輝度抽出
			GetDeviceContext()->GenerateMips(tex);				//ミップマップ作成 ★
		}

		//平行移動行列の作成（表示座標を決める）
		XMMATRIX	TranslationMatrix = XMMatrixTranslation(Position.x, Position.y, 0.0f);
		//回転行列（Z回転）の作成
		XMMATRIX	RotationMatrix = XMMatrixRotationZ(XMConvertToRadians(Rotate));
		//スケーリング行列の作成
		XMMATRIX	ScalingMatrix = XMMatrixScaling(Scale.x, Scale.y, 1.0f);
		//ワールド行列の作成
		XMMATRIX	WorldMatrix = ScalingMatrix * RotationMatrix * TranslationMatrix;
		SetWorldMatrix(WorldMatrix);

		// ポリゴン描画
		DrawSprite(Size, Color);

		//次のフレームでRTとして使うので、テクスチャのセットを外しておく
		ID3D11ShaderResourceView* nullSRV[2] = { NULL, NULL };
		GetDeviceContext()->PSSetShaderResources(0, 2, nullSRV);
	}
}