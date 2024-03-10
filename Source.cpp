#define _WIN32_WINNT 0x0501 
#define WINVER 0x0501 
#define NTDDI_VERSION 0x05010000
#define VC_EXTRALEAN
#define _ATL_XP_TARGETING

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <Windows.h>

class JackWriter {
public:
	JackWriter(const std::string& filename) : file(filename, std::ios::binary) {}
	JackWriter(const std::wstring& filename) : file(filename, std::ios::binary) {}

	~JackWriter() {
		if (file.is_open()) {
			file.close();
		}
	}

	template <typename T>
	void write(const T& value) {
		file.write(reinterpret_cast<const char*>(&value), sizeof(T));
	}

	void writeLenStr(const std::string& str) {
		int size = (int)str.length();
		//write(size);
		write<int>(size);
		if (size > 0)
			file.write(str.data(), size);
	}

	void writeKeyVal(const std::string& key, const std::string& val)
	{
		writeLenStr(key);
		writeLenStr(val);
	}

	bool is_open() const {
		return file && file.is_open();
	}

private:
	std::ofstream file;
};

class JackReader {
public:
	JackReader(const std::string& filename) : file(filename, std::ios::binary) {}
	JackReader(const std::wstring& filename) : file(filename, std::ios::binary) {}

	~JackReader() {
		if (file.is_open()) {
			file.close();
		}
	}

	template <typename T>
	bool read(T& value) {
		file.read(reinterpret_cast<char*>(&value), sizeof(T));
		return file.good();
	}

	bool readLenStr(std::string& str) {
		int size = 0;
		if (!read<int>(size))
		{
			return false;
		}
		str.resize(size);
		if (size > 0) {
			file.read(&str[0], size);
		}
		return file.good();
	}

	bool readKeyVal(std::string& key, std::string& val) {
		return readLenStr(key) && readLenStr(val);
	}

	bool is_open() const {
		return file && file.is_open();
	}

private:
	std::ifstream file;
};


#pragma pack(push, 1)
struct COLOR4
{
	unsigned char r, g, b, a;
	COLOR4() : r(0), g(0), b(0), a(0) {};
	COLOR4(unsigned char r, unsigned char g, unsigned char b, unsigned char a) : r(r), g(g), b(b), a(a)
	{}
	void Randomize()
	{
		r = 55 + rand() % 200;
		g = 55 + rand() % 200;
		b = 55 + rand() % 200;
		a = 200 + rand() % 55;
	}
};
struct vec3
{
	float x, y, z;
};
struct vec2
{
	float x, y;
};
struct MeshPoint
{
	vec3 position;
	vec3 normal;
	vec2 texture_uv;
	int is_selected; // like in faces, or really unsigned char?
};
#pragma pack(pop, 1)

struct ReplaceStr
{
	char tex1[64];
	char tex2[64];
	float scaleX;
	float scaleY;

	float shiftX;
	float shiftY;
};

int main(int argc, char* argv[])
{
	std::cout << "JackTextureReplaceTool 1.0 by Karaulov" << std::endl;
	setlocale(LC_NUMERIC, "C");

	int nArgs;
	LPWSTR* szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (nArgs < 2)
	{
		std::cout << "NEED JackTextureReplaceTool.exe [.jmf path] + command" << std::endl;
		std::cout << "commands : " << std::endl;
		std::cout << std::endl;
		std::cout << "replace [old texture] [new texture] [scale x] [scale y]" << std::endl;
		std::cout << std::endl;
		std::cout << "colorize -- random colors for all groups : no arguments" << std::endl;
		std::cout << std::endl;
		std::cout << std::endl;
		std::cout << std::endl;
		std::cout << "Example JackTextureReplaceTool \"path \\ to \\ map.jmf\" tex00001 tex00002 2.0 2.0 tex00003 tex00004 2.0 2.0" << std::endl;
		return 1;
	}

	int replaced_textures = 0;
	std::vector<ReplaceStr> replaceList;

	bool replace = false;
	bool colorize = false;

	if (szArglist[2] == std::wstring(L"colorize"))
	{
		colorize = true;
		std::cout << "Entered colorize command! Now all groups has random colors!" << std::endl;
	}
	else if (szArglist[2] == std::wstring(L"replace") && nArgs < 5)
	{
		std::cout << "bad replace usage!!" << std::endl;
		std::cout << "NEED replace [old texture] [new texture] [scale x] [scale y]" << std::endl;
		return 1;
	}
	else if (szArglist[2] == std::wstring(L"replace_ex") && nArgs < 5)
	{
		std::cout << "bad replace usage!!" << std::endl;
		std::cout << "NEED replace [old texture] [new texture] [scale x] [scale y] [shift x] [shift y]" << std::endl;
		return 1;
	}
	else if (szArglist[2] != std::wstring(L"replace") && szArglist[2] != std::wstring(L"replace_ex"))
	{
		std::cout << "Bad command input:";
		std::wcout << szArglist[2];
		std::cout << "! Allowed commands : " << std::endl;
		std::cout << std::endl;
		std::cout << "replace [old texture] [new texture] [scale x] [scale y]" << std::endl;
		std::cout << std::endl;
		std::cout << "colorize -- random colors for all groups : no arguments" << std::endl;
		return 1;
	}
	else if (szArglist[2] == std::wstring(L"replace_ex"))
	{
		replace = true;
		for (int i = 3; i + 3 < argc; i += 6)
		{
			ReplaceStr tmpReplaceStr;
			snprintf(tmpReplaceStr.tex1, sizeof(tmpReplaceStr.tex1), "%s", argv[i]);
			snprintf(tmpReplaceStr.tex2, sizeof(tmpReplaceStr.tex1), "%s", argv[i + 1]);
			tmpReplaceStr.scaleX = std::stof(argv[i + 2]);
			tmpReplaceStr.scaleY = std::stof(argv[i + 3]);
			tmpReplaceStr.shiftX = std::stof(argv[i + 4]);
			tmpReplaceStr.shiftY = std::stof(argv[i + 5]);

			std::cout << "Added new replace command: Texture " << tmpReplaceStr.tex1 << " to " <<
				tmpReplaceStr.tex2 << " with scale " << std::to_string(tmpReplaceStr.scaleX) << "/" << std::to_string(tmpReplaceStr.scaleY) << " offset: " <<
				std::to_string(tmpReplaceStr.shiftX) << "/" << std::to_string(tmpReplaceStr.shiftY) << std::endl;

			replaceList.push_back(tmpReplaceStr);
		}
	}
	else
	{
		replace = true;
		for (int i = 3; i + 3 < argc; i += 4)
		{
			ReplaceStr tmpReplaceStr;
			snprintf(tmpReplaceStr.tex1, sizeof(tmpReplaceStr.tex1), "%s", argv[i]);
			snprintf(tmpReplaceStr.tex2, sizeof(tmpReplaceStr.tex1), "%s", argv[i + 1]);
			tmpReplaceStr.scaleX = std::stof(argv[i + 2]);
			tmpReplaceStr.scaleY = std::stof(argv[i + 3]);
			tmpReplaceStr.shiftX = 0;
			tmpReplaceStr.shiftY = 0;

			std::cout << "Added new replace command: Texture " << tmpReplaceStr.tex1 << " to " <<
				tmpReplaceStr.tex2 << " with scale " << std::to_string(tmpReplaceStr.scaleX) << "/" << std::to_string(tmpReplaceStr.scaleY) << std::endl;

			replaceList.push_back(tmpReplaceStr);
		}
	}

	try
	{
		JackReader tmpJackReader(szArglist[1]);
		if (tmpJackReader.is_open())
		{
			std::wstring newName = std::wstring(szArglist[1]);
			newName.pop_back(); newName.pop_back(); newName.pop_back(); newName.pop_back();
			newName += L"_replaced.jmf";


			// MAGIC NUMBER
			int magic;
			tmpJackReader.read<int>(magic);

			JackWriter tmpJackWriter(newName);
			tmpJackWriter.write<int>(magic);

			// VERSION
			int version;
			tmpJackReader.read<int>(version);
			tmpJackWriter.write<int>(version);

			if (version != 122)
			{
				std::cout << "UNSUPPORTED JMF VERSION!\n";
				return 1;
			}

			// EXPORT PATHES
			int exportPaths;
			tmpJackReader.read<int>(exportPaths);
			tmpJackWriter.write<int>(exportPaths);

			while (exportPaths)
			{
				std::string path;
				tmpJackReader.readLenStr(path);
				tmpJackWriter.writeLenStr(path);
				exportPaths--;
			}

			// BACKGROUND IMAGES
			for (int img = 0; img < 3; img++)
			{
				std::string path;
				double doubleScale;
				int luminance;
				int filtering;
				int invert;
				int offset_x;
				int offset_y;
				int offset_z;

				tmpJackReader.readLenStr(path);
				tmpJackReader.read<double>(doubleScale);
				tmpJackReader.read<int>(luminance);
				tmpJackReader.read<int>(filtering);
				tmpJackReader.read<int>(invert);
				tmpJackReader.read<int>(offset_x);
				tmpJackReader.read<int>(offset_y);
				tmpJackReader.read<int>(offset_z);

				tmpJackWriter.writeLenStr(path);
				tmpJackWriter.write<double>(doubleScale);
				tmpJackWriter.write<int>(luminance);
				tmpJackWriter.write<int>(filtering);
				tmpJackWriter.write<int>(invert);
				tmpJackWriter.write<int>(offset_x);
				tmpJackWriter.write<int>(offset_y);
				tmpJackWriter.write<int>(offset_z);
			}

			// GROUP
			int groups;
			tmpJackReader.read<int>(groups);
			tmpJackWriter.write<int>(groups);

			while (groups)
			{
				int group_id;
				int group_parent_id;
				int flags;
				int count;
				COLOR4 color;

				tmpJackReader.read<int>(group_id);
				tmpJackReader.read<int>(group_parent_id);
				tmpJackReader.read<int>(flags);
				tmpJackReader.read<int>(count);
				tmpJackReader.read<COLOR4>(color);

				if (colorize)
					color.Randomize();

				tmpJackWriter.write<int>(group_id);
				tmpJackWriter.write<int>(group_parent_id);
				tmpJackWriter.write<int>(flags);
				tmpJackWriter.write<int>(count);
				tmpJackWriter.write<COLOR4>(color);

				groups--;
			}

			// VIS GROUPS
			int visgroups;
			tmpJackReader.read<int>(visgroups);
			tmpJackWriter.write<int>(visgroups);

			while (visgroups)
			{
				std::string name;
				int visgroup_id;
				COLOR4 color;
				unsigned char visible;


				tmpJackReader.readLenStr(name);
				tmpJackReader.read<int>(visgroup_id);
				tmpJackReader.read<COLOR4>(color);
				tmpJackReader.read<unsigned char>(visible);

				if (colorize)
					color.Randomize();

				tmpJackWriter.writeLenStr(name);
				tmpJackWriter.write<int>(visgroup_id);
				tmpJackWriter.write<COLOR4>(color);
				tmpJackWriter.write<unsigned char>(visible);

				visgroups--;
			}

			// CORDON MINS/MAX
			vec3 gondonMins, gondonMaxs;

			tmpJackReader.read<vec3>(gondonMins);
			tmpJackReader.read<vec3>(gondonMaxs);


			tmpJackWriter.write<vec3>(gondonMins);
			tmpJackWriter.write<vec3>(gondonMaxs);

			// CAMERAS
			int cameras;
			tmpJackReader.read<int>(cameras);
			tmpJackWriter.write<int>(cameras);

			while (cameras)
			{
				vec3 eye_pos;
				vec3 lookat_pos;
				int flags;
				COLOR4 color;

				tmpJackReader.read<vec3>(eye_pos);
				tmpJackReader.read<vec3>(lookat_pos);
				tmpJackReader.read<int>(flags);
				tmpJackReader.read<COLOR4>(color);

				if (colorize)
					color.Randomize();

				tmpJackWriter.write<vec3>(eye_pos);
				tmpJackWriter.write<vec3>(lookat_pos);
				tmpJackWriter.write<int>(flags);
				tmpJackWriter.write<COLOR4>(color);

				cameras--;
			}

			// PATH OBJECTS
			int pathes;
			tmpJackReader.read<int>(pathes);
			tmpJackWriter.write<int>(pathes);

			while (pathes)
			{
				std::string classname;
				std::string path_name;
				int path_type;
				int flags;
				COLOR4 color;
				int node_count;


				tmpJackReader.readLenStr(classname);
				tmpJackReader.readLenStr(path_name);
				tmpJackReader.read<int>(path_type);
				tmpJackReader.read<int>(flags);
				tmpJackReader.read<COLOR4>(color);
				tmpJackReader.read<int>(node_count);

				if (colorize)
					color.Randomize();

				tmpJackWriter.writeLenStr(classname);
				tmpJackWriter.writeLenStr(path_name);
				tmpJackWriter.write<int>(path_type);
				tmpJackWriter.write<int>(flags);
				tmpJackWriter.write<COLOR4>(color);
				tmpJackWriter.write<int>(node_count);

				while (node_count)
				{
					std::string name_override;
					std::string fire_on_pass;
					vec3 position;
					vec3 angles;
					int flags;
					int kv_count;


					tmpJackReader.readLenStr(name_override);
					tmpJackReader.readLenStr(fire_on_pass);
					tmpJackReader.read<vec3>(position);
					tmpJackReader.read<vec3>(angles);
					tmpJackReader.read<int>(flags);
					tmpJackReader.read<int>(kv_count);


					tmpJackWriter.writeLenStr(name_override);
					tmpJackWriter.writeLenStr(fire_on_pass);
					tmpJackWriter.write<vec3>(position);
					tmpJackWriter.write<vec3>(angles);
					tmpJackWriter.write<int>(flags);
					tmpJackWriter.write<int>(kv_count);

					while (kv_count)
					{
						std::string key;
						std::string value;

						tmpJackReader.readLenStr(key);
						tmpJackReader.readLenStr(value);

						tmpJackWriter.writeLenStr(key);
						tmpJackWriter.writeLenStr(value);

						kv_count--;
					}

					node_count--;
				}

				pathes--;
			}

			//Entities
			std::string className;

			while (tmpJackReader.readLenStr(className))
			{
				tmpJackWriter.writeLenStr(className);

				vec3 origin;
				int flags;
				int group_id;
				int root_group_id;
				COLOR4 color;
				int sp_spawnflags;
				vec3 sp_angles;
				int sp_rendering;
				COLOR4 sp_fx_color;
				int sp_rendermode;
				int sp_render_fx;
				short sp_body;
				short sp_skin;
				int sp_sequence;
				float sp_framerate;
				float sp_scale;
				float sp_radius;
				unsigned char unknown[28];


				tmpJackReader.read<vec3>(origin);
				tmpJackReader.read<int>(flags);
				tmpJackReader.read<int>(group_id);
				tmpJackReader.read<int>(root_group_id);
				tmpJackReader.read<COLOR4>(color);

				if (colorize)
					color.Randomize();

				tmpJackWriter.write<vec3>(origin);
				tmpJackWriter.write<int>(flags);
				tmpJackWriter.write<int>(group_id);
				tmpJackWriter.write<int>(root_group_id);
				tmpJackWriter.write<COLOR4>(color);

				for (int i = 0; i < 13; i++)
				{
					std::string spec_key;
					tmpJackReader.readLenStr(spec_key);
					tmpJackWriter.writeLenStr(spec_key);
				}

				tmpJackReader.read<int>(sp_spawnflags);
				tmpJackReader.read<vec3>(sp_angles);
				tmpJackReader.read<int>(sp_rendering);
				tmpJackReader.read<COLOR4>(sp_fx_color);
				tmpJackReader.read<int>(sp_rendermode);
				tmpJackReader.read<int>(sp_render_fx);
				tmpJackReader.read<short>(sp_body);
				tmpJackReader.read<short>(sp_skin);
				tmpJackReader.read<int>(sp_sequence);
				tmpJackReader.read<float>(sp_framerate);
				tmpJackReader.read<float>(sp_scale);
				tmpJackReader.read<float>(sp_radius);
				tmpJackReader.read(unknown);

				tmpJackWriter.write<int>(sp_spawnflags);
				tmpJackWriter.write<vec3>(sp_angles);
				tmpJackWriter.write<int>(sp_rendering);
				tmpJackWriter.write<COLOR4>(sp_fx_color);
				tmpJackWriter.write<int>(sp_rendermode);
				tmpJackWriter.write<int>(sp_render_fx);
				tmpJackWriter.write<short>(sp_body);
				tmpJackWriter.write<short>(sp_skin);
				tmpJackWriter.write<int>(sp_sequence);
				tmpJackWriter.write<float>(sp_framerate);
				tmpJackWriter.write<float>(sp_scale);
				tmpJackWriter.write<float>(sp_radius);
				tmpJackWriter.write(unknown);


				int keyvalues;
				tmpJackReader.read<int>(keyvalues);
				tmpJackWriter.write<int>(keyvalues);

				while (keyvalues)
				{
					std::string key, value;

					tmpJackReader.readLenStr(key);
					tmpJackReader.readLenStr(value);

					tmpJackWriter.writeLenStr(key);
					tmpJackWriter.writeLenStr(value);


					keyvalues--;
				}

				int entvisgroups;
				tmpJackReader.read<int>(entvisgroups);
				tmpJackWriter.write<int>(entvisgroups);

				while (entvisgroups)
				{
					int group;

					tmpJackReader.read<int>(group);
					tmpJackWriter.write<int>(group);

					entvisgroups--;
				}

				int brush_count;
				tmpJackReader.read<int>(brush_count);
				tmpJackWriter.write<int>(brush_count);

				while (brush_count)
				{
					int mesh_count;
					int flags;
					int group_id;
					int root_group_id;
					COLOR4 color;


					tmpJackReader.read<int>(mesh_count);
					tmpJackReader.read<int>(flags);
					tmpJackReader.read<int>(group_id);
					tmpJackReader.read<int>(root_group_id);
					tmpJackReader.read<COLOR4>(color);

					if (colorize)
						color.Randomize();

					tmpJackWriter.write<int>(mesh_count);
					tmpJackWriter.write<int>(flags);
					tmpJackWriter.write<int>(group_id);
					tmpJackWriter.write<int>(root_group_id);
					tmpJackWriter.write<COLOR4>(color);

					int brushvisgroups;
					tmpJackReader.read<int>(brushvisgroups);
					tmpJackWriter.write<int>(brushvisgroups);
					while (brushvisgroups)
					{
						int group;

						tmpJackReader.read<int>(group);
						tmpJackWriter.write<int>(group);

						brushvisgroups--;
					}
					int facecount;
					tmpJackReader.read<int>(facecount);
					tmpJackWriter.write<int>(facecount);

					while (facecount)
					{
						int render_flags;
						int vertex_count;
						vec3 right_axis;
						float shift_x;
						vec3 down_axis;
						float shift_y;
						vec2 scale;
						float angle;
						int texture_alignment;
						unsigned char face_unknown[12];
						int surface_flags;
						char texture_name[64];
						vec3 normal;
						float distance;
						int aligned_axis;


						tmpJackReader.read<int>(render_flags);
						tmpJackReader.read<int>(vertex_count);
						tmpJackReader.read<vec3>(right_axis);
						tmpJackReader.read<float>(shift_x);
						tmpJackReader.read<vec3>(down_axis);
						tmpJackReader.read<float>(shift_y);
						tmpJackReader.read<vec2>(scale);
						tmpJackReader.read<float>(angle);
						tmpJackReader.read<int>(texture_alignment);
						tmpJackReader.read(face_unknown);
						tmpJackReader.read<int>(surface_flags);
						tmpJackReader.read(texture_name);
						tmpJackReader.read<vec3>(normal);
						tmpJackReader.read<float>(distance);
						tmpJackReader.read<int>(aligned_axis);

						for (auto& r : replaceList)
						{
							if (_stricmp(r.tex1, texture_name) == 0)
							{
								replaced_textures++;
								memcpy(texture_name, r.tex2, sizeof(texture_name));

								float newscale_x = scale.x * r.scaleX;
								float newscale_y = scale.y * r.scaleY;

								float newshift_x = shift_x / r.scaleX;
								float newshift_y = shift_y / r.scaleY;

								if (fabs(r.shiftX) > 0.0001f)
									newshift_x = r.shiftX;
								if (fabs(r.shiftY) > 0.0001f)
									newshift_y = r.shiftY;

								std::cout << "Replace[" << replaced_textures << "] " << r.tex1 << " to " << r.tex2 << std::endl;

								std::cout << "ScaleX " << std::to_string(scale.x) << " to " << std::to_string(newscale_x) << std::endl;
								std::cout << "ScaleY " << std::to_string(scale.x) << " to " << std::to_string(newscale_x) << std::endl;


								std::cout << "ShiftX " << std::to_string(shift_x) << " to " << std::to_string(newshift_x) << std::endl;
								std::cout << "ShiftY " << std::to_string(shift_y) << " to " << std::to_string(newshift_y) << std::endl;

								scale.x = newscale_x;
								scale.y = newscale_y;

								shift_x = newshift_x;
								shift_y = newshift_y;
							}
						}

						tmpJackWriter.write<int>(render_flags);
						tmpJackWriter.write<int>(vertex_count);
						tmpJackWriter.write<vec3>(right_axis);
						tmpJackWriter.write<float>(shift_x);
						tmpJackWriter.write<vec3>(down_axis);
						tmpJackWriter.write<float>(shift_y);
						tmpJackWriter.write<vec2>(scale);
						tmpJackWriter.write<float>(angle);
						tmpJackWriter.write<int>(texture_alignment);
						tmpJackWriter.write(face_unknown);
						tmpJackWriter.write<int>(surface_flags);
						tmpJackWriter.write(texture_name);
						tmpJackWriter.write<vec3>(normal);
						tmpJackWriter.write<float>(distance);
						tmpJackWriter.write<int>(aligned_axis);

						while (vertex_count)
						{
							vec3 coordinates;
							vec2 texture_uv;
							int selection_state;

							tmpJackReader.read<vec3>(coordinates);
							tmpJackReader.read<vec2>(texture_uv);
							tmpJackReader.read<int>(selection_state);

							tmpJackWriter.write<vec3>(coordinates);
							tmpJackWriter.write<vec2>(texture_uv);
							tmpJackWriter.write<int>(selection_state);


							vertex_count--;
						}

						facecount--;
					}

					while (mesh_count)
					{
						int width;
						int height;
						vec3 right_axis;
						float shift_x;
						vec3 down_axis;
						float shift_y;
						vec2 scale;
						float angle;
						int texture_alignment;
						unsigned char unknown[12];
						int surface_flags;
						char texture_name[64];
						unsigned char mesh_unknown[4];
						MeshPoint points[1024];



						tmpJackReader.read<int>(width);
						tmpJackReader.read<int>(height);
						tmpJackReader.read<vec3>(right_axis);
						tmpJackReader.read<float>(shift_x);
						tmpJackReader.read<vec3>(down_axis);
						tmpJackReader.read<float>(shift_y);
						tmpJackReader.read<vec2>(scale);
						tmpJackReader.read<float>(angle);
						tmpJackReader.read<int>(texture_alignment);
						tmpJackReader.read(unknown);
						tmpJackReader.read<int>(surface_flags);
						tmpJackReader.read(texture_name);
						tmpJackReader.read(mesh_unknown);
						tmpJackReader.read(points);


						for (auto& r : replaceList)
						{
							if (_stricmp(r.tex1, texture_name) == 0)
							{
								replaced_textures++;
								memcpy(texture_name, r.tex2, sizeof(texture_name));

								float newscale_x = scale.x * r.scaleX;
								float newscale_y = scale.y * r.scaleY;

								float newshift_x = shift_x / r.scaleX;
								float newshift_y = shift_y / r.scaleY;

								if (fabs(r.shiftX) > 0.0001f)
									newshift_x = r.shiftX;
								if (fabs(r.shiftY) > 0.0001f)
									newshift_y = r.shiftY;


								std::cout << "Replace[" << replaced_textures << "] " << r.tex1 << " to " << r.tex2 << std::endl;

								std::cout << "ScaleX " << std::to_string(scale.x) << " to " << std::to_string(newscale_x) << std::endl;
								std::cout << "ScaleY " << std::to_string(scale.x) << " to " << std::to_string(newscale_x) << std::endl;


								std::cout << "ShiftX " << std::to_string(shift_x) << " to " << std::to_string(newshift_x) << std::endl;
								std::cout << "ShiftY " << std::to_string(shift_y) << " to " << std::to_string(newshift_y) << std::endl;

								scale.x = newscale_x;
								scale.y = newscale_y;

								shift_x = newshift_x;
								shift_y = newshift_y;
							}
						}

						tmpJackWriter.write<int>(width);
						tmpJackWriter.write<int>(height);
						tmpJackWriter.write<vec3>(right_axis);
						tmpJackWriter.write<float>(shift_x);
						tmpJackWriter.write<vec3>(down_axis);
						tmpJackWriter.write<float>(shift_y);
						tmpJackWriter.write<vec2>(scale);
						tmpJackWriter.write<float>(angle);
						tmpJackWriter.write<int>(texture_alignment);
						tmpJackWriter.write(unknown);
						tmpJackWriter.write<int>(surface_flags);
						tmpJackWriter.write(texture_name);
						tmpJackWriter.write(mesh_unknown);
						tmpJackWriter.write(points);

						mesh_count--;
					}

					brush_count--;
				}
			}

			if (replace)
			{
				std::cout << "Replaced:" << replaced_textures << " textures!";
			}
		}
		else
		{
			std::cout << "CANT OPEN JMF!";
			return 1;
		}
	}
	catch (const std::exception& exc)
	{
		std::cout << "Fatal error:" << exc.what();
		return 1;
	}
	catch (...)
	{
		std::cout << "Unknown fatal error";
		return 1;
	}
	return 0;
}