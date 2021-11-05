#pragma once
#ifndef _saveload
#define _saveload

#include <fstream>
#include <string>
#include "database.h"

class saveLoad
{
public:

#pragma pack(push, 1)
	// ������ ������ ����ü
	// SaveData ��ü�� ���Ͽ� �����ϰ�, (save_bin)
	// ������ SaveData ��ü�� �ҷ��´�. (load_bin)
	// SaveData ��ü�� ���Ͻ÷� �ڵ� ���� ������� ��ȣ�ۿ��ϸ� �ȴ�.
	struct saveData
	{
		// ����, �ҷ����⿡ ���� �������� ���⿡�� �߰��ϰų� �����Ѵ�.
	};
#pragma pack(pop)

	static void save_bin( const char* file_path, const saveData& data )
	{
		save_bin_impl( file_path, data );
	}

	static void save_bin( const char* file_path, saveData&& data )
	{
		save_bin_impl( file_path, std::move( data ) );
	}

	static void save_bin( const wchar_t* file_path, const saveData& data )
	{
		save_bin_impl( file_path, data );
	}

	static void save_bin( const wchar_t* file_path, saveData&& data )
	{
		save_bin_impl( file_path, std::move( data ) );
	}

	static void save_bin( const std::string& file_path, const saveData& data )
	{
		save_bin_impl( file_path, data );
	}

	static void save_bin( const std::string& file_path, saveData&& data )
	{
		save_bin_impl( file_path, std::move( data ) );
	}

	static void save_bin( const std::wstring& file_path, const saveData& data )
	{
		save_bin_impl( file_path, data );
	}

	static void save_bin( const std::wstring& file_path, saveData&& data )
	{
		save_bin_impl( file_path, std::move( data ) );
	}

	static void load_bin( const char* file_path, saveData& data )
	{
		load_bin_impl( file_path, data );
	}

	static void load_bin( const wchar_t* file_path, saveData& data )
	{
		load_bin_impl( file_path, data );
	}

	static void load_bin( const std::string& file_path, saveData& data )
	{
		load_bin_impl( file_path, data );
	}

	static void load_bin( const std::wstring& file_path, saveData& data )
	{
		load_bin_impl( file_path, data );
	}

	saveLoad() = delete;

private:
	template < typename Str_t, typename SV >
	static void save_bin_impl( Str_t&& file_path, SV&& data )
	{
		std::ofstream out{ file_path, std::ios_base::binary | std::ios_base::out };

		if ( out.fail() )
			throw std::ios_base::failure{ "SaveLoad::save_bin_impl : cannot open file" };

		out.write( reinterpret_cast< const char* >( &data ), sizeof( decltype( data ) ) );
	}

	template < typename Str_t, typename SV >
	static void load_bin_impl( Str_t&& file_path, SV&& data )
	{
		std::ifstream in{ file_path, std::ios_base::binary | std::ios_base::in };

		if ( in.fail() )
			throw std::ios_base::failure{ "SaveLoad::load_bin_impl : cannot open file" };

		in.read( reinterpret_cast< char* >( &data ), sizeof( decltype( data ) ) );
	}
};

#endif