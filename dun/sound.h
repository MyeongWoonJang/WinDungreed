#ifndef _sound
#define _sound

#include "fmod.hpp"
#include <array>
#include <vector>
#include <list>
#include "TMP.h"
#include <cassert>

class sound
{
public:
	enum class mode {
		normal = FMOD_LOOP_OFF, loop = FMOD_LOOP_NORMAL
	};

	void play()
	{
		if ( !( fmod_rc.available_channel_cnt ) )
		{
			// ��� ä���� ������� ���
			// ���� ���� �������� ����Ǵ� ä���� �����Ѵ�.
			sound* min_sound = this;
			float min_volume_found = min_volume;
			for ( auto other_sound : sound_insts )
			{
				auto other_min_volume = other_sound->min_volume;
				if ( other_min_volume < min_volume_found )
				{
					min_sound = other_sound;
					min_volume_found = other_min_volume;
				}
			}

			min_sound->fmod_channels.pop_back();
			min_sound->min_volume /= min_sound->gradient;
			++fmod_rc.available_channel_cnt;
		}

		fmod_channels.emplace_back( nullptr );

		// �� ���尡 �� �̻��� ä�ο��� ����� ��� ( ���� ��� )
		// ���߿� ����� ������ ������ ���δ�.
		min_volume *= gradient;
		fmod_channels.back()->setVolume( min_volume );
		fmod_rc.system->playSound( fmod_sound, nullptr, false, &fmod_channels.back() );
		--fmod_rc.available_channel_cnt;
	}

	void amplify( const float val )
	{
		volume *= val;
		min_volume *= val;
		for ( auto ch : fmod_channels )
		{
			float vol;
			ch->getVolume( &vol );
			ch->setVolume( vol * val );
		}
	}

	void pause()
	{
		for ( auto ch : fmod_channels )
		{
			ch->setPaused( true );
		}
	}

	void resume()
	{
		for ( auto ch : fmod_channels )
		{
			ch->setPaused( false );
		}
	}

	void stop()
	{
		fmod_channels.clear();
	}

	// ��� ��� �Ϸ�� ���忡 ���� ���� ó���� �Ѵ�.
	// ���� �ð� �������� �� �� �� ȣ���Ѵ�. ( ���帶�� ȣ������ �ʴ´�. ���� �Լ��̴�. )
	static void update()
	{
		fmod_rc.system->update();

		for ( auto s : sound_insts )
		{
			// ���߿� ����� ä���� ���� �������� ����ȴ�.
			// �ڿ������� �����Ͽ� ����� ä���� ������, �� ���� ä�ε��� ���� ����� ä���̴�.
			for ( auto iter = s->fmod_channels.rbegin(); iter != s->fmod_channels.rend(); ++iter )
			{
				bool is_playing = false;
				if ( !( ( *iter )->isPlaying( &is_playing ) ) )
				{
					s->fmod_channels.erase( s->fmod_channels.begin(), iter.base() );
					break;
				}
			}
		}
	}

	sound( const char* file_path, mode mod, const float volume = 1.0f, const float gradient = default_gradient )
		: volume{ volume }, gradient{ gradient }
	{
		assert( fmode_rc.available_sound_cnt > 0 );
		--fmod_rc.available_sound_cnt;
		fmod_rc.system->createSound( file_path, etoi( mod ) | FMOD_LOWMEM, nullptr, &fmod_sound );
		sound_insts.push_back( this );
		at = --sound_insts.end();
	}

	~sound()
	{
		fmod_sound->release();
		sound_insts.erase( at );
	}


private:
	static constexpr size_t max_subsounds = 5;
	static constexpr float default_gradient = 0.5f;

	float min_volume;
	float volume;
	float gradient;
	FMOD::Sound* fmod_sound;
	std::vector< FMOD::Channel* > fmod_channels;
	static std::list< sound* > sound_insts;
	std::list< sound* >::const_iterator at;

	struct fmod_resource
	{
		static constexpr size_t fmod_max_channels = 32;
		static constexpr size_t fmod_max_sounds = 100;

		FMOD::System* system;
		size_t available_sound_cnt;
		size_t available_channel_cnt;

		fmod_resource() : available_channel_cnt{ fmod_max_channels }, available_sound_cnt{ fmod_max_sounds }
		{
			FMOD::Memory_Initialize( malloc( 4 * 1024 * 1024 ), 4 * 1024 * 1024, 0, 0, 0 );

			FMOD::System_Create( &system );
			system->init( fmod_max_channels, FMOD_INIT_NORMAL, nullptr );
		}

		~fmod_resource()
		{
			system->release();
		}

		fmod_resource( const fmod_resource& other ) = delete;
		fmod_resource& operator=( const fmod_resource& other ) = delete;
	};

	static fmod_resource fmod_rc;
};

// ���带 �±׺��� �����ϴ� Ŭ����
class soundcomponent
{
public:
	enum class sound_tag {
		BGM = 0, SE,
		SIZE
	};

	// Ư�� �±��� ���带 ������ ������ŭ �����Ѵ�.
	void reserve( sound_tag tag, const size_t cnt )
	{
		sounds[ etoi( tag ) ].reserve( cnt );
	}

	void push( sound_tag tag, const sound& s )
	{
		sounds[ etoi( tag ) ].push_back( s );
	}

	void push( sound_tag tag, sound&& s )
	{
		sounds[ etoi( tag ) ].push_back( std::move( s ) );
	}

	void clear()
	{
		for ( auto& tagged_sound_set : sounds )
		{
			tagged_sound_set.clear();
		}
	}

	void clear( sound_tag tag )
	{
		sounds[ etoi( tag ) ].clear();
	}

	void play( sound_tag tag )
	{
		for ( auto& s : sounds[ etoi( tag ) ] )
		{
			s.play();
		}
	}

	void amplify( sound_tag tag, const float val )
	{
		for ( auto& s : sounds[ etoi( tag ) ] )
		{
			s.amplify( val );
		}
	}

	void pause( sound_tag tag )
	{
		for ( auto& s : sounds[ etoi( tag ) ] )
		{
			s.pause();
		}
	}

	void resume( sound_tag tag )
	{
		for ( auto& s : sounds[ etoi( tag ) ] )
		{
			s.resume();
		}
	}

	void stop( sound_tag tag )
	{
		for ( auto& s : sounds[ etoi( tag ) ] )
		{
			s.stop();
		}
	}

	void update()
	{
		sound::update();
	}

private:
	std::array< std::vector< sound >, etoi( sound_tag::SIZE ) > sounds;
};

#endif