// ShadowDiorama.cpp
// "A Day in Shadow Town" - diorama scene for SP31 assignment 07.

#include "ShadowDiorama.h"
#include "texture.h"
#include <string.h>

//************************************************************
// Helpers
//************************************************************
static float Clamp01(float x) { return x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x); }

static float SmoothStep(float e0, float e1, float x)
{
    float t = Clamp01((x - e0) / (e1 - e0));
    return t * t * (3.0f - 2.0f * t);
}

static float Lerp(float a, float b, float t) { return a + (b - a) * t; }

static XMFLOAT3 Lerp3(const XMFLOAT3& a, const XMFLOAT3& b, float t)
{
    return XMFLOAT3(Lerp(a.x, b.x, t), Lerp(a.y, b.y, t), Lerp(a.z, b.z, t));
}

// Rotate an offset around Y (degrees), same direction as XMMatrixRotationY
static XMFLOAT3 RotY(const XMFLOAT3& v, float yawDeg)
{
    XMVECTOR r = XMVector3Transform(XMLoadFloat3(&v), XMMatrixRotationY(XMConvertToRadians(yawDeg)));
    XMFLOAT3 out;
    XMStoreFloat3(&out, r);
    return out;
}

// Rotate an offset around X then Y (degrees), same order as XMMatrixRotationRollPitchYaw
static XMFLOAT3 RotXY(const XMFLOAT3& v, float pitchDeg, float yawDeg)
{
    XMMATRIX m = XMMatrixRotationX(XMConvertToRadians(pitchDeg)) *
                 XMMatrixRotationY(XMConvertToRadians(yawDeg));
    XMVECTOR r = XMVector3Transform(XMLoadFloat3(&v), m);
    XMFLOAT3 out;
    XMStoreFloat3(&out, r);
    return out;
}

static XMFLOAT3 Add3(const XMFLOAT3& a, const XMFLOAT3& b)
{
    return XMFLOAT3(a.x + b.x, a.y + b.y, a.z + b.z);
}

//************************************************************
// Scene layout constants
//************************************************************
static const float WALK_RADIUS   = 1.2f;    // character walks in a circle around the sundial
static const float WALK_PERIOD   = 30.0f;   // seconds per lap
static const float WHEEL_Z       = -1.6f;   // rolling donut lane
static const float WHEEL_RANGE   = 1.05f;
static const float DONUT_SCALE   = 0.2f;    // model.fbx is a torus with outer radius 1.25
static const float RING_HEIGHT   = 1.8f;    // high enough that the rings and their shadows do not overlap on screen

// Olympic ring formation (x, z) seen from above, and colors
static const XMFLOAT2 RING_FORMATION[5] =
{
    XMFLOAT2(-0.55f,  0.12f), XMFLOAT2(0.0f, 0.12f), XMFLOAT2(0.55f, 0.12f),
    XMFLOAT2(-0.275f, -0.13f), XMFLOAT2(0.275f, -0.13f),
};
//------------------------------------------------------------
// Shadow theatre
//------------------------------------------------------------
static const XMFLOAT3 LANTERN_HOME(0.0f, 0.8f, -2.2f);  // lantern position between the acts
static const float    WALL_Z = 2.39f;                   // front face of the back wall (the screen)

// Hidden pictures: 1 bitmap pixel = 1 floating block. 'X' = block
static const char* const HEART_BITMAP[] =
{
    ".XX...XX.",
    "XXXX.XXXX",
    "XXXXXXXXX",
    "XXXXXXXXX",
    ".XXXXXXX.",
    "..XXXXX..",
    "...XXX...",
    "....X....",
};
static const char* const STAR_BITMAP[] =
{
    ".....X.....",
    "....XXX....",
    "....XXX....",
    "XXXXXXXXXXX",
    ".XXXXXXXXX.",
    "..XXXXXXX..",
    "..XXXXXXX..",
    ".XXXX.XXXX.",
    ".XXX...XXX.",
    "XX.......XX",
};
// When each picture starts, and the ONLY lantern position from which it can be seen
static const float    REVEAL_START[2] = { 54.0f, 65.0f };
static const XMFLOAT3 REVEAL_LIGHT[2] = { XMFLOAT3(-0.8f, 0.9f, -2.0f), XMFLOAT3(0.8f, 1.0f, -2.0f) };

// Per reveal (seconds after REVEAL_START):
//   0 - 4    blocks fly in and settle, lantern moves to REVEAL_LIGHT (picture appears on the wall)
//   4 - 7.5  hold
//   7.5-9.5  lantern moves away -> the same blocks now cast only noise
//   9 - 11   blocks drop and vanish
static const float REVEAL_LENGTH = 11.0f;

static float Hash(unsigned n)
{
    n = (n << 13) ^ n;
    n = n * (n * n * 15731u + 789221u) + 1376312589u;
    return (float)(n & 0x7fffffffu) / 2147483647.0f;   // 0..1
}

static const XMFLOAT4 RING_COLOR[5] =
{
    XMFLOAT4(0.10f, 0.35f, 0.85f, 0.0f),    // blue
    XMFLOAT4(0.12f, 0.12f, 0.14f, 0.0f),    // black
    XMFLOAT4(0.90f, 0.15f, 0.15f, 0.0f),    // red
    XMFLOAT4(0.95f, 0.75f, 0.10f, 0.0f),    // yellow
    XMFLOAT4(0.10f, 0.65f, 0.25f, 0.0f),    // green
};

//************************************************************
// 1x1 texture filled with one color
//************************************************************
ID3D11ShaderResourceView* ShadowDiorama::GetColorTexture(const XMFLOAT4& c)
{
    for (int i = 0; i < NumColor; i++)
    {
        if (ColorValue[i].x == c.x && ColorValue[i].y == c.y && ColorValue[i].z == c.z)
            return ColorTex[i];
    }
    if (NumColor >= MAX_COLOR) return ColorTex[MAX_COLOR - 1];

    unsigned char pixel[4] =
    {
        (unsigned char)(Clamp01(c.x) * 255.0f + 0.5f),
        (unsigned char)(Clamp01(c.y) * 255.0f + 0.5f),
        (unsigned char)(Clamp01(c.z) * 255.0f + 0.5f),
        255
    };

    D3D11_TEXTURE2D_DESC td;
    ZeroMemory(&td, sizeof(td));
    td.Width = 1;
    td.Height = 1;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_IMMUTABLE;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.pSysMem = pixel;
    sd.SysMemPitch = 4;

    ID3D11Texture2D* texture = NULL;
    ID3D11ShaderResourceView* srv = NULL;
    GetDevice()->CreateTexture2D(&td, &sd, &texture);
    if (texture)
    {
        GetDevice()->CreateShaderResourceView(texture, NULL, &srv);
        texture->Release();   // the view keeps its own reference
    }

    ColorValue[NumColor] = c;
    ColorTex[NumColor] = srv;
    NumColor++;
    return srv;
}

//************************************************************
// Init
//************************************************************
int ShadowDiorama::Add(MODEL_ID model, XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale, XMFLOAT4 color)
{
    if (NumObject >= MAX_OBJECT) return MAX_OBJECT - 1;
    OBJECT& o = Object[NumObject];
    o.Model    = model;
    o.Position = pos;
    o.Rotate   = rot;
    o.Scale    = scale;
    o.Texture  = GetColorTexture(color);
    o.Hidden   = false;
    o.NoShadow = false;
    return NumObject++;
}

// House = body + roof + chimney + door + windows; the door faces the plaza center
void ShadowDiorama::AddHouse(float x, float z, XMFLOAT4 wall, XMFLOAT4 roof)
{
    // yaw so that local -Z (front) points to the center
    float yaw = XMConvertToDegrees(atan2f(x, z));
    XMFLOAT3 base(x, 0.0f, z);

    Add(MODEL_CUBE, XMFLOAT3(x, 0.25f, z), XMFLOAT3(0, yaw, 0), XMFLOAT3(0.6f, 0.5f, 0.6f), wall);
    // roof: cube rolled 45 degrees = triangular roof with the ridge along local Z
    Add(MODEL_CUBE, XMFLOAT3(x, 0.5f, z), XMFLOAT3(0, yaw, 45.0f), XMFLOAT3(0.43f, 0.43f, 0.68f), roof);
    // chimney
    Add(MODEL_CUBE, Add3(base, RotY(XMFLOAT3(0.15f, 0.68f, 0.12f), yaw)), XMFLOAT3(0, yaw, 0),
        XMFLOAT3(0.08f, 0.22f, 0.08f), XMFLOAT4(0.45f, 0.4f, 0.38f, 0.0f));
    // door
    Add(MODEL_CUBE, Add3(base, RotY(XMFLOAT3(0.0f, 0.12f, -0.305f), yaw)), XMFLOAT3(0, yaw, 0),
        XMFLOAT3(0.14f, 0.24f, 0.02f), XMFLOAT4(0.35f, 0.2f, 0.12f, 0.0f));
    // windows (they light up at night)
    const XMFLOAT3 win[2] = { XMFLOAT3(-0.19f, 0.32f, -0.305f), XMFLOAT3(0.19f, 0.32f, -0.305f) };
    for (int i = 0; i < 2; i++)
    {
        int id = Add(MODEL_CUBE, Add3(base, RotY(win[i], yaw)), XMFLOAT3(0, yaw, 0),
                     XMFLOAT3(0.11f, 0.11f, 0.02f), XMFLOAT4(0.55f, 0.65f, 0.75f, 0.0f));
        if (WindowCount == 0) WindowStart = id;
        WindowCount++;
    }
}

// Build one hidden picture.
// For every 'X' pixel, the target point T on the wall is chosen, then a block is put
// somewhere on the line lantern -> T. A block at fraction s of the way casts a shadow
// 1/s times bigger, so its size is pixel * s. From the lantern all shadows tile into the picture;
// from anywhere else (the camera) the blocks look like a random cloud.
void ShadowDiorama::AddReveal(int index, float start, XMFLOAT3 light, const char* const* bitmap, int height,
                              float centerX, float centerY, float pixel, XMFLOAT4 color, unsigned seed)
{
    REVEAL& r = Reveal[index];
    r.Start = start;
    r.PieceStart = NumPiece;
    r.PieceCount = 0;
    r.ObjectStart = NumObject;

    int width = (int)strlen(bitmap[0]);
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            if (bitmap[j][i] != 'X' || NumPiece >= MAX_PIECE) continue;

            unsigned k = seed * 1000u + (unsigned)(j * width + i);
            XMFLOAT3 target(centerX + (i - (width - 1) * 0.5f) * pixel,
                            centerY + ((height - 1) * 0.5f - j) * pixel,
                            WALL_Z);
            float s = 0.32f + 0.43f * Hash(k * 3u + 1u);

            PIECE& p = Piece[NumPiece++];
            p.Final = Lerp3(light, target, s);
            p.Size = pixel * s * 1.12f;     // a little bigger so the shadows overlap without gaps
            p.Delay = Hash(k * 3u + 2u);

            float sx = p.Final.x + (Hash(k * 7u + 3u) * 2.0f - 1.0f) * 1.4f;
            float sy = p.Final.y + 0.4f + Hash(k * 7u + 4u) * 0.9f;
            float sz = p.Final.z + (Hash(k * 7u + 5u) * 2.0f - 1.0f) * 1.0f;
            sx = (sx < -2.2f) ? -2.2f : ((sx > 2.2f) ? 2.2f : sx);
            sz = (sz < -2.2f) ? -2.2f : ((sz > 2.2f) ? 2.2f : sz);
            p.Scatter = XMFLOAT3(sx, sy, sz);

            int id = Add(MODEL_CUBE, p.Final, XMFLOAT3(0, 0, 0), XMFLOAT3(p.Size, p.Size, p.Size), color);
            Object[id].Hidden = true;
            r.PieceCount++;
        }
    }
}

float ShadowDiorama::NightRate(float t)
{
    // dusk 38 - 43, dawn 86 - 90
    return SmoothStep(38.0f, 43.0f, t) * (1.0f - SmoothStep(86.0f, 90.0f, t));
}

XMFLOAT3 ShadowDiorama::NightLightPosition(float t)
{
    // the lantern sways a little between the acts...
    XMFLOAT3 p(LANTERN_HOME.x + 0.15f * sinf(t * 0.7f),
               LANTERN_HOME.y + 0.05f * sinf(t * 1.3f),
               LANTERN_HOME.z);
    // ...and goes to the exact viewpoint of each hidden picture
    for (int i = 0; i < NUM_REVEAL; i++)
    {
        float st = REVEAL_START[i];
        float w = SmoothStep(st + 1.0f, st + 4.0f, t) * (1.0f - SmoothStep(st + 7.5f, st + 9.5f, t));
        p = Lerp3(p, REVEAL_LIGHT[i], w);
    }
    return p;
}

HRESULT ShadowDiorama::Init(void)
{
    CreateVertexShader(&VertexShader, &VertexLayout, "DepthShadowVS.cso");
    CreatePixelShader(&PixelShader, "DepthShadowPS.cso");

    Model[MODEL_CUBE]  = ModelLoad("asset\\model\\cube.fbx");
    Model[MODEL_BALL]  = ModelLoad("asset\\model\\ball.fbx");   // sphere, radius 0.2
    Model[MODEL_DONUT] = ModelLoad("asset\\model\\model.fbx");  // torus, outer radius 1.25
    Model[MODEL_TREE]  = ModelLoad("asset\\model\\tree.fbx");   // height 1.5

    NumObject = 0;
    NumPiece = 0;
    NumColor = 0;
    WindowCount = 0;
    WindowDayTex = GetColorTexture(XMFLOAT4(0.55f, 0.65f, 0.75f, 0.0f));
    WindowNightTex = GetColorTexture(XMFLOAT4(1.0f, 0.85f, 0.4f, 0.0f));

    const XMFLOAT3 R0(0, 0, 0);
    const XMFLOAT4 stone(0.62f, 0.58f, 0.52f, 0.0f);

    //------------------------------------------------------------
    // Walls: tall back wall works as a "shadow screen"
    //------------------------------------------------------------
    Add(MODEL_CUBE, XMFLOAT3(0.0f, 0.8f, 2.45f), R0, XMFLOAT3(5.0f, 1.6f, 0.1f), XMFLOAT4(0.88f, 0.85f, 0.78f, 0.0f));
    Add(MODEL_CUBE, XMFLOAT3(-2.45f, 0.08f, 0.0f), R0, XMFLOAT3(0.1f, 0.16f, 4.8f), stone);
    Add(MODEL_CUBE, XMFLOAT3( 2.45f, 0.08f, 0.0f), R0, XMFLOAT3(0.1f, 0.16f, 4.8f), stone);
    Add(MODEL_CUBE, XMFLOAT3(0.0f, 0.08f, -2.45f), R0, XMFLOAT3(4.8f, 0.16f, 0.1f), stone);

    //------------------------------------------------------------
    // Sundial in the middle: its shadow works as a clock hand
    //------------------------------------------------------------
    Add(MODEL_CUBE, XMFLOAT3(0.0f, 0.03f, 0.0f), R0, XMFLOAT3(0.9f, 0.06f, 0.9f), XMFLOAT4(0.78f, 0.75f, 0.7f, 0.0f));
    GnomonIndex = Add(MODEL_CUBE, XMFLOAT3(0.0f, 0.51f, 0.0f), R0, XMFLOAT3(0.1f, 0.9f, 0.1f), XMFLOAT4(0.6f, 0.58f, 0.64f, 0.0f));
    GnomonTopIndex = Add(MODEL_BALL, XMFLOAT3(0.0f, 1.04f, 0.0f), R0, XMFLOAT3(0.4f, 0.4f, 0.4f), XMFLOAT4(0.95f, 0.75f, 0.25f, 0.0f));
    for (int i = 0; i < 12; i++)   // hour markers
    {
        float a = XM_2PI * i / 12.0f;
        Add(MODEL_CUBE, XMFLOAT3(0.68f * sinf(a), 0.015f, 0.68f * cosf(a)),
            XMFLOAT3(0, XMConvertToDegrees(a), 0), XMFLOAT3(0.05f, 0.03f, (i % 3 == 0) ? 0.16f : 0.1f),
            XMFLOAT4(0.3f, 0.28f, 0.32f, 0.0f));
    }

    //------------------------------------------------------------
    // Houses
    //------------------------------------------------------------
    AddHouse(-1.6f,  1.4f, XMFLOAT4(0.88f, 0.68f, 0.48f, 0.0f), XMFLOAT4(0.68f, 0.22f, 0.15f, 0.0f));
    AddHouse( 1.55f, 1.5f, XMFLOAT4(0.92f, 0.88f, 0.75f, 0.0f), XMFLOAT4(0.2f, 0.35f, 0.62f, 0.0f));
    AddHouse(-1.75f, -1.3f, XMFLOAT4(0.78f, 0.82f, 0.68f, 0.0f), XMFLOAT4(0.3f, 0.45f, 0.25f, 0.0f));

    //------------------------------------------------------------
    // Trees (they sway in the wind)
    //------------------------------------------------------------
    const XMFLOAT3 treePos[6] =
    {
        XMFLOAT3(2.05f, 0, -0.3f), XMFLOAT3(1.7f, 0, -1.9f), XMFLOAT3(0.95f, 0, 2.1f),
        XMFLOAT3(-0.95f, 0, 2.1f), XMFLOAT3(-2.05f, 0, 0.35f), XMFLOAT3(-1.0f, 0, -2.15f),
    };
    TreeStart = NumObject;
    TreeCount = 6;
    for (int i = 0; i < TreeCount; i++)
    {
        float s = 0.5f + 0.08f * (i % 3);
        Add(MODEL_TREE, treePos[i], XMFLOAT3(0, 40.0f * i, 0), XMFLOAT3(s, s, s),
            XMFLOAT4(0.22f, 0.5f + 0.05f * (i % 2), 0.22f, 0.0f));
    }

    //------------------------------------------------------------
    // Street lamps (posts + self-lit bulbs)
    //------------------------------------------------------------
    const XMFLOAT2 lampPos[2] = { XMFLOAT2(1.45f, -0.55f), XMFLOAT2(-1.45f, 0.4f) };
    for (int i = 0; i < 2; i++)
    {
        Add(MODEL_CUBE, XMFLOAT3(lampPos[i].x, 0.35f, lampPos[i].y), R0, XMFLOAT3(0.04f, 0.7f, 0.04f),
            XMFLOAT4(0.2f, 0.2f, 0.22f, 0.0f));
        Add(MODEL_BALL, XMFLOAT3(lampPos[i].x, 0.74f, lampPos[i].y), R0, XMFLOAT3(0.3f, 0.3f, 0.3f),
            XMFLOAT4(1.0f, 0.9f, 0.6f, 0.0f));
    }

    //------------------------------------------------------------
    // Animated objects (positions are set in Update)
    //------------------------------------------------------------
    // 5 flying donuts -> Olympic rings shadow at noon
    RingStart = NumObject;
    for (int i = 0; i < 5; i++)
        Add(MODEL_DONUT, R0, R0, XMFLOAT3(DONUT_SCALE, DONUT_SCALE, DONUT_SCALE), RING_COLOR[i]);

    // Character with a top hat
    WalkerStart = NumObject;
    const XMFLOAT4 coat(0.9f, 0.35f, 0.3f, 0.0f);
    const XMFLOAT4 pants(0.25f, 0.25f, 0.42f, 0.0f);
    const XMFLOAT4 hat(0.12f, 0.1f, 0.12f, 0.0f);
    Add(MODEL_CUBE, R0, R0, XMFLOAT3(0.14f, 0.18f, 0.09f), coat);                        // 0 body
    Add(MODEL_BALL, R0, R0, XMFLOAT3(0.45f, 0.45f, 0.45f), XMFLOAT4(0.98f, 0.85f, 0.7f, 0.0f)); // 1 head
    Add(MODEL_CUBE, R0, R0, XMFLOAT3(0.05f, 0.13f, 0.05f), pants);                       // 2 left leg
    Add(MODEL_CUBE, R0, R0, XMFLOAT3(0.05f, 0.13f, 0.05f), pants);                       // 3 right leg
    Add(MODEL_CUBE, R0, R0, XMFLOAT3(0.035f, 0.14f, 0.035f), coat);                      // 4 left arm
    Add(MODEL_CUBE, R0, R0, XMFLOAT3(0.035f, 0.14f, 0.035f), coat);                      // 5 right arm
    Add(MODEL_CUBE, R0, R0, XMFLOAT3(0.22f, 0.02f, 0.22f), hat);                         // 6 hat brim
    Add(MODEL_CUBE, R0, R0, XMFLOAT3(0.12f, 0.12f, 0.12f), hat);                         // 7 hat top

    // Bouncing balls
    BallStart = NumObject;
    Add(MODEL_BALL, R0, R0, XMFLOAT3(0.6f, 0.6f, 0.6f), XMFLOAT4(0.3f, 0.6f, 0.95f, 0.0f));
    Add(MODEL_BALL, R0, R0, XMFLOAT3(0.6f, 0.6f, 0.6f), XMFLOAT4(0.95f, 0.8f, 0.2f, 0.0f));

    // Rolling donut (upright torus = wheel)
    WheelIndex = Add(MODEL_DONUT, R0, R0, XMFLOAT3(DONUT_SCALE, DONUT_SCALE, DONUT_SCALE),
                     XMFLOAT4(0.95f, 0.55f, 0.7f, 0.0f));

    //------------------------------------------------------------
    // Shadow theatre (night)
    //------------------------------------------------------------
    // Lantern = the light source. Not drawn into the shadow map.
    // The bulb sits just in front of the light so its back (camera side) is lit.
    LanternPole = Add(MODEL_CUBE, R0, R0, XMFLOAT3(0.03f, 0.7f, 0.03f), XMFLOAT4(0.25f, 0.2f, 0.18f, 0.0f));
    LanternBulb = Add(MODEL_BALL, R0, R0, XMFLOAT3(0.3f, 0.3f, 0.3f), XMFLOAT4(1.0f, 0.9f, 0.55f, 0.0f));
    Object[LanternPole].NoShadow = true;
    Object[LanternBulb].NoShadow = true;

    // Bird puppet: flat paper parts (thin in Z, facing the wall) on a stick
    const XMFLOAT4 paperBlue(0.35f, 0.55f, 0.85f, 0.0f);
    const XMFLOAT4 stick(0.45f, 0.32f, 0.2f, 0.0f);
    BirdStart = NumObject;
    Add(MODEL_CUBE, R0, R0, XMFLOAT3(0.20f, 0.07f, 0.02f), paperBlue);    // 0 body
    Add(MODEL_CUBE, R0, R0, XMFLOAT3(0.075f, 0.075f, 0.02f), paperBlue);  // 1 head
    Add(MODEL_CUBE, R0, R0, XMFLOAT3(0.04f, 0.04f, 0.02f), XMFLOAT4(0.95f, 0.7f, 0.2f, 0.0f)); // 2 beak
    Add(MODEL_CUBE, R0, R0, XMFLOAT3(0.10f, 0.035f, 0.02f), paperBlue);   // 3 tail
    Add(MODEL_CUBE, R0, R0, XMFLOAT3(0.17f, 0.035f, 0.02f), paperBlue);   // 4 front wing
    Add(MODEL_CUBE, R0, R0, XMFLOAT3(0.17f, 0.035f, 0.02f), paperBlue);   // 5 back wing
    Add(MODEL_CUBE, R0, R0, XMFLOAT3(0.012f, 1.0f, 0.012f), stick);       // 6 stick

    // Rabbit puppet
    const XMFLOAT4 paperWhite(0.92f, 0.9f, 0.86f, 0.0f);
    RabbitStart = NumObject;
    Add(MODEL_CUBE, R0, R0, XMFLOAT3(0.18f, 0.11f, 0.02f), paperWhite);   // 0 body
    Add(MODEL_CUBE, R0, R0, XMFLOAT3(0.09f, 0.08f, 0.02f), paperWhite);   // 1 head
    Add(MODEL_CUBE, R0, R0, XMFLOAT3(0.028f, 0.13f, 0.02f), paperWhite);  // 2 ear
    Add(MODEL_CUBE, R0, R0, XMFLOAT3(0.028f, 0.13f, 0.02f), paperWhite);  // 3 ear
    Add(MODEL_CUBE, R0, R0, XMFLOAT3(0.05f, 0.05f, 0.02f), paperWhite);   // 4 tail
    Add(MODEL_CUBE, R0, R0, XMFLOAT3(0.03f, 0.07f, 0.02f), paperWhite);   // 5 front leg
    Add(MODEL_CUBE, R0, R0, XMFLOAT3(0.08f, 0.035f, 0.02f), paperWhite);  // 6 back leg
    Add(MODEL_CUBE, R0, R0, XMFLOAT3(0.012f, 1.0f, 0.012f), stick);       // 7 stick

    // Hidden pictures (positions are computed from the lantern position)
    AddReveal(0, REVEAL_START[0], REVEAL_LIGHT[0], HEART_BITMAP, 8, 0.0f, 0.85f, 0.13f,
              XMFLOAT4(0.9f, 0.3f, 0.35f, 0.0f), 1u);
    AddReveal(1, REVEAL_START[1], REVEAL_LIGHT[1], STAR_BITMAP, 10, 0.0f, 0.85f, 0.11f,
              XMFLOAT4(0.95f, 0.8f, 0.3f, 0.0f), 2u);

    Update(0.0f);
    return S_OK;
}

//************************************************************
// Finalize
//************************************************************
void ShadowDiorama::Finalize(void)
{
    for (int i = 0; i < MODEL_MAX; i++) ModelRelease(Model[i]);
    for (int i = 0; i < NumColor; i++)
    {
        if (ColorTex[i]) ColorTex[i]->Release();
    }
    NumColor = 0;
    VertexLayout->Release();
    VertexShader->Release();
    PixelShader->Release();
}

//************************************************************
// Update
//************************************************************
void ShadowDiorama::Update(float t)
{
    //------------------------------------------------------------
    // Olympic rings: scattered in the sky, gather at noon (t = 20), scatter again.
    // At night they fly away so the theatre screen is clear.
    //------------------------------------------------------------
    float night = NightRate(t);
    float gather = SmoothStep(12.0f, 17.0f, t) * (1.0f - SmoothStep(23.0f, 28.0f, t));
    for (int i = 0; i < 5; i++)
    {
        OBJECT& o = Object[RingStart + i];
        float a = XM_2PI * i / 5.0f + t * 0.35f;
        XMFLOAT3 fly(1.75f * cosf(a), 1.25f + 0.35f * sinf(t * 0.9f + i * 1.7f), 1.75f * sinf(a));
        XMFLOAT3 form(RING_FORMATION[i].x, RING_HEIGHT + ((i % 2) ? 0.02f : -0.02f), RING_FORMATION[i].y);
        o.Position = Lerp3(fly, form, gather);

        XMFLOAT3 spin(90.0f + 50.0f * sinf(t * 1.3f + i), t * 70.0f + i * 40.0f, 25.0f * cosf(t + i));
        XMFLOAT3 flat(90.0f, 0.0f, 0.0f);   // lying flat so the hole shows from above
        o.Rotate = Lerp3(spin, flat, gather);
        o.Position.y += 8.0f * night;
        o.Hidden = (night > 0.98f);
    }

    //------------------------------------------------------------
    // Sundial pillar sinks into the ground at night (it would cut the screen in half)
    //------------------------------------------------------------
    Object[GnomonIndex].Position.y = 0.51f - 0.9f * night;
    Object[GnomonTopIndex].Position.y = 1.04f - 0.9f * night;

    //------------------------------------------------------------
    // Character walking around the sundial
    //------------------------------------------------------------
    {
        float a = XM_2PI * t / WALK_PERIOD;
        float yaw = XMConvertToDegrees(-a);                 // local +Z faces the walking direction
        float step = t * XM_2PI * 1.6f;
        float swing = 35.0f * sinf(step) * (1.0f - night);
        float bob = 0.012f * fabsf(cosf(step)) * (1.0f - night);
        XMFLOAT3 root(WALK_RADIUS * cosf(a), bob, WALK_RADIUS * sinf(a));

        // at night he stands at the front and watches the show
        root = Lerp3(root, XMFLOAT3(-1.25f, 0.0f, -1.75f), night);
        if (night > 0.5f) yaw = 15.0f;

        OBJECT* w = &Object[WalkerStart];
        w[0].Position = Add3(root, RotY(XMFLOAT3(0, 0.22f, 0), yaw));  w[0].Rotate = XMFLOAT3(0, yaw, 0);
        w[1].Position = Add3(root, RotY(XMFLOAT3(0, 0.40f, 0), yaw));  w[1].Rotate = XMFLOAT3(0, yaw, 0);

        // limbs swing around the hip / shoulder
        const float side[2] = { -1.0f, 1.0f };
        for (int i = 0; i < 2; i++)
        {
            float legSwing = swing * side[i];
            XMFLOAT3 hip(0.04f * side[i], 0.13f, 0.0f);
            w[2 + i].Position = Add3(root, Add3(RotY(hip, yaw), RotXY(XMFLOAT3(0, -0.065f, 0), legSwing, yaw)));
            w[2 + i].Rotate = XMFLOAT3(legSwing, yaw, 0);

            float armSwing = -legSwing;
            XMFLOAT3 shoulder(0.09f * side[i], 0.30f, 0.0f);
            w[4 + i].Position = Add3(root, Add3(RotY(shoulder, yaw), RotXY(XMFLOAT3(0, -0.07f, 0), armSwing, yaw)));
            w[4 + i].Rotate = XMFLOAT3(armSwing, yaw, 0);
        }
        w[6].Position = Add3(root, XMFLOAT3(0, 0.48f, 0));  w[6].Rotate = XMFLOAT3(0, yaw, 0);
        w[7].Position = Add3(root, XMFLOAT3(0, 0.55f, 0));  w[7].Rotate = XMFLOAT3(0, yaw, 0);
    }

    //------------------------------------------------------------
    // Bouncing balls: watch the shadow shrink as the ball goes up
    //------------------------------------------------------------
    {
        const XMFLOAT2 ballPos[2] = { XMFLOAT2(0.8f, 0.75f), XMFLOAT2(-0.85f, 0.55f) };
        for (int i = 0; i < 2; i++)
        {
            float h = fabsf(sinf(t * XM_PI / (1.1f + 0.3f * i) + i)) * (1.0f - night);
            Object[BallStart + i].Position = XMFLOAT3(ballPos[i].x, 0.12f + 0.8f * h, ballPos[i].y);
        }
    }

    //------------------------------------------------------------
    // Rolling donut: distance / radius = rotation
    //------------------------------------------------------------
    {
        float radius = 1.25f * DONUT_SCALE;
        float x = WHEEL_RANGE * sinf(XM_2PI * t / 12.0f);
        x = Lerp(x, WHEEL_RANGE, night);   // parks at the side at night
        OBJECT& o = Object[WheelIndex];
        o.Position = XMFLOAT3(x, radius, WHEEL_Z);
        o.Rotate = XMFLOAT3(0, 0, XMConvertToDegrees(-x / radius));
    }

    //------------------------------------------------------------
    // Trees sway in the wind
    //------------------------------------------------------------
    for (int i = 0; i < TreeCount; i++)
    {
        Object[TreeStart + i].Rotate.z = 3.0f * sinf(t * 1.7f + i * 1.3f);
    }

    //------------------------------------------------------------
    // Windows light up at night
    //------------------------------------------------------------
    for (int i = 0; i < WindowCount; i++)
    {
        Object[WindowStart + i].Texture = (night > 0.5f) ? WindowNightTex : WindowDayTex;
    }

    //------------------------------------------------------------
    // Lantern
    //------------------------------------------------------------
    {
        XMFLOAT3 L = NightLightPosition(t);
        float poleH = L.y - 0.1f;
        OBJECT& pole = Object[LanternPole];
        pole.Position = XMFLOAT3(L.x, poleH * 0.5f, L.z - 0.05f);
        pole.Scale = XMFLOAT3(0.03f, poleH * night, 0.03f);
        pole.Hidden = (night < 0.02f);

        OBJECT& bulb = Object[LanternBulb];
        bulb.Position = XMFLOAT3(L.x, L.y, L.z + 0.1f);
        bulb.Scale = XMFLOAT3(0.3f * night, 0.3f * night, 0.3f * night);
        bulb.Hidden = (night < 0.02f);
    }

    UpdatePuppets(t);
    UpdateReveals(t);
}

//************************************************************
// Shadow puppets
//************************************************************
void ShadowDiorama::UpdatePuppets(float t)
{
    //------------------------------------------------------------
    // Bird: flies left -> right in front of the lantern, flapping (44 - 54 s)
    //------------------------------------------------------------
    {
        bool hidden = (t < 43.6f || t > 54.4f);
        float x = Lerp(-2.6f, 2.6f, (t - 44.0f) / 10.0f);
        XMFLOAT3 B(x, 1.05f + 0.08f * sinf(t * 3.0f), 0.2f);
        OBJECT* b = &Object[BirdStart];

        b[0].Position = B;                                       b[0].Rotate = XMFLOAT3(0, 0, 5.0f * sinf(t * 3.0f));
        b[1].Position = Add3(B, XMFLOAT3(0.12f, 0.035f, 0.0f));  b[1].Rotate = XMFLOAT3(0, 0, 0);
        b[2].Position = Add3(B, XMFLOAT3(0.175f, 0.03f, 0.0f));  b[2].Rotate = XMFLOAT3(0, 0, 45.0f);
        b[3].Position = Add3(B, XMFLOAT3(-0.135f, 0.015f, 0.0f)); b[3].Rotate = XMFLOAT3(0, 0, 25.0f);

        // wings rotate around the shoulder
        XMFLOAT3 shoulder = Add3(B, XMFLOAT3(0.0f, 0.03f, 0.0f));
        for (int w = 0; w < 2; w++)
        {
            float phi = 35.0f + 50.0f * sinf(t * 8.0f) - 15.0f * w;     // wing angle (up = +)
            float rad = XMConvertToRadians(phi);
            b[4 + w].Position = Add3(shoulder, XMFLOAT3(-0.085f * cosf(rad), 0.085f * sinf(rad), w ? -0.015f : 0.015f));
            b[4 + w].Rotate = XMFLOAT3(0, 0, -phi);
        }

        // stick from the floor to the bird
        float stickH = B.y - 0.03f;
        b[6].Position = XMFLOAT3(B.x, stickH * 0.5f, B.z + 0.015f);
        b[6].Scale = XMFLOAT3(0.012f, stickH, 0.012f);

        for (int i = 0; i < 7; i++) b[i].Hidden = hidden;
    }

    //------------------------------------------------------------
    // Rabbit: hops right -> left (76 - 86 s)
    //------------------------------------------------------------
    {
        bool hidden = (t < 75.6f || t > 86.4f);
        float x = Lerp(2.6f, -2.6f, (t - 76.0f) / 10.0f);
        float phase = (t - 76.0f) / 0.75f;
        float h = 0.2f * fabsf(sinf(XM_PI * phase));
        XMFLOAT3 B(x, 0.6f + h, 0.3f);
        float tilt = -8.0f * cosf(XM_PI * phase);
        float ear = 8.0f * sinf(t * 7.0f);
        OBJECT* r = &Object[RabbitStart];

        r[0].Position = Add3(B, XMFLOAT3(0.0f, 0.07f, 0.0f));     r[0].Rotate = XMFLOAT3(0, 0, tilt);
        r[1].Position = Add3(B, XMFLOAT3(-0.11f, 0.14f, 0.0f));   r[1].Rotate = XMFLOAT3(0, 0, 0);
        r[2].Position = Add3(B, XMFLOAT3(-0.10f, 0.25f, 0.0f));   r[2].Rotate = XMFLOAT3(0, 0, -10.0f + ear);
        r[3].Position = Add3(B, XMFLOAT3(-0.135f, 0.235f, 0.01f)); r[3].Rotate = XMFLOAT3(0, 0, -28.0f - ear);
        r[4].Position = Add3(B, XMFLOAT3(0.105f, 0.09f, 0.0f));   r[4].Rotate = XMFLOAT3(0, 0, 45.0f);
        r[5].Position = Add3(B, XMFLOAT3(-0.065f, 0.0f, 0.0f));   r[5].Rotate = XMFLOAT3(0, 0, 20.0f * sinf(XM_PI * phase));
        r[6].Position = Add3(B, XMFLOAT3(0.06f, 0.005f, 0.0f));   r[6].Rotate = XMFLOAT3(0, 0, -10.0f * sinf(XM_PI * phase));

        float stickH = B.y;
        r[7].Position = XMFLOAT3(B.x, stickH * 0.5f, B.z + 0.015f);
        r[7].Scale = XMFLOAT3(0.012f, stickH, 0.012f);

        for (int i = 0; i < 8; i++) r[i].Hidden = hidden;
    }
}

//************************************************************
// Hidden pictures
//************************************************************
void ShadowDiorama::UpdateReveals(float t)
{
    for (int k = 0; k < NUM_REVEAL; k++)
    {
        const REVEAL& rv = Reveal[k];
        float rel = t - rv.Start;

        for (int i = 0; i < rv.PieceCount; i++)
        {
            const PIECE& p = Piece[rv.PieceStart + i];
            OBJECT& o = Object[rv.ObjectStart + i];

            float grow = SmoothStep(p.Delay, p.Delay + 0.6f, rel) * (1.0f - SmoothStep(10.0f, REVEAL_LENGTH, rel));
            o.Hidden = (rel < 0.0f || rel > REVEAL_LENGTH || grow < 0.001f);
            if (o.Hidden) continue;

            float arrive = SmoothStep(p.Delay, p.Delay + 3.0f, rel);   // all settled at rel = 4
            float fall = SmoothStep(9.0f, 10.5f, rel);

            XMFLOAT3 pos = Lerp3(p.Scatter, p.Final, arrive);
            pos.y = Lerp(pos.y, p.Size * 0.5f, fall);
            o.Position = pos;

            float spin = (1.0f - arrive) * 360.0f;
            o.Rotate = XMFLOAT3(spin * (0.5f + p.Delay), spin, fall * 60.0f * p.Delay);

            float size = p.Size * grow;
            o.Scale = XMFLOAT3(size, size, size);
        }
    }
}

//************************************************************
// Draw
//************************************************************
void ShadowDiorama::Draw(bool depthPass)
{
    GetDeviceContext()->IASetInputLayout(VertexLayout);
    GetDeviceContext()->VSSetShader(VertexShader, NULL, 0);

    if (depthPass)
    {
        // Shadow map only needs depth, so no pixel shader
        GetDeviceContext()->PSSetShader(NULL, NULL, 0);
    }
    else
    {
        GetDeviceContext()->PSSetShader(PixelShader, NULL, 0);
        ID3D11ShaderResourceView* tex = GetDepthShadowShaderResourceView();
        GetDeviceContext()->PSSetShaderResources(1, 1, &tex);
    }

    MATERIAL material;
    ZeroMemory(&material, sizeof(MATERIAL));
    material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    SetMaterial(material);

    for (int i = 0; i < NumObject; i++)
    {
        const OBJECT& o = Object[i];
        if (o.Hidden) continue;
        if (depthPass && o.NoShadow) continue;

        // Move the model's origin to its center / bottom first
        XMFLOAT3 pivot(0.0f, 0.0f, 0.0f);
        if (o.Model == MODEL_CUBE) pivot.z = CubePivotZ;
        if (o.Model == MODEL_TREE) pivot.y = 0.5f;      // trunk bottom at y = 0

        XMMATRIX world =
            XMMatrixTranslation(pivot.x, pivot.y, pivot.z) *
            XMMatrixScaling(o.Scale.x, o.Scale.y, o.Scale.z) *
            XMMatrixRotationRollPitchYaw(
                XMConvertToRadians(o.Rotate.x),
                XMConvertToRadians(o.Rotate.y),
                XMConvertToRadians(o.Rotate.z)) *
            XMMatrixTranslation(o.Position.x, o.Position.y, o.Position.z);
        SetWorldMatrix(world);

        if (!depthPass) GetDeviceContext()->PSSetShaderResources(0, 1, &o.Texture);

        ModelDraw(Model[o.Model]);
    }
}
