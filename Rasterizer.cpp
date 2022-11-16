#include "olcConsoleGameEngine.h"
#include <fstream>
#include <strstream>
#include <algorithm>

struct vec3d
{
	float x, y, z;
};

struct triangle
{
	vec3d p[3];

	wchar_t symbol;
	short colour;
};

struct mesh 
{
	std::vector<triangle> tris;

	bool LoadFromObjFile(std::string fileName) 
	{
		std::ifstream f(fileName);
		if (!f.is_open())
			return false;

		std::vector<vec3d> verts;

		while(!f.eof())
		{
			char line[128];
			f.getline(line, 128);

			std::strstream s;
			s << line;

			char junk;

			if(line[0] == 'v')
			{
				vec3d v;
				s >> junk >> v.x >> v.y >> v.z;
				verts.push_back(v);
			}

			if(line[0] == 'f')
			{
				int f[3];
				s >> junk >> f[0] >> f[1] >> f[2];
				tris.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
			}
		}

		return true;
	}
};

struct mat4x4
{
	float m[4][4] = { 0 };
};

class GraphicsEngine3D : public olcConsoleGameEngine
{
public:
	GraphicsEngine3D()
	{
		m_sAppName = L"3D Demo";
	}

private:
	mesh cubeMesh;
	mat4x4 projectionMatrix;

	vec3d camera;

	float theta;

	void MultiplyMatrixVector(vec3d &i, vec3d &o, mat4x4 &m)
	{
		o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
		o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
		o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
		float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

		if (w != 0.0f) 
		{
			o.x /= w;
			o.y /= w;
			o.z /= w;
		}
	}

	CHAR_INFO GetColour(float lum)
	{
		short bg_col, fg_col;
		wchar_t sym;
		int pixel_bw = (int)(13.0f * lum);
		switch (pixel_bw)
		{
		case 0: bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID; break;

		case 1: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_QUARTER; break;
		case 2: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_HALF; break;
		case 3: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_THREEQUARTERS; break;
		case 4: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_SOLID; break;

		case 5: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_QUARTER; break;
		case 6: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_HALF; break;
		case 7: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_THREEQUARTERS; break;
		case 8: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_SOLID; break;

		case 9:  bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_QUARTER; break;
		case 10: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_HALF; break;
		case 11: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_THREEQUARTERS; break;
		case 12: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_SOLID; break;
		default:
			bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID;
		}

		CHAR_INFO c;
		c.Attributes = bg_col | fg_col;
		c.Char.UnicodeChar = sym;
		return c;
	}

public:
	bool OnUserCreate() override 
	{
		cubeMesh.tris =
		{
		//south
		{0.0f, 0.0f, 0.0f,	0.0f, 1.0f, 0.0f,	1.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 0.0f,	1.0f, 1.0f, 0.0f,	1.0f, 0.0f, 0.0f},
		//east
		{1.0f, 0.0f, 0.0f,	1.0f, 1.0f, 0.0f,	1.0f, 1.0f, 1.0f},
		{1.0f, 0.0f, 0.0f,	1.0f, 1.0f, 1.0f,	1.0f, 0.0f, 1.0f},
		//north
		{1.0f, 0.0f, 1.0f,	1.0f, 1.0f, 1.0f,	0.0f, 1.0f, 1.0f},
		{1.0f, 0.0f, 1.0f,	0.0f, 1.0f, 1.0f,	0.0f, 0.0f, 1.0f},
		//west
		{0.0f, 0.0f, 1.0f,	0.0f, 1.0f, 1.0f,	0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 1.0f,	0.0f, 1.0f, 0.0f,	0.0f, 0.0f, 0.0f},
		//top
		{0.0f, 1.0f, 0.0f,	0.0f, 1.0f, 1.0f,	1.0f, 1.0f, 1.0f},
		{0.0f, 1.0f, 0.0f,	1.0f, 1.0f, 1.0f,	1.0f, 1.0f, 0.0f},
		//bottom
		{1.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f,	0.0f, 0.0f, 0.0f},
		{1.0f, 0.0f, 1.0f,	0.0f, 0.0f, 0.0f,	1.0f, 0.0f, 0.0f},
		};

		//cubeMesh.LoadFromObjFile("untitled.obj");

		//projection matrix
		float nearPlane = 0.1f;
		float farPlane = 1000.0f;
		float fov = 90.0f;
		float aspectRatio = (float)ScreenHeight() / (float)ScreenWidth();
		float fovRad = 1.0f / tanf(fov * 0.5f / 180.0f * 3.14159f);

		projectionMatrix.m[0][0] = aspectRatio * fovRad;
		projectionMatrix.m[1][1] = fovRad;
		projectionMatrix.m[2][2] = farPlane / (farPlane - nearPlane);
		projectionMatrix.m[3][2] = (-farPlane * nearPlane) / (farPlane - nearPlane);
		projectionMatrix.m[2][3] = 1.0f;
		projectionMatrix.m[3][3] = 0.0f;

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override 
	{
		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

		mat4x4 zRotationMatrix, xRotationMatrix;
		theta += 1.0f * fElapsedTime;

		//z
		zRotationMatrix.m[0][0] = cosf(theta);
		zRotationMatrix.m[0][1] = sinf(theta);
		zRotationMatrix.m[1][0] = -sinf(theta);
		zRotationMatrix.m[1][1] = cosf(theta);
		zRotationMatrix.m[2][2] = 1;
		zRotationMatrix.m[3][3] = 1;

		//x
		xRotationMatrix.m[0][0] = 1;
		xRotationMatrix.m[1][1] = cosf(theta * 0.5f);
		xRotationMatrix.m[1][2] = sinf(theta * 0.5f);
		xRotationMatrix.m[2][1] = -sinf(theta * 0.5f);
		xRotationMatrix.m[2][2] = cosf(theta * 0.5f);
		xRotationMatrix.m[3][3] = 1;

		std::vector<triangle> trianglesToRaster;

		//draw tris
		for(auto tri : cubeMesh.tris)
		{
			triangle triProjected, triTranslated, zRotatedTri, zxRotatedTri;

			//rotate z
			MultiplyMatrixVector(tri.p[0], zRotatedTri.p[0], zRotationMatrix);
			MultiplyMatrixVector(tri.p[1], zRotatedTri.p[1], zRotationMatrix);
			MultiplyMatrixVector(tri.p[2], zRotatedTri.p[2], zRotationMatrix);

			//rotate x
			MultiplyMatrixVector(zRotatedTri.p[0], zxRotatedTri.p[0], xRotationMatrix);
			MultiplyMatrixVector(zRotatedTri.p[1], zxRotatedTri.p[1], xRotationMatrix);
			MultiplyMatrixVector(zRotatedTri.p[2], zxRotatedTri.p[2], xRotationMatrix);

			//offset into screen 
			triTranslated = zxRotatedTri;
			triTranslated.p[0].z = zxRotatedTri.p[0].z + 5.0f;
			triTranslated.p[1].z = zxRotatedTri.p[1].z + 5.0f;
			triTranslated.p[2].z = zxRotatedTri.p[2].z + 5.0f;

			vec3d normal, line1, line2;
			line1.x = triTranslated.p[1].x - triTranslated.p[0].x;
			line1.y = triTranslated.p[1].y - triTranslated.p[0].y;
			line1.z = triTranslated.p[1].z - triTranslated.p[0].z;
			
			line2.x = triTranslated.p[2].x - triTranslated.p[0].x;
			line2.y = triTranslated.p[2].y - triTranslated.p[0].y;
			line2.z = triTranslated.p[2].z - triTranslated.p[0].z;

			normal.x = line1.y * line2.z - line1.z * line2.y;
			normal.y = line1.z * line2.x - line1.x * line2.z;
			normal.z = line1.x * line2.y - line1.y * line2.x;

			float l = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
			normal.x /= l; normal.y /= l; normal.z /= l;

			//if (normal.z < 0)
			if(normal.x * (triTranslated.p[0].x - camera.x) + normal.y * (triTranslated.p[0].y - camera.y) + normal.z * (triTranslated.p[0].z - camera.z) < 0.0f)
			{
				//Illumination
				vec3d lightDirection = { 0.0f, 0.0f, -1.0f };
				float l = sqrtf(lightDirection.x * lightDirection.x + lightDirection.y * lightDirection.y + lightDirection.z * lightDirection.z);
				lightDirection.x /= l; lightDirection.y /= l; lightDirection.z /= l;

				float dotProduct = normal.x * lightDirection.x + normal.y * lightDirection.y + normal.z * lightDirection.z;

				CHAR_INFO c = GetColour(dotProduct);
				triTranslated.colour = c.Attributes;
				triTranslated.symbol = c.Char.UnicodeChar;

				//project 3D triangles into 2D space
				MultiplyMatrixVector(triTranslated.p[0], triProjected.p[0], projectionMatrix);
				MultiplyMatrixVector(triTranslated.p[1], triProjected.p[1], projectionMatrix);
				MultiplyMatrixVector(triTranslated.p[2], triProjected.p[2], projectionMatrix);
				triProjected.colour = triTranslated.colour;
				triProjected.symbol = triTranslated.symbol;

				//scaling into view
				triProjected.p[0].x += 1.0f; triProjected.p[0].y += 1.0f;
				triProjected.p[1].x += 1.0f; triProjected.p[1].y += 1.0f;
				triProjected.p[2].x += 1.0f; triProjected.p[2].y += 1.0f;

				triProjected.p[0].x *= 0.5f * (float)ScreenWidth(); triProjected.p[0].y *= 0.5f * (float)ScreenHeight();
				triProjected.p[1].x *= 0.5f * (float)ScreenWidth(); triProjected.p[1].y *= 0.5f * (float)ScreenHeight();
				triProjected.p[2].x *= 0.5f * (float)ScreenWidth(); triProjected.p[2].y *= 0.5f * (float)ScreenHeight();

				//store triangle for sorting
				trianglesToRaster.push_back(triProjected);
			}

			//sort triangles from back to front
			std::sort(trianglesToRaster.begin(), trianglesToRaster.end(), [](triangle& t1, triangle& t2)
			{
				float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
				float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
				return z1 > z2;
			});

			for (auto& triProjected : trianglesToRaster)
			{
				//Rasterize
				FillTriangle(triProjected.p[0].x, triProjected.p[0].y, triProjected.p[1].x, triProjected.p[1].y, triProjected.p[2].x, triProjected.p[2].y, triProjected.symbol, triProjected.colour);
				//DrawTriangle(triProjected.p[0].x, triProjected.p[0].y, triProjected.p[1].x, triProjected.p[1].y, triProjected.p[2].x, triProjected.p[2].y, PIXEL_SOLID, FG_DARK_CYAN);
			}
		}

		return true;
	}

};

int main()
{
	GraphicsEngine3D demo;
	if (demo.ConstructConsole(256, 240, 4, 4))
	{
		demo.Start();
	}

    return 0;
}
