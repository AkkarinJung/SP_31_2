#pragma once

#include "main.h"
#include "renderer.h"
#include "model.h"

class ProjectionShadow
{
protected:
    XMFLOAT3 Position;
    XMFLOAT3 Scale;
    XMFLOAT3 Rotate;
    int      TexID;

    ID3D11VertexShader* VertexShader[2];   // 頂点シェーダーオブジェクト
    ID3D11PixelShader* PixelShader[2];    // ピクセルシェーダーオブジェクト
    ID3D11InputLayout* VertexLayout[2];   // 頂点レイアウトオブジェクト

    LIGHT Light;

    MODEL* Model;

public:
    HRESULT Init(void);
    void Finalize(void);
    void Update(void);
    void Draw(int n );
    void Draw(int n , LIGHT& Light);
};

