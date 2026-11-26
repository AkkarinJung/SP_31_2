#pragma once

// ShadowDiorama.h
// "A Day in Shadow Town" - diorama scene for SP31 assignment 07.
// Every object is drawn with the depth-buffer shadow shaders
// (DepthShadowVS / DepthShadowPS), once into the shadow map and once normally.
// The shader colors pixels from texture slot 0, so each color gets a 1x1 texture.
//
// Timeline (one loop = 90 s)
//   0 - 40  Day      : the sun crosses the sky, the 5 flying donuts make the Olympic rings at noon (t = 20)
//  40 - 86  Night    : SHADOW THEATRE. A lantern lights the back wall like a screen.
//              44 - 54  a bird puppet flies across the wall
//              54 - 65  floating blocks gather -> from the lantern they cast a HEART
//              65 - 76  the lantern moves -> new blocks cast a STAR
//              76 - 86  a rabbit puppet hops across the wall
//  86 - 90  Dawn     : back to the sun

#include "main.h"
#include "renderer.h"
#include "model.h"

class ShadowDiorama
{
public:
    // Length of one day/night loop in seconds
    static constexpr float LOOP_TIME = 90.0f;

    HRESULT Init(void);
    void Finalize(void);
    void Update(float time);        // time = seconds into the loop (0 .. LOOP_TIME)
    void Draw(bool depthPass);      // depthPass = true while rendering the shadow map

    // 0 = day, 1 = night (shadow theatre)
    static float NightRate(float time);
    // Position of the theatre lantern at night (the shapes are designed for these positions)
    static XMFLOAT3 NightLightPosition(float time);

    float CubePivotZ = -0.5f;       // cube.fbx is not centered on Z; this re-centers it

private:
    enum MODEL_ID { MODEL_CUBE, MODEL_BALL, MODEL_DONUT, MODEL_TREE, MODEL_MAX };

    struct OBJECT
    {
        MODEL_ID Model;
        XMFLOAT3 Position;
        XMFLOAT3 Rotate;    // degrees
        XMFLOAT3 Scale;
        ID3D11ShaderResourceView* Texture;  // 1x1 texture of the object's color
        bool     Hidden;    // not drawn at all
        bool     NoShadow;  // not drawn into the shadow map (lantern itself)
    };

    static const int MAX_OBJECT = 320;
    OBJECT  Object[MAX_OBJECT];
    int     NumObject = 0;

    // Indices of animated objects
    int RingStart;          // 5 flying donuts that form the Olympic rings at noon
    int WalkerStart;        // character parts
    int BallStart;          // 2 bouncing balls
    int WheelIndex;         // rolling donut
    int TreeStart, TreeCount;
    int WindowStart, WindowCount;
    int GnomonIndex, GnomonTopIndex;    // sundial pillar sinks into the ground at night
    int LanternPole, LanternBulb;
    int BirdStart;          // bird puppet parts
    int RabbitStart;        // rabbit puppet parts

    // "Hidden picture" blocks: scattered in 3D, but from one lantern position
    // their shadows line up into a picture on the wall
    struct PIECE
    {
        XMFLOAT3 Final;     // position where the shadow lines up
        XMFLOAT3 Scatter;   // where it flies in from
        float    Size;
        float    Delay;
    };
    static const int MAX_PIECE = 128;
    PIECE Piece[MAX_PIECE];
    int   NumPiece = 0;
    struct REVEAL
    {
        float Start;        // time the blocks start flying in
        int   PieceStart, PieceCount;
        int   ObjectStart;
    };
    static const int NUM_REVEAL = 2;
    REVEAL Reveal[NUM_REVEAL];

    MODEL*              Model[MODEL_MAX];
    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader*  PixelShader;
    ID3D11InputLayout*  VertexLayout;

    // 1x1 color textures (shared between objects with the same color)
    static const int MAX_COLOR = 64;
    XMFLOAT4                  ColorValue[MAX_COLOR];
    ID3D11ShaderResourceView* ColorTex[MAX_COLOR];
    int                       NumColor = 0;
    ID3D11ShaderResourceView* WindowDayTex;
    ID3D11ShaderResourceView* WindowNightTex;
    ID3D11ShaderResourceView* GetColorTexture(const XMFLOAT4& color);

    int  Add(MODEL_ID model, XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale, XMFLOAT4 color);
    void AddHouse(float x, float z, XMFLOAT4 wall, XMFLOAT4 roof);
    void AddReveal(int index, float start, XMFLOAT3 light, const char* const* bitmap, int height,
                   float centerX, float centerY, float pixel, XMFLOAT4 color, unsigned seed);
    void UpdatePuppets(float t);
    void UpdateReveals(float t);
};
