#include <Novice.h>
#include<cmath>
#include <assert.h>
#include<MTk.h>
#include<ImGuiManager.h>
#define PI 3.14159265359f

const char kWindowTitle[] = "LC1C_02_イシイハヤト_タイトル";

struct TransformSRT {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

struct  Sphere
{
	Vector3 center;
	float radius;
};

struct  Plane
{
	Vector3 normal;
	float distance;
};

//直線
struct  Line
{
	Vector3 origin;//始点
	Vector3 diff;//終点への差分ベクトル
};

//半直線
struct  Ray
{
	Vector3 origin;//始点
	Vector3 diff;//終点への差分ベクトル
};

//線分
struct  Segment
{
	Vector3 origin;//始点
	Vector3 diff;//終点への差分ベクトル
};

//直線と平面の当たり判定
bool IsCollision(const Line& line, const Plane& plane) {
	float dot = Dot(plane.normal, line.diff);

	if (dot == 0.0f) {
		return false;
	}

	float t = (plane.distance - Dot(line.origin, plane.normal)) / dot;

	if (t >= 0 && t <= 1) {
		return true;
	}

	if (t == -1.0f) {
		return true;
	}

	if (t == 2.0f) {
		return true;
	}

	return false;


}


//半直線と平面の当たり判定
bool IsCollision(const Ray& ray, const Plane& plane) {
	float dot = Dot(plane.normal, ray.diff);

	if (dot == 0.0f) {
		return false;
	}

	float t = (plane.distance - Dot(ray.origin, plane.normal)) / dot;

	if (t >= 0 && t <= 1) {
		return true;
	}

	if (t == -1.0f) {
		return false;
	}

	if (t == 2.0f) {
		return true;
	}

	return false;

}


//線分と平面の当たり判定
bool IsCollision(const Segment& segment, const Plane& plane) {
	float dot = Dot(plane.normal, segment.diff);

	if (dot == 0.0f) {
		return false;
	}

	float t = (plane.distance - Dot(segment.origin, plane.normal)) / dot;

	if (t >= 0 && t <= 1) {
		return true;
	}

	if (t == -1.0f) {
		return false;
	}

	if (t == 2.0f) {
		return false;
	}

	return false;
}

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

Vector3 Perpendicular(const Vector3& vector) {
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return { -vector.y,vector.x,0.0f };
	}
	return { 0.0f,-vector.z,vector.y };
}

void DrawPlane(const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 center = Multiply(plane.distance, plane.normal);
	Vector3 perpendiculars[4];
	perpendiculars[0] = Normalize(Perpendicular(plane.normal));
	perpendiculars[1] = { -perpendiculars[0].x,-perpendiculars[0].y,-perpendiculars[0].z };
	perpendiculars[2] = Cross(plane.normal, perpendiculars[0]);
	perpendiculars[3] = { -perpendiculars[2].x,-perpendiculars[2].y,-perpendiculars[2].z };

	Vector3 points[4];
	for (uint32_t index = 0; index < 4; index++) {
		Vector3 extend = Multiply(2.0f, perpendiculars[index]);
		Vector3 point = Add(center, extend);
		points[index] = Transform(Transform(point, viewProjectionMatrix), viewportMatrix);
	}
	//Novice::DrawLine((int)points[0].x, (int)points[0].y,(int) points[1].x,(int) points[1].y, color);
	Novice::DrawLine((int)points[0].x, (int)points[0].y, (int)points[2].x, (int)points[2].y, color);
	Novice::DrawLine((int)points[0].x, (int)points[0].y, (int)points[3].x, (int)points[3].y, color);


	Novice::DrawLine((int)points[1].x, (int)points[1].y, (int)points[2].x, (int)points[2].y, color);
	Novice::DrawLine((int)points[1].x, (int)points[1].y, (int)points[3].x, (int)points[3].y, color);

	//Novice::DrawLine((int)points[2].x, (int)points[2].y,(int) points[3].x,(int) points[3].y, color);


}
void DrawSphere(Sphere sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {

	const uint32_t kSubdivision = 8;


	float pi = PI;

	//経度分数1つ分の角度
	const float kLonEvery = pi * 2.0f / float(kSubdivision);
	//緯度分数1つ分の角度
	const float kLatEvery = pi / float(kSubdivision);
	//経度の方向に分割
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -pi / 2.0f + kLatEvery * latIndex;
		//経度の方向に分割しながら線を描く
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			//uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;

			float lon = lonIndex * kLonEvery;

			Vector3 A, B, C;


			//A
			A = { sphere.center.x + sphere.radius * cos(lat) * cos(lon),
				sphere.center.y + sphere.radius * sin(lat),
				sphere.center.z + sphere.radius * cos(lat) * sin(lon) };
			//B
			B = { sphere.center.x + sphere.radius * cos(lat + kLatEvery) * cos(lon),
				sphere.center.y + sphere.radius * sin(lat + kLatEvery)
				,sphere.center.z + sphere.radius * cos(lat + kLatEvery) * sin(lon) };
			// 
			//C
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

Vector3 Project(const Vector3& v1, const Vector3& v2) {

	// v1 を v2 に正射影する
	float dotProduct = v1.dot(v2);
	float v2LengthSquared = v2.dot(v2);
	return v2.scale(dotProduct / v2LengthSquared);

}

Vector3 ClosestPoint(const Vector3& point, const Segment& segment) {
	Vector3 segmentVec = segment.diff;
	Vector3 pointToOrigin = Subtract(point, segment.origin);
	float t = Dot(pointToOrigin, segmentVec) / Dot(segmentVec, segmentVec);

	Vector3 a = Add(segment.origin, Multiply(t, segmentVec));

	return a;
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

	Vector3 cameraPosition{ 0.0f,-.0f,-6.49f };
	Vector3 cameraScale{ 1.0f, 1.0f, 1.0f };
	Vector3 cameraRotate{ -3.140f,0.0f,0.0f };

	Plane plane;
	plane.distance = {};
	plane.normal = { 0.0f,1.0f,0.0f };

	Ray ray{ {-2.0f,-1.0f,0.0f},{3.0f,2.0f,2.0f} };

	bool isCollision{};

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

		 // 更新
		Matrix4x4 worldMatrix = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, rotate, translate);
		Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraScale, cameraRotate, cameraPosition);
		Matrix4x4 projectionMatrix = MakePerspectiveMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(cameraMatrix, projectionMatrix));
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0, 0, 1280.0f, 720.0f, 0.0f, 1.0f);

		isCollision = IsCollision(ray, plane);

		if (isCollision) {
			color = 0x00ffffff;
		}
		else {
			color = 0xffffffff;
		}

		Vector3 start = Transform(Transform(ray.origin, worldViewProjectionMatrix), viewportMatrix);
		Vector3 end = Transform(Transform(Add(ray.origin, ray.diff), worldViewProjectionMatrix), viewportMatrix);

		ImGui::DragFloat3("cameraPos", &cameraPosition.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("cameraScale", &cameraScale.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("cameraRotate", &cameraRotate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("Plane.normal", &plane.normal.x, 0.01f);
		ImGui::DragFloat("Plane.distance", &plane.distance, 0.01f);
		ImGui::DragFloat3("ray.origin", &ray.origin.x, 0.01f);
		ImGui::DragFloat3("ray.diff", &ray.diff.x, 0.01f);

		plane.normal = Normalize(plane.normal);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		Novice::DrawLine(int(start.x), int(start.y), int(end.x), int(end.y), WHITE);
		// グリッドの描画
		DrawGrid(worldViewProjectionMatrix, viewportMatrix);

		DrawPlane(plane, worldViewProjectionMatrix, viewportMatrix, color);

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