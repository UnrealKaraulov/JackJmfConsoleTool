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
	bool isZero()
	{
		return std::fabs(x) < 0.0001f && std::fabs(y) < 0.0001f && std::fabs(z) < 0.0001f;
	}
	std::string toKeyvalue(bool truncate = false, const std::string& suffix_x = " ", const std::string& suffix_y = " ", const std::string& suffix_z = "")
	{
		std::string parts[3] = { std::to_string(x) ,std::to_string(y), std::to_string(z) };

		// remove trailing zeros to save some space
		for (int i = 0; i < 3; i++)
		{
			auto it = parts[i].find('.');

			if (it != std::string::npos)
			{
				parts[i].erase(parts[i].find_last_not_of('0') + 1, std::string::npos);

				// strip dot if there's no fractional part
				if (parts[i][parts[i].size() - 1] == '.')
				{
					parts[i] = parts[i].substr(0, parts[i].size() - 1);
				}
				if (truncate)
				{
					size_t dotPosition = parts[i].find('.');
					if (dotPosition != std::string::npos) {
						parts[i] = parts[i].substr(0, dotPosition);
					}
				}
			}
		}

		return parts[0] + suffix_x + parts[1] + suffix_y + parts[2] + suffix_z;
	}

};

std::string flt_to_str(float f)
{
	std::string retstr = std::to_string(f);
	auto it = retstr.find('.');
	if (it != std::string::npos)
	{
		retstr.erase(retstr.find_last_not_of('0') + 1, std::string::npos);
		if (retstr[retstr.size() - 1] == '.')
		{
			retstr = retstr.substr(0, retstr.size() - 1);
		}
	}
	return retstr;
}

std::vector<std::string> splitString(const std::string& str, const std::string& delimiter, int maxParts = 0)
{
	std::string s = str;
	std::vector<std::string> split;
	size_t pos;
	while ((pos = s.find(delimiter)) != std::string::npos && (maxParts == 0 || (int)split.size() < maxParts - 1)) {
		if (pos != 0) {
			split.push_back(s.substr(0, pos));
		}
		s.erase(0, pos + delimiter.length());
	}
	if (!s.empty())
		split.push_back(s);
	return split;
}

vec3 parseVector(const std::string& s)
{
	vec3 v;
	std::vector<std::string> parts = splitString(s, " ");

	while (parts.size() < 3)
	{
		parts.push_back("0");
	}

	v.x = (float)atof(parts[0].c_str());
	v.y = (float)atof(parts[1].c_str());
	v.z = (float)atof(parts[2].c_str());
	return v;
}


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
struct VertPoint
{
	vec3 position;
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

void WriteKeyval(std::ofstream& f, const std::string& key, const std::string& val)
{
	f << "\"" << key << "\" \"" << val << "\"" << std::endl;
}

int main(int argc, char* argv[])
{
	std::cout << "JackJmfConsoleTool 1.1 by Karaulov" << std::endl;
	setlocale(LC_NUMERIC, "C");

	int nArgs;
	LPWSTR* szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (nArgs <= 2)
	{
		std::cout << "NEED JackJmfConsoleTool.exe [.jmf path] + command" << std::endl;
		std::cout << "commands [SUPPORT MULTIPLE COMMANDS AT SAME TIME!] : " << std::endl;
		std::cout << std::endl;
		std::cout << "replace [old texture] [new texture] [scale x] [scale y]" << std::endl;
		std::cout << std::endl;
		std::cout << "replace_ex [old texture] [new texture] [scale x] [scale y] [shift x] [shift y]" << std::endl;
		std::cout << std::endl;
		std::cout << "colorize -- random colors for all groups : no arguments" << std::endl;
		std::cout << std::endl;
		std::cout << "exportmap -- save directly to .map : no arguments" << std::endl;
		std::cout << std::endl;
		std::cout << std::endl;
		std::cout << std::endl;
		std::cout << "Example JackJmfConsoleTool \"path \\ to \\ map.jmf\" tex00001 tex00002 2.0 2.0 tex00003 tex00004 2.0 2.0" << std::endl;
		return 1;
	}

	int replaced_textures = 0;
	std::vector<ReplaceStr> replaceList;

	bool replace = false;
	bool colorize = false;
	bool exportmap = false;
	bool flip = false;
	bool rotate = false;

	if (szArglist[2] == std::wstring(L"colorize"))
	{
		colorize = true;
		std::cout << "Entered colorize command! Now all groups has random colors!" << std::endl;
	}
	else if (szArglist[2] == std::wstring(L"exportmap"))
	{
		exportmap = true;
		std::cout << "Entered exportmap command! Exporting .map file ...!" << std::endl;
	}
	else if (szArglist[2] == std::wstring(L"flip"))
	{
		flip = true;
		std::cout << "Entered flip command! Output jmf is flipped(swap x/y)! ...!" << std::endl;
	}
	else if (szArglist[2] == std::wstring(L"rotate"))
	{
		rotate = true;
		std::cout << "Entered flip command! Output jmf is rotated in 90 CW! ...!" << std::endl;
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
				tmpReplaceStr.tex2 << " with scale " << flt_to_str(tmpReplaceStr.scaleX) << "/" << flt_to_str(tmpReplaceStr.scaleY) << " offset: " <<
				flt_to_str(tmpReplaceStr.shiftX) << "/" << flt_to_str(tmpReplaceStr.shiftY) << std::endl;

			replaceList.push_back(tmpReplaceStr);
		}
	}
	else if (szArglist[2] == std::wstring(L"replace"))
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
				tmpReplaceStr.tex2 << " with scale " << flt_to_str(tmpReplaceStr.scaleX) << "/" << flt_to_str(tmpReplaceStr.scaleY) << std::endl;

			replaceList.push_back(tmpReplaceStr);
		}
	}
	else
	{
		std::cout << "Bad command input:";
		std::wcout << szArglist[2];
		std::cout << "! Allowed commands : " << std::endl;
		std::cout << std::endl;
		std::cout << "replace [old texture] [new texture] [scale x] [scale y]" << std::endl;
		std::cout << std::endl;
		std::cout << "replace_ex [old texture] [new texture] [scale x] [scale y] [shift x] [shift y]" << std::endl;
		std::cout << std::endl;
		std::cout << "colorize -- random colors for all groups : no arguments" << std::endl;
		std::cout << std::endl;
		std::cout << "exportmap -- save directly to .map : no arguments" << std::endl;
		std::cout << std::endl;
		std::cout << std::endl;
		std::cout << std::endl;
		return 1;
	}

	try
	{
		std::ofstream mapFile;
		if (exportmap)
		{
			std::wstring mapfileName = std::wstring(szArglist[1]);
			mapfileName.pop_back(); mapfileName.pop_back(); mapfileName.pop_back(); mapfileName.pop_back();
			mapfileName += L"_converted.map";
			mapFile = std::ofstream(mapfileName);
		}

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

			if (version != 122 && version != 121)
			{
				std::cout << "UNSUPPORTED JMF VERSION [" << version << "]!\n";
				return 1;
			}

			std::cout << "[INFO] JMF VERSION : [v" << version << "]!\n";

			// EXPORT PATHES
			int exportPaths;
			tmpJackReader.read<int>(exportPaths);
			tmpJackWriter.write<int>(exportPaths);

			while (exportPaths > 0)
			{
				std::string path;
				tmpJackReader.readLenStr(path);
				tmpJackWriter.writeLenStr(path);
				exportPaths--;
			}

			// BACKGROUND IMAGES
			for (int img = 0; version >=122 && img < 3; img++)
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

			while (groups > 0)
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

			while (visgroups > 0)
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

			while (cameras > 0)
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

				if (flip)
				{
					// swap pos
					std::swap(eye_pos.x, eye_pos.y);

					lookat_pos.y = 90.0f - lookat_pos.y;
				}
				if (rotate)
				{
					// swap and flip (rotate CW +90)
					std::swap(eye_pos.x, eye_pos.y);
					eye_pos.y *= -1;

					lookat_pos.y -= 90.0f;
				}

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

			while (pathes > 0)
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

				while (node_count > 0)
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



					if (flip)
					{
						// swap pos
						std::swap(position.x, position.y);
						// flip angles
						angles.y = 90.0f - angles.y;
					}
					if (rotate)
					{
						// swap and flip (rotate CW +90)
						std::swap(position.x, position.y);
						position.y *= -1;
						// rotate angles
						angles.y -= 90.0f;
					}



					tmpJackWriter.writeLenStr(name_override);
					tmpJackWriter.writeLenStr(fire_on_pass);
					tmpJackWriter.write<vec3>(position);
					tmpJackWriter.write<vec3>(angles);
					tmpJackWriter.write<int>(flags);
					tmpJackWriter.write<int>(kv_count);

					while (kv_count > 0)
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
				if (exportmap)
				{
					mapFile << "{" << std::endl;
					WriteKeyval(mapFile, "classname", className);
					if (className == "worldspawn")
					{
						WriteKeyval(mapFile, "mapversion", "220");
					}
				}

				vec3 origin;
				int flags;
				int group_id;
				int root_group_id;
				COLOR4 color;

				std::string specialkeys[13];

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


				if (flip)
				{
					// swap pos
					std::swap(origin.x, origin.y);
				}

				if (rotate)
				{
					// swap and flip (rotate CW +90)
					std::swap(origin.x, origin.y);
					origin.y *= -1;
				}

				for (int i = 0; i < 13; i++)
				{
					tmpJackReader.readLenStr(specialkeys[i]);
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


				if (exportmap && !origin.isZero())
				{
					WriteKeyval(mapFile, "origin", origin.toKeyvalue());
				}

				int keyvalues;
				tmpJackReader.read<int>(keyvalues);

				std::vector<std::string> key_list;
				std::vector<std::string> val_list;

				while (keyvalues > 0)
				{
					std::string key, value;

					tmpJackReader.readKeyVal(key, value);

					key_list.push_back(key);
					val_list.push_back(value);


					if (exportmap)
					{
						WriteKeyval(mapFile, key, value);
					}

					keyvalues--;
				}


				int entvisgroups;
				tmpJackReader.read<int>(entvisgroups);
				std::vector<int> entvgroups;

				while (entvisgroups > 0)
				{
					int group;

					tmpJackReader.read<int>(group);

					entvgroups.push_back(group);

					entvisgroups--;
				}

				int brush_count;
				tmpJackReader.read<int>(brush_count);


				tmpJackWriter.writeLenStr(className);

				tmpJackWriter.write<vec3>(origin);
				tmpJackWriter.write<int>(flags);
				tmpJackWriter.write<int>(group_id);
				tmpJackWriter.write<int>(root_group_id);
				tmpJackWriter.write<COLOR4>(color);

				for (int i = 0; i < 13; i++)
				{
					tmpJackWriter.writeLenStr(specialkeys[i]);
				}

				/*if (exportmap && !sp_angles.isZero())
				{
					WriteKeyval(mapFile, "angles", sp_angles.toKeyvalue());
				}*/


				if (flip)
				{
					sp_angles.y = 90.0f - sp_angles.y;
				}

				if (rotate)
				{
					sp_angles.y -= 90.0f;
				}


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



				tmpJackWriter.write<int>((int)key_list.size());

				for (size_t i = 0; i < key_list.size(); i++)
				{
					if (brush_count == 0)
					{
						if (key_list[i] == "angle")
						{
							float aval = atof(val_list[i].c_str());

							if (flip)
							{
								aval = 90.0f - aval;
							}

							if (rotate)
							{
								aval -= 90.0f;
							}

							val_list[i] = flt_to_str(aval);
						}
						else if (key_list[i] == "angles")
						{
							vec3 vangle = parseVector(val_list[i]);

							if (flip)
							{
								vangle.y = 90.0f - vangle.y;
							}

							if (rotate)
							{
								vangle.y -= 90.0f;
							}

							val_list[i] = vangle.toKeyvalue();
						}
					}

					tmpJackWriter.writeKeyVal(key_list[i], val_list[i]);
				}

				tmpJackWriter.write<int>((int)entvgroups.size());

				for (auto g : entvgroups)
				{
					tmpJackWriter.write<int>(g);
				}

				tmpJackWriter.write<int>(brush_count);


				while (brush_count > 0)
				{
					if (exportmap)
					{
						mapFile << "{" << std::endl;
					}
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
					while (brushvisgroups > 0)
					{
						int group;

						tmpJackReader.read<int>(group);
						tmpJackWriter.write<int>(group);

						brushvisgroups--;
					}
					int facecount;
					tmpJackReader.read<int>(facecount);
					tmpJackWriter.write<int>(facecount);

					while (facecount > 0)
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

						if (flip)
						{
							// swap 
							std::swap(right_axis.x, right_axis.y);
							std::swap(down_axis.x, down_axis.y);
							std::swap(normal.x, normal.y);
							// flip angles
							angle = 90.0f - angle;
						}
						if (rotate)
						{
							// swap and flip (rotate CW +90)
							std::swap(right_axis.x, right_axis.y);
							std::swap(down_axis.x, down_axis.y);
							std::swap(normal.x, normal.y);
							right_axis.y *= -1;
							down_axis.y *= -1;
							normal.y *= -1;
							// rotate angles
							angle -= 90.0f;
						}


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

								std::cout << "ScaleX " << flt_to_str(scale.x) << " to " << flt_to_str(newscale_x) << std::endl;
								std::cout << "ScaleY " << flt_to_str(scale.x) << " to " << flt_to_str(newscale_x) << std::endl;


								std::cout << "ShiftX " << flt_to_str(shift_x) << " to " << flt_to_str(newshift_x) << std::endl;
								std::cout << "ShiftY " << flt_to_str(shift_y) << " to " << flt_to_str(newshift_y) << std::endl;

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

						std::vector<VertPoint> verts;

						while (vertex_count > 0)
						{
							VertPoint tmpVertPoint;

							tmpJackReader.read(tmpVertPoint);
							verts.push_back(tmpVertPoint);

							vertex_count--;
						}

						if (flip)
						{
							std::reverse(verts.begin(), verts.end());
						}

						for (auto& v : verts)
						{
							if (flip)
							{
								// swap 
								std::swap(v.position.x, v.position.y);
								std::swap(v.texture_uv.x, v.texture_uv.y);
							}
							if (rotate)
							{
								// swap and flip (rotate CW +90)
								std::swap(v.position.x, v.position.y);
								std::swap(v.texture_uv.x, v.texture_uv.y);
								v.position.y *= -1;
								v.texture_uv.y *= -1;
							}

							tmpJackWriter.write(v);
						}

						std::reverse(verts.begin(), verts.end());

						for (int v = 0; v < 3; v++)
						{
							mapFile << "( " << verts[v].position.x << " " << verts[v].position.y << " " << verts[v].position.z << " ) ";
						}

						mapFile << std::string(texture_name);
						mapFile << " [ " << right_axis.toKeyvalue() << " " << flt_to_str(shift_x) << " ] ";
						mapFile << "[ " << down_axis.toKeyvalue() << " " << flt_to_str(shift_y) << " ] ";
						mapFile << flt_to_str(angle) << " " << flt_to_str(scale.x) << " " << flt_to_str(scale.y) << std::endl;


						facecount--;
					}

					while (mesh_count > 0)
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
						unsigned char mesh_unknown[4]; // points?
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


						if (flip)
						{
							// swap 
							std::swap(right_axis.x, right_axis.y);
							std::swap(down_axis.x, down_axis.y);
							// flip angles
							angle = 90.0f - angle;
						}
						if (rotate)
						{
							// swap and flip (rotate CW +90)
							std::swap(right_axis.x, right_axis.y);
							std::swap(down_axis.x, down_axis.y);
							right_axis.y *= -1;
							down_axis.y *= -1;
							// rotate angles
							angle -= 90.0f;
						}

						if (flip || rotate)
						{
							for (int i = 0; i < 1024; i++)
							{
								MeshPoint& tmpPoint = points[i];

								if (flip)
								{
									// swap 
									std::swap(tmpPoint.position.x, tmpPoint.position.y);
									std::swap(tmpPoint.normal.x, tmpPoint.normal.y);
									std::swap(tmpPoint.texture_uv.x, tmpPoint.texture_uv.y);
								}

								if (rotate)
								{
									// swap and flip (rotate CW +90)
									std::swap(tmpPoint.position.x, tmpPoint.position.y);
									std::swap(tmpPoint.normal.x, tmpPoint.normal.y);
									std::swap(tmpPoint.texture_uv.x, tmpPoint.texture_uv.y);
									tmpPoint.position.y *= -1;
									tmpPoint.normal.y *= -1;
									tmpPoint.texture_uv.y *= -1;
								}
							}
						}

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

								std::cout << "ScaleX " << flt_to_str(scale.x) << " to " << flt_to_str(newscale_x) << std::endl;
								std::cout << "ScaleY " << flt_to_str(scale.x) << " to " << flt_to_str(newscale_x) << std::endl;


								std::cout << "ShiftX " << flt_to_str(shift_x) << " to " << flt_to_str(newshift_x) << std::endl;
								std::cout << "ShiftY " << flt_to_str(shift_y) << " to " << flt_to_str(newshift_y) << std::endl;

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

					if (exportmap)
					{
						mapFile << "}" << std::endl;
					}
				}


				if (exportmap)
				{
					mapFile << "}" << std::endl;
				}
			}

			if (replace)
			{
				std::cout << "Replaced:" << replaced_textures << " textures!";
			}

			std::cout << "[INFO] File saved!\n";
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