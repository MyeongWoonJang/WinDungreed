#ifndef _videocomponent
#define _videocomponent

#include "image.h"
#include <vector>
#include "animation.h"

class VideoComponent
{
public:
	void push( image_ptr image )
	{
		images.push_back( image );
		clips.emplace_back( 0, 0, image->width(), image->height(), 0, 0.f );
		animations.push_back( nullptr );
	}

	void push( image_ptr image, const ClipArea& iv )
	{
		images.push_back( image );
		clips.push_back( iv );
		animations.push_back( nullptr );
	}

	void push( image_ptr image, animation_ptr animation )
	{
		images.push_back( image );
		clips.push_back( animation->get_clip_area() );
		animations.push_back( animation );
	}

	void clear()
	{
		images.clear();
		clips.clear();
		animations.clear();
	}

	void select( size_t n )
	{
		if ( n < images.size() )
		{
			selected = n;
		}
		else
		{
			std::cerr << "VideoComponent - �̹��� �ε��� ���� �ʰ�\n";
		}
	}

	void draw( HDC dc, int dx, int dy, int dw, int dh )
	{
		auto& cur_image = images[ selected ];
		auto& clip_area = clips[ selected ];

		cur_image->draw( dc, dx, dy, dw, dh, clip_area.x, clip_area.y, clip_area.w, clip_area.h );
	}

	void enable_hflip()
	{
		images[ selected ]->flag |= etoi( Image::Flag::h );
	}

	void enable_vflip()
	{
		images[ selected ]->flag |= etoi( Image::Flag::v );
	}

	void enable_alpha()
	{
		images[ selected ]->flag |= etoi( Image::Flag::a );
	}

	void enable_rotation()
	{
		images[ selected ]->flag |= etoi( Image::Flag::r );
	}

	void disable_hflip()
	{
		images[ selected ]->flag &= !etoi( Image::Flag::h );
	}

	void disable_vflip()
	{
		images[ selected ]->flag &= !etoi( Image::Flag::v );
	}

	void disable_alpha()
	{
		images[ selected ]->flag &= !etoi( Image::Flag::a );
	}

	void disable_rotation()
	{
		images[ selected ]->flag &= !etoi( Image::Flag::r );
	}

	void add_radian( const float val )
	{
		images[ selected ]->radian += val;
	}

	void set_radian( const float val )
	{
		images[ selected ]->radian = val;
	}

	void update_animation()
	{
		if ( animations[ selected ] )
		{
			++( *animations[ selected ] );
			clips[ selected ] = animations[ selected ]->get_clip_area();
		}
	}

	void update_animation_revert()
	{
		if ( animations[ selected ] )
		{
			--( *animations[ selected ] );
			clips[ selected ] = animations[ selected ]->get_clip_area();
		}
	}

private:
	std::vector< image_ptr > images;
	std::vector< ClipArea > clips;
	std::vector< animation_ptr > animations;
	size_t selected;
};

#endif