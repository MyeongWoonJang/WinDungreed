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
private:
	struct fmod_resource
	{
		static constexpr size_t fmod_max_channels = 32;
		static constexpr size_t fmod_max_sounds = 100;

		FMOD::System* system;
		size_t available_sound_cnt;
		size_t available_channel_cnt;

		fmod_resource() : available_channel_cnt{ fmod_max_channels }, available_sound_cnt{ fmod_max_sounds }
		{
			FMOD::System_Create( &system );
			FMOD::Memory_Initialize( malloc( 4 * 1024 * 1024 ), 4 * 1024 * 1024, 0, 0, 0 );
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

public:
	enum class mode {
		normal = FMOD_LOOP_OFF, loop = FMOD_LOOP_NORMAL
	};

	void play()
	{
		if ( !( fmod_rc.available_channel_cnt ) )
		{
			// 모든 채널을 사용중일 경우
			// 가장 작은 볼륨으로 재생되는 채널을 선점한다.
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

		// 한 사운드가 둘 이상의 채널에서 재생될 경우 ( 동시 재생 )
		// 나중에 재생된 사운드의 볼륨을 줄인다.
		min_volume *= gradient;
		fmod_channels.back()->setVolume( min_volume );
		fmod_rc.system->playSound( fmod_sound, nullptr, false, &fmod_channels.back() );
		--fmod_rc.available_channel_cnt;

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
		fmod_rc.available_channel_cnt += fmod_channels.size();
		fmod_channels.clear();
		min_volume = volume;
	}

	// 모든 재생 완료된 사운드에 대해 종료 처리를 한다.
	// 일정 시간 간격으로 단 한 번 호출한다. ( 사운드마다 호출하지 않는다. 정적 함수이다. )
	static void update()
	{
		fmod_rc.system->update();

		for ( auto s : sound_insts )
		{
			// 나중에 재생된 채널이 가장 마지막에 종료된다.
			// 뒤에서부터 조사하여 종료된 채널을 만나면, 그 앞의 채널들은 전부 종료된 채널이다.
			for ( auto iter = s->fmod_channels.rbegin(); iter != s->fmod_channels.rend(); ++iter )
			{
				bool is_playing = false;
				bool is_paused = false;
				( *iter )->isPlaying( &is_playing );
				( *iter )->getPaused( &is_paused );

				if ( !is_playing && !is_paused )
				{   
					fmod_rc.available_channel_cnt += std::distance( iter, s->fmod_channels.rend() );
					s->fmod_channels.erase( s->fmod_channels.begin(), iter.base() );
					s->min_volume = pow( s->volume, s->fmod_channels.size() - 1 );
					break;
				}
			}
		}
	}

	sound( const char* file_path, mode mod, const float volume = 1.0f, const float gradient = default_gradient )
		: volume{ volume }, gradient{ gradient }, min_volume{ volume / gradient }
	{
		assert( fmod_rc.available_sound_cnt > 0 );
		fmod_rc.system->createSound( file_path, etoi( mod ) | FMOD_LOWMEM, nullptr, &fmod_sound );
		sound_insts.push_back( this );
		at = --sound_insts.end();
		--fmod_rc.available_sound_cnt;
	}

	// copy는 권장하지 않는다. move samentics를 이용하라.
	sound( const sound& other ) : volume{ other.volume }, gradient{ other.gradient }, min_volume{ other.volume / other.min_volume },
		fmod_sound{ other.fmod_sound }
	{
		sound_insts.push_back( this );
		at = --sound_insts.end();
	}

	// copy는 권장하지 않는다. move samentics를 이용하라.
	sound& operator=( const sound& other )
	{
		if ( this != &other && fmod_sound != other.fmod_sound )
		{
			this->~sound();

			volume = other.volume;
			gradient = other.gradient;
			min_volume = other.min_volume;
			fmod_sound = other.fmod_sound;

			sound_insts.push_back( this );
			at = --sound_insts.end();
		}

		return *this;
	}

	sound( sound&& other ) noexcept : volume{ other.volume }, gradient{ other.gradient }, min_volume{ other.min_volume },
		fmod_sound{ other.fmod_sound }, fmod_channels{ std::move( other.fmod_channels ) }
	{
		other.fmod_sound = nullptr;
		sound_insts.erase( other.at );
		sound_insts.push_back( this );
		at = --sound_insts.end();
	}

	sound& operator=( sound&& other ) noexcept
	{
		if ( this != &other )
		{
			this->~sound();

			volume = other.volume;			gradient = other.gradient;
			min_volume = other.min_volume;	fmod_sound = other.fmod_sound;
			fmod_channels = std::move( other.fmod_channels );

			other.fmod_sound = nullptr;
			sound_insts.erase( other.at );
			sound_insts.push_back( this );
			at = --sound_insts.end();
		}

		return *this;
	}

	~sound()
	{
		if ( fmod_sound )
		{
			fmod_sound->release();
			sound_insts.erase( at );
			++fmod_rc.available_sound_cnt;
		}
	}


private:
	static constexpr float default_gradient = 0.5f;

	float min_volume;
	float volume;
	float gradient;
	FMOD::Sound* fmod_sound;
	std::vector< FMOD::Channel* > fmod_channels;
	static std::list< sound* > sound_insts;
	std::list< sound* >::const_iterator at;
};

sound::fmod_resource sound::fmod_rc;
std::list< sound* > sound::sound_insts;

// 사운드를 태그별로 관리하는 클래스
class soundcomponent
{
public:
	enum class sound_tag {
		BGM = 0, SE,
		SIZE
	};

	// 특정 태그의 사운드를 지정한 개수만큼 예약한다.
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