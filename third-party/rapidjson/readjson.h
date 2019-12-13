#ifndef READJSON_H_
#define READJSON_H_

/*! \file readjson.h */

#include "document.h"
#include <array>


#ifdef __GNUC__
template<typename... Args>
static constexpr auto makeArray(Args&&... args)
->std::array<typename std::common_type<Args...>::type, sizeof...(args)>
{
	return { { std::forward<Args>(args)... } };
}
#elif 0
// XXX: The implementation of common_type in VS2012 using OLLVM has bug, which may incorrectly produce void.
template<typename Arg0, typename... Args>
static constexpr auto makeArray(Arg0&& arg0, Args&&... args)
->std::array < typename std::decay<Arg0>::type, 1 + sizeof...(args) >
{
	return { { std::forward<Arg0>(arg0), std::forward<Args>(args)... } };
}
#else
// XXX: VS2012 using VC doesn't support variadic templates, either...
// Rewrite the implementation in UGLY way.

#define TYPE_NAME( id ) typename Arg ## id
#define FWD( id ) std::forward<Arg ## id >( arg ## id )
#define ARG( id ) Arg ## id && arg ## id

template<TYPE_NAME(0)>
static auto makeArray(ARG(0))
->std::array<typename std::decay<Arg0>::type, 1>
{
	std::array<typename std::decay<Arg0>::type, 1> v = { { FWD(0) } };
	return v;
}

template<TYPE_NAME(0), TYPE_NAME(1)>
static auto makeArray(ARG(0), ARG(1))
->std::array<typename std::decay<Arg0>::type, 2>
{
	std::array<typename std::decay<Arg0>::type, 2> v = { { FWD(0), FWD(1) } };
	return v;
}

template<TYPE_NAME(0), TYPE_NAME(1), TYPE_NAME(2)>
static auto makeArray(ARG(0), ARG(1), ARG(2))
->std::array<typename std::decay<Arg0>::type, 3>
{
	std::array<typename std::decay<Arg0>::type, 3> v = { { FWD(0), FWD(1), FWD(2) } };
	return v;
}

template<TYPE_NAME(0), TYPE_NAME(1), TYPE_NAME(2), TYPE_NAME(3)>
static auto makeArray(ARG(0), ARG(1), ARG(2), ARG(3))
->std::array<typename std::decay<Arg0>::type, 4>
{
	std::array<typename std::decay<Arg0>::type, 4> v = { { FWD(0), FWD(1), FWD(2), FWD(3) } };
	return v;
}

#undef TYPE_NAME
#undef FWD
#undef ARG

#endif

namespace rapidjson
{
	class Config
	{
	public:
		Config(Document& jsCfg, bool invalid)
			: jsCfg(jsCfg), invalid(invalid)
		{
			std::memset(cfgProfile, 0, sizeof(cfgProfile));
			std::memset(cfgModel, 0, sizeof(cfgModel));
			std::memset(defaultProfile, 0, sizeof(defaultProfile));
			sprintf(defaultProfile, "%0*d", (int)sizeof(defaultProfile) - 1, 0);
		}

		void setProfile(unsigned char profile)
		{
			sprintf(cfgProfile, "%0*d", (int)sizeof(cfgProfile) - 1, (int)profile);
		}

		void setModel(unsigned char model)
		{
			sprintf(cfgModel, "%0*d", (int)sizeof(cfgModel) - 1, (int)model);
		}

		template<typename T, typename Array>
		T get(const Array& configs, const T& default_value)
		{
			if (invalid)
			{
#ifdef DEBUG_CONFIG
				showConfigValue("(invalid) ", configs, default_value);
#endif
				return default_value;
			}

#ifdef DEBUG_CONFIG
			std::string message;
#endif
			Document::ValueType *value = static_cast<Document::ValueType*>(&jsCfg);
			bool not_found = false;
			/****
			Use profile and model to choose proper configuration from json
			For XXXXYYYY
			- Profile (XXXX)
			0000 for default
			0001 for user track
			- Model (YYYY)
			0000 for default
			0001 for DEVICE_FT1
			0002 for DEVICE_FT6
			0003 for DEVICE_GT1
			0004 for DEVICE_GT2
			0005 for DEVICE_HT8
			- For example, to configure GT1 for user tracker, it should check the key by order :
			P0_00010003 -> P0_0001 -->P0_00000003 -> P0
			****/
			char candidated_name_fmt[3][64] = { { 0 } };
			int count_candidated_name = 0;

			if (strlen(cfgProfile) && strcmp(cfgProfile, defaultProfile))
			{
				if (strlen(cfgModel))
				{
					sprintf(candidated_name_fmt[count_candidated_name++], "%%s_%s%s", cfgProfile, cfgModel);
				}

				sprintf(candidated_name_fmt[count_candidated_name++], "%%s_%s", cfgProfile);
			}

			if (strlen(cfgModel))
			{
				sprintf(candidated_name_fmt[count_candidated_name++], "%%s_%s%s", defaultProfile, cfgModel);
			}

			char full_name[64] = { 0 };

			for (const auto & name : configs)
			{
				int candidated_name_index = 0;

				for (; candidated_name_index < count_candidated_name; ++candidated_name_index)
				{
					sprintf(full_name, candidated_name_fmt[candidated_name_index], name);
					if (value->HasMember(full_name))
					{
						value = &(*value)[full_name];
						break;
					}
				}

				if (candidated_name_index == count_candidated_name)
				{
					if (value->HasMember(name))
					{
						value = &(*value)[name];
					}
					else
					{
#ifdef DEBUG_CONFIG
						std::ostringstream oss;
						oss << "(" << name << " not found) ";
						message = oss.str();
#endif
						not_found = true;
						break;
					}
				}
			}

			T config_value = not_found ? default_value : getValue<T>(*value);

#ifdef DEBUG_CONFIG
			showConfigValue(message.c_str(), configs, config_value);
#endif

			return config_value;
		}

	private:
		template<typename T>
		T getValue(Document::ValueType& value);

		Document& jsCfg;
		bool invalid;
		char defaultProfile[5];
		char cfgProfile[5];
		char cfgModel[5];
	};

	template<>
	bool Config::getValue<bool>(Document::ValueType& value)
	{
		return value.GetBool();
	}

	template<>
	double Config::getValue<double>(Document::ValueType& value)
	{
		return value.GetDouble();
	}

	template<>
	std::string Config::getValue<std::string>(Document::ValueType& value)
	{
		return (value.IsString() ? value.GetString() : "");
	}

	template<>
	unsigned Config::getValue<unsigned>(Document::ValueType& value)
	{
		return value.GetUint();
	}



	int readConfigFile(std::string filename, Document& jsCfg)
	{
		std::ifstream inFile;
#if defined(__WIN32) || defined(__WIN32__) || defined(WIN32)
#define CONFIG_CHAR_LENGTH 128
		TCHAR configFileBuf[CONFIG_CHAR_LENGTH];
		char szConfigFile[CONFIG_CHAR_LENGTH] = { 0 };
		std::wstring w_filename(filename.begin(), filename.end());
		ExpandEnvironmentStrings(w_filename.c_str(), configFileBuf, CONFIG_CHAR_LENGTH);
		wcstombs(szConfigFile, configFileBuf, wcslen(configFileBuf));
		inFile.open(szConfigFile);
#else
		inFile.open(filename);
#endif
		if (!inFile.good())
		{
#if defined(__ANDROID__)
			printf("Cannot open config file in /sdcard, try /data \n");
			filename = std::string(CFG_FILE_NAME_LEGACY);
			inFile.open(filename);
			if (!inFile.good())
			{
#endif
				printf("Failed to open config file.\n");
				inFile.close();
				return -1; //something wrong.
#if defined(__ANDROID__)
			}
#endif
		}

#if defined(__WIN32) || defined(__WIN32__) || defined(WIN32)
		if (strcmp(szConfigFile, "network.json") != 0)
		{
			printf("RS2ModuleConfig file is found at %s\n", szConfigFile);
		}
#else
		if (strcmp(filename.c_str(), "network.json") != 0)
		{
			printf("RS2ModuleConfig file is found at %s\n", filename.c_str());
		}
#endif

		std::stringstream strStream;
		strStream << inFile.rdbuf();
		ParseResult ok = jsCfg.Parse(strStream.str().data());
		if (!ok)
		{
			printf("JSON parse error : %d (%d)\n", ok.Code(), (int)ok.Offset());
			inFile.close();
			return 1; // something wrong.
		}
		else
		{
			inFile.close();
			return 0;
		}
	}
}// namespace rapidjson
#endif // READJSON_H_
