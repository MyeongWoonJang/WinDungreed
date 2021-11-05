#ifndef _sound
#define _sound

#include "fmod.hpp"
#include <array>
#include <vector>
#include <list>
#include "TMP.h"
#include <cassert>
#include <cmath>

class sound
{
public:
	enum class mode {
		normal = FMOD_LOOP_OFF, loop = FMOD_LOOP_NORMAL
	};

	enum class sound_tag {
		BGM = 0, SE,
		SIZE
	};

	void play()
	{
		if ( !( available_channel_cnt ) )
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
			++available_channel_cnt;
		}

		fmod_channels.emplace_back( nullptr );

		// �� ���尡 �� �̻��� ä�ο��� ����� ��� ( ���� ��� )
		// ���߿� ����� ������ ������ ���δ�.
		min_volume *= gradient;
		fmod_channels.back()->setVolume( min_volume );
		system->playSound( fmod_sound, nullptr, false, &fmod_channels.back() );
		--available_channel_cnt;

		std::cout << "volume: " << volume << '\n';
		std::cout << "min_volume: " << min_volume << '\n';
		std::cout << "gradient: " << gradient << '\n';
		std::cout << "fmod_sound: " << fmod_sound << '\n';
		std::cout << "fmod_channel: " << fmod_channels.back() << '\n';
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
		available_channel_cnt += fmod_channels.size();
		fmod_channels.clear();
		min_volume = volume;
	}

	// ��� ��� �Ϸ�� ���忡 ���� ���� ó���� �Ѵ�.
	// ���� �ð� �������� �� �� �� ȣ���Ѵ�. ( ���帶�� ȣ������ �ʴ´�. ���� �Լ��̴�. )
	static void update()
	{
		system->update();

		for ( auto s : sound_insts )
		{
			// ���߿� ����� ä���� ���� �������� ����ȴ�.
			// �ڿ������� �����Ͽ� ����� ä���� ������, �� ���� ä�ε��� ���� ����� ä���̴�.
			for ( auto iter = s->fmod_channels.rbegin(); iter != s->fmod_channels.rend(); ++iter )
			{
				bool is_playing = false;
				bool is_paused = false;
				( *iter )->isPlaying( &is_playing );
				( *iter )->getPaused( &is_paused );

				if ( !is_playing && !is_paused )
				{   
					available_channel_cnt += std::distance( iter, s->fmod_channels.rend() );
					s->fmod_channels.erase( s->fmod_channels.begin(), iter.base() );
					s->min_volume = static_cast< float >( pow( s->volume, s->fmod_channels.size() - 1 ) );
					break;
				}
			}
		}
	}

	// Ư�� �±��� ���带 ������ ������ŭ �����Ѵ�.
	static void tag_reserve( sound_tag tag, const size_t cnt )
	{
		tagged_sounds[ etoi( tag ) ].reserve( cnt );
	}

	static void tag_push( sound_tag tag, const std::shared_ptr< sound >& s )
	{
		tagged_sounds[ etoi( tag ) ].push_back( s );
	}

	static void tag_push( sound_tag tag, std::shared_ptr< sound >&& s )
	{
		tagged_sounds[ etoi( tag ) ].push_back( std::move( s ) );
	}

	static void clear()
	{
		for ( auto& tagged_sound_set : tagged_sounds )
		{
			tagged_sound_set.clear();
		}
	}

	static void tag_clear( sound_tag tag )
	{
		tagged_sounds[ etoi( tag ) ].clear();
	}

	static void tag_play( sound_tag tag )
	{
		for ( auto s : tagged_sounds[ etoi( tag ) ] )
		{
			s->play();
		}
	}

	static void tag_amplify( sound_tag tag, const float val )
	{
		for ( auto s : tagged_sounds[ etoi( tag ) ] )
		{
			s->amplify( val );
		}
	}

	static void tag_pause( sound_tag tag )
	{
		for ( auto s : tagged_sounds[ etoi( tag ) ] )
		{
			s->pause();
		}
	}

	static void tag_resume( sound_tag tag )
	{
		for ( auto s : tagged_sounds[ etoi( tag ) ] )
		{
			s->resume();
		}
	}

	static void tag_stop( sound_tag tag )
	{
		for ( auto s : tagged_sounds[ etoi( tag ) ] )
		{
			s->stop();
		}
	}

	sound( const sound& other ) = delete;
	sound& operator=( const sound& other ) = delete;

	~sound()
	{
		if ( fmod_sound )
		{
			fmod_sound->release();
			sound_insts.erase( at );
			++available_sound_cnt;
		}
	}

	friend auto make_sound( const char* file_path, mode mod, const float volume = 1.0f, const float gradient = sound::default_gradient )
	{
		return std::shared_ptr< sound >{ new sound{ file_path, mod, volume, gradient } };
	}

private:
	static constexpr float default_gradient = 0.5f;
	static size_t available_sound_cnt;
	static size_t available_channel_cnt;
	static constexpr size_t fmod_max_channels = 32;
	static constexpr size_t fmod_max_sounds = 100;
	static std::list< sound* > sound_insts;
	static FMOD::System* system;
	static std::array< std::vector< std::shared_ptr< sound > >, etoi( sound_tag::SIZE ) > tagged_sounds;

	float min_volume;
	float volume;
	float gradient;
	FMOD::Sound* fmod_sound;
	std::vector< FMOD::Channel* > fmod_channels;
	std::list< sound* >::const_iterator at;

	sound( const char* file_path, mode mod, const float volume = 1.0f, const float gradient = default_gradient )
		: volume{ volume }, gradient{ gradient }, min_volume{ volume / gradient }, fmod_sound{ nullptr }
	{
		if ( !available_sound_cnt )
		{
			std::cerr << "available_sound_cnt was 0.\n";
			return;
		}

		system->createSound( file_path, etoi( mod ) | FMOD_LOWMEM, nullptr, &fmod_sound );
		sound_insts.push_back( this );
		at = --sound_insts.end();
		--available_sound_cnt;
	}

	struct _auto_system
	{
		_auto_system()
		{
			FMOD::System_Create( &system );
			FMOD::Memory_Initialize( malloc( 4 * 1024 * 1024 ), 4 * 1024 * 1024, 0, 0, 0 );
			system->init( fmod_max_channels, FMOD_INIT_NORMAL, nullptr );
		}

		~_auto_system()
		{
			system->release();
		}
	};

	static _auto_system _s;
};

using sound_ptr = std::shared_ptr< sound >;

FMOD::System* sound::system;
std::list< sound* > sound::sound_insts;
size_t sound::available_sound_cnt;
size_t sound::available_channel_cnt;
sound::_auto_system sound::_s;
std::array< std::vector< std::shared_ptr< sound > >, etoi( sound::sound_tag::SIZE ) > sound::tagged_sounds;

#endif