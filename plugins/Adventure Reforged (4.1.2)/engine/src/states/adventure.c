#pragma bank 255

#include "data/states_defines.h"
#include "states/adventure.h"

#include "actor.h"
#include "camera.h"
#include "collision.h"
#include "game_time.h"
#include "input.h"
#include "scroll.h"
#include "trigger.h"
#include "data_manager.h"
#include "rand.h"
#include "vm.h"
#include "math.h"
#include "fade_manager.h"

#ifndef ADVENTURE_CAMERA_DEADZONE
#define ADVENTURE_CAMERA_DEADZONE 8
#endif

UBYTE initialized = 0;

WORD dimensions_x = 160;
WORD dimensions_y = 88;
UBYTE wrap_player = 1;
UBYTE wrapping = FALSE;

point16_t dimensions_padding;

point16_t visual_pos;
point16_t new_pos;
point16_t old_pos;

WORD z;			  // Vertical offset for jumping
WORD z_velocity;  // Velocity for jumping
WORD jump_height; // Height of the jump
WORD gravity;	  // Gravity for the jump

UBYTE angle = 0;
UBYTE old_angle = 0;
UBYTE checked_tile = 0;
UBYTE solid_ladders = 0;
UBYTE swimming = 0;
UBYTE swim_speed = 0;

UBYTE sprite_mode;		// Sprite mode for the player
direction_e visual_dir; // Direction the player is facing

direction_e new_dir = DIR_NONE;

void adventure_update_sprite_mode(WORD new_direction) BANKED
{
	if (sprite_mode == 0)
	{
		if (new_direction == DIR_LEFT || new_direction == DIR_RIGHT)
		{
			visual_dir = new_direction;
		}
	}

	actor_set_dir(&PLAYER, (wrapping) ? DIR_DOWN : visual_dir, player_moving);
}

void adventure_init(void) BANKED
{
	if (initialized)
	{
		return;
	}
	initialized = 1;

	wrapping = FALSE;

	// Set camera to follow player
	camera_offset_x = 0;
	camera_offset_y = 0;
	camera_deadzone_x = ADVENTURE_CAMERA_DEADZONE;
	camera_deadzone_y = ADVENTURE_CAMERA_DEADZONE;

	dimensions_padding.x = 16;
	dimensions_padding.y = 8;

	visual_pos = PLAYER.pos;

	if (PLAYER.dir == DIR_LEFT || PLAYER.dir == DIR_RIGHT)
	{
		visual_dir = PLAYER.dir;
	}
	else
	{
		visual_dir = DIR_RIGHT;
	}
	PLAYER.dir = visual_dir;

	// Set player z-axis
	z = 0;
	z_velocity = 0;
}

void gravity_update(void) BANKED
{
	// Gravity
	z += z_velocity;
	if (z > 0)
	{
		z_velocity -= gravity;

		if (PLAYER.dir == DIR_LEFT)
		{
			actor_set_anim(&PLAYER, ANIM_JUMP_LEFT);
		}
		else
		{
			actor_set_anim(&PLAYER, ANIM_JUMP_RIGHT);
		}
	}
	else
	{
		z = 0;
		z_velocity = 0;
	}
}

void jump(WORD height) BANKED
{
	z_velocity = height;
}

void adventure_update(void) BANKED
{
	actor_t *hit_actor;
	UBYTE tile_start, tile_end;
	player_moving = FALSE;
	old_pos = PLAYER.pos;

	// Fade in when wrapping
	if (wrapping)
	{
		wrapping = FALSE;
		return;
	}
	else
	{
		new_dir = DIR_NONE;
	}

	if (INPUT_RECENT_LEFT)
	{
		new_dir = DIR_LEFT;
	}
	else if (INPUT_RECENT_RIGHT)
	{
		new_dir = DIR_RIGHT;
	}
	else if (INPUT_RECENT_UP)
	{
		new_dir = DIR_UP;
	}
	else if (INPUT_RECENT_DOWN)
	{
		new_dir = DIR_DOWN;
	}

	if (INPUT_LEFT)
	{
		player_moving = TRUE;
		if (INPUT_UP)
		{
			angle = ANGLE_315DEG;
		}
		else if (INPUT_DOWN)
		{
			angle = ANGLE_225DEG;
		}
		else
		{
			angle = ANGLE_270DEG;
		}
	}
	else if (INPUT_RIGHT)
	{
		player_moving = TRUE;
		if (INPUT_UP)
		{
			angle = ANGLE_45DEG;
		}
		else if (INPUT_DOWN)
		{
			angle = ANGLE_135DEG;
		}
		else
		{
			angle = ANGLE_90DEG;
		}
	}
	else if (INPUT_UP)
	{
		player_moving = TRUE;
		angle = ANGLE_0DEG;
	}
	else if (INPUT_DOWN)
	{
		player_moving = TRUE;
		angle = ANGLE_180DEG;
	}
	else
	{
		angle = 0;
	}

	new_pos.x = visual_pos.x;
	new_pos.y = visual_pos.y;

	// If changed movement angle this frame, round subpixel position to nearest pixel position. Prevents jittering on diagonal movement
	if (angle != old_angle)
	{
		visual_pos.x = visual_pos.x & 0xFFF0;
		visual_pos.y = visual_pos.y & 0xFFF0;
	}
	old_angle = angle;

	if (swimming)
	{
		if (INPUT_A_PRESSED && swim_speed == 0)
		{
			swim_speed = 16;
		}
		else if (swim_speed > 0)
		{
			swim_speed--;
		}
		switch (angle)
		{
		case ANGLE_0DEG:
			new_pos.y = visual_pos.y - 8 - swim_speed;
			break;
		case ANGLE_45DEG:
			new_pos.x = visual_pos.x + 5 + (swim_speed * 7 / 10);
			new_pos.y = visual_pos.y - 5 - (swim_speed * 7 / 10);
			break;
		case ANGLE_90DEG:
			new_pos.x = visual_pos.x + 8 + swim_speed;
			break;
		case ANGLE_135DEG:
			new_pos.x = visual_pos.x + 5 + (swim_speed * 7 / 10);
			new_pos.y = visual_pos.y + 5 + (swim_speed * 7 / 10);
			break;
		case ANGLE_180DEG:
			new_pos.y = visual_pos.y + 8 + swim_speed;
			break;
		case ANGLE_225DEG:
			new_pos.x = visual_pos.x - 5 - (swim_speed * 7 / 10);
			new_pos.y = visual_pos.y + 5 + (swim_speed * 7 / 10);
			break;
		case ANGLE_270DEG:
			new_pos.x = visual_pos.x - 8 - swim_speed;
			break;
		case ANGLE_315DEG:
			new_pos.x = visual_pos.x - 5 - (swim_speed * 7 / 10);
			new_pos.y = visual_pos.y - 5 - (swim_speed * 7 / 10);
			break;
		}
	}
	else if (player_moving)
	{
		// Update new pos
		// point_translate_angle(&new_pos, angle, PLAYER.move_speed);
		switch (angle)
		{
		case ANGLE_0DEG:
			new_pos.y = visual_pos.y - 16;
			break;
		case ANGLE_45DEG:
			new_pos.x = visual_pos.x + 11;
			new_pos.y = visual_pos.y - 11;
			break;
		case ANGLE_90DEG:
			new_pos.x = visual_pos.x + 16;
			break;
		case ANGLE_135DEG:
			new_pos.x = visual_pos.x + 11;
			new_pos.y = visual_pos.y + 11;
			break;
		case ANGLE_180DEG:
			new_pos.y = visual_pos.y + 16;
			break;
		case ANGLE_225DEG:
			new_pos.x = visual_pos.x - 11;
			new_pos.y = visual_pos.y + 11;
			break;
		case ANGLE_270DEG:
			new_pos.x = visual_pos.x - 16;
			break;
		case ANGLE_315DEG:
			new_pos.x = visual_pos.x - 11;
			new_pos.y = visual_pos.y - 11;
			break;
		}
	}

	if (player_moving)
	{
		// Step X
		tile_start = (((visual_pos.y >> 4) + PLAYER.bounds.top) >> 3);
		tile_end = (((visual_pos.y >> 4) + PLAYER.bounds.bottom) >> 3) + 1;
		if (angle < ANGLE_180DEG)
		{
			UBYTE tile_x = ((new_pos.x >> 4) + PLAYER.bounds.right) >> 3;
			while (tile_start != tile_end)
			{
				checked_tile = tile_at(tile_x, tile_start);
				if (checked_tile & COLLISION_LEFT && (!(checked_tile & TILE_PROP_LADDER) || checked_tile & TILE_PROP_LADDER && solid_ladders))
				{
					new_pos.x = (((tile_x << 3) - PLAYER.bounds.right) << 4) - 1;
					break;
				}
				tile_start++;
			}
		}
		else
		{
			UBYTE tile_x = ((new_pos.x >> 4) + PLAYER.bounds.left) >> 3;
			while (tile_start != tile_end)
			{
				checked_tile = tile_at(tile_x, tile_start);
				if (checked_tile & COLLISION_RIGHT && (!(checked_tile & TILE_PROP_LADDER) || checked_tile & TILE_PROP_LADDER && solid_ladders))
				{
					new_pos.x = ((((tile_x + 1) << 3) - PLAYER.bounds.left) << 4) + 1;
					break;
				}
				tile_start++;
			}
		}
		visual_pos.x = new_pos.x;

		// Step Y
		tile_start = (((visual_pos.x >> 4) + PLAYER.bounds.left) >> 3);
		tile_end = (((visual_pos.x >> 4) + PLAYER.bounds.right) >> 3) + 1;
		if (angle > ANGLE_90DEG && angle < ANGLE_270DEG)
		{
			UBYTE tile_y = ((new_pos.y >> 4) + PLAYER.bounds.bottom) >> 3;
			while (tile_start != tile_end)
			{
				checked_tile = tile_at(tile_start, tile_y);
				if (checked_tile & COLLISION_TOP && (!(checked_tile & TILE_PROP_LADDER) || checked_tile & TILE_PROP_LADDER && solid_ladders))
				{
					new_pos.y = ((((tile_y) << 3) - PLAYER.bounds.bottom) << 4) - 1;
					break;
				}
				tile_start++;
			}
			visual_pos.y = new_pos.y;
		}
		else
		{
			UBYTE tile_y = (((new_pos.y >> 4) + PLAYER.bounds.top) >> 3);
			while (tile_start != tile_end)
			{
				checked_tile = tile_at(tile_start, tile_y);
				if (checked_tile & COLLISION_BOTTOM && (!(checked_tile & TILE_PROP_LADDER) || checked_tile & TILE_PROP_LADDER && solid_ladders))
				{
					new_pos.y = ((((UBYTE)(tile_y + 1) << 3) - PLAYER.bounds.top) << 4) + 1;
					break;
				}
				tile_start++;
			}
		}
		visual_pos.y = new_pos.y;
	}

	if (wrap_player)
	{
		// Wrap around screen edges
		if (visual_pos.x < 0)
		{
			visual_pos.x = (dimensions_x - dimensions_padding.x) << 4;
			wrapping = TRUE;
		}
		else if (visual_pos.x > ((dimensions_x - dimensions_padding.x) << 4))
		{
			visual_pos.x = 0;
			wrapping = TRUE;
		}

		if (visual_pos.y < dimensions_padding.y << 4)
		{
			visual_pos.y = (dimensions_y - dimensions_padding.y) << 4;
			wrapping = TRUE;
		}
		else if (visual_pos.y > ((dimensions_y - dimensions_padding.y) << 4))
		{
			visual_pos.y = dimensions_padding.y << 4;
			wrapping = TRUE;
		}
	}
	else
	{
		if (visual_pos.x < 0)
		{
			visual_pos.x = 0;
		}
		else if (visual_pos.x > ((dimensions_x - dimensions_padding.x) << 4))
		{
			visual_pos.x = (dimensions_x - dimensions_padding.x) << 4;
		}

		if (visual_pos.y < dimensions_padding.y << 4)
		{
			visual_pos.y = dimensions_padding.y << 4;
		}
		else if (visual_pos.y > ((dimensions_y - dimensions_padding.y) << 4))
		{
			visual_pos.y = (dimensions_y - dimensions_padding.y) << 4;
		}

		wrapping = FALSE;
	}

	if (new_dir != DIR_NONE)
	{
		adventure_update_sprite_mode(new_dir);
	}
	else
	{
		actor_set_dir(&PLAYER, visual_dir, player_moving);
	}

	// Gravity
	gravity_update();

	hit_actor = NULL;
	if (IS_FRAME_ODD)
	{
		// Check for trigger collisions
		if (trigger_activate_at_intersection(&PLAYER.bounds, &PLAYER.pos, FALSE))
		{
			// Landed on a trigger
			return;
		}
	}

	// Check for actor collisions
	// If overlapping solid actor
	hit_actor = actor_overlapping_player(FALSE);
	if (hit_actor != NULL)
	{
		if (hit_actor->collision_group == 0 || hit_actor->collision_group == 8)
		{
			// Move player back to previous frame position
			visual_pos.x = old_pos.x;
			visual_pos.y = old_pos.y;
			// If actor in front of player direction
			hit_actor = actor_in_front_of_player(1, TRUE);
			if (hit_actor != NULL && hit_actor->collision_group == 0 || hit_actor != NULL && hit_actor->collision_group == 8)
			{
				// Slide is blocked, no movement
			}
			else
			{
				// Allow slide, movement in slide direction only
				if (new_dir == DIR_LEFT || new_dir == DIR_RIGHT)
				{
					visual_pos.x = new_pos.x;
				}
				else
				{
					visual_pos.y = new_pos.y;
				}
			}
		}
		// Else trigger on hit script
		else
		{
			player_register_collision_with(hit_actor);
		}
	}

	if (INPUT_A_PRESSED && initialized)
	{
		if (!hit_actor)
		{
			hit_actor = actor_in_front_of_player(8, TRUE);
		}
		if (hit_actor && !hit_actor->collision_group && hit_actor->script.bank)
		{
			script_execute(hit_actor->script.bank, hit_actor->script.ptr, 0, 1, 0);
		}
		else if (z == 0)
		{
			jump(jump_height);
		}
	}

	// Update the player's position
	if (!wrapping)
	{
		PLAYER.pos.x = visual_pos.x;
		PLAYER.pos.y = visual_pos.y - z;
	}
}
