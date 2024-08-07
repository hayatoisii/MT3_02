#include <Novice.h>
#include<cmath>
#include <assert.h>
#include<MTk.h>
#include<ImGuiManager.h>
#include <algorithm>
#define PI 3.14159265359f

const char kWindowTitle[] = "LC1C_02_イシイハヤト_タイトル";

struct TransformSRT {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

struct AABB {
	Vector3 min;
	Vector3 max;
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

	const float kGridHalfWidth = 2.0f;
	const uint32_t kSubdivision = 20;
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

	Novice::DrawLine((int)points[0].x, (int)points[0].y, (int)points[2].x, (int)points[2].y, color);
	Novice::DrawLine((int)points[0].x, (int)points[0].y, (int)points[3].x, (int)points[3].y, color);

	Novice::DrawLine((int)points[1].x, (int)points[1].y, (int)points[2].x, (int)points[2].y, color);
	Novice::DrawLine((int)points[1].x, (int)points[1].y, (int)points[3].x, (int)points[3].y, color);

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

Vector3 Project(const Vector3& v1, const Vector3& v2) {

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
struct Triangle {
	Vector3 vertices[3];
};

bool IsCollision(const Triangle& triangle, const Segment& segment) {

	Vector3 edge[2];
	edge[0] = Subtract(triangle.vertices[1], triangle.vertices[0]);
	edge[1] = Subtract(triangle.vertices[2], triangle.vertices[0]);

	Vector3 normal = Cross(edge[0], edge[1]);
	normal = Normalize(normal);

	Vector3 dir = segment.diff;
	dir = Normalize(dir);

	Vector3 diff = Subtract(triangle.vertices[0], segment.origin);

	float dotND = Dot(normal, dir);

	if (fabs(dotND) < 1e-6f) {
		return false;
	}

	float t = Dot(normal, diff) / dotND;
	if (t < 0.0f || t > 1.0f) {
		return false;
	}

	Vector3 intersection = {
	   segment.origin.x + t * dir.x,
	   segment.origin.y + t * dir.y,
	   segment.origin.z + t * dir.z
	};


	Vector3 cross01 = Cross(Subtract(triangle.vertices[1], triangle.vertices[0]), Subtract(intersection, triangle.vertices[0]));
	Vector3 cross12 = Cross(Subtract(triangle.vertices[2], triangle.vertices[1]), Subtract(intersection, triangle.vertices[1]));
	Vector3 cross20 = Cross(Subtract(triangle.vertices[0], triangle.vertices[2]), Subtract(intersection, triangle.vertices[2]));


	if (Dot(cross01, normal) >= 0.0f &&
		Dot(cross12, normal) >= 0.0f &&
		Dot(cross20, normal) >= 0.0f)
	{
		return true;

	}
	return false;
}
void DrawTriangle(const Triangle triangle, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {

	Vector3 A, B, C = {};
	A = Transform(triangle.vertices[0], Multiply(viewProjectionMatrix, viewportMatrix));
	B = Transform(triangle.vertices[1], Multiply(viewProjectionMatrix, viewportMatrix));
	C = Transform(triangle.vertices[2], Multiply(viewProjectionMatrix, viewportMatrix));

	Novice::DrawLine((int)A.x, (int)A.y, (int)B.x, (int)B.y, color);
	Novice::DrawLine((int)B.x, (int)B.y, (int)C.x, (int)C.y, color);
	Novice::DrawLine((int)C.x, (int)C.y, (int)A.x, (int)A.y, color);
}

bool IsCollision(const Sphere& sphere, const Plane& plane) {

	float k = (plane.normal.x * sphere.center.x + plane.normal.y * sphere.center.y + plane.normal.z * sphere.center.z) - plane.distance;
	k = fabs(k);

	if (k <= sphere.radius) {
		return true;
	}
	else {
		return false;
	}
}

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
	Vector3 cameraRotate{ 2.6f,0.0f,0.0f };

	Sphere sphere;
	sphere.center = {};
	sphere.radius = 1.0f;

	Plane plane;
	plane.distance = {};
	plane.normal = { 0.0f,1.0f,0.0f };


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

		 // 更新
		Matrix4x4 worldMatrix = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, rotate, translate);
		Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraScale, cameraRotate, cameraPosition);
		Matrix4x4 projectionMatrix = MakePerspectiveMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(cameraMatrix, projectionMatrix));
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0, 0, 1280.0f, 720.0f, 0.0f, 1.0f);

		isCollision = IsCollision(sphere, plane);

		if (!isCollision) {
			color = WHITE;
		}
		else {
			color = RED;
		}

		plane.normal = Normalize(plane.normal);


		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		ImGui::DragFloat3("cameraPos", &cameraPosition.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("cameraScale", &cameraScale.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("cameraRotate", &cameraRotate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("sphere.pos", &sphere.center.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("Plane.normal", &plane.normal.x, 0.01f);
		ImGui::DragFloat("Plane.distance", &plane.distance, 0.01f);

		 // グリッドの描画
		DrawGrid(worldViewProjectionMatrix, viewportMatrix);
		DrawSphere(sphere, worldViewProjectionMatrix, viewportMatrix, color);

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