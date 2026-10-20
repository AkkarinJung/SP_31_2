/*==============================================================================

   ホラー演出 [Gaussian.h]
														 Author :
														 Date   :
--------------------------------------------------------------------------------

==============================================================================*/
#pragma once

#include "main.h"
#include "renderer.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************


////////////////
//構造体
////////////////

class Gaussian
{
protected:
	XMFLOAT3	Position;		//表示座標
	XMFLOAT4	Color;			//色
	XMFLOAT2	Size;			//サイズ
	XMFLOAT2	Scale;			//拡大縮小率
	float		Rotate;			//回転角度（360表現）
	int			TexID;			//テクスチャID
	ID3D11VertexShader* VertexShader; //頂点シェーダーオブジェクト
	ID3D11PixelShader* PixelShader[2]; //ピクセルシェーダーオブジェクト
	ID3D11InputLayout* VertexLayout; //頂点レイアウトオブジェクト

	XMFLOAT4 Parameter;		
	float Weight[8];

public:
	HRESULT Init(void);
	void Finalize(void);
	void Update(void);
	void Draw(int n);
	void CalcGaussianWeight(float);
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************

