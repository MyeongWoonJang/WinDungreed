#ifndef _sound
#define _sound

#include "fmod.hpp"
#include "pool.h"

// �Ϲ� ���� ���
// �±׸� ����, ���� ���
// �����ڸ� ����, ���� ���
// �±׿� �����ڸ� ����, ���� ���
// Ư�� �±��� ���� ����
// Ư�� �������� ���� ����
// Ư�� �±��� ���� �簳
// Ư�� �������� ���� �簳
// Ư�� �±��� ���� ���� ����
// Ư�� �������� ���� ���� ����
// ���� ���ӿ���ð�(ms) ������ ��쿡 ���� ���� �����ε�

class sound
{
public:
	// volume : [0.0f, 1.0f]
	static void set_default_volume( const float vol )
	{
		fmods.default_volume = vol;
	}


private:
	struct fmod_service
	{
		fmod_service() : default_volume{ 1.0f }, sounds{ fmod_num_sounds }, channels{ fmod_num_channels }
		{
			FMOD::System_Create( &system );
			system->init( fmod_num_channels, FMOD_INIT_NORMAL, nullptr );
		}

		~fmod_service()
		{
			system->release();
		}
		
		fmod_service( const fmod_service& other ) = delete;
		fmod_service& operator=( const fmod_service& other ) = delete;

		// -----------------------------------------------------------
		// fmod datas

		static constexpr size_t fmod_num_sounds = 320;
		static constexpr size_t fmod_num_channels = 32;

		float default_volume;
		FMOD::System* system;
		pool< FMOD::Sound > sounds;
		pool< FMOD::Channel > channels;
	};

	static fmod_service fmods;

	float volume;
	int sound_id;
	bool will_roop;
};

#endif