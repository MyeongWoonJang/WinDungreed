#ifndef _timer
#define _timer

#include <chrono>
#include <iostream>
#include "TMP.h"
#include <algorithm>
#include <queue>
#include <numeric>
#include <deque>
#include <functional>
#include <utility>


class timer
{
private:
	struct delayed
	{
		const bool operator>( const delayed& other )
		{
			return ms_time_exact > other.ms_time_exact;
		}

		float ms_time_exact;
		std::function< void() > to_do;
	};

public:
	float clock;
	const UINT timer_id;

	template < typename Func >
	void alarm( float ms_delay, Func to_do )
	{
		alarms.push( delayed{ ms_delay + ms_time, to_do } );
	}

	void setFPS( const float fps )
	{
		objfps = fps;
		curfps = fps;
		ms_per_frame = 1000 / fps;
		KillTimer( hWnd, timer_id );
		SetTimer( hWnd, timer_id, static_cast< UINT >( ms_per_frame ), nullptr );
	}

	void update()
	{
		using namespace std::chrono;
		static system_clock::time_point last_tp = system_clock::now();

		prevent_overflow();
		process_alarm();
		update_curfps( frame_time );

		auto cur_tp = system_clock::now();
		frame_time = duration_cast< nanoseconds >( cur_tp - last_tp ).count()
			/ static_cast<float>( nanoseconds::period::den / milliseconds::period::den );

		ms_time += frame_time;
		lag = max( lag + frame_time - ms_time, 0.f );

		last_tp = cur_tp;
	}

	const float getlag() const
	{
		return lag;
	}

	const float getobjFPS() const 
	{
		return objfps;
	}

	const float getcurFPS() const
	{
		return curfps;
	}

	const float get_frame_time() const
	{
		return frame_time;
	}

	const float get_ms_per_frame() const
	{
		return ms_per_frame;
	}

	timer( HWND hWnd, const UINT timer_id, const float fps, const float clock = 10.f ) : ms_time{ 0 }, lag{ 0 },
		hWnd{ hWnd }, timer_id{ timer_id }, clock{ clock }, objfps{ fps }, curfps{ fps }, ms_per_frame{ 1000 / fps }
	{
		SetTimer( hWnd, timer_id, static_cast< UINT >( ms_per_frame ), nullptr );
	}

	~timer()
	{
		KillTimer( hWnd, timer_id );
	}

	// 복사는 이용되지 않을 것이라 가정해 구현하지 않았다.
	// timer는 id 정보를 들고 있으므로, 후에 구현한다면 독립된 id를 보장하는 복사 동작이 필요하다.

	timer( const timer& other ) = default;

	timer& operator=( const timer& ) = default;

private:
	float frame_time;
	float lag;
	float ms_per_frame;
	float curfps;
	float objfps;
	float ms_time;
	std::priority_queue< delayed, std::vector< delayed >, std::greater<> > alarms;
	HWND hWnd;

	void prevent_overflow()
	{
		static constexpr float limit = 10'000'000.f;
		if ( ms_time > limit )
		{
			ms_time -= limit;
			sub_delays( limit );
		}
	}

	void sub_delays( const float val )
	{
		decltype( alarms ) temp;

		while ( !alarms.empty() )
		{
			auto a = alarms.top();
			a.ms_time_exact -= val;

			temp.push( std::move( a ) );
			alarms.pop();
		}

		alarms = std::move( temp );
	}

	void process_alarm()
	{
		while ( is_there_an_alarm_on_time() )
		{
			alarms.top().to_do();
			alarms.pop();
		}
	}

	const bool is_there_an_alarm_on_time()
	{
		if ( alarms.empty() )
		{
			return false;
		}
		return alarms.top().ms_time_exact < ms_time;
	}

	void update_curfps( const float frame_time )
	{
		curfps = 1000 / frame_time;
	}
};

#endif