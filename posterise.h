/*==============================================================================

   ホラー演出 [posterise.h]
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

class posterise
{
protected:
	XMFLOAT3	Position;		//表示座標
	XMFLOAT4	Color;			//色
	XMFLOAT2	Size;			//サイズ
	XMFLOAT2	Scale;			//拡大縮小率
	float		Rotate;			//回転角度（360表現）
	//int			TexID;			//テクスチャID
	ID3D11VertexShader* VertexShader; //頂点シェーダーオブジェクト
	ID3D11PixelShader* PixelShader; //ピクセルシェーダーオブジェクト
	ID3D11InputLayout* VertexLayout; //頂点レイアウトオブジェクト

	XMFLOAT4 Parameter;		// x:Seed値  y:MIN  z:MAX  w:POW

	static const int TEXTURE_NUM = 3;    // 2.png / 3.png / 4.png
	static constexpr float SWITCH_TIME = 4.0f;   // 1枚あたり2秒

	int TexID[TEXTURE_NUM];   // ← 旧: int TexID;
	int TexIndex;             // 今表示している画像
	float Timer;           // 経過フレーム

	XMFLOAT4 StartParameter;   // 開始値
	XMFLOAT4 EndParameter;     // 終了値
	bool     AutoParameter;    // false でスライダー手動調整

public:
	HRESULT Init(void);
	void Finalize(void);
	void Update(void);
	void Draw(void);

};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************

