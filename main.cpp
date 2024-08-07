#include <Novice.h>
#include<MTk.h>
#include <assert.h>
#define NOMINMAX
#include <cmath>
#include <algorithm> 
#include<ImGuiManager.h>
# define PI 3.14159265359f

const char kWindowTitle[] = "LC1C_02_イシイハヤト_タイトル";

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

struct OBB {
	Vector3 center;
	Vector3 orientations[3];
	Vector3 size;
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
	const uint32_t kSubdivision = 16;
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision);

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

	const uint32_t kSubdivision = 8;

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

bool IsCollision(const AABB& a, const AABB& b) {

	if ((a.min.x <= b.max.x && a.max.x >= b.min.x) &&
		(a.min.y <= b.max.y && a.max.y >= b.min.y) &&
		(a.min.z <= b.max.z && a.max.z >= b.min.z)) {
		return true;
	}
	return false;
}

void DrawAABB(const AABB& aabb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {

	Vector3 vertices[8]{};

	vertices[0].x = aabb.min.x;	vertices[1].x = aabb.max.x;	vertices[2].x = aabb.max.x;	vertices[2].x = aabb.max.x;
	vertices[0].y = aabb.max.y;	vertices[1].y = aabb.max.y;	vertices[2].y = aabb.min.y;	vertices[2].y = aabb.min.y;
	vertices[0].z = aabb.min.z;	vertices[1].z = aabb.min.z;	vertices[2].z = aabb.min.z;	vertices[2].z = aabb.min.z;

	vertices[3].x = aabb.min.x;	vertices[4].x = aabb.min.x;	vertices[5].x = aabb.max.x;	vertices[6].x = aabb.max.x;
	vertices[3].y = aabb.min.y;	vertices[4].y = aabb.max.y;	vertices[5].y = aabb.max.y;	vertices[6].y = aabb.min.y;
	vertices[3].z = aabb.min.z;	vertices[4].z = aabb.max.z;	vertices[5].z = aabb.max.z;	vertices[6].z = aabb.max.z;

	vertices[7].x = aabb.min.x;
	vertices[7].y = aabb.min.y;
	vertices[7].z = aabb.max.z;

	for (int i = 0; i < 8; i++) {
		vertices[i] = Transform(vertices[i], Multiply(viewProjectionMatrix, viewportMatrix));
	}

	Novice::DrawLine((int)vertices[0].x, (int)vertices[0].y, (int)vertices[1].x, (int)vertices[1].y, color);
	Novice::DrawLine((int)vertices[1].x, (int)vertices[1].y, (int)vertices[2].x, (int)vertices[2].y, color);
	Novice::DrawLine((int)vertices[2].x, (int)vertices[2].y, (int)vertices[3].x, (int)vertices[3].y, color);
	Novice::DrawLine((int)vertices[3].x, (int)vertices[3].y, (int)vertices[0].x, (int)vertices[0].y, color);

	Novice::DrawLine((int)vertices[4].x, (int)vertices[4].y, (int)vertices[5].x, (int)vertices[5].y, color);
	Novice::DrawLine((int)vertices[5].x, (int)vertices[5].y, (int)vertices[6].x, (int)vertices[6].y, color);
	Novice::DrawLine((int)vertices[6].x, (int)vertices[6].y, (int)vertices[7].x, (int)vertices[7].y, color);
	Novice::DrawLine((int)vertices[7].x, (int)vertices[7].y, (int)vertices[4].x, (int)vertices[4].y, color);

	Novice::DrawLine((int)vertices[0].x, (int)vertices[0].y, (int)vertices[4].x, (int)vertices[4].y, color);
	Novice::DrawLine((int)vertices[1].x, (int)vertices[1].y, (int)vertices[5].x, (int)vertices[5].y, color);
	Novice::DrawLine((int)vertices[2].x, (int)vertices[2].y, (int)vertices[6].x, (int)vertices[6].y, color);
	Novice::DrawLine((int)vertices[3].x, (int)vertices[3].y, (int)vertices[7].x, (int)vertices[7].y, color);

}

bool IsCollision(const AABB& aabb, const Sphere& sphere) {

	Vector3 closestPoint
	{
		std::clamp(sphere.center.x,aabb.min.x,aabb.max.x),
		std::clamp(sphere.center.y,aabb.min.y,aabb.max.y),
		std::clamp(sphere.center.z,aabb.min.z,aabb.max.z)
	};

	float distance = Length(closestPoint - sphere.center);
	if (distance <= sphere.radius) {
		return true;
	}
	return false;
}


bool IsCollision(const AABB& aabb, const Segment& segment) {

	float tNearX = (aabb.min.x - segment.origin.x) / segment.diff.x;
	float tFarX = (aabb.max.x - segment.origin.x) / segment.diff.x;

	if (std::isnan(tNearX) || std::isnan(tFarX)) return false; 
	if (tNearX > tFarX) std::swap(tNearX, tFarX);

	float tNearY = (aabb.min.y - segment.origin.y) / segment.diff.y;
	float tFarY = (aabb.max.y - segment.origin.y) / segment.diff.y;

	if (std::isnan(tNearY) || std::isnan(tFarY)) return false;
	if (tNearY > tFarY) std::swap(tNearY, tFarY);

	float tNearZ = (aabb.min.z - segment.origin.z) / segment.diff.z;
	float tFarZ = (aabb.max.z - segment.origin.z) / segment.diff.z;

	if (std::isnan(tNearZ) || std::isnan(tFarZ)) return false;
	if (tNearZ > tFarZ) std::swap(tNearZ, tFarZ);

	float tmin = max(max(tNearX, tNearY), tNearZ);
	float tmax = min(min(tFarX, tFarY), tFarZ);

	if (std::isnan(tmin) || std::isnan(tmax)) return false;

	if (tmin <= tmax && tmax >= 0.0f && tmin <= 1.0f) {
		return true;
	}
	return false;
}

Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t) {
	return v1 + (v2 - v1) * t;
}
Vector3 Bezier(const Vector3& p0, const Vector3& p1, const Vector3& p2, float t) {
	Vector3 p0p1 =
		Lerp(p0, p1, t);
	Vector3 p1p2 =
		Lerp(p1, p2, t);

	return Lerp(p0p1, p1p2, t);
}

void DrawBezier(const Vector3& controlPoint0, const Vector3& controlPoint1, const Vector3& controlPoint2,
	const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {

	int split = 32;
	for (int i = 0; i < split; i++) {
		float t0 = i / (float)split;
		float t1 = (i + 1) / (float)split;

		Vector3 bezier0 = Bezier(controlPoint0, controlPoint1, controlPoint2, t0);
		Vector3 bezier1 = Bezier(controlPoint0, controlPoint1, controlPoint2, t1);

		Vector3 bazierLine[2];
		bazierLine[0] = Transform(bezier0, Multiply(viewProjectionMatrix, viewportMatrix));
		bazierLine[1] = Transform(bezier1, Multiply(viewProjectionMatrix, viewportMatrix));

		Novice::DrawLine((int)bazierLine[0].x, (int)bazierLine[0].y, (int)bazierLine[1].x, (int)bazierLine[1].y, color);
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

	Vector3 translates[3] = {
		{-0.4f, 0.0f, 0.0f}, // 左側
		{ 0.0f, 0.0f, 0.0f}, // 中央
		{ 0.4f, 0.0f, 0.0f}  // 右側
	};

	Vector3 rotates[3] = {
		{0.0f,0.0f,-1.0f},
		{0.0f,0.0f,-1.0f},
		{0.0f,0.0f,0.0f}
	};

	Vector3 scales[3] = {
		{1.0f,1.0f,1.0f},
		{1.0f,1.0f,1.0f},
		{1.0f,1.0f,1.0f}
	};
	Vector3 cameraPosition{ 0.0f,-0.0f,-6.49f };
	Vector3 cameraScale{ 1.0f, 1.0f, 1.0f };
	Vector3 cameraRotate{ 2.6f,0.0f,0.0f };

	Sphere sphere[3];

	sphere[0].center = translates[0];
	sphere[0].radius = 0.1f;

	sphere[1].center = translates[1];
	sphere[1].radius = 0.1f;

	sphere[2].center = translates[2];
	sphere[2].radius = 0.1f;

	Vector3 shoulderTranslate = translates[0];
	Vector3 elbowTranslate = translates[1];
	Vector3 handTranslate = translates[2];

	Vector3 shoulderScale = scales[0];
	Vector3 elbowScale = scales[1];
	Vector3 handScale = scales[2];

	Vector3 shoulderRotate = rotates[0];
	Vector3 elbowRotate = rotates[1];
	Vector3 handRotate = rotates[2];

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
		
		shoulderTranslate = translates[0];
		elbowTranslate = translates[1];
		handTranslate = translates[2];

		shoulderScale = scales[0];
		elbowScale = scales[1];
		handScale = scales[2];

		shoulderRotate = rotates[0];
		elbowRotate = rotates[1];
		handRotate = rotates[2];


		Matrix4x4 worldMatrix = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, rotate, translate);
		Matrix4x4 shoulderWorldMatrix = MakeAffineMatrix(shoulderScale, shoulderRotate, shoulderTranslate);
		Matrix4x4 elbowWorldMatrix = Multiply(MakeAffineMatrix(elbowScale, elbowRotate, elbowTranslate), shoulderWorldMatrix);
		Matrix4x4 handWorldMatrix = Multiply(MakeAffineMatrix(handScale, handRotate, handTranslate), elbowWorldMatrix);
		Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraScale, cameraRotate, cameraPosition);
		Matrix4x4 projectionMatrix = MakePerspectiveMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
		Matrix4x4 shoulderViewProjectionMatrix = Multiply(shoulderWorldMatrix, Multiply(cameraMatrix, projectionMatrix));
		Matrix4x4 elbowViewProjectionMatrix = Multiply(elbowWorldMatrix, Multiply(cameraMatrix, projectionMatrix));
		Matrix4x4 handViewProjectionMatrix = Multiply(handWorldMatrix, Multiply(cameraMatrix, projectionMatrix));
		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(cameraMatrix, projectionMatrix));
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0, 0, 1280.0f, 720.0f, 0.0f, 1.0f);


		Vector3 start[2] = {
			{Transform(Transform(sphere[0].center, shoulderViewProjectionMatrix), viewportMatrix)},
			{Transform(Transform(sphere[1].center, elbowViewProjectionMatrix), viewportMatrix)}
		};
		Vector3 end[2] = {
			{Transform(Transform(sphere[1].center, elbowViewProjectionMatrix), viewportMatrix)},
			{Transform(Transform(sphere[2].center, handViewProjectionMatrix), viewportMatrix)}
		};


		sphere[0].center = shoulderTranslate;
		sphere[0].radius = 0.1f;

		sphere[1].center = elbowTranslate;
		sphere[1].radius = 0.1f;

		sphere[2].center = handTranslate;
		sphere[2].radius = 0.1f;


		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		ImGui::DragFloat3("cameraPos", &cameraPosition.x, 0.01f);
		ImGui::DragFloat3("cameraScale", &cameraScale.x, 0.01f);
		ImGui::DragFloat3("cameraRotate", &cameraRotate.x, 0.01f);

		ImGui::DragFloat3("translates[0]", &translates[0].x, 0.01f);
		ImGui::DragFloat3("translates[1]", &translates[1].x, 0.01f);
		ImGui::DragFloat3("translates[2]", &translates[2].x, 0.01f);

		ImGui::DragFloat3("rotates[0]", &rotates[0].x, 0.01f);
		ImGui::DragFloat3("rotates[1]", &rotates[1].x, 0.01f);
		ImGui::DragFloat3("rotates[2]", &rotates[2].x, 0.01f);

		ImGui::DragFloat3("scales[0]", &scales[0].x, 0.01f);
		ImGui::DragFloat3("scales[1]", &scales[1].x, 0.01f);
		ImGui::DragFloat3("scales[2]", &scales[2].x, 0.01f);

		DrawGrid(worldViewProjectionMatrix, viewportMatrix);
		DrawSphere(sphere[0], shoulderViewProjectionMatrix, viewportMatrix, RED);
		DrawSphere(sphere[1], elbowViewProjectionMatrix, viewportMatrix, GREEN);
		DrawSphere(sphere[2], handViewProjectionMatrix, viewportMatrix, BLUE);
		Novice::DrawLine(int(start[0].x), int(start[0].y), int(end[0].x), int(end[0].y), WHITE);
		Novice::DrawLine(int(start[1].x), int(start[1].y), int(end[1].x), int(end[1].y), WHITE);

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