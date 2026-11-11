#include "ProjectionShadow.h"
#include "sprite.h"
#include "camera.h"
#include "texture.h"
#include "model.h"
#include "keyboard.h"

//************************************************************
// グローバル変数
//************************************************************

//************************************************************
// 初期化処理
//************************************************************
HRESULT ProjectionShadow::Init(void)
{
    //シェーダー読み込み
    CreateVertexShader(&VertexShader[0], &VertexLayout[0], "PixelLightingBlinnPhongVS.cso");
    CreatePixelShader(&PixelShader[0], "PixelLightingBlinnPhongPS.cso");
    CreateVertexShader(&VertexShader[1], &VertexLayout[1], "ProjectionShadow0VS.cso");
    CreatePixelShader(&PixelShader[1], "ProjectionShadow0PS.cso");

    //3Dオブジェクト管理構造体の初期化

    Position = XMFLOAT3(0.0f , 0.2f, -0.5f);
    Rotate = XMFLOAT3(0.0f, 0.0f, 0.0f);
    Scale = XMFLOAT3(0.2f, 0.2f, 0.2f);

    //モデル読み込み
    //Model = ModelLoad("asset\\model\\model.fbx");
    Model = ModelLoad("asset\\model\\cube.fbx");

    return S_OK;
}

//************************************************************
// 終了処理
//************************************************************
void ProjectionShadow::Finalize(void)
{
    for (int i = 0; i < 2; i++)
    {
        //作ったものを解放
        VertexLayout[i]->Release();
        VertexShader[i]->Release();
        PixelShader[i]->Release();
    }

    ModelRelease(Model);
}

//************************************************************
// 更新処理
//************************************************************
void ProjectionShadow::Update(void)
{
    //適当に回転
    if (Keyboard_IsKeyDown(KK_UP))
    {
        Position.z += 0.3f * (1.0f / 60.0f);
    }
    else if (Keyboard_IsKeyDown(KK_DOWN))
    {
        Position.z -= 0.3f * (1.0f / 60.0f);
    }
    if (Keyboard_IsKeyDown(KK_RIGHT))
    {
        Position.x += 0.3f * (1.0f / 60.0f);
    }
    else if (Keyboard_IsKeyDown(KK_LEFT))
    {
        Position.x -= 0.3f * (1.0f / 60.0f);
    }
    if (Keyboard_IsKeyDown(KK_Z))
    {
        Rotate.x += 60.0f * (1.0f / 60.0f);
    }
    else if (Keyboard_IsKeyDown(KK_X))
    {
        Rotate.x -= 60.0f * (1.0f / 60.0f);
    }
}

//************************************************************
// 描画処理
//************************************************************
void ProjectionShadow::Draw(int n)
{
    // 頂点レイアウト設定
    GetDeviceContext()->IASetInputLayout(VertexLayout[n]);

    // 頂点シェーダーをセット
    GetDeviceContext()->VSSetShader(VertexShader[n], NULL, 0);

    // ピクセルシェーダーをセット
    GetDeviceContext()->PSSetShader(PixelShader[n], NULL, 0);

    // 3Dポリゴン1つずつの処理
    // テクスチャをセット
    ID3D11ShaderResourceView* tex = GetTexture(TexID);
    GetDeviceContext()->PSSetShaderResources(0, 1, &tex);

    // 平行移動行列作成
    XMMATRIX TranslationMatrix =
        XMMatrixTranslation(
            Position.x,
            Position.y,
            Position.z
        );

    // 回転行列作成
    XMMATRIX RotationMatrix =
        XMMatrixRotationRollPitchYaw(
            XMConvertToRadians(Rotate.x),
            XMConvertToRadians(Rotate.y),
            XMConvertToRadians(Rotate.z)
        );

    // スケーリング行列作成
    XMMATRIX ScalingMatrix =
        XMMatrixScaling(
            Scale.x,
            Scale.y,
            Scale.z
        );

    // ワールド行列作成 ※乗算の順番に注意
    XMMATRIX WorldMatrix =
        ScalingMatrix *
        RotationMatrix *
        TranslationMatrix;

    // DirectXへセット
    SetWorldMatrix(WorldMatrix);

    // プリミティブトポロジーの設定
    GetDeviceContext()->IASetPrimitiveTopology(
        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
    );

    // マテリアル設定
    MATERIAL material;
    ZeroMemory(&material, sizeof(MATERIAL));
    material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    SetMaterial(material);

    // 描画
    ModelDraw(Model);
}

void ProjectionShadow::Draw(int n, LIGHT& Light)
{
    // 頂点レイアウト設定
    GetDeviceContext()->IASetInputLayout(VertexLayout[n]);

    // 頂点シェーダーをセット
    GetDeviceContext()->VSSetShader(VertexShader[n], NULL, 0);

    // ピクセルシェーダーをセット
    GetDeviceContext()->PSSetShader(PixelShader[n], NULL, 0);

    // 3Dポリゴン1つずつの処理
    // テクスチャをセット
    ID3D11ShaderResourceView* tex = GetTexture(TexID);
    GetDeviceContext()->PSSetShaderResources(0, 1, &tex);

    // 平行移動行列作成
    XMMATRIX TranslationMatrix =
        XMMatrixTranslation(
            Position.x,
            Position.y,
            Position.z
        );

    // 回転行列作成
    XMMATRIX RotationMatrix =
        XMMatrixRotationRollPitchYaw(
            XMConvertToRadians(Rotate.x),
            XMConvertToRadians(Rotate.y),
            XMConvertToRadians(Rotate.z)
        );

    // スケーリング行列作成
    XMMATRIX ScalingMatrix =
        XMMatrixScaling(
            Scale.x,
            Scale.y,
            Scale.z
        );

    // ワールド行列作成 ※乗算の順番に注意
    XMMATRIX WorldMatrix =
        ScalingMatrix *
        RotationMatrix *
        TranslationMatrix;

    // DirectXへセット
    SetWorldMatrix(WorldMatrix);

    // プリミティブトポロジーの設定
    GetDeviceContext()->IASetPrimitiveTopology(
        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
    );

    // マテリアル設定
    MATERIAL material;
    ZeroMemory(&material, sizeof(MATERIAL));
    material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    SetMaterial(material);

    // 描画
    ModelDraw(Model);
}