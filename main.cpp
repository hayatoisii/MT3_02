#include <Novice.h>
#include<cmath>
#include <assert.h>
#include<MTk.h>
#include<ImGuiManager.h>
#define PI 3.14159265359f

const char kWindowTitle[] = "LC1C_02_イシイハヤト_タイトル";

struct  Line
{
	Vector3 origin;//始点
	Vector3 diff;//終点への差分ベクトル
};


struct  Ray
{
	Vector3 origin;//始点
	Vector3 diff;//終点への差分ベクトル
};

struct  Segment
{
	Vector3 origin;//始点
	Vector3 diff;//終点への差分ベクトル
};

struct  Sphere
{
	Vector3 center;
	float radius;
};

void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {

	const float kGridHalfWidth = 2.0f;//Gridの半分の幅
	const uint32_t kSubdivision = 10;//分割数
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision);//ひとつ分の長さ

	for (uint32_t xIndex = 0; xIndex <= kSubdivision; ++xIndex) {
		float x = -kGridHalfWidth + xIndex * kGridEvery;

		Vector3 startX = { x, 0.0f, -kGridHalfWidth };
		Vector3 endX = { x, 0.0f, kGridHalfWidth };

		startX = Transform(startX, Multiply(viewProjectionMatrix, viewportMatrix));
		endX = Transform(endX, Multiply(viewProjectionMatrix, viewportMatrix));

		for (uint32_t zIndex = 0; zIndex <= kSubdivision; ++zIndex) {
			float z = -kGridHalfWidth + zIndex * kGridEvery;

			Vector3 startZ = { -kGridHalfWidth, 0.0f, z };
			Vector3 endZ = { kGridHalfWidth, 0.0f, z };

			startZ = Transform(startZ, Multiply(viewProjectionMatrix, viewportMatrix));
			endZ = Transform(endZ, Multiply(viewProjectionMatrix, viewportMatrix));

			Novice::DrawLine((int)startX.x, (int)startX.y, (int)endX.x, (int)endX.y, 0xaaaaaaff);
			Novice::DrawLine((int)startZ.x, (int)startZ.y, (int)endZ.x, (int)endZ.y, 0xaaaaaaff);

		}
	}
}

void DrawSphere(Sphere sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {

	const uint32_t kSubdivision = 16;
	float pi = PI;

	const float kLonEvery = pi * 2.0f / float(kSubdivision);
	const float kLatEvery = pi / float(kSubdivision);
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -pi / 2.0f + kLatEvery * latIndex;
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {

			float lon = lonIndex * kLonEvery;

			Vector3 A, B, C;

			A = { sphere.center.x + sphere.radius * cos(lat) * cos(lon),
				sphere.center.y + sphere.radius * sin(lat),
				sphere.center.z + sphere.radius * cos(lat) * sin(lon) };

			B = { sphere.center.x + sphere.radius * cos(lat + kLatEvery) * cos(lon),
				sphere.center.y + sphere.radius * sin(lat + kLatEvery)
				,sphere.center.z + sphere.radius * cos(lat + kLatEvery) * sin(lon) };

			C = { sphere.center.x + sphere.radius * cos(lat) * cos(lon + kLonEvery),
				sphere.center.y + sphere.radius * sin(lat),
				sphere.center.z + sphere.radius * cos(lat) * sin(lon + kLonEvery) };

			A = Transform(A, Multiply(viewProjectionMatrix, viewportMatrix));
			B = Transform(B, Multiply(viewProjectionMatrix, viewportMatrix));
			C = Transform(C, Multiply(viewProjectionMatrix, viewportMatrix));

			Novice::DrawLine((int)A.x, (int)A.y, (int)B.x, (int)B.y, color);
			Novice::DrawLine((int)A.x, (int)A.y, (int)C.x, (int)C.y, color);

		}
	}
}

bool IsCollision(const Sphere& s1, const Sphere& s2) {

	float distance = Length(s2.center - s1.center);

	if (distance <= s1.radius + s2.radius) {
		return true;
	}
	else {
		return false;
	}
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	Vector3 rotate = {};
	Vector3 translate{};

	Vector3 cameraPosition{ 0.0f,-0.2f,-6.49f };
	Vector3 cameraScale{ 1.0f, 1.0f, 1.0f };
	Vector3 cameraRotate{ 2.6f,0.0f,0.0f };

	Sphere sphere[2];
	sphere[0].center = { 1.0f };
	sphere[0].radius = 1.0f;

	sphere[1].center = { -1.0 };
	sphere[1].radius = 0.5f;

	bool isCollision;
	unsigned int color = 0xffffffff;

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		Matrix4x4 worldMatrix = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, rotate, translate);
		Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraScale, cameraRotate, cameraPosition);
		Matrix4x4 projectionMatrix = MakePerspectiveMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(cameraMatrix, projectionMatrix));
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0, 0, 1280.0f, 720.0f, 0.0f, 1.0f);

		isCollision = IsCollision(sphere[0], sphere[1]);

		if (isCollision) {
			color = 0x00ffffff;
		}
		else {
			color = 0xffffffff;
		}

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		ImGui::DragFloat3("cameraPos", &cameraPosition.x, 0.01f, -30.0f, 10.0f);
		ImGui::DragFloat3("cameraScale", &cameraScale.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("cameraRotate", &cameraRotate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("sphre[0].pos", &sphere[0].center.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("sphre[1].pos", &sphere[1].center.x, 0.01f, -10.0f, 10.0f);

		// グリッドの描画
		DrawGrid(worldViewProjectionMatrix, viewportMatrix);

		//
		DrawSphere(sphere[0], worldViewProjectionMatrix, viewportMatrix, 0xffffffff);
		DrawSphere(sphere[1], worldViewProjectionMatrix, viewportMatrix, color);

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}